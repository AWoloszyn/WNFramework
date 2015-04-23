// Copyright (c) 2014, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#ifndef __WN_CORE_UTILITY_H__
#define __WN_CORE_UTILITY_H__

#include "WNCore/inc/WNBase.h"

#include <type_traits>
#include <utility>

namespace wn {
    template <typename type>
    struct dependent_true : std::true_type {};

    template <typename type>
    struct dependent_false : std::false_type {};

    class non_constructable {
    public:
        non_constructable() = delete;
    };

    class non_copyable {
    public:
        non_copyable() = default;
        non_copyable(const non_copyable&) = delete;

        non_copyable& operator = (const non_copyable&) = delete;
    };

    class non_constructable_non_copyable {
    public:
        non_constructable_non_copyable() = delete;
        non_constructable_non_copyable(const non_constructable_non_copyable&) = delete;

        non_constructable_non_copyable& operator = (const non_constructable_non_copyable&) = delete;
    };

    template <typename type>
    WN_FORCE_INLINE typename std::decay<type>::type decay_copy(type&& value) {
        return(std::forward<type>(value));
    }
}

#endif // __WN_CORE_UTILITY_H__