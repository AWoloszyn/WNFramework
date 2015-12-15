// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

// Due to a bug in llvm, it must be included before windows.h

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4127)
#pragma warning(disable : 4310)
#pragma warning(disable : 4244)
#pragma warning(disable : 4458)
#pragma warning(disable : 4800)
#pragma warning(disable : 4624)
#pragma warning(disable : 4267)
#pragma warning(disable : 4512)
#pragma warning(disable : 4310)
#endif

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>

#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "WNContainers/inc/WNContiguousRange.h"
#include "WNContainers/inc/WNDynamicArray.h"
#include "WNContainers/inc/WNList.h"
#include "WNMemory/inc/WNAllocator.h"
#include "WNScripting/inc/WNJITGenerator.h"
#include "WNScripting/inc/WNASTWalker.h"
#include "WNScripting/inc/WNNodeTypes.h"
#include "WNScripting/inc/WNScriptHelpers.h"
#include "WNLogging/inc/WNLog.h"

#include <algorithm>

namespace {

template <typename T>
typename llvm::ArrayRef<T> make_array_ref(
    const wn::containers::dynamic_array<T>& _array) {
  return llvm::ArrayRef<T>(_array.data(), _array.size());
}

llvm::StringRef make_string_ref(const wn::containers::string_view& _view) {
  return llvm::StringRef(_view.data(), _view.length());
}

llvm::StringRef make_string_ref(const wn::containers::string& _view) {
  return llvm::StringRef(_view.data(), _view.length());
}
}

namespace wn {
namespace scripting {

void ast_jit_engine::walk_type(const wn::scripting::type* _type,
                               llvm::Type** _value) {
  switch (_type->get_classification()) {
    case wn::scripting::type_classification::invalid_type:
      WN_RELEASE_ASSERT_DESC(wn_false, "Cannot classify invalid types");
      break;
    case wn::scripting::type_classification::void_type:
      *_value = llvm::Type::getVoidTy(*m_context);
      return;
    case wn::scripting::type_classification::int_type:
      *_value = llvm::IntegerType::getInt32Ty(*m_context);
      return;
    case wn::scripting::type_classification::float_type:
      *_value = llvm::Type::getFloatTy(*m_context);
      return;
    case wn::scripting::type_classification::char_type:
      *_value = llvm::IntegerType::get(*m_context, 8);
      return;
    case wn::scripting::type_classification::string_type:
      WN_RELEASE_ASSERT_DESC(wn_false, "Not implemented: string type");
      break;
    case wn::scripting::type_classification::bool_type:
      *_value = llvm::IntegerType::get(*m_context, 1);
      return;
      break;
    case wn::scripting::type_classification::custom_type:
      WN_RELEASE_ASSERT_DESC(wn_false, "Not implemented: custom types");
      break;
    case wn::scripting::type_classification::max:
      WN_RELEASE_ASSERT_DESC(wn_false, "Type out of range");
      break;
  }
}

void ast_jit_engine::walk_expression(const constant_expression* _const,
                                     expression_dat* _val) {
  switch (_const->get_type()->get_classification()) {
    case type_classification::int_type: {
      long long val = atoll(_const->get_type_text().c_str());
      _val->value = llvm::ConstantInt::getSigned(
          m_generator->get_data(_const->get_type()), val);
      return;
    }
    case type_classification::bool_type: {
      bool bVal = containers::string_view(_const->get_type_text()) == "true";
      _val->value = llvm::ConstantInt::get(
          m_generator->get_data(_const->get_type()), bVal ? 1 : 0);
      return;
    }
    default:
      WN_RELEASE_ASSERT_DESC(wn_false, "Not Implemented");
  }
}

void ast_jit_engine::walk_expression(const id_expression* _id_expr,
                                     expression_dat* _val) {
  if (_id_expr->get_id_source().param_source) {
    _val->instructions =
        wn::containers::dynamic_array<llvm::Instruction*>(m_allocator);
    _val->instructions.push_back(new llvm::LoadInst(
        m_generator->get_data(_id_expr->get_id_source().param_source),
        make_string_ref(_id_expr->get_id_source().param_source->get_name())));
    _val->value = _val->instructions.back();
  }
}

namespace {
static const llvm::BinaryOperator::BinaryOps integer_ops[] = {
    llvm::BinaryOperator::BinaryOps::Add, llvm::BinaryOperator::BinaryOps::Sub,
    llvm::BinaryOperator::BinaryOps::Mul, llvm::BinaryOperator::BinaryOps::SDiv,
    llvm::BinaryOperator::BinaryOps::SRem};

static const wn_int16 integer_compares[] = {
    llvm::CmpInst::Predicate::ICMP_EQ,  llvm::CmpInst::Predicate::ICMP_NE,
    llvm::CmpInst::Predicate::ICMP_SLE, llvm::CmpInst::Predicate::ICMP_SGE,
    llvm::CmpInst::Predicate::ICMP_SLT, llvm::CmpInst::Predicate::ICMP_SGT};

static_assert(sizeof(integer_ops) / sizeof(integer_ops[0]) +
                      sizeof(integer_compares) / sizeof(integer_compares[0]) ==
                  static_cast<size_t>(arithmetic_type::max),
              "Arithmetic type was added");
}  // anonymous namespace

void ast_jit_engine::walk_expression(const binary_expression* _binary,
                                     expression_dat* _val) {
  const expression_dat& lhs = m_generator->get_data(_binary->get_lhs());
  const expression_dat& rhs = m_generator->get_data(_binary->get_rhs());
  llvm::Instruction* inst = wn_nullptr;
  arithmetic_type type = _binary->get_arithmetic_type();
  switch (_binary->get_type()->get_classification()) {
    case type_classification::int_type:
    case type_classification::bool_type:  // Booleans are just 1-bit integers.
      if (type <= arithmetic_type::arithmetic_mod) {
        inst = llvm::BinaryOperator::Create(
            integer_ops[static_cast<size_t>(type)], lhs.value, rhs.value);
      } else {
        inst = llvm::CmpInst::Create(
            llvm::Instruction::ICmp,
            integer_compares
                [static_cast<short>(type) -
                 static_cast<short>(arithmetic_type::arithmetic_mod) - 1],
            lhs.value, rhs.value);
      }
      break;
    default:
      WN_RELEASE_ASSERT_DESC(wn_false,
                             "Not implemnted, non-integer arithmetic");
  }
  _val->instructions =
      wn::containers::dynamic_array<llvm::Instruction*>(m_allocator);
  _val->instructions.insert(_val->instructions.end(), lhs.instructions.begin(),
                            lhs.instructions.end());
  _val->instructions.insert(_val->instructions.end(), rhs.instructions.begin(),
                            rhs.instructions.end());
  _val->instructions.push_back(inst);
  _val->value = inst;
}

void ast_jit_engine::walk_instruction(const else_if_instruction* _inst,
                                      instruction_dat* _val) {
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);
  _val->blocks = containers::dynamic_array<llvm::BasicBlock*>(m_allocator);

