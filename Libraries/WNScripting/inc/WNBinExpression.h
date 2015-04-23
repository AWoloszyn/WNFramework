// Copyright (c) 2014, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __WN_BIN_EXPRESSION_H__
#define __WN_BIN_EXPRESSION_H__

#include "WNScripting/inc/WNExpression.h"
#include "WNScripting/inc/WNScriptingEnums.h"
namespace WNScripting {
    class WNBinExpression : public WNExpression {
    public:
        WNBinExpression(WNArithmeticType _type, WNExpression* _lhs, WNExpression* _rhs);
        virtual ~WNBinExpression();
        virtual eWNTypeError GenerateCode(WNCodeModule& _module, const WNFunctionDefinition* _def, WNLogging::WNLog& _compilationLog);
    private:
        WNArithmeticType mType;
        WNExpression* mLHS;
        WNExpression* mRHS;
    };
}
#endif//__WN_BIN_EXPRESSION_H__