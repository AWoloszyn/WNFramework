// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#ifndef __WN_GRAPHICS_OBJECT_BASE__
#define __WN_GRAPHICS_OBJECT_BASE__

#include "WNCore/inc/WNBase.h"
#include "WNGraphics/inc/Internal/WNConfig.h"
#include "WNGraphics/inc/WNDevice.h"

WN_GRAPHICS_FORWARD(queue);
WN_GRAPHICS_FORWARD(device);

namespace wn {
namespace graphics {

template <uint64_t NUM_64_BIT_WORDS>
class base_object : public core::non_copyable {
protected:
  WN_GRAPHICS_ADD_FRIENDS(queue)
  WN_GRAPHICS_ADD_FRIENDS(device)

protected:
  template <typename T>
  WN_FORCE_INLINE T& data_as() {
    static_assert(sizeof(opaque_data) >= sizeof(T),
        "invalid cast, target type size does not match opaque data size");

    return *reinterpret_cast<T*>(&m_data);
  }

  template <typename T>
  WN_FORCE_INLINE const T& data_as() const {
    static_assert(sizeof(opaque_data) >= sizeof(T),
        "invalid cast, target type size does not match opaque data size");

    return *reinterpret_cast<const T*>(&m_data);
  }

  // The opaque_data must be trivially copyable.
  // It also must be considered uninitialized when
  // memset to 0.
  struct opaque_data WN_FINAL {
    uint64_t _dummy[NUM_64_BIT_WORDS];
  } m_data = {};
};

}  // namespace graphics
}  // namespace wn

#endif  // __WN_GRAPHICS_OBJECT_BASE__
