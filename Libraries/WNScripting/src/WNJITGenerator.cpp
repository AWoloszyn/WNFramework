// Copyright (c) 2016, WNProject Authors. All rights reserved.
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
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/TargetSelect.h>

#ifdef _MSC_VER
#undef PRIi64
#undef PRId64
#undef PRIo64
#undef PRIu64
#undef PRIx64
#undef PRIX64
#pragma warning(pop)
#endif

#include "WNContainers/inc/WNContiguousRange.h"
#include "WNContainers/inc/WNDynamicArray.h"
#include "WNContainers/inc/WNList.h"
#include "WNLogging/inc/WNLog.h"
#include "WNMemory/inc/WNAllocator.h"
#include "WNScripting/inc/WNASTWalker.h"
#include "WNScripting/inc/WNJITGenerator.h"
#include "WNScripting/inc/WNNodeTypes.h"
#include "WNScripting/inc/WNScriptHelpers.h"
#include "WNScripting/inc/WNTypeValidator.h"

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

void ast_jit_engine::walk_type(
    const scripting::type* _type, llvm::Type** _value) {
  switch (_type->get_index()) {
    case static_cast<uint32_t>(type_classification::invalid_type):
      WN_RELEASE_ASSERT_DESC(false, "Cannot classify invalid types");
      break;
    case static_cast<uint32_t>(type_classification::void_type):
      *_value = llvm::Type::getVoidTy(*m_context);
      return;
    case static_cast<uint32_t>(type_classification::int_type):
      *_value = llvm::IntegerType::getInt32Ty(*m_context);
      return;
    case static_cast<uint32_t>(type_classification::float_type):
      *_value = llvm::Type::getFloatTy(*m_context);
      return;
    case static_cast<uint32_t>(type_classification::char_type):
      *_value = llvm::IntegerType::get(*m_context, 8);
      return;
    case static_cast<uint32_t>(type_classification::string_type):
      WN_RELEASE_ASSERT_DESC(false, "Not implemented: string type");
      break;
    case static_cast<uint32_t>(type_classification::bool_type):
      *_value = llvm::IntegerType::get(*m_context, 1);
      return;
      break;
    default: {
      // First we have to normalize the id with the reference type.
      uint32_t index = _type->get_index();
      uint32_t normalized_index =
          index - static_cast<uint32_t>(_type->get_reference_type());
      auto elem = m_struct_map.find(normalized_index);
      WN_RELEASE_ASSERT_DESC(
          elem != m_struct_map.end(), "Cannot determine type");

      switch (_type->get_reference_type()) {
        case reference_type::raw:
          *_value = elem->second;
          break;
        case reference_type::unique:
        case reference_type::self:
          *_value = elem->second->getPointerTo();
          break;
        default:
          WN_RELEASE_ASSERT_DESC(false, "Not Implemented");
      }
    }
  }
}

void ast_jit_engine::walk_expression(
    const cast_expression* _cast, expression_dat* _val) {
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);

  const expression_dat& sub_dat =
      m_generator->get_data(_cast->get_expression());
  _val->instructions.insert(_val->instructions.begin(),
      sub_dat.instructions.begin(), sub_dat.instructions.end());

  if (!_cast->get_type()->get_type_value() -
          static_cast<uint32_t>(_cast->get_type()->get_reference_type()) ==
      _cast->get_expression()->get_type()->get_type_value() -
          static_cast<uint32_t>(
              _cast->get_expression()->get_type()->get_reference_type())) {
    WN_RELEASE_ASSERT_DESC(false, "Not implemented: Other types of cast");
    return;
  }

  // If we are converting from raw to reference, then we can do away
  // with the load-inst that caused us.
  if ((_cast->get_type()->get_reference_type() == reference_type::self ||
          _cast->get_type()->get_reference_type() == reference_type::unique) &&
      _cast->get_expression()->get_type()->get_reference_type() ==
          reference_type::raw) {
    // TODO(awoloszyn): Delete the load instruction?
    llvm::LoadInst* val = llvm::dyn_cast<llvm::LoadInst>(sub_dat.value);
    _val->value = val->getPointerOperand();
    return;
  }

  _val->value = sub_dat.value;
}

