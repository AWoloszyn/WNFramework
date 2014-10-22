// Copyright (c) 2014, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#ifndef __WN_MATH_INTERNAL_X86_VECTOR_OP_LENGTH_SSE4_1_H__
#define __WN_MATH_INTERNAL_X86_VECTOR_OP_LENGTH_SSE4_1_H__

#ifndef __WN_SSE4_1_AVAILABLE
    #error "This header requires a minimum of SSE4.1 to be enabled in order to work."
#endif

namespace WNMath {
    namespace __WNInternal {
        template <WN_UINT32 Dimension>
        class __WNVectorOpLength<WN_FLOAT32, Dimension, typename WNCore::WNEnableWhen<(Dimension == 2)>::Value> {
        public:
            static WN_FORCE_INLINE WN_FLOAT32 Execute(const __WNElements<WN_FLOAT32, Dimension>& _elements) {
                const __m128 lengthSquared = _mm_dp_ps(_elements.mXMMValues[0], _elements.mXMMValues[0], 0x31);

                #ifdef __WN_MATH_APPROXIMATIONS_ENABLED
                     const __m128 length = _mm_mul_ss(lengthSquared, _mm_rsqrt_ss(lengthSquared));
                #else
                     const __m128 length = _mm_sqrt_ss(lengthSquared);
                #endif

                return(_mm_cvtss_f32(length));
            }

        private:
            WN_FORCE_INLINE __WNVectorOpLength() {}
        };

        template <WN_UINT32 Dimension>
        class __WNVectorOpLength<WN_FLOAT32, Dimension, typename WNCore::WNEnableWhen<(Dimension == 3)>::Value> {
        public:
            static WN_FORCE_INLINE WN_FLOAT32 Execute(const __WNElements<WN_FLOAT32, Dimension>& _elements) {
                const __m128 lengthSquared = _mm_dp_ps(_elements.mXMMValues[0], _elements.mXMMValues[0], 0x71);

                #ifdef __WN_MATH_APPROXIMATIONS_ENABLED
                     const __m128 length = _mm_mul_ss(lengthSquared, _mm_rsqrt_ss(lengthSquared));
                #else
                     const __m128 length = _mm_sqrt_ss(lengthSquared);
                #endif

                return(_mm_cvtss_f32(length));
            }

        private:
            WN_FORCE_INLINE __WNVectorOpLength() {}
        };

        template <WN_UINT32 Dimension>
        class __WNVectorOpLength<WN_FLOAT32, Dimension, typename WNCore::WNEnableWhen<(Dimension == 4)>::Value> {
        public:
            static WN_FORCE_INLINE WN_FLOAT32 Execute(const __WNElements<WN_FLOAT32, Dimension>& _elements) {
                const __m128 lengthSquared = _mm_dp_ps(_elements.mXMMValues[0], _elements.mXMMValues[0], 0xF1);

                #ifdef __WN_MATH_APPROXIMATIONS_ENABLED
                     const __m128 length = _mm_mul_ss(lengthSquared, _mm_rsqrt_ss(lengthSquared));
                #else
                     const __m128 length = _mm_sqrt_ss(lengthSquared);
                #endif

                return(_mm_cvtss_f32(length));
            }

        private:
            WN_FORCE_INLINE __WNVectorOpLength() {}
        };

        template <WN_UINT32 Dimension>
        class __WNVectorOpLength<WN_FLOAT64, Dimension, typename WNCore::WNEnableWhen<(Dimension == 2)>::Value> {
        public:
            static WN_FORCE_INLINE WN_FLOAT64 Execute(const __WNElements<WN_FLOAT64, Dimension>& _elements) {
                return(_mm_cvtsd_f64(_mm_sqrt_sd(_mm_setzero_pd(), _mm_dp_pd(_elements.mXMMValues[0], _elements.mXMMValues[0], 0x31))));
            }

        private:
            WN_FORCE_INLINE __WNVectorOpLength() {}
        };

        template <WN_UINT32 Dimension>
        class __WNVectorOpLength<WN_FLOAT64, Dimension, typename WNCore::WNEnableWhen<(Dimension == 3)>::Value> {
        public:
            static WN_FORCE_INLINE WN_FLOAT64 Execute(const __WNElements<WN_FLOAT64, Dimension>& _elements) {
                const __m128d lengthSquared1 = _mm_dp_pd(_elements.mXMMValues[0], _elements.mXMMValues[0], 0x31);
                const __m128d lengthSquared2 = _mm_dp_pd(_elements.mXMMValues[1], _elements.mXMMValues[1], 0x11);

                return(_mm_cvtsd_f64(_mm_sqrt_sd(_mm_setzero_pd(), _mm_add_sd(lengthSquared1, lengthSquared2))));
            }

        private:
            WN_FORCE_INLINE __WNVectorOpLength() {}
        };

        template <WN_UINT32 Dimension>
        class __WNVectorOpLength<WN_FLOAT64, Dimension, typename WNCore::WNEnableWhen<(Dimension == 4)>::Value> {
        public:
            static WN_FORCE_INLINE WN_FLOAT64 Execute(const __WNElements<WN_FLOAT64, Dimension>& _elements) {
                const __m128d lengthSquared1 = _mm_dp_pd(_elements.mXMMValues[0], _elements.mXMMValues[0], 0x31);
                const __m128d lengthSquared2 = _mm_dp_pd(_elements.mXMMValues[1], _elements.mXMMValues[1], 0x31);

                return(_mm_cvtsd_f64(_mm_sqrt_sd(_mm_setzero_pd(), _mm_add_sd(lengthSquared1, lengthSquared2))));
            }

        private:
            WN_FORCE_INLINE __WNVectorOpLength() {}
        };
    }
}

#endif // __WN_MATH_INTERNAL_X86_VECTOR_OP_LENGTH_SSE4_1_H__