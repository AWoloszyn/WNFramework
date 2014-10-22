// Copyright (c) 2014, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#ifndef __WN_MATH_INTERNAL_ARM_CONFIG_H__
#define __WN_MATH_INTERNAL_ARM_CONFIG_H__

#ifdef __WN_NEON_AVAILABLE
    #include <arm_neon.h>

    #include "WNMath/inc/Internal/ARM/WNUtilities_NEON.h"
#else
    #undef __WN_MATH_EXTENSIONS_ENABLED
#endif

#endif // __WN_MATH_INTERNAL_ARM_CONFIG_H__