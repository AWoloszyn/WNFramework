// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#ifndef __WN_GRAPHICS_FRAMEBUFFER_H__
#define __WN_GRAPHICS_FRAMEBUFFER_H__

#include "WNGraphics/inc/WNCommandList.h"
#include "WNGraphics/inc/WNGraphicsObjectBase.h"
#include "WNGraphics/inc/WNImageView.h"

WN_GRAPHICS_FORWARD(command_list)

namespace wn {
namespace runtime {
namespace graphics {

class framebuffer : public base_object<2> {
public:
  WN_FORCE_INLINE framebuffer(framebuffer&& _other)
    : m_device(_other.m_device) {
    _other.m_device = nullptr;

    memory::memcpy(&m_data, &_other.m_data, sizeof(opaque_data));
    memory::memzero(&_other.m_data, sizeof(opaque_data));
  }

  ~framebuffer() {
    if (m_device) {
      m_device->destroy_framebuffer(this);
    }
  }

private:
  WN_GRAPHICS_ADD_FRIENDS(device);
  WN_GRAPHICS_ADD_FRIENDS(command_list);

  WN_FORCE_INLINE framebuffer(device* _device) : m_device(_device) {}

  device* m_device;
};

}  // namespace graphics
}  // namespace runtime
}  // namespace wn

#endif  // __WN_GRAPHICS_IMAGE_VIEW_H__
