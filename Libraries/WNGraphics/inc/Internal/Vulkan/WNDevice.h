// Copyright (c) 2016, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#ifndef __WN_GRAPHICS_INTERNAL_VULKAN_DEVICE_H__
#define __WN_GRAPHICS_INTERNAL_VULKAN_DEVICE_H__

#include "WNGraphics/inc/Internal/Vulkan/WNVulkanContext.h"
#include "WNGraphics/inc/WNDevice.h"

namespace wn {
namespace graphics {
namespace internal {
namespace vulkan {

class device : public internal::device {
public:
  device(memory::allocator* _allocator, WNLogging::WNLog* _log,
      VkDevice _device, PFN_vkDestroyDevice _destroy_device,
      const VkPhysicalDeviceMemoryProperties* _memory_properties);
  ~device();
  bool initialize(vulkan_context* _context, uint32_t _graphics_and_device_queue);
  upload_heap create_upload_heap(size_t _num_bytes) final;
  download_heap create_download_heap(size_t _num_bytes) final;

private:
  template <typename T>
  friend class graphics::heap;

  uint8_t* acquire_range(
      upload_heap* _buffer, size_t _offset, size_t _num_bytes) final;
  void release_range(
      upload_heap* _buffer, size_t _offset, size_t _num_bytes) final;
  void destroy_heap(upload_heap* _heap) final;

  uint8_t* acquire_range(
      download_heap* _buffer, size_t _offset, size_t _num_bytes) final;
  void release_range(
      download_heap* _buffer, size_t _offset, size_t _num_bytes) final;
  void destroy_heap(download_heap* _heap) final;

  template <typename heap_type>
  heap_type create_heap(size_t _num_bytes,
      const VkBufferCreateInfo& _info, uint32_t memory_type_index);
  template <typename heap_type>
  void destroy_typed_heap(heap_type* type);

  uint32_t get_memory_type_index(uint32_t _types, VkFlags _properties) const;

  PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
  PFN_vkGetDeviceQueue vkGetDeviceQueue;
  PFN_vkDestroyDevice vkDestroyDevice;

  PFN_vkAllocateMemory vkAllocateMemory;
  PFN_vkFreeMemory vkFreeMemory;

  PFN_vkCreateBuffer vkCreateBuffer;
  PFN_vkDestroyBuffer vkDestroyBuffer;
  PFN_vkBindBufferMemory vkBindBufferMemory;
  PFN_vkMapMemory vkMapMemory;
  PFN_vkUnmapMemory vkUnmapMemory;

  PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
  PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;

  PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;

  VkDevice m_device;
  VkQueue m_queue;
  uint32_t m_upload_memory_type_index;
  uint32_t m_download_memory_type_index;
  bool m_upload_heap_is_coherent;
  bool m_download_heap_is_coherent;

  const VkPhysicalDeviceMemoryProperties* m_physical_device_memory_properties;
};

}  // namespace vulkan
}  // namespace internal
}  // namespace graphics
}  // namespace wn

#endif  // __WN_GRAPHICS_INTERNAL_VULKAN_DEVICE_H__