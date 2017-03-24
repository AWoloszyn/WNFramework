// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include "WNGraphics/inc/Internal/Vulkan/WNAdapter.h"
#include "WNContainers/inc/WNDynamicArray.h"
#include "WNLogging/inc/WNLog.h"

#ifndef _WN_GRAPHICS_SINGLE_DEVICE_TYPE
#include "WNGraphics/inc/Internal/Vulkan/WNDevice.h"
#else
#include "WNGraphics/inc/WNDevice.h"
#endif

namespace wn {
namespace graphics {
namespace internal {
namespace vulkan {
namespace {

#ifndef _WN_GRAPHICS_SINGLE_DEVICE_TYPE
using vulkan_device_constructable = vulkan_device;
#else
using vulkan_device_constructable = device;
#endif

}  // anonymous namespace

device_ptr vulkan_adapter::make_device(
    memory::allocator* _allocator, logging::log* _log) const {
  const float queue_priority = 1.0f;
  VkDeviceQueueCreateInfo queue_create_info{
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,  // sType;
      nullptr,                                     // pNext;
      0,                                           // flags;
      m_compute_and_graphics_queue,                // queueFamilyIndex;
      1,                                           // queueCount;
      &queue_priority,                             // pQueuePriorities;
  };

  VkPhysicalDeviceFeatures physical_features{0};

  const uint32_t num_device_extensions = 1;
  const char* device_extensions[num_device_extensions] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkDeviceCreateInfo create_info{
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,  // sType;
      nullptr,                               // pNext;
      0,                                     // flags;
      1,                                     // queueCreateInfoCount;
      &queue_create_info,                    // pQueueCreateInfos;
      0,                                     // enabledLayerCount;
      nullptr,                               // ppEnabledLayerNames;
      num_device_extensions,                 // enabledExtensionCount;
      device_extensions,                     // ppEnabledExtensionNames;
      &physical_features                     // pEnabledFeatures;
  };

  VkDevice vk_device;

  // TODO(awoloszyn): Add alloactors.
  if (m_context->vkCreateDevice(
          m_physical_device, &create_info, nullptr, &vk_device) != VK_SUCCESS) {
    _log->log_error("Could not create device");

    return nullptr;
  }

  memory::unique_ptr<vulkan_device_constructable> ptr(
      memory::make_unique_delegated<vulkan_device_constructable>(
          _allocator, [](void* _memory) {
            return new (_memory) vulkan_device_constructable();
          }));

  if (ptr) {
    if (!ptr->initialize(_allocator, _log, vk_device,
            m_context->vkDestroyDevice, &m_memory_properties, m_context.get(),
            m_compute_and_graphics_queue)) {
      return nullptr;
    }
  }

  return core::move(ptr);
}

void vulkan_adapter::initialize_device() {
  m_context->vkGetPhysicalDeviceMemoryProperties(
      m_physical_device, &m_memory_properties);
}

}  // namespace vulkan
}  // namesapce internal
}  // namespace graphics
}  // namespace wn