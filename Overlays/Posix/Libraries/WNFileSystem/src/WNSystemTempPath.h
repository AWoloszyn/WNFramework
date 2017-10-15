// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#ifndef __WN_FILE_SYSTEM_POSIX_SYSTEM_TEMP_PATH_H__
#define __WN_FILE_SYSTEM_POSIX_SYSTEM_TEMP_PATH_H__

#include "WNContainers/inc/WNString.h"

namespace wn {
namespace entry {

struct system_data;

}  // namespace entry

namespace memory {

class allocator;

}  // namespace memory

namespace file_system {
namespace internal {

containers::string get_temp_path(
    memory::allocator* _allocator, const entry::system_data* _system_data);

}  // namespace internal
}  // namespace file_system
}  // namespace wn

#endif  // __WN_FILE_SYSTEM_POSIX_SYSTEM_TEMP_PATH_H__