// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#ifndef __WN_FILE_LOGGER_H__
#define __WN_FILE_LOGGER_H__

#include "WNLogging/inc/WNLog.h"

namespace WNLogging {
WN_INLINE const char* DefaultFunctionName() {
  return (nullptr);
}

template <const char* (*T_FileFunction)() = DefaultFunctionName>
class WNFileLogger : public WNLogger {
public:
  WNFileLogger() {
    const char* logName = T_FileFunction();
    if (logName) {
      mOutFile = fopen(logName, "w+");
    } else {
      mOutFile = nullptr;
    }
  }

  ~WNFileLogger() {
    if (mOutFile) {
      fclose(mOutFile);
      mOutFile = nullptr;
    }
  }
  void FlushBuffer(const char* _buffer, size_t bufferSize,
      const std::vector<WNLogColorElement>&) {
    if (mOutFile) {
      size_t written = 0;
      size_t toWrite = bufferSize;
      do {
        written = fwrite(_buffer + written, 1, toWrite, mOutFile);
        toWrite -= written;
        if (written == 0) {
          toWrite = 0;
        }
      } while (toWrite > 0);
    } else {
      printf("%.*s", static_cast<int32_t>(bufferSize), _buffer);
    }
  }

private:
  FILE* mOutFile;
};
};

#endif  //__WN_FILE_LOGGER_H__
