// Copyright (c) 2018, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#ifndef __WN_RUNTIME_WINDOW_WINDOWS_WINDOW_H__
#define __WN_RUNTIME_WINDOW_WINDOWS_WINDOW_H__

#include "WNContainers/inc/WNHashMap.h"
#include "WNLogging/inc/WNLog.h"
#include "WNMultiTasking/inc/job_signal.h"
#include "WNWindow/inc/WNWindow.h"

namespace wn {
namespace memory {
class allocator;
}  // namespace memory

namespace multi_tasking {
class job_pool;
}  // namespace multi_tasking

namespace runtime {
namespace application {
struct application_data;
}  // namespace application
namespace window {

struct native_handle {
  HWND handle;
  HINSTANCE instance;
};

class windows_window : public window {
public:
  windows_window(memory::allocator* _allocator, logging::log* _log,
      multi_tasking::job_pool* _job_pool,
      multi_tasking::job_signal* _creation_signal,
      const application::application_data* _data, uint32_t _x, uint32_t _y,
      uint32_t _width, uint32_t _height)
    : window(_allocator),
      m_log(_log),
      m_job_pool(_job_pool),
      m_app_data(_data),
      m_x(_x),
      m_y(_y),
      m_width(_width),
      m_height(_height),
      m_exit(false),
      m_window({0}),
      m_signal(_job_pool, 0),
      m_creation_signal(_creation_signal) {}
  ~windows_window() {
    m_signal.wait_until(1);
    if (m_window.handle) {
      SendNotifyMessage(m_window.handle, WM_USER, 0, 0);
      m_signal.wait_until(3);
    }
  }
  window_error initialize() override;
  bool is_valid() const override {
    return m_window.handle != 0;
  }

  virtual window_type type() const {
    return window_type::system;
  }

  const void* get_native_handle() const override {
    return reinterpret_cast<const void*>(&m_window);
  }

  uint32_t get_width() const override {
    return m_width;
  }

  uint32_t get_height() const override {
    return m_height;
  }

  bool get_key_state(key_code _key) const override {
    return m_key_states[static_cast<uint32_t>(_key)];
  }

  bool get_mouse_state(mouse_button _button) const override {
    return m_mouse_states[static_cast<uint32_t>(_button)];
  }

  uint32_t get_cursor_x() const override {
    return m_cursor_x;
  }

  uint32_t get_cursor_y() const override {
    return m_cursor_y;
  }

private:
  void dispatch_loop(RECT data);

  void process_callback(UINT _msg, LPARAM _lparam, WPARAM _wparam);

  static LRESULT CALLBACK wnd_proc(
      HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  logging::log* m_log;
  multi_tasking::job_pool* m_job_pool;
  const application::application_data* m_app_data;
  uint32_t m_x;
  uint32_t m_y;
  uint32_t m_width;
  uint32_t m_height;
  bool m_exit;
  native_handle m_window;
  multi_tasking::job_signal m_signal;
  multi_tasking::job_signal* m_creation_signal;

  std::atomic<uint32_t> m_cursor_x;
  std::atomic<uint32_t> m_cursor_y;
};

}  // namespace window
}  // namespace runtime
}  // namespace wn

#endif  // __WN_RUNTIME_WINDOW_WINDOWS_WINDOW_H__
