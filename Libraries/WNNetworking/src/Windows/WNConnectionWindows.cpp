// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include "WNNetworking/inc/Internal/Windows/WNConnectionWindows.h"
#include "WNCore/inc/WNEndian.h"
#include "WNMath/inc/WNBasic.h"
#include "WNMemory/inc/WNBasic.h"
#include "WNNetworking/inc/Internal/Windows/WNNetworkManagerWindows.h"

#include <mutex>

using namespace WNNetworking;

WNConnectionWindows::WNConnectionWindows(WNNetworkManager& _manager)
  : WNConnection(),
    mSocket(0),
    mInProcessedBytes(0),
    mReadHead(0),
    mOverflowAmount(0),
    mBufferBase(0),
    mCurrentReadBuffer(_manager),
    mManager(_manager) {
  wn::memory::memory_zero(&mReceiveOverlap);
  wn::memory::memory_zero(&mSendOverlap);

  mReadLocation =
      wn::memory::make_intrusive<WNBufferResource, WNNetworkManager&>(
          &allocator, _manager);
}

WNConnectionWindows::~WNConnectionWindows() {
  if (mSocket != 0) {
    closesocket(mSocket);

    mSocket = 0;
  }
}

void WNConnectionWindows::Invalidate() {
  WNConnection::Invalidate();

  if (mSocket != 0) {
    closesocket(mSocket);

    mSocket = 0;
  }
}

SOCKET WNConnectionWindows::GetWindowsSocket() {
  return (mSocket);
}

bool WNConnectionWindows::Receive() {
  DWORD bytes;
  DWORD flags = 0;

  mReadLocation->PrepareRead();

  int received = WSARecv(mSocket, mReadLocation->GetReadWinBuf(), 1, &bytes,
      &flags, &mReceiveOverlap, NULL);
  int lastError = WSAGetLastError();

  if (SOCKET_ERROR == received && (WSA_IO_PENDING != lastError)) {
    return (false);
  }

  return (true);
}

WNConnectionWindows::eWNNetworkOperation WNConnectionWindows::GetOperation(
    LPOVERLAPPED _overlapped) {
  if (_overlapped == &mReceiveOverlap) {
    return (eWNReadOp);
  } else if (_overlapped == &mSendOverlap) {
    return (eWNWriteOp);
  } else {
    return (eWNNoOp);
  }
}

bool WNConnectionWindows::ProcessRead(
    WNNetworkManagerWindows* _windowsManager, DWORD _bytesTransferred) {
  size_t processedBytes = 0;
  WN_RELEASE_ASSERT(mReadHead <= wn::containers::MAX_DATA_WRITE);
  while (processedBytes != _bytesTransferred) {
    WN_RELEASE_ASSERT(processedBytes < _bytesTransferred);
    size_t transferToOverflow =
        wn::min<size_t>(8 - mOverflowAmount, _bytesTransferred);
    wn::memory::memcpy(mOverflowLocation + mOverflowAmount,
        (mReadLocation->GetBaseLocation()) + mReadHead, transferToOverflow);
    mOverflowAmount += transferToOverflow;
    processedBytes += transferToOverflow;
    mInProcessedBytes += transferToOverflow;
    mBufferBase += transferToOverflow;
    mReadLocation->AddData(transferToOverflow);

    if (mOverflowAmount < 8) {
      mReadHead += transferToOverflow;
      WN_RELEASE_ASSERT(processedBytes == _bytesTransferred);
      if (mBufferBase == wn::containers::MAX_DATA_WRITE) {
        mReadLocation =
            wn::memory::make_intrusive<WNBufferResource, WNNetworkManager&>(
                &allocator, mManager);
        mReadHead = 0;
        mBufferBase = 0;
      }
      break;
    }

    uint32_t num = *reinterpret_cast<uint32_t*>(mOverflowLocation);
    uint32_t callback = *reinterpret_cast<uint32_t*>(mOverflowLocation + 4);

    callback = wn::core::from_big_endian(callback);
    num = wn::core::from_big_endian(num);

    size_t mMaxWrite =
        wn::min<size_t>(_bytesTransferred, (num - mInProcessedBytes));

    mReadHead += mMaxWrite;
    processedBytes += mMaxWrite;

    if (mInProcessedBytes + mMaxWrite == num) {
      // We got all the data we needed
      mCurrentReadBuffer.AppendBuffer(mReadLocation, mReadHead, mBufferBase);
      _windowsManager->FireCallback(callback, this, mCurrentReadBuffer);
      mCurrentReadBuffer.Clear();
      mReadHead += transferToOverflow;
      mBufferBase = mReadHead;
      mInProcessedBytes = 0;
      mOverflowAmount = 0;
    } else {
      mInProcessedBytes += mMaxWrite;
      if (mReadHead == wn::containers::MAX_DATA_WRITE) {
        mCurrentReadBuffer.AppendBuffer(mReadLocation, mReadHead, mBufferBase);
      }
    }
    if (mReadHead == wn::containers::MAX_DATA_WRITE) {
      mReadLocation =
          wn::memory::make_intrusive<WNBufferResource, WNNetworkManager&>(
              &allocator, mManager);
      mReadHead = 0;
      mBufferBase = 0;
    } else {
      mReadLocation->AddData(mMaxWrite);
    }
  }

  return (true);
}

void WNConnectionWindows::AppendSendBuffer(WNNetworkWriteBuffer& _buff) {
  std::lock_guard<wn::multi_tasking::mutex> guard(mSendMutex);

  mWriteBuffers.push_back(_buff);

  if (mWriteBuffers.size() == 1) {
    Send();
  }
}

void WNConnectionWindows::Send() {
  WNNetworkWriteBuffer& buff = mWriteBuffers.front();
  const std::vector<wn::memory::intrusive_ptr<WNBufferResource>>& sendBuffs =
      buff.GetChunks();
  mWSAWriteBuffers.clear();
  for (std::vector<wn::memory::intrusive_ptr<WNBufferResource>>::const_iterator
           i = sendBuffs.begin();
       i != sendBuffs.end(); ++i) {
    mWSAWriteBuffers.push_back(*(*i)->GetWriteWinBuf());
  }
  DWORD bytes;
  int sent = WSASend(mSocket, &mWSAWriteBuffers[0],
      static_cast<DWORD>(sendBuffs.size()), &bytes, 0, &mSendOverlap, NULL);
  if (SOCKET_ERROR == sent && WSA_IO_PENDING != WSAGetLastError()) {
    mManager.DestroyConnection(this);
  }
}

void WNConnectionWindows::ProcessWrite() {
  std::lock_guard<wn::multi_tasking::mutex> guard(mSendMutex);
  mWriteBuffers.pop_front();
  if (mWriteBuffers.size() > 0) {
    Send();
  }
}

void WNConnectionWindows::SendBuffer(WNNetworkWriteBuffer& _buffer) {
  _buffer.FlushWrite();
  AppendSendBuffer(_buffer);
}