void ast_jit_engine::walk_expression(
    const constant_expression* _const, expression_dat* _val) {
  switch (_const->get_type()->get_index()) {
    case static_cast<uint32_t>(type_classification::int_type): {
      long long val = atoll(_const->get_type_text().c_str());
      _val->value = llvm::ConstantInt::getSigned(
          m_generator->get_data(_const->get_type()), val);
      return;
    }
    case static_cast<uint32_t>(type_classification::bool_type): {
      bool bVal = containers::string_view(_const->get_type_text()) == "true";
      _val->value = llvm::ConstantInt::get(
          m_generator->get_data(_const->get_type()), bVal ? 1 : 0);
      return;
    }
    case static_cast<uint32_t>(type_classification::float_type): {
      double val = atof(_const->get_type_text().c_str());
      _val->value =
          llvm::ConstantFP::get(m_generator->get_data(_const->get_type()), val);
      return;
    }
    default:
      WN_RELEASE_ASSERT_DESC(false, "Not Implemented");
  }
}

void ast_jit_engine::walk_expression(
    const id_expression* _id_expr, expression_dat* _val) {
  llvm::Value* v =
      _id_expr->get_id_source().param_source
          ? m_generator->get_data(_id_expr->get_id_source().param_source)
          : *m_generator->get_data(_id_expr->get_id_source().declaration_source)
                 .instructions.begin();

  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);
  _val->instructions.push_back(
      new llvm::LoadInst(v, make_string_ref(_id_expr->get_name())));
  _val->value = _val->instructions.back();
}

namespace {
static const llvm::BinaryOperator::BinaryOps integer_ops[] = {
    llvm::BinaryOperator::BinaryOps::Add, llvm::BinaryOperator::BinaryOps::Sub,
    llvm::BinaryOperator::BinaryOps::Mul, llvm::BinaryOperator::BinaryOps::SDiv,
    llvm::BinaryOperator::BinaryOps::SRem};

static const int16_t integer_compares[] = {llvm::CmpInst::Predicate::ICMP_EQ,
    llvm::CmpInst::Predicate::ICMP_NE, llvm::CmpInst::Predicate::ICMP_SLE,
    llvm::CmpInst::Predicate::ICMP_SGE, llvm::CmpInst::Predicate::ICMP_SLT,
    llvm::CmpInst::Predicate::ICMP_SGT};

static_assert(sizeof(integer_ops) / sizeof(integer_ops[0]) +
                      sizeof(integer_compares) / sizeof(integer_compares[0]) ==
                  static_cast<size_t>(arithmetic_type::max),
    "Arithmetic type was added");
}  // anonymous namespace

void ast_jit_engine::walk_expression(
    const binary_expression* _binary, expression_dat* _val) {
  const expression_dat& lhs = m_generator->get_data(_binary->get_lhs());
  const expression_dat& rhs = m_generator->get_data(_binary->get_rhs());
  llvm::Instruction* inst = nullptr;
  arithmetic_type type = _binary->get_arithmetic_type();
  switch (_binary->get_type()->get_index()) {
    case static_cast<uint32_t>(type_classification::int_type):
    case static_cast<uint32_t>(
        type_classification::bool_type):  // Booleans are just 1-bit integers.
      if (type <= arithmetic_type::arithmetic_mod) {
        inst = llvm::BinaryOperator::Create(
            integer_ops[static_cast<size_t>(type)], lhs.value, rhs.value);
      } else {
        inst = llvm::CmpInst::Create(llvm::Instruction::ICmp,
            integer_compares[static_cast<short>(type) -
                                         static_cast<short>(
                                             arithmetic_type::arithmetic_mod) -
                                         1],
            lhs.value, rhs.value);
      }
      break;
    default:
      WN_RELEASE_ASSERT_DESC(false, "Not implemnted, non-integer arithmetic");
  }
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);
  _val->instructions.insert(_val->instructions.end(), lhs.instructions.begin(),
      lhs.instructions.end());
  _val->instructions.insert(_val->instructions.end(), rhs.instructions.begin(),
      rhs.instructions.end());
  _val->instructions.push_back(inst);
  _val->value = inst;
}

