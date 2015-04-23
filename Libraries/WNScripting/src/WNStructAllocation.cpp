// Copyright (c) 2014, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WNScripting/inc/WNStructAllocation.h"
#include "WNScripting/inc/WNCodeModule.h"
#include "WNScripting/inc/WNTypeNode.h"
#include "WNScripting/inc/WNNodeHelpers.h"

#ifdef _WN_MSVC
    #pragma warning(push)
    #pragma warning(disable: 4100)
    #pragma warning(disable: 4127)
    #pragma warning(disable: 4152)
    #pragma warning(disable: 4244)
    #pragma warning(disable: 4245)
    #pragma warning(disable: 4267)
    #pragma warning(disable: 4355)
    #pragma warning(disable: 4512)
    #pragma warning(disable: 4800)
#endif

#include "llvm/IR/IRBuilder.h"

#ifdef _WN_MSVC
    #pragma warning(pop)
#endif


using namespace WNScripting;

WNStructAllocation::WNStructAllocation() :
    mType(wn_nullptr),
    mCopyInitializer(wn_nullptr) {
}

WNStructAllocation::~WNStructAllocation() {
    if(mType) {
        wn::memory::destroy(mType);
    }
}

wn_void WNStructAllocation::SetType(WNTypeNode* _type) {
    mType = _type;
}

wn_void WNStructAllocation::SetCopyInitializer(WNExpression* _expr) {
    mCopyInitializer = _expr;
}

eWNTypeError WNStructAllocation::GenerateCode(WNCodeModule& _module, const WNFunctionDefinition* _def, WNLogging::WNLog& _compilationLog) {
    mNewlyCreated = wn_true;
    mForceUse = wn_true;
    WNScriptType structType = wn_nullptr;
    eWNTypeError err = ok;
    if(ok != (err = mType->GetType(_module.GetTypeManager(), structType, _compilationLog))) {
        return(err);
    }
    if(!structType->mLLVMStructType) {
        _compilationLog.Log(WNLogging::eError, 0, structType->mName, " is not a valid Struct type");
        LogLine(_compilationLog, WNLogging::eError);
        return(eWNBadType);
    }
    llvm::Value* copyValue = wn_nullptr;
    if(mCopyInitializer) {
        if(ok != (err = mCopyInitializer->GenerateCode(_module, _def, _compilationLog))) {
            return(err);
        }
        if(mCopyInitializer->GetType() != structType) {
            const GenerateCastingOperation* op = _module.GetTypeManager().GetCastingOperation(mCopyInitializer->GetType(), structType);
            if(!op) {
                _compilationLog.Log(WNLogging::eError, 0, "Could not copy construct object of type ", structType->mName, " with object of type ", mCopyInitializer->GetType()->mName);
                LogLine(_compilationLog, WNLogging::eError);
                return(eWNInvalidCast);
            }
            if(ok != (err = op->Execute(_module.GetBuilder(), mCopyInitializer->GetValue(), copyValue))) {
                _compilationLog.Log(WNLogging::eCritical, 0, "Error copy constructing ", structType->mName );
                LogLine(_compilationLog, WNLogging::eCritical);
                return(eWNBadType);
            }
        } else {
            copyValue = mCopyInitializer->GetValue();
        }
    }

    llvm::Value* outValue = NULL;
    if(!mCopyInitializer) {
        const GenerateConstruction* construction = _module.GetTypeManager().GetConstructionOperation(structType);
        if(!construction) {
            _compilationLog.Log(WNLogging::eCritical, 0, "No Constructor for ", structType->mName );
            LogLine(_compilationLog, WNLogging::eCritical);
        }
        if(ok != (err = construction->Execute(_module, outValue, structType, wn_nullptr, _def, _compilationLog))) {
            _compilationLog.Log(WNLogging::eError, 0, "Cannot allocate Struct: ", structType->mName);
            LogLine(_compilationLog, WNLogging::eError);
            return(err);
        }
    } else {
        const GenerateCopyConstruction* construction = _module.GetTypeManager().GetCopyConstructionOperation(structType);
        if(!construction) {
            _compilationLog.Log(WNLogging::eCritical, 0, "No Constructor for ", structType->mName );
            LogLine(_compilationLog, WNLogging::eCritical);
        }
        if(ok != (err = construction->Execute(_module, outValue, structType, copyValue, _def, _compilationLog))) {
            _compilationLog.Log(WNLogging::eError, 0, "Cannot allocate Struct: ", structType->mName);
            LogLine(_compilationLog, WNLogging::eError);
            return(err);
        }
    }

    mValue = outValue;
    mScriptType = structType;
    return(ok);
}
