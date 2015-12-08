// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#ifndef __WN_SCRIPTING_JIT_GENERATOR_H__
#define __WN_SCRIPTING_JIT_GENERATOR_H__

#include "WNContainers/inc/WNContiguousRange.h"
#include "WNContainers/inc/WNDeque.h"
#include "WNContainers/inc/WNDynamicArray.h"
#include "WNContainers/inc/WNList.h"
#include "WNContainers/inc/WNString.h"
#include "WNMemory/inc/WNAllocator.h"

#include "WNScripting/inc/WNASTCodeGenerator.h"
#include "WNScripting/inc/WNNodeTypes.h"
namespace llvm {
class Instruction;
class Type;
class Value;
}

namespace wn {
namespace scripting {

struct expression_dat {
  containers::dynamic_array<llvm::Instruction*> instructions;
  llvm::Value* value;
};

class ast_jit_engine;
struct ast_jit_traits {
  using instruction_data = containers::dynamic_array<llvm::Instruction*>;
  using expression_data = expression_dat;
  using parameter_data = llvm::Instruction*;
  using function_data = llvm::Function*;
  using type_data = llvm::Type*;
  using code_gen = ast_jit_engine;
};

class ast_jit_engine {
public:
  ast_jit_engine(memory::allocator* _allocator, llvm::LLVMContext* _context,
                 llvm::Module* _module,
                 ast_code_generator<ast_jit_traits>* _generator)
      : m_allocator(_allocator),
        m_context(_context),
        m_module(_module),
        m_generator(_generator) {}

  void walk_expression(const expression*,
                       expression_dat*) {}
  void walk_expression(const constant_expression* _const,
                       expression_dat* _str);
  void walk_expression(const id_expression* _const,
                       expression_dat* _str);

  void walk_type(const type* _type, llvm::Type** _val);
  void walk_instruction(const instruction*,
                        containers::dynamic_array<llvm::Instruction*>*) {}
  void walk_instruction(const return_instruction* _inst,
                        containers::dynamic_array<llvm::Instruction*>*);
  void walk_parameter(const parameter* _param, llvm::Instruction**);
  void walk_function(const function* _func, llvm::Function**);
  void walk_script_file(const script_file* _file);

 private:
  memory::allocator* m_allocator;
  ast_code_generator<ast_jit_traits>* m_generator;
  llvm::Module* m_module;
  llvm::LLVMContext* m_context;
};

}  // namespace scripting
}  // namespace wn

#endif  // __WN_SCRIPTING_JIT_GENERATOR_H__