void ast_jit_engine::walk_expression(
    const struct_allocation_expression* _alloc, expression_dat* _val) {
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);

  switch (_alloc->get_type()->get_reference_type()) {
    case reference_type::raw:
      // We actually have nothing to do in this case. The
      // declaration will have turned this into a Alloca(type),
      // and we have nothing interesting to store in it.
      _val->value = nullptr;
      break;
    default:
      WN_RELEASE_ASSERT_DESC(false, "Not Implemented");
  }
}

void ast_jit_engine::walk_expression(
    const member_access_expression* _alloc, expression_dat* _val) {
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);

  const expression_dat& dat =
      m_generator->get_data(_alloc->get_base_expression());

  WN_RELEASE_ASSERT_DESC(dat.instructions.back() == dat.value,
      "The last value is not the one returned, something is wrong");
  _val->instructions.insert(_val->instructions.end(), dat.instructions.begin(),
      dat.instructions.end());

  uint32_t member_offset =
      m_validator->get_operations(
                     _alloc->get_base_expression()->get_type()->get_index())
          .get_member_index(_alloc->get_name());
  llvm::Type* int32_type = llvm::IntegerType::getInt32Ty(*m_context);

  llvm::Instruction* v = llvm::GetElementPtrInst::CreateInBounds(
      dat.value, llvm::makeArrayRef<llvm::Value*, 2>(
                     {llvm::ConstantInt::get(int32_type, 0),
                         llvm::ConstantInt::get(int32_type, member_offset)}));
  llvm::Instruction* l =
      new llvm::LoadInst(v, make_string_ref(_alloc->get_name()));
  _val->instructions.push_back(v);
  _val->instructions.push_back(l);
  _val->value = l;
}

void ast_jit_engine::walk_expression(
    const function_call_expression* _call, expression_dat* _val) {
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);

  containers::dynamic_array<llvm::Value*> parameters(m_allocator);
  for (const auto& expr : _call->get_expressions()) {
    const auto& dat = m_generator->get_data(expr->m_expr.get());
    _val->instructions.insert(_val->instructions.end(),
        dat.instructions.begin(), dat.instructions.end());
    parameters.push_back(dat.value);
  }

  llvm::CallInst* call;
  // We are not allowed naming void values.
  if (_call->get_type()->get_index() ==
      static_cast<uint32_t>(type_classification::void_type)) {
    call = llvm::CallInst::Create(m_function_map[_call->callee()],
        make_array_ref(parameters));
  } else {
    call = llvm::CallInst::Create(m_function_map[_call->callee()],
        make_array_ref(parameters),
        make_string_ref(_call->callee()->get_mangled_name()));
  }
  _val->instructions.push_back(call);
  _val->value = call;
}

