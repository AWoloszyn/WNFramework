// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include <android_native_app_glue.h>

#pragma once
#ifndef __WN_ENTRYPOINT_WN_ENTRY_DATA_H__
#define __WN_ENTRYPOINT_WN_ENTRY_DATA_H__

namespace wn {
namespace entry {

struct host_specific_data {
  struct android_app* android_app;
  const char* package_name;
};

}  // namespace entry
}  // namespace wn

#endif  // __WN_ENTRYPOINT_WN_ENTRY_H__