// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#ifndef __WN_RUNTIME_WINDOW_XCB_WINDOW_H__
#define __WN_RUNTIME_WINDOW_XCB_WINDOW_H__

#include "WNLogging/inc/WNLog.h"
#include "WNMultiTasking/inc/WNJobSignal.h"
#include "WNWindow/inc/WNWindow.h"

#include <xcb/xcb.h>

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

class xcb_window : public window {
public:
  xcb_window(logging::log* _log, multi_tasking::job_pool* _job_pool,
      multi_tasking::job_signal* _creation_signal,
      const application::application_data* _data, uint32_t _x, uint32_t _y,
      uint32_t _width, uint32_t _height)
    : m_log(_log),
      m_job_pool(_job_pool),
      m_app_data(_data),
      m_x(_x),
      m_y(_y),
      m_width(_width),
      m_height(_height),
      m_exit(false),
      m_window(0),
      m_create_signal(_job_pool, 0),
      m_destroy_signal(_job_pool, 0),
      m_creation_signal(_creation_signal) {}
  ~xcb_window() {
    m_create_signal.wait_until(1);
    if (m_connection) {
      xcb_destroy_notify_event_t* event =
          reinterpret_cast<xcb_destroy_notify_event_t*>(alloca(32));
      event->event = m_window;
      event->window = m_window;
      event->response_type = XCB_DESTROY_NOTIFY;
      xcb_send_event(m_connection, false, m_window,
          XCB_EVENT_MASK_STRUCTURE_NOTIFY, (char*)event);
      xcb_flush(m_connection);
      m_destroy_signal.wait_until(1);
      xcb_disconnect(m_connection);
    }
  }
  window_error initialize() override;
  bool is_valid() override {
    return m_window != 0;
  }

private:
  void dispatch_loop(void* _unused);

  logging::log* m_log;
  multi_tasking::job_pool* m_job_pool;
  const application::application_data* m_app_data;
  uint32_t m_x;
  uint32_t m_y;
  uint32_t m_width;
  uint32_t m_height;
  bool m_exit;
  xcb_connection_t* m_connection;
  xcb_screen_t* m_screen;
  xcb_drawable_t m_window;
  xcb_intern_atom_reply_t* m_delete_window;

  multi_tasking::job_signal m_create_signal;
  multi_tasking::job_signal m_destroy_signal;
  multi_tasking::job_signal* m_creation_signal;
};

}  // namespace window
}  // namespace runtime
}  // namespace wn

#endif  // __WN_RUNTIME_WINDOW_XCB_WINDOW_H__