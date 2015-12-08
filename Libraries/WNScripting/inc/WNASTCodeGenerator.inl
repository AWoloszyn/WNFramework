// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#pragma once

#ifndef __WN_SCRIPTING_AST_CODE_GENERATOR_INL__
#define __WN_SCRIPTING_AST_CODE_GENERATOR_INL__

#include "WNCore/inc/WNTypes.h"
#include "WNContainers/inc/WNContiguousRange.h"
#include "WNMemory/inc/WNAllocator.h"
#include "WNScripting/inc/WNNodeTypes.h"
#include "WNScripting/inc/WNTypeValidator.h"

namespace wn {
namespace scripting {

template <typename Traits>
template <typename T>
void ast_code_generator<Traits>::walk_expression(T* _expr) {
  m_generator->walk_expression(_expr, &m_expression_map[_expr]);
}

template <typename Traits>
template <typename T>
void ast_code_generator<Traits>::walk_instruction(T* _inst) {
  m_generator->walk_instruction(_inst, &m_instruction_map[_inst]);
}

template <typename Traits>
void ast_code_generator<Traits>::walk_parameter(parameter* _param) {
  m_generator->walk_parameter(_param, &m_parameter_map[_param]);
}

template <typename Traits>
void ast_code_generator<Traits>::walk_function(function* _func) {
  m_generator->walk_function(_func, &m_function_map[_func]);
}

template <typename Traits>
void ast_code_generator<Traits>::walk_type(type* _type) {
  m_generator->walk_type(_type, &m_type_map[_type]);
}

template <typename Traits>
void ast_code_generator<Traits>::walk_script_file(script_file* _file) {
  m_generator->walk_script_file(_file);
}

template <typename Traits>
typename Traits::instruction_data& ast_code_generator<Traits>::get_data(
    const instruction* _inst) {
  return m_instruction_map[_inst];
}

template <typename Traits>
typename Traits::expression_data& ast_code_generator<Traits>::get_data(
    const expression* _expr) {
  return m_expression_map[_expr];
}

template <typename Traits>
typename Traits::parameter_data& ast_code_generator<Traits>::get_data(
    const parameter* _param) {
  return m_parameter_map[_param];
}

template <typename Traits>
typename Traits::function_data& ast_code_generator<Traits>::get_data(
    const function* _func) {
  return m_function_map[_func];
}

template <typename Traits>
typename Traits::type_data& ast_code_generator<Traits>::get_data(
    const type* _type) {
  return m_type_map[_type];
}

}  // namespace scripting
}  // namespace wn

#endif  //__WN_SCRIPTING_AST_CODE_GENERATOR_INL__