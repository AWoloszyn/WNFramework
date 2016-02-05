// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#ifndef __WN_NETWORKING_NETWORK_READ_BUFFER_H__
#define __WN_NETWORKING_NETWORK_READ_BUFFER_H__

#include "WNCore/inc/WNBase.h"
#include "WNCore/inc/WNTypes.h"
#include "WNContainers/inc/WNDataBuffer.h"
#include "WNNetworking/inc/WNBufferResource.h"
#include "WNMemory/inc/WNIntrusivePtr.h"

#ifdef _WN_MSVC
    #pragma warning(push)
    #pragma warning(disable: 4275)  // non � DLL-interface used as base for DLL-interface
#endif

#include <deque>

#ifdef _WN_MSVC
    #pragma warning(pop)
#endif

namespace WNContainers {
    class WNSerializerBase;
}

namespace WNNetworking {
    class WNNetworkManager;

    class WNNetworkReadBuffer : public wn::containers::data_buffer {
    public:
        typedef std::deque<wn::memory::intrusive_ptr<WNNetworking::WNBufferResource>> WNBufferQueue;

    public:
        WNNetworkReadBuffer(WNNetworkManager& _manager);

        virtual WN_FORCE_INLINE ~WNNetworkReadBuffer() {}

        virtual bool serialize(const wn::containers::serializer_base& _serializer, const uint32_t _flags) override;
        virtual char* reserve(const size_t _numBytes, size_t& _returnedBytes) override;
        virtual wn::containers::data_buffer_type type() const override;

        void AppendBuffer(wn::memory::intrusive_ptr<WNBufferResource>& _buffer, size_t _dataCount, size_t _dataOffset);
        bool Initialized();
        void Clear();
        char* GetLastBuffer();
        void PrepareRead();

    private:
        WNNetworkReadBuffer& operator = (const WNNetworkReadBuffer&);

    private:
        wn::memory::basic_allocator allocator;
        WNNetworkManager& mManager;
        wn::memory::intrusive_ptr<WNNetworking::WNBufferResource> mDataOverflow;
        WNBufferQueue mChunks;

        WNBufferQueue::iterator mCurrentChunk;
        size_t mBufferPointer;
        size_t mWriteOffset;
        size_t mTotalSize;
        size_t mTotalRead;
        bool   mLastChunk;
        bool   mInitialized;
    };
}

#endif // __WN_NETWORKING_NETWORK_READ_BUFFER_H__
