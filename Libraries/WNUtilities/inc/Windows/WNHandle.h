// Copyright (c) 2016, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#ifndef __WN_UTILITIES_WINDOWS_HANDLE_H__
#define __WN_UTILITIES_WINDOWS_HANDLE_H__

#include "WNCore/inc/WNAssert.h"
#include "WNCore/inc/WNUtility.h"

#include <Windows.h>

namespace wn {
namespace utilities {
namespace windows {

class handle final : core::non_copyable {
public:
  WN_FORCE_INLINE handle() : m_handle(nullptr) {}

  WN_FORCE_INLINE handle(const nullptr_t) : handle() {}

  WN_FORCE_INLINE handle(const HANDLE _handle) : m_handle(_handle) {}

  WN_FORCE_INLINE handle(handle&& _other)
    : m_handle(std::move(_other.m_handle)) {
    _other.m_handle = nullptr;
  }

  WN_FORCE_INLINE ~handle() {
    dispose();
  }

  WN_FORCE_INLINE handle& operator=(const nullptr_t) {
    handle(nullptr).swap(*this);

    return *this;
  }

  WN_FORCE_INLINE handle& operator=(const HANDLE _handle) {
    handle(_handle).swap(*this);

    return *this;
  }

  WN_FORCE_INLINE handle& operator=(handle&& _other) {
    handle(std::move(_other)).swap(*this);

    return *this;
  }

  WN_FORCE_INLINE bool is_null() const {
    return (m_handle == nullptr);
  }

  WN_FORCE_INLINE bool is_valid() const {
    return (!is_null() && m_handle != INVALID_HANDLE_VALUE);
  }

  WN_FORCE_INLINE HANDLE value() const {
    return m_handle;
  }

  WN_FORCE_INLINE void dispose() {
    if (is_valid()) {
      const BOOL close_result = ::CloseHandle(m_handle);

      m_handle = nullptr;

      WN_DEBUG_ASSERT_DESC(close_result == TRUE, "failed to close handle");

#ifndef _WN_DEBUG
      WN_UNUSED_ARGUMENT(close_result);
#endif
    }
  }

  WN_FORCE_INLINE void swap(handle& _other) {
    core::swap(m_handle, _other.m_handle);
  }

private:
  HANDLE m_handle;
};

WN_FORCE_INLINE bool operator==(const handle& _handle, const nullptr_t) {
  return _handle.is_null();
}

WN_FORCE_INLINE bool operator==(const nullptr_t, const handle& _handle) {
  return (_handle == nullptr);
}

WN_FORCE_INLINE bool operator!=(const handle& _handle, const nullptr_t) {
  return !(_handle == nullptr);
}

WN_FORCE_INLINE bool operator!=(const nullptr_t, const handle& _handle) {
  return (_handle != nullptr);
}

}  // namespace windows
}  // namespace utilities
}  // namespace wn

#endif  // __WN_UTILITIES_WINDOWS_HANDLE_H__