  expression_dat& dat = m_generator->get_data(_inst->get_condition());
  containers::dynamic_array<llvm::BasicBlock*>& body =
      m_generator->get_data(_inst->get_body());

  llvm::BasicBlock* no_execute =
      llvm::BasicBlock::Create(*m_context, "no_execute");

  _val->instructions.insert(_val->instructions.begin(),
                            dat.instructions.begin(), dat.instructions.end());
  _val->blocks.insert(_val->blocks.begin(), body.begin(), body.end());

  _val->instructions.push_back(
      llvm::BranchInst::Create(*_val->blocks.begin(), no_execute, dat.value));
  _val->blocks.insert(_val->blocks.end(), no_execute);
}

void ast_jit_engine::walk_instruction(const if_instruction* _inst,
                                      instruction_dat* _val) {
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);
  _val->blocks = containers::dynamic_array<llvm::BasicBlock*>(m_allocator);

  expression_dat& dat = m_generator->get_data(_inst->get_condition());
  containers::dynamic_array<llvm::BasicBlock*>& body =
      m_generator->get_data(_inst->get_body());

  llvm::BasicBlock* post_if =
      _inst->returns() ? wn_nullptr
                       : llvm::BasicBlock::Create(*m_context, "end_if");
  llvm::BasicBlock* next_if = llvm::BasicBlock::Create(*m_context, "next_if");

  _val->blocks.insert(_val->blocks.begin(), body.begin(), body.end());

  _val->instructions.insert(_val->instructions.begin(),
                            dat.instructions.begin(), dat.instructions.end());
  _val->instructions.push_back(llvm::BranchInst::Create(_val->blocks.front(), next_if, dat.value));

  if (!_inst->get_body()->returns()) {
    _val->blocks.back()->getInstList().push_back(
        llvm::BranchInst::Create(post_if));
  }
  _val->blocks.push_back(next_if);

