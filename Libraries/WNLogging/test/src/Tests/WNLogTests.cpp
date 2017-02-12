// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include "WNLogging/inc/WNLog.h"
#include "WNExecutableTest/inc/WNTestHarness.h"
#include "WNLogging/inc/WNConsoleLogger.h"
#include "WNLogging/inc/WNFileLogger.h"

#ifndef __WN_USE_PRECOMPILED_HEADER
#include "WNMultiTasking/inc/WNThread.h"
#endif

// using namespace wn::logging;
// TODO(awoloszyn): This will probably have to be moved
//    under the runtime at some point.
TEST(WNLogInterface, SimpleTest) {
  wn::logging::console_logger<> logger;
  wn::logging::static_log<wn::logging::log_level::max> default_log(&logger);
  default_log.log()->log_critical("asdf", "asfd");
  default_log.log()->flush();
  default_log.log()->log_critical("asdf", "asfd");
  default_log.log()->flush();
}
