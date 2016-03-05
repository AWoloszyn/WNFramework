// Copyright (c) 2016, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#ifndef __WN_GRAPHICS_SRC_VULKAN_VULKAN_HELPERS_H__
#define __WN_GRAPHICS_SRC_VULKAN_VULKAN_HELPERS_H__

#include "WNContainers/inc/WNDynamicArray.h"
#include "WNGraphics/inc/Internal/Vulkan/WNPhysicalDevice.h"
#include "WNGraphics/inc/WNPhysicalDevice.h"

namespace WNLogging {
class WNLog;
}
namespace wn {
namespace memory {
  class allocator;
}
namespace graphics {
namespace internal {
namespace vulkan {

void enumerate_physical_devices(memory::allocator* _allocator,
    WNLogging::WNLog* _log,
    containers::dynamic_array<physical_device_ptr>& _arr);

}  // namespace vulkan
}  // namespace internal
}  // namespace graphics
}  // namespace wn

#endif  // __WN_GRAPHICS_INTERNAL_VULKAN_PHYSICAL_DEVICE_H__