void ast_jit_engine::walk_instruction(
    const if_instruction* _inst, instruction_dat* _val) {
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);
  _val->blocks = containers::dynamic_array<llvm::BasicBlock*>(m_allocator);

  expression_dat& dat = m_generator->get_data(_inst->get_condition());
  containers::dynamic_array<llvm::BasicBlock*>& body =
      m_generator->get_data(_inst->get_body()).blocks;

  llvm::BasicBlock* post_if =
      _inst->returns() ? nullptr
                       : llvm::BasicBlock::Create(*m_context, "end_if");
  llvm::BasicBlock* next_if = llvm::BasicBlock::Create(*m_context, "next_if");

  _val->blocks.insert(_val->blocks.begin(), body.begin(), body.end());

  _val->instructions.insert(_val->instructions.begin(),
      dat.instructions.begin(), dat.instructions.end());
  _val->instructions.push_back(
      llvm::BranchInst::Create(_val->blocks.front(), next_if, dat.value));

  if (!_inst->get_body()->returns()) {
    _val->blocks.back()->getInstList().push_back(
        llvm::BranchInst::Create(post_if));
  }
  _val->blocks.push_back(next_if);

  if (_inst->get_else()) {
    containers::dynamic_array<llvm::BasicBlock*>& list_dat =
        m_generator->get_data(_inst->get_else()).blocks;
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

void ast_jit_engine::walk_instruction(
    const expression_instruction* _expr, instruction_dat* _val) {
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);
  if (_expr->get_expression()) {
    const expression_dat& dat = m_generator->get_data(_expr->get_expression());
    _val->instructions.insert(_val->instructions.end(), dat.instructions.data(),
        dat.instructions.data() + dat.instructions.size());
  }
}

void ast_jit_engine::walk_instruction(
    const return_instruction* _inst, instruction_dat* _val) {
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);
  if (_inst->get_expression()) {
    const expression_dat& dat = m_generator->get_data(_inst->get_expression());
    _val->instructions.insert(_val->instructions.end(), dat.instructions.data(),
        dat.instructions.data() + dat.instructions.size());
    _val->instructions.push_back(
        llvm::ReturnInst::Create(*m_context, dat.value));
  } else {
    _val->instructions.push_back(llvm::ReturnInst::Create(*m_context));
  }
}

void ast_jit_engine::walk_instruction(
    const declaration* _inst, instruction_dat* _val) {
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);

  _val->instructions.push_back(
      new llvm::AllocaInst(m_generator->get_data(_inst->get_type()),
          make_string_ref(_inst->get_name())));

  expression_dat& expr = m_generator->get_data(_inst->get_expression());
  _val->instructions.insert(_val->instructions.end(), expr.instructions.data(),
      expr.instructions.data() + expr.instructions.size());

  // There are cases where there is no RHS to a declaration.
  // Specifically Foo x = Foo(); effectively becomes
  // Foo x;
  // construct(&x);
  // so we just don't initialize if RHS is empty.
  // TODO(awoloszyn): If this seems too ugly, make the RHS
  // an undef value.
  if (expr.value) {
  _val->instructions.push_back(
      new llvm::StoreInst(expr.value, _val->instructions.front()));
  }
}

void ast_jit_engine::walk_instruction(
    const assignment_instruction* _inst, instruction_dat* _val) {
  WN_RELEASE_ASSERT_DESC(_inst->get_assignment_type() == assign_type::equal,
      "Not Supported: non-equality assignments");
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);

  expression_dat& store_dat = m_generator->get_data(_inst->get_expression());
  _val->instructions.insert(_val->instructions.end(),
      store_dat.instructions.data(),
      store_dat.instructions.data() + store_dat.instructions.size());

  expression_dat& location_dat =
      m_generator->get_data(_inst->get_lvalue()->get_expression());
  llvm::LoadInst* inst =
      llvm::dyn_cast<llvm::LoadInst>(location_dat.instructions.back());
  WN_RELEASE_ASSERT_DESC(
      inst, "Last instruction in lvalue is expected to be a LoadInst");

  location_dat.instructions.pop_back();
  llvm::Value* target = inst->getPointerOperand();
  // We need to store instead of load to this instruction.
  delete inst;

  location_dat.instructions.push_back(
      new llvm::StoreInst(store_dat.value, target));
  _val->instructions.insert(_val->instructions.end(),
      location_dat.instructions.data(),
      location_dat.instructions.data() + location_dat.instructions.size());
}

