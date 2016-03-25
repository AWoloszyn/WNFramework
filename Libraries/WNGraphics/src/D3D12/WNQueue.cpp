// Copyright (c) 2016, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WNCore/inc/WNUtility.h"
#include "WNGraphics/inc/Internal/D3D12/WNCommandList.h"
#include "WNGraphics/inc/Internal/D3D12/WNFenceData.h"
#include "WNGraphics/inc/Internal/D3D12/WNQueue.h"
#include "WNGraphics/inc/WNDevice.h"
#include "WNGraphics/inc/WNFence.h"

namespace wn {
namespace graphics {
namespace internal {
namespace d3d12 {

void d3d12_queue::enqueue_signal(fence& _fence) {
  fence_data& data = _fence.data_as<fence_data>();
  m_queue->Signal(data.fence.Get(), 1);
}

d3d12_queue::~d3d12_queue() {
  m_device->destroy_queue(this);
}

void d3d12_queue::enqueue_command_list(command_list* _command) {
  d3d12_command_list* command_list =
      reinterpret_cast<d3d12_command_list*>(_command);
  ID3D12CommandList* list = command_list->command_list();
  m_queue->ExecuteCommandLists(1, &list);
}

}  // namespace d3d12
}  // namespace internal
}  // namespace graphics
}  // namespace wn