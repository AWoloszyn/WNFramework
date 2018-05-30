// Copyright (c) 2018, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#ifndef __WN_MULTI_TASKING_ARM64_INTERNAL_UTILITIES_H__
#define __WN_MULTI_TASKING_ARM64_INTERNAL_UTILITIES_H__

#include "WNCore/inc/base.h"

#include <arm_acle.h>

namespace wn {
namespace multi_tasking {
namespace internal {

inline void processor_relax() {
  __yield();
}

}  // namespace internal
}  // namespace multi_tasking
}  // namespace wn

#endif  // __WN_MULTI_TASKING_ARM64_INTERNAL_UTILITIES_H__
