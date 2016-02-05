// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#ifndef __WN_CONSOLE_LOGGER_H__
#define __WN_CONSOLE_LOGGER_H__
#include "WNCore/inc/WNBase.h"
#include "WNLogging/inc/WNLog.h"
#include <vector>
namespace WNLogging {
    enum WNConsoleLocation {
        eStdOut,
        eStdErr
    };
    template<WNConsoleLocation T_Level = eStdOut>
    class WNConsoleLogger: public WNLogger {
    public:
        WNConsoleLogger();
        ~WNConsoleLogger();
        virtual WN_INLINE void FlushBuffer(const char* _buffer, size_t bufferSize, const std::vector<WNLogColorElement>&);
    private:
#if defined(_WN_WINDOWS)
        HANDLE mConsoleHandle;
        BOOL   mConsoleCreated;
        BOOL   mInitialized;
#elif defined(_WN_ANDROID)
        int32_t mLogPriority;
#endif
    };
}

#ifdef _WN_WINDOWS
#include "WNLogging/inc/Internal/WNConsoleLoggerWindows.inl"
#elif defined _WN_ANDROID
#include "WNLogging/inc/Internal/WNConsoleLoggerAndroid.inl"
#else
#include "WNLogging/inc/Internal/WNConsoleLoggerLinux.inl"
#endif

#endif
