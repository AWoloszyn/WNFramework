// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#ifndef __WN_GRAPHICS_INTERNAL_D3D12_FENCE_DATA_H__
#define __WN_GRAPHICS_INTERNAL_D3D12_FENCE_DATA_H__

#include "WNUtilities/inc/WNHandle.h"

#include <d3d12.h>
#include <wrl.h>

namespace wn {
namespace graphics {
namespace internal {
namespace d3d12 {

struct fence_data WN_FINAL {
  Microsoft::WRL::ComPtr<ID3D12Fence> fence;
  utilities::windows::handle event;
};

}  // namespace d3d12
}  // namespace internal
}  // namespace graphics
}  // namespace wn

#endif  // __WN_GRAPHICS_INTERNAL_D3D12_FENCE_DATA_H__