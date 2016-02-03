// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#ifndef __WN_NETWORKING_NETWORK_MANAGER_LINUX_H__
#define __WN_NETWORKING_NETWORK_MANAGER_LINUX_H__

#include "WNNetworking/inc/WNNetworkManager.h"
#include "WNMultiTasking/inc/WNSpinLock.h"

#include <vector>

namespace wn {
    template <typename Return>
    class thread;
}

namespace WNNetworking {
    class WNConnectionLinux;
    class WNListenConnectionLinux;

    class WNNetworkManagerLinux : public WNNetworkManager {
    public:
        WNNetworkManagerLinux();
        virtual ~WNNetworkManagerLinux();

        virtual WNNetworkManagerReturnCode::type Initialize(wn_uint32 _numWorkerThreads);
        virtual WNNetworkManagerReturnCode::type ConnectTo(WNConnection*& _outHandle, WNConnectionType::type _type, const wn_char* _target, wn_uint16 _port);
        virtual WNNetworkManagerReturnCode::type CreateListener(WNConnection*& _outHandle, WNConnectionType::type _type, wn_uint16 _port, const WNConnectedCallback& _callback);
        virtual wn_void Cleanup();
        virtual wn_void DestroyConnection(WNConnection* _connection);

    private:
        enum eWNLinuxInitializationState {
            eWNNotStarted,
            eWNEPollWriteCreated,
            eWNWriteThreadsCreated,
            eWNEPollReadCreated,
            eWNReadThreadsCreated,
            eWNEPollListenCreated,
            eWNListenThreadCreated,
            eWNInitializationComplete
        };

    private:
        wn_bool AddToReadEPoll(WNConnectionLinux* _conn);
        wn_bool AddToWriteEPoll(WNConnectionLinux* _conn);
        wn_void CleanAllConnections();
        wn_void ReadThread();
        wn_void WriteThread();
        wn_void ListenThread();

    private:
        wn::multi_tasking::spin_lock mListenMutex;
        wn::multi_tasking::spin_lock mIncommingMutex;
        wn::multi_tasking::spin_lock mOutgoingMutex;
        wn::multi_tasking::spin_lock mInvalidMutex;

        wn::memory::basic_allocator allocator;
        eWNLinuxInitializationState mInitializationState;
        wn_int32 mWriteEPollInstance;
        wn_int32 mReadEPollInstance;
        wn_int32 mListenEPollInstance;
        wn::multi_tasking::thread<wn_void>* mListenThread;
        wn_bool mShuttingDown;
        std::vector<wn::multi_tasking::thread<wn_void>*> mReadThreads;
        std::vector<wn::multi_tasking::thread<wn_void>*> mWriteThreads;
        std::list<WNListenConnectionLinux*> mListenConnections;
        std::list<WNConnection*> mInvalidConnections;
        std::list<WNConnectionLinux*> mIncommingConnections;
        std::list<WNConnectionLinux*> mOutgoingConnections;
    };
};

#endif // __WN_NETWORKING_NETWORK_MANAGER_WINDOWS_H__
