// Copyright (c) 2016, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#ifndef __WN_GRAPHICS_INTERNAL_DEVICE_H__
#define __WN_GRAPHICS_INTERNAL_DEVICE_H__

#include "WNCore/inc/WNUtility.h"
#include "WNMemory/inc/WNUniquePtr.h"

namespace WNLogging {

class WNLog;

}  // namespace WNLogging

namespace wn {
namespace memory {

class allocator;

}  // namespace memory

namespace graphics {

class upload_heap;
using upload_heap_ptr = memory::unique_ptr<upload_heap>;

namespace internal {

class device : public core::non_copyable {
public:
  WN_FORCE_INLINE device(memory::allocator* _allocator, WNLogging::WNLog* _log)
    : m_allocator(_allocator), m_log(_log) {}

  // On success, returns an upload_heap object. This heap must be at least,
  // 1 byte in size. If creation fails a nullptr will be returned.
  virtual upload_heap_ptr create_upload_heap(size_t num_bytes) = 0;

protected:
  friend class upload_heap;
  // Upload heap classes
  virtual void flush_mapped_range(
      upload_heap* _buffer, size_t offset, size_t num_bytes) = 0;
  virtual void destroy_upload_heap(upload_heap* _heap) = 0;

  upload_heap_ptr construct_upload_heap();
  memory::allocator* m_allocator;
  WNLogging::WNLog* m_log;
};

}  // namespace internal
}  // namespace graphics
}  // namespace wn

#endif  // __WN_GRAPHICS_INTERNAL_DEVICE_H__