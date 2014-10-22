// Copyright (c) 2014, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#ifndef __WN_PLATFORM_INTERNAL_WINDOWS_TIME_INL__
#define __WN_PLATFORM_INTERNAL_WINDOWS_TIME_INL__

#ifndef __WN_PLATFORM_TIME_H__
    #error "WNTime_Windows.inl should never be included directly. Please include WNTime.h instead"
#elif !defined _WN_WINDOWS
    #error "WNTime_Windows.inl has been included on a non Linux platform. Please rectify this."
#endif

#include <windows.h>

namespace WNPlatform {
    WN_FORCE_INLINE WN_UINT64 WNGetBigTime() {
        SYSTEMTIME systemTime;

        GetSystemTime(&systemTime);

        FILETIME fileTime;

        SystemTimeToFileTime(&systemTime, &fileTime);

        LARGE_INTEGER largerIntegerTime;

        largerIntegerTime.LowPart = fileTime.dwLowDateTime;
        largerIntegerTime.HighPart = fileTime.dwHighDateTime;

        return(static_cast<WN_UINT64>((largerIntegerTime.QuadPart / 10000LL) - 11644473600000LL));
    }

    WN_FORCE_INLINE WN_UINT32 WNGetTickCount() {
        return(static_cast<WN_UINT32>(GetTickCount()));
    }

    WN_FORCE_INLINE WN_UINT64 WNGetBigTickCount() {
        return(static_cast<WN_UINT64>(GetTickCount64()));
    }
}

#endif // __WN_PLATFORM_INTERNAL_WINDOWS_TIME_INL__