// Copyright (c) 2014, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WNNetworking/inc/Internal/Linux/WNOutConnectionLinux.h"
#include "WNMemory/inc/WNManipulation.h"

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

using namespace WNNetworking;

WNOutConnectionLinux::WNOutConnectionLinux(WNNetworkManager& _manager) :
    WNConnectionLinux(_manager) {
}

WNNetworkManagerReturnCode::type WNOutConnectionLinux::Initialize(WNConnectionType::type& _type, const wn_char* _target, wn_uint16 _port) {
    addrinfo* result;
    addrinfo* rp;
    addrinfo hints;

    wn_int32 sock;

    wn::memory::memzero(&hints, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    wn_char port[16];

    WN_SNPRINTF(port, 16, "%d", _port);

    sock = getaddrinfo(_target, port, &hints, &result);

    if (sock != 0) {
        return(WNNetworkManagerReturnCode::eWNCannotCreateSocket);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        mSockFD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if(mSockFD == -1) {
            continue;
        }

        connect(mSockFD , rp->ai_addr, rp->ai_addrlen);

        if (sock == 0) {
            break;
        }

        close(mSockFD);
    }

    if (rp == NULL) {
        return(WNNetworkManagerReturnCode::eWNBadHostName);
    }

    freeaddrinfo(result);

    wn_int32 flags;

    flags = fcntl(mSockFD, F_GETFL, 0);

    if (flags == -1) {
        return(WNNetworkManagerReturnCode::eWNCannotUnblock);
    }

#ifdef _WN_ANDROID
    flags |= O_NONBLOCK;
#else
    flags |= SOCK_NONBLOCK;
#endif

    if (fcntl(mSockFD, F_SETFL, flags) == -1) {
        return(WNNetworkManagerReturnCode::eWNCannotUnblock);
    }

    const wn_size_t length = WN_SNPRINTF(NULL,0, "%s:%d", _target, _port);

    mConnectionName = wn::memory::heap_allocate<wn_char>(length + 1);

    WN_SNPRINTF(mConnectionName,length +1, "%s:%d", _target, _port);

    return(WNNetworkManagerReturnCode::ok);
}