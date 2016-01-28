// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include "WNContainers/inc/WNString.h"
#include "WNScripting/inc/WNCGenerator.h"
#include "WNScripting/inc/WNScriptHelpers.h"

namespace wn {
namespace scripting {
namespace {
void initialize_data(memory::allocator* _allocator,
    containers::pair<containers::string, containers::string>* expr_dat) {
  expr_dat->first = containers::string(_allocator);
  expr_dat->second = containers::string(_allocator);
}
}  // anonymous namespace

void ast_c_translator::walk_type(const type* _type, containers::string* _str) {
  switch (_type->get_index()) {
    case static_cast<uint32_t>(type_classification::invalid_type):
      WN_RELEASE_ASSERT_DESC(wn_false, "Cannot classify invalid types");
      break;
    case static_cast<uint32_t>(type_classification::void_type):
      *_str = std::move(containers::string(m_allocator) + "void");
      return;
    case static_cast<uint32_t>(type_classification::int_type):
      *_str = std::move(containers::string(m_allocator) + "wn_int32");
      return;
    case static_cast<uint32_t>(type_classification::float_type):
      *_str = std::move(containers::string(m_allocator) + "wn_float32");
      return;
    case static_cast<uint32_t>(type_classification::char_type):
      *_str = std::move(containers::string(m_allocator) + "wn_char");
      return;
    case static_cast<uint32_t>(type_classification::string_type):
      WN_RELEASE_ASSERT_DESC(wn_false, "Unimplemented string types");
      break;
    case static_cast<uint32_t>(type_classification::bool_type):
      *_str = std::move(containers::string(m_allocator) + "wn_bool");
      break;
    case static_cast<uint32_t>(type_classification::custom_type):
      *_str = _type->custom_type_name().to_string(m_allocator);
      break;
    case static_cast<uint32_t>(type_classification::max):
      WN_RELEASE_ASSERT_DESC(wn_false, "Type out of range");
      break;
  }
}

void ast_c_translator::walk_expression(const constant_expression* _const,
    containers::pair<containers::string, containers::string>* _str) {
  initialize_data(m_allocator, _str);
  // TODO(awoloszyn): Validate this somewhere.
  switch (_const->get_type()->get_index()) {
    case static_cast<uint32_t>(type_classification::int_type):
      _str->second.append(_const->get_type_text());
      break;
    case static_cast<uint32_t>(type_classification::bool_type):
      _str->second.append(_const->get_type_text());
      break;
    default:
      WN_RELEASE_ASSERT_DESC(
          wn_false, "Non-integer constants not supported yet.");
  }
}

void ast_c_translator::walk_expression(const binary_expression* _binary,
    containers::pair<containers::string, containers::string>* _str) {
  initialize_data(m_allocator, _str);
  const wn_char* m_operators[] = {" + ", " - ", " * ", " / ", " % ", " == ",
      " != ", " <= ", " >= ", " < ", " > "};
  static_assert(sizeof(m_operators) / sizeof(m_operators[0]) ==
                    static_cast<wn_size_t>(arithmetic_type::max),
      "New oeprator type detected");
  const auto& lhs = m_generator->get_data(_binary->get_lhs());
  const auto& rhs = m_generator->get_data(_binary->get_rhs());
  _str->first.append(lhs.first);
  _str->first.append(rhs.first);

  switch (_binary->get_type()->get_index()) {
    case static_cast<uint32_t>(type_classification::int_type):
    case static_cast<uint32_t>(type_classification::bool_type):
      _str->second.append("(");
      _str->second.append(lhs.second);
      _str->second.append(
          m_operators[static_cast<wn_size_t>(_binary->get_arithmetic_type())]);
      _str->second.append(rhs.second);
      _str->second.append(")");
      break;
    default:
      WN_RELEASE_ASSERT_DESC(
          wn_false, "Non-integer binary expressions not supported yet.");
  }
}

void ast_c_translator::walk_expression(const id_expression* _id,
    containers::pair<containers::string, containers::string>* _str) {
  initialize_data(m_allocator, _str);
  _str->second = _id->get_name().to_string(m_allocator);
}

void ast_c_translator::walk_parameter(
    const parameter* _param, containers::string* _str) {
  *_str = containers::string(m_allocator) +
          m_generator->get_data(_param->get_type()) + " " +
          _param->get_name().to_string(m_allocator);
}

void ast_c_translator::walk_instruction(const return_instruction* _ret,
    containers::pair<containers::string, containers::string>* _str) {
  initialize_data(m_allocator, _str);
  if (_ret->get_expression()) {
    const auto& expr = m_generator->get_data(_ret->get_expression());
    _str->second.append(expr.first);
    _str->second.append("return ");
    _str->second.append(expr.second);
    _str->second.append(";");
  } else {
    _str->second.append("return;");
  }
}

void ast_c_translator::walk_instruction(const declaration* _decl,
    containers::pair<containers::string, containers::string>* _str) {
  initialize_data(m_allocator, _str);
  const auto& expr = m_generator->get_data(_decl->get_expression());
  _str->second.append(expr.first);

  _str->second += m_generator->get_data(_decl->get_type()) + " " +
           _decl->get_name().to_string(m_allocator) + " = ";
  _str->second += expr.second;
  _str->second += ";";
}

void ast_c_translator::walk_instruction(const assignment_instruction* _inst,
    containers::pair<containers::string, containers::string>* _str) {
  initialize_data(m_allocator, _str);
  WN_RELEASE_ASSERT_DESC(_inst->get_assignment_type() == assign_type::equal,
      "Not Implemented: Non equality assignment");
  const auto& expr = m_generator->get_data(_inst->get_expression());
  const auto& lvalue =
      m_generator->get_data(_inst->get_lvalue()->get_expression());
  _str->second += expr.first;
  _str->second += lvalue.first;
  _str->second += lvalue.second + " = " + expr.second + ";";
}

void ast_c_translator::walk_instruction(const else_if_instruction* _i,
    containers::pair<containers::string, containers::string>* _str) {
  initialize_data(m_allocator, _str);
  const auto& cond = m_generator->get_data(_i->get_condition());
  _str->first += cond.first;
  _str->second += " else if (";
  _str->second += cond.second;
  _str->second += ") ";
  _str->second += m_generator->get_data(_i->get_body());
}

void ast_c_translator::walk_instruction(const if_instruction* _i,
    containers::pair<containers::string, containers::string>* _str) {
  initialize_data(m_allocator, _str);
  const auto& condition = m_generator->get_data(_i->get_condition());

  _str->first.append(condition.first);
  _str->second += "if (";
  _str->second += condition.second;
  _str->second += ") ";
  _str->second += m_generator->get_data(_i->get_body());
  for (auto& else_inst : _i->get_else_if_instructions()) {
    const auto& else_data = m_generator->get_data(else_inst.get());
    _str->first.append(else_data.first);
    _str->second.append(else_data.second);
  }

  const instruction_list* else_clause = _i->get_else();
  if (else_clause) {
    _str->second += " else ";
    _str->second += m_generator->get_data(else_clause);
  }
}

void ast_c_translator::walk_instruction_list(const instruction_list* l,
    containers::string* _str) {
  *_str = containers::string(m_allocator) + "{\n";
  for (auto& a : l->get_instructions()) {
    const auto& dat = m_generator->get_data(a.get());
    *_str += dat.first;
    *_str += dat.second + "\n";
  }
  *_str += "}";
}

void ast_c_translator::walk_function(
    const function* _func, containers::string* _str) {
  *_str = containers::string(m_allocator) +
          m_generator->get_data(_func->get_signature()->get_type()) + " " +
          _func->get_mangled_name() + "(";
  wn_bool first_param = wn_true;
  if (_func->get_parameters()) {
    for (auto& a : _func->get_parameters()->get_parameters()) {
      if (!first_param) {
        *_str += ",";
      }
      first_param = wn_false;
      *_str += m_generator->get_data(a.get());
    }
  }
  *_str += ") ";
  *_str += m_generator->get_data(_func->get_body());
  *_str += "\n";
}

void ast_c_translator::walk_struct_definition(
    const struct_definition* _definition, containers::string* _str) {
  *_str = containers::string(m_allocator) + "typedef struct {\n";
  for (const auto& a : _definition->get_struct_members()) {
    *_str += "  " + m_generator->get_data(a->get_type()) + " ";
    _str->append(a->get_name().data(), a->get_name().size());
    _str->append(";\n");
  }
  *_str += "} ";
  *_str += _definition->get_name();
  *_str += ";\n\n";
}

void ast_c_translator::walk_script_file(const script_file* _file) {
  wn_bool first = wn_true;
  for (auto& strt : _file->get_structs()) {
    m_output_string += m_generator->get_data(strt.get());
  }
  for (auto& func : _file->get_functions()) {
    if (!first) {
      m_output_string += "\n";
    }
    first = wn_false;
    m_output_string += m_generator->get_data(func.get());
  }
}

}  // namespace scripting
}  // namespace wn
