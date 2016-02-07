// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#ifndef __WN_BUILTIN_TYPE_INITIALIZATION_H__
#define __WN_BUILTIN_TYPE_INITIALIZATION_H__

#include "WNDeprecatedScripting/inc/WNScriptingErrors.h"

namespace WNScripting {
class WNTypeManager;
class WNScriptingEngine;
class WNBuiltIninitializer {
public:
  static eWNTypeError InitializeIntTypes(
      WNScriptingEngine* _engine, WNTypeManager& _manager);
  static eWNTypeError InitializeFloatTypes(
      WNScriptingEngine* _engine, WNTypeManager& _manager);
  // static eWNTypeError initializeStringTypes();
  static eWNTypeError InitializeScriptingCasts(
      WNScriptingEngine* _engine, WNTypeManager& _manager);
  static eWNTypeError InitializeFunctions(WNScriptingEngine& _engine);
};
}

#endif  //__WN_BUILTIN_TYPE_INITIALIZATION_H__