  for(auto& else_if: _inst->get_else_if_instructions()) {
    instruction_dat& inst_dat = m_generator->get_data(else_if.get());
    // The very last block in the else_if is the block that gets branched
    // to if the else_if did not get run.
    // The second to last block, is the last block inside of the if statement.
    // If this doesn't return it should branch to the post_if block.
    if (!else_if->returns()) {
      (*(inst_dat.blocks.rbegin() + 1))
          ->getInstList()
          .push_back(llvm::BranchInst::Create(post_if));
    }
    next_if->getInstList().insert(next_if->getInstList().end(),
                                  inst_dat.instructions.begin(),
                                  inst_dat.instructions.end());
    _val->blocks.insert(_val->blocks.end(), inst_dat.blocks.begin(),
                       inst_dat.blocks.end());
    next_if = _val->blocks.back();
  }
  if (_inst->get_else()) {
    containers::dynamic_array<llvm::BasicBlock*>& list_dat =
        m_generator->get_data(_inst->get_else());
    next_if->getInstList().push_back(
        llvm::BranchInst::Create(list_dat.front()));
    if (!_inst->get_else()->returns()) {
      list_dat.back()->getInstList().push_back(
          llvm::BranchInst::Create(post_if));
    }
    _val->blocks.insert(_val->blocks.end(), list_dat.begin(), list_dat.end());
  } else {
    next_if->getInstList().push_back(llvm::BranchInst::Create(post_if));
  }

  if (post_if) {
    _val->blocks.insert(_val->blocks.end(), post_if);
  }
}

void ast_jit_engine::walk_instruction(const return_instruction* _inst,
                                      instruction_dat* _val) {
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);
  if (_inst->get_expression()) {
    const expression_dat& dat = m_generator->get_data(_inst->get_expression());
    _val->instructions.insert(
        _val->instructions.end(), dat.instructions.data(),
        dat.instructions.data() + dat.instructions.size());
    _val->instructions.push_back(
        llvm::ReturnInst::Create(*m_context, dat.value));
  } else {
    _val->instructions.push_back(llvm::ReturnInst::Create(*m_context));
  }
}

void ast_jit_engine::walk_instruction_list(
    const instruction_list* _instructions,
    containers::dynamic_array<llvm::BasicBlock*>* _list) {
  *_list = containers::dynamic_array<llvm::BasicBlock*>(m_allocator);
  llvm::BasicBlock* bb = llvm::BasicBlock::Create(*m_context, "block");
  _list->push_back(bb);
  for (auto& inst : _instructions->get_instructions()) {
    instruction_dat& instructions = m_generator->get_data(inst.get());
    for (llvm::Instruction* i : instructions.instructions) {
      bb->getInstList().push_back(i);
    }
    for (llvm::BasicBlock* b : instructions.blocks) {
      _list->push_back(b);
      bb = b;
    }
  }
}

void ast_jit_engine::walk_parameter(const parameter* _param,
                                    llvm::Instruction** _val) {
  *_val = new llvm::AllocaInst(m_generator->get_data(_param->get_type()),
                               make_string_ref(_param->get_name()));
}

void ast_jit_engine::walk_function(const function* _func, llvm::Function** _f) {
  wn::containers::dynamic_array<llvm::Type*> parameters;

  llvm::BasicBlock* bb = llvm::BasicBlock::Create(*m_context);
  if (_func->get_parameters()) {
    for (auto& a : _func->get_parameters()->get_parameters()) {
      parameters.push_back(m_generator->get_data(a->get_type()));
    }
  }

  llvm::FunctionType* t = llvm::FunctionType::get(
      m_generator->get_data(_func->get_signature()->get_type()),
      make_array_ref(parameters), wn_false);
  *_f = llvm::Function::Create(
      t, llvm::GlobalValue::LinkageTypes::ExternalLinkage,
      make_string_ref(_func->get_signature()->get_name()));
  if (_func->get_parameters()) {
    auto llvm_args = (*_f)->arg_begin();
    auto arg_names = _func->get_parameters()->get_parameters().begin();
    while (llvm_args != (*_f)->arg_end()) {
      llvm_args->setName(make_string_ref((*arg_names)->get_name()));
      llvm::Instruction* loc = m_generator->get_data((*arg_names).get());
      bb->getInstList().push_back(loc);
      bb->getInstList().push_back(new llvm::StoreInst(&(*llvm_args), loc));
      ++llvm_args;
      ++arg_names;
    }
  }

  (*_f)->getBasicBlockList().push_front(bb);
  bool first = true;
  for (auto& block : m_generator->get_data(_func->get_body())) {
    if (first) {
      bb->getInstList().push_back(llvm::BranchInst::Create(block));
      first = false;
    }
    (*_f)->getBasicBlockList().push_back(block);
  }
}

void ast_jit_engine::walk_script_file(const script_file* _file) {
  for (auto& function : _file->get_functions()) {
    llvm::Function* func = m_generator->get_data(function.get());
    m_module->getFunctionList().push_back(func);
    llvm::verifyFunction(*func);
  }
  llvm::verifyModule(*m_module);
}

}  // namespace scripting
}  // namespace wn
