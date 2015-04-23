// Copyright (c) 2014, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __WN_UN_EXPRESSION_H__
#define __WN_UN_EXPRESSION_H__
#include "WNScripting/inc/WNExpression.h"
#include "WNScripting/inc/WNScriptingEnums.h"

namespace WNScripting {
    class WNUNExpression : public WNExpression {
    public:
        WNUNExpression(WNUnaryType _type, WNExpression* _expr);
        virtual ~WNUNExpression();
        virtual eWNTypeError GenerateCode(WNCodeModule& _module, const WNFunctionDefinition* _def, WNLogging::WNLog& _compilationLog);
    private:
        WNUnaryType mType;
        WNExpression* mExpression;
    };
}
#endif//__WN_UN_EXPRESSION_H__