void ast_jit_engine::walk_instruction_list(
    const instruction_list* _instructions, instruction_dat* _val) {
  _val->instructions =
      containers::dynamic_array<llvm::Instruction*>(m_allocator);
  _val->blocks = containers::dynamic_array<llvm::BasicBlock*>(m_allocator);
  llvm::BasicBlock* bb = llvm::BasicBlock::Create(*m_context, "block");
  _val->blocks.push_back(bb);

  for (auto& inst : _instructions->get_instructions()) {
    instruction_dat& instructions = m_generator->get_data(inst.get());
    for (llvm::Instruction* i : instructions.instructions) {
      bb->getInstList().push_back(i);
    }

    if (instructions.instructions.empty() && !instructions.blocks.empty()) {
      // This was a naked basic block. There is nothing to
      // let us enter. These are created by naked scope blocks.
      // Our 2 options are to take all of the instructions FROM them,
      // or just branch into them, we choose the latter.
      bb->getInstList().push_back(
          llvm::BranchInst::Create(*instructions.blocks.begin()));
    }

    for (llvm::BasicBlock* b : instructions.blocks) {
      _val->blocks.push_back(b);
      bb = b;
    }
  }
}

void ast_jit_engine::walk_parameter(
    const parameter* _param, llvm::Instruction** _val) {
  *_val = new llvm::AllocaInst(m_generator->get_data(_param->get_type()),
      make_string_ref(_param->get_name()));
}

void ast_jit_engine::walk_struct_definition(
    const struct_definition*, llvm::Type**) {
}

void ast_jit_engine::pre_walk_struct_definition(
    const struct_definition* _def) {
  containers::dynamic_array<llvm::Type*> types(m_allocator);

  for (auto& a : _def->get_struct_members()) {
    llvm::Type* t;
    walk_type(a->get_type(), &t);
    types.push_back(t);
  }

  llvm::StructType* t =
      llvm::StructType::get(*m_context, make_array_ref(types), false);
  m_struct_map[m_validator->get_type(_def->get_name())] = t;
}

void ast_jit_engine::pre_walk_function_definition(const function* _func){
  llvm::Type* t;
  walk_type(_func->get_signature()->get_type(), &t);
  containers::dynamic_array<llvm::Type*> parameters(m_allocator);

  if (_func->get_parameters()) {
    for (auto& a : _func->get_parameters()->get_parameters()) {
      llvm::Type* param_type;
      walk_type(a->get_type(), &param_type);
      parameters.push_back(param_type);
    }
  }

  llvm::FunctionType* func_t =
      llvm::FunctionType::get(t, make_array_ref(parameters), false);
  llvm::Function* f = llvm::Function::Create(func_t,
      llvm::GlobalValue::LinkageTypes::ExternalLinkage,
      make_string_ref(_func->get_mangled_name()));
  m_module->getFunctionList().push_back(f);
  m_function_map[_func] = f;
}

void ast_jit_engine::walk_function(const function* _func, llvm::Function** _f) {

  llvm::BasicBlock* bb = llvm::BasicBlock::Create(*m_context);
  *_f = m_module->getFunction(make_string_ref(_func->get_mangled_name()));
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

  for (auto& block : m_generator->get_data(_func->get_body()).blocks) {
    if (first) {
      bb->getInstList().push_back(llvm::BranchInst::Create(block));
      first = false;
    }
    (*_f)->getBasicBlockList().push_back(block);
  }
}

void ast_jit_engine::pre_walk_script_file(const script_file* _file) {
  for(auto& strt: _file->get_structs()) {
    pre_walk_struct_definition(strt.get());
  }
  for(auto& func: _file->get_functions()) {
    pre_walk_function_definition(func.get());
  }
}

void ast_jit_engine::walk_script_file(const script_file* _file) {
  for (auto& function : _file->get_functions()) {
    llvm::Function* func = m_generator->get_data(function.get());
    llvm::verifyFunction(*func);
  }
  llvm::verifyModule(*m_module);
}

}  // namespace scripting
}  // namespace wn
