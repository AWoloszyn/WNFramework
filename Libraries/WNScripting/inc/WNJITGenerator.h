// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#ifndef __WN_SCRIPTING_JIT_GENERATOR_H__
#define __WN_SCRIPTING_JIT_GENERATOR_H__

// Include LLVM before anything else, they re-define stuff
// that is in the windows headers.
#include <llvm/IR/DataLayout.h>

#include "WNContainers/inc/WNContiguousRange.h"
#include "WNContainers/inc/WNDeque.h"
#include "WNContainers/inc/WNDynamicArray.h"
#include "WNContainers/inc/WNHashMap.h"
#include "WNContainers/inc/WNString.h"
#include "WNMemory/inc/WNAllocator.h"
#include "WNScripting/inc/WNASTCodeGenerator.h"
#include "WNScripting/inc/WNNodeTypes.h"

namespace llvm {
class Instruction;
class Type;
class Value;
}  // namespace llvm

namespace wn {
namespace scripting {

class type_validator;
struct expression_dat {
  containers::dynamic_array<llvm::Instruction*> instructions;
  llvm::Value* value;
};

struct instruction_dat {
  containers::dynamic_array<llvm::Instruction*> instructions;
  containers::dynamic_array<llvm::BasicBlock*> blocks;
};

// Implements the ast_walker interface for creating
// llvm modules from WNScript ASTs.
class ast_jit_engine;
struct ast_jit_traits {
  using instruction_data = instruction_dat;
  using expression_data = expression_dat;
  using parameter_data = llvm::Instruction*;
  using function_data = llvm::Function*;
  using type_data = llvm::Type*;
  using struct_definition_data = llvm::Type*;
  using code_gen = ast_jit_engine;
};

class ast_jit_engine {
public:
  ast_jit_engine(memory::allocator* _allocator, type_validator* _validator,
      llvm::LLVMContext* _context, llvm::Module* _module,
      ast_code_generator<ast_jit_traits>* _generator)
    : m_allocator(_allocator),
      m_validator(_validator),
      m_function_map(m_allocator),
      m_struct_map(m_allocator),
      m_array_type_map(m_allocator),
      m_context(_context),
      m_module(_module),
      m_generator(_generator),
      m_data_layout(_module),
      m_break_instructions(_allocator),
      m_continue_instructions(_allocator) {}

  void walk_expression(const expression*, expression_dat*) {
    WN_RELEASE_ASSERT_DESC(false, "Unimplemented expression type");
  }

  void walk_expression(
      const array_access_expression* _access, expression_dat* _val);
  void walk_expression(
      const array_allocation_expression* _alloc, expression_dat* _val);
  void walk_expression(const cast_expression* _const, expression_dat* _val);
  void walk_expression(const sizeof_expression* _sizeof, expression_dat* _val);
  void walk_expression(const constant_expression* _const, expression_dat* _str);
  void walk_expression(const id_expression* _const, expression_dat* _str);
  void walk_expression(const binary_expression* _binary, expression_dat* _str);
  void walk_expression(
      const function_pointer_expression* _ptr, expression_dat* _str);
  void walk_expression(
      const struct_allocation_expression* _alloc, expression_dat* _str);
  void walk_expression(
      const member_access_expression* _alloc, expression_dat* _str);
  void walk_expression(
      const function_call_expression* _call, expression_dat* _str);

  void walk_type(const type* _type, llvm::Type** _val);
  void walk_type(const array_type* _type, llvm::Type** _val);
  void walk_type(const concretized_array_type* _type, llvm::Type** _val);

  void walk_instruction(const instruction* i, instruction_dat*) {
    WN_RELEASE_ASSERT_DESC(
        false && i, "Unimplemented instruction type implemented");
  }
  void walk_instruction_list(const instruction_list*, instruction_dat*);
  void walk_instruction(const expression_instruction* _inst, instruction_dat*);
  void walk_instruction(const return_instruction* _inst, instruction_dat*);
  void walk_instruction(const declaration* _inst, instruction_dat*);
  void walk_instruction(const assignment_instruction* _inst, instruction_dat*);
  void walk_instruction(const if_instruction*, instruction_dat*);
  void walk_instruction(const do_instruction* _inst, instruction_dat*);
  void walk_instruction(const break_instruction* _inst, instruction_dat*);
  void walk_instruction(const continue_instruction* _inst, instruction_dat*);
  void walk_instruction(const set_array_length* _inst, instruction_dat*);

  void walk_parameter(const parameter* _param, llvm::Instruction**);
  void walk_struct_definition(const struct_definition* _def, llvm::Type**);
  void walk_function(const function* _func, llvm::Function**);
  void pre_walk_function(const function* _func);
  void walk_script_file(const script_file* _file);
  void pre_walk_script_file(const script_file* _file);
  void post_walk_structs(const script_file* _file);

private:
  void pre_walk_struct_definition(const struct_definition* _def);
  void pre_walk_function_definition(const function* _func);

  memory::allocator* m_allocator;
  type_validator* m_validator;
  containers::hash_map<uint32_t, llvm::Type*> m_struct_map;
  containers::hash_map<const function*, llvm::Function*> m_function_map;
  containers::hash_map<uint32_t, llvm::Type*> m_array_type_map;
  ast_code_generator<ast_jit_traits>* m_generator;
  llvm::Module* m_module;
  llvm::LLVMContext* m_context;
  llvm::DataLayout m_data_layout;
  containers::deque<llvm::BranchInst*> m_break_instructions;
  containers::deque<llvm::BranchInst*> m_continue_instructions;
};

}  // namespace scripting
}  // namespace wn

#endif  // __WN_SCRIPTING_JIT_GENERATOR_H__