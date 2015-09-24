// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "WNDeprecatedScripting/inc/WNIncludeLLVM.h"
#include "llvm/IR/IRBuilder.h"
#include "WNDeprecatedScripting/inc/WNEndIncludeLLVM.h"

#include "WNDeprecatedScripting/inc/WNSSExpression.h"
#include "WNDeprecatedScripting/inc/WNCodeModule.h"
#include "WNDeprecatedScripting/inc/WNTypeManager.h"

using namespace WNScripting;

WNSSExpression::WNSSExpression(WNSSType _type, WNExpression* _lhs, WNExpression* _rhs):
    mSSType(_type),
    mLHS(_lhs),
    mRHS(_rhs){
}

WNSSExpression::~WNSSExpression() {
    wn::memory::destroy(mLHS);
    wn::memory::destroy(mRHS);
}

eWNTypeError WNSSExpression::GenerateCode(WNCodeModule& _module, const WNFunctionDefinition* _def, WNLogging::WNLog& _compilationLog) {
    llvm::IRBuilder<>* builder = reinterpret_cast<llvm::IRBuilder<>*>(_module.GetBuilder());
    llvm::BasicBlock* conditionalBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "", _def->mFunction);
    llvm::BasicBlock* endingBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "", _def->mFunction);

    eWNTypeError err = ok;
    if(ok != (err = _module.GetTypeManager().get_type_by_name("Bool", mScriptType))) {
        _compilationLog.Log(WNLogging::eError, 0, "Cannot find Bool type");
        LogLine(_compilationLog, WNLogging::eError);
        return(err);
    }

    if(ok != (err = mLHS->GenerateCode(_module, _def, _compilationLog))) {
        return(err);
    }
    llvm::BasicBlock* currentBlock = builder->GetInsertBlock();
    llvm::Value* lhsValue = mLHS->GetValue();
    if(mLHS->GetType() != mScriptType) {
        const GenerateCastingOperation* castOp = _module.GetTypeManager().GetCastingOperation(mLHS->GetType(), mScriptType);
        if(!castOp) {
            _compilationLog.Log(WNLogging::eError, 0, "Cannot cast from ", mLHS->GetType()->mName, " to ", mScriptType->mName);
            LogLine(_compilationLog, WNLogging::eError);
            return(eWNInvalidCast);
        }
        if(ok != (err = castOp->Execute(builder, mLHS->GetValue(), lhsValue))) {
            _compilationLog.Log(WNLogging::eCritical, 0, "Cannot generate case for ", mLHS->GetType()->mName, " to ", mScriptType->mName);
            LogLine(_compilationLog, WNLogging::eCritical);
            return(err);
        }
    }

    if(ST_AND == mSSType) {
        llvm::Value* cv = llvm::ConstantInt::get(mScriptType->mLLVMType, 0);
        llvm::Value* branchVal = builder->CreateICmpNE(lhsValue, cv, "");
        builder->CreateCondBr(branchVal, conditionalBlock, endingBlock);
    } else if (ST_OR == mSSType) {
        llvm::Value* cv = llvm::ConstantInt::get(mScriptType->mLLVMType, 0);
        llvm::Value* branchVal = builder->CreateICmpEQ(lhsValue, cv, "");
        builder->CreateCondBr(branchVal, conditionalBlock, endingBlock);
    } else {
        _compilationLog.Log(WNLogging::eError, 0, "Unknown Short-Circuit Operator");
        LogLine(_compilationLog, WNLogging::eError);
        return(invalid_parameters);
    }

    builder->SetInsertPoint(conditionalBlock);
    if(ok != (err = mRHS->GenerateCode(_module, _def, _compilationLog))) {
        return(err);
    }

    llvm::Value* rhsValue = mRHS->GetValue();
    if(mRHS->GetType() != mScriptType) {
        const GenerateCastingOperation* castOp = _module.GetTypeManager().GetCastingOperation(mRHS->GetType(), mScriptType);
        if(!castOp) {
            _compilationLog.Log(WNLogging::eError, 0, "Cannot cast from ", mRHS->GetType()->mName, " to ", mScriptType->mName);
            LogLine(_compilationLog, WNLogging::eError);
            return(eWNInvalidCast);
        }
        if(ok != (err = castOp->Execute(builder, mRHS->GetValue(), rhsValue))) {
            _compilationLog.Log(WNLogging::eCritical, 0, "Error generating casting operation cast from ", mRHS->GetType()->mName, " to ", mScriptType->mName);
            LogLine(_compilationLog, WNLogging::eCritical);
            return(err);
        }
    }
    conditionalBlock = builder->GetInsertBlock();
    builder->CreateBr(endingBlock);
    builder->SetInsertPoint(endingBlock);
    llvm::PHINode* phi = builder->CreatePHI(mScriptType->mLLVMType, 2, "");
    phi->addIncoming(lhsValue, currentBlock);
    phi->addIncoming(rhsValue, conditionalBlock);

    mValue = phi;
    return(ok);
}
