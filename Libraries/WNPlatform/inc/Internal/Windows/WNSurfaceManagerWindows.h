// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#ifndef __WN_PLATFORM_INTERNAL_WINDOWS_SURFACE_MANAGER_WINDOWS_H__
#define __WN_PLATFORM_INTERNAL_WINDOWS_SURFACE_MANAGER_WINDOWS_H__

#include "WNPlatform/inc/WNSurfaceManager.h"
#include "WNPlatform/inc/Internal/Windows/WNSurfaceWindows.h"
#include "WNThreading/inc/WNThread.h"
#include "WNThreading/inc/WNSemaphore.h"
#include "WNThreading/inc/WNMutex.h"

#include <vector>

namespace wn {
    class WNSurfaceManagerWindows : public WNSurfaceManager {
    public:
        WNSurfaceManagerWindows();
        virtual WN_FORCE_INLINE ~WNSurfaceManagerWindows() {}

        virtual WNSurfaceManagerReturnCode::type Initialize();
        virtual WNSurfaceManagerReturnCode::type CreateSurface(wn_uint32 _x, wn_uint32 _y, wn_uint32 _width, wn_uint32 _height, wn::memory::intrusive_ptr<surface>& _surface);
        virtual WNSurfaceManagerReturnCode::type Release();

    private:
        WNSurfaceManagerWindows(const WNSurfaceManagerWindows&);
        WNSurfaceManagerWindows& operator = (const WNSurfaceManagerWindows&);

    private:
        class WNWindowThreadData {
        public:
            WNWindowThreadData(wn::memory::intrusive_ptr<wn::WNSurfaceWindows> _wnd);

        public:
            wn::threading::thread<wn_bool>* mThread;
            wn::memory::intrusive_ptr<wn::WNSurfaceWindows> mWindow;
            wn_atom_t mExit;
        };

    private:
        HWND mPendingHwnd;
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static wn_bool MessagePump(WNWindowThreadData* _data);
        std::vector<WNWindowThreadData*> mMessagePumps;

        wn::threading::semaphore mCreatedWindowLock;
        wn::threading::mutex mCallbackLock;
        wn::threading::mutex mWindowCreationLock;
    };
};

#endif // __WN_PLATFORM_INTERNAL_WINDOWS_SURFACE_MANAGER_WINDOWS_H__