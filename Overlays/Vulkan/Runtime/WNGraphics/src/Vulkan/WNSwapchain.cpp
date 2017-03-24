// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include "WNGraphics/inc/Internal/Vulkan/WNSwapchain.h"
#include "WNGraphics/inc/Internal/Vulkan/WNDevice.h"
#include "WNGraphics/inc/Internal/Vulkan/WNQueue.h"
#include "WNGraphics/inc/Internal/Vulkan/WNVulkanImage.h"
#include "WNGraphics/inc/WNImage.h"
#include "WNGraphics/inc/WNSwapchain.h"

namespace wn {
namespace graphics {
namespace internal {
namespace vulkan {

void vulkan_swapchain::initialize(memory::allocator* _allocator,
    vulkan_device* _device, uint32_t _width, uint32_t _height,
    const swapchain_create_info& _create_info, VkSwapchainKHR swapchain,
    VkSurfaceKHR surface) {
  m_allocator = _allocator;
  m_device = _device;
  m_swapchain = swapchain;
  m_surface = surface;
  m_images = containers::dynamic_array<image>(m_allocator);
  uint32_t num_buffers = _create_info.num_buffers;
  m_images.reserve(_create_info.num_buffers);
  containers::dynamic_array<VkImage> images(
      m_allocator, _create_info.num_buffers);
  m_device->vkGetSwapchainImagesKHR(
      m_device->m_device, m_swapchain, &num_buffers, images.data());
  for (size_t i = 0; i < num_buffers; ++i) {
    m_images.push_back(image(reinterpret_cast<device*>(m_device)));
    image& image = m_images.back();
    image.m_resource_info.depth = 1;
    image.m_resource_info.height = _height;
    image.m_resource_info.width = _width;
    image.m_resource_info.offset_in_bytes = 0;
    image.m_resource_info.row_pitch_in_bytes = 0;
    image.m_resource_info.total_memory_required = 0;
    image.m_resource_info.format = _create_info.format;
    image.m_is_swapchain_image = true;
    VulkanImage& img = image.data_as<VulkanImage>();
    img.image = images[i];
  }
}

vulkan_swapchain::~vulkan_swapchain() {
  if (m_device) {
    m_device->vkDestroySwapchainKHR(m_device->m_device, m_swapchain, nullptr);
    m_device->m_surface_helper.destroy_surface(m_surface);
  }
}

uint32_t vulkan_swapchain::get_backbuffer_index() const {
  uint32_t idx;
  m_device->vkAcquireNextImageKHR(m_device->m_device, m_swapchain,
      0xFFFFFFFFFFFFFFFF, VK_NULL_HANDLE, VK_NULL_HANDLE, &idx);
  return idx;
}

image* vulkan_swapchain::get_image_for_index(uint32_t index) {
  WN_DEBUG_ASSERT_DESC(index < m_images.size(), "Invalid image index");
  return &m_images[index];
}

void vulkan_swapchain::present_internal(
    queue* q, const swapchain_create_info&, uint32_t index) const {
  vulkan_queue* queue = reinterpret_cast<vulkan_queue*>(q);
  queue->present(m_swapchain, index);
}

}  // namespace vulkan
}  // namespace internal
}  // namespace graphics
}  // namespace wn