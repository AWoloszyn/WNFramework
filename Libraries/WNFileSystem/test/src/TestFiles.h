
// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#include "WNContainers/inc/WNContiguousRange.h"
#include "WNContainers/inc/WNStringView.h"
#include "WNCore/inc/pair.h"

namespace TestFiles {

const wn::containers::contiguous_range<const wn::core::pair<
    wn::containers::string_view, wn::containers::string_view>>
get_files();

}  // namespace TestFiles
