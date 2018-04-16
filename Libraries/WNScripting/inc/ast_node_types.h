// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#ifndef __WN_SCRIPTING_AST_NODE_TYPES_H__
#define __WN_SCRIPTING_AST_NODE_TYPES_H__

#include "WNContainers/inc/WNDeque.h"
#include "WNContainers/inc/WNString.h"
#include "WNContainers/inc/WNStringView.h"
#include "WNFunctional/inc/WNFunction.h"
#include "WNLogging/inc/WNLog.h"
#include "WNMemory/inc/WNIntrusivePtr.h"
#include "WNMemory/inc/WNUniquePtr.h"
#include "WNScripting/inc/WNEnums.h"
#include "WNScripting/inc/WNErrors.h"
#include "WNScripting/inc/WNNodeTypes.h"
#include "WNScripting/inc/source_location.h"

namespace wn {
namespace scripting {

struct ast_node;
template <typename T>
const T* cast_to(const ast_node* _node) {
  return static_cast<const T*>(_node);
}

template <typename T>
T* cast_to(ast_node* _node) {
  return static_cast<T*>(_node);
}

template <typename T>
memory::unique_ptr<T> clone_node(memory::allocator* _allocator, const T* val) {
  if (!val) {
    return nullptr;
  }
  memory::unique_ptr<ast_node> n = val->clone(_allocator);
  // FUNCTION CALLS DO NOT GUARANTEE ARGUMENT ORDERING.
  // the allocator may have already been destroyed if you call
  // n.get_allocator()         vvvvv    there
  return memory::unique_ptr<T>(_allocator, static_cast<T*>(n.release()));
}

// clang-format off
enum class ast_node_type {
  ast_type,
  ast_node,
  ast_vtable,
  ast_statement,
    ast_declaration,
    ast_scope_block,
    ast_evaluate_expression,
    ast_assignment,
    ast_loop,
    ast_break,
    ast_continue_instruction,
    ast_if_chain,
    ast_return_instruction,
    ast_builtin_statement,
  ast_expression,
    ast_binary_expression,
    ast_constant,
    ast_id,
    ast_array_access_expression,
    ast_member_access_expression,
    ast_unary_expression,
    ast_cast_expression,
    ast_function_pointer_expression,
    ast_builtin_expression,
    ast_function_call_expression,
  ast_function,
  array_initializer_function,
  dynamic_array_initializer_function,
  ast_script_file,
};
// clang-format on

enum class builtin_type {
  not_builtin,
  void_type,
  integral_type,
  floating_point_type,
  bool_type,
  size_type,
  unresolved_function_type,
  void_ptr_type,
  nullptr_type,
  vtable_type,
};

enum class ast_type_classification {
  primitive,
  reference,
  shared_reference,
  weak_reference,
  struct_type,
  static_array,
  dynamic_array,
  function_pointer,
};

struct array_initializer_function;
struct ast_array_access_expression;
struct ast_assignment;
struct ast_binary_expression;
struct ast_break;
struct ast_builtin_expression;
struct ast_cast_expression;
struct ast_constant;
struct ast_continue_instruction;
struct ast_declaration;
struct ast_evaluate_expression;
struct ast_expression;
struct ast_function_call_expression;
struct ast_function;
struct ast_id;
struct ast_if_chain;
struct ast_loop;
struct ast_member_access_expression;
struct ast_node;
struct ast_return_instruction;
struct ast_scope_block;
struct ast_script_file;
struct ast_statement;
struct ast_type;
struct ast_unary_expression;
struct ast_vtable;
struct dynamic_array_initializer_function;

struct ast_type {
  ast_type() {}

  bool can_implicitly_cast_to(const ast_type* _other) const {
    if (_other == this) {
      return true;
    }

    if (m_classification == ast_type_classification::struct_type &&
        _other->m_classification == ast_type_classification::reference) {
      return (can_implicitly_cast_to(_other->m_implicitly_contained_type));
    }

    if (m_classification != ast_type_classification::shared_reference &&
        m_classification != ast_type_classification::reference &&
        !(m_classification == ast_type_classification::primitive &&
            m_builtin == builtin_type::nullptr_type)) {
      // Cannot cast from a non-ref non-nullptr type
      return false;
    }
    if (_other->m_classification != ast_type_classification::shared_reference &&
        _other->m_classification != ast_type_classification::reference) {
      // You cannot cast to a non-ref type.
      return false;
    }

    if (m_classification == ast_type_classification::primitive &&
        m_builtin == builtin_type::nullptr_type) {
      if (_other->m_classification !=
          ast_type_classification::shared_reference) {
        return false;
      }
      return true;
    }

    const ast_type* from_struct = m_implicitly_contained_type;
    const ast_type* to_type = _other->m_implicitly_contained_type;

    while (from_struct->m_parent_type) {
      if (from_struct->m_parent_type == to_type) {
        return true;
      }
      from_struct = from_struct->m_parent_type;
    }
    return false;
  }

  void calculate_mangled_name(memory::allocator* _allocator) {
    if (m_mangled_name != "") {
      return;
    }
    switch (m_classification) {
      case ast_type_classification::primitive: {
        switch (m_builtin) {
          case builtin_type::void_type:
            m_mangled_name = containers::string(_allocator, "v");
            return;
          case builtin_type::integral_type:
            switch (m_bit_width) {
              case 8:
                m_mangled_name = containers::string(_allocator, "h");
                return;
              case 32:
                m_mangled_name = containers::string(_allocator, "i");
                return;
              default: { WN_RELEASE_ASSERT(false, "Cannot mangle type"); }
            }
          case builtin_type::floating_point_type:
            switch (m_bit_width) {
              case 32:
                m_mangled_name = containers::string(_allocator, "f");
                return;
              default: { WN_RELEASE_ASSERT(false, "Cannot mangle type"); }
            }
          case builtin_type::bool_type:
            m_mangled_name = containers::string(_allocator, "b");
            return;
          case builtin_type::size_type:
            m_mangled_name = containers::string(_allocator, "N3wns4sizeE");
            return;
          case builtin_type::void_ptr_type:
            m_mangled_name = containers::string(_allocator, "Pv");
            return;
          default:
            break;
        }
      }
      case ast_type_classification::function_pointer:
        m_mangled_name = containers::string(_allocator, "F");
        for (auto t : m_contained_types) {
          m_mangled_name += t->m_mangled_name;
        }
        m_mangled_name += containers::string(_allocator, "E");
        return;
      case ast_type_classification::static_array:
        m_mangled_name = containers::string(_allocator, "A0_");
        m_mangled_name += m_implicitly_contained_type->m_mangled_name;
        return;
      case ast_type_classification::dynamic_array:
        m_mangled_name = containers::string(_allocator, "A__");
        m_mangled_name += m_implicitly_contained_type->m_mangled_name;
        return;
      case ast_type_classification::reference:
        m_mangled_name = containers::string(_allocator, "P");
        m_mangled_name += m_implicitly_contained_type->m_mangled_name;
        return;
      case ast_type_classification::shared_reference:
        m_mangled_name = containers::string(_allocator, "R");
        m_mangled_name += m_implicitly_contained_type->m_mangled_name;
        return;
      case ast_type_classification::weak_reference:
        m_mangled_name = containers::string(_allocator, "O");
        m_mangled_name += m_implicitly_contained_type->m_mangled_name;
        return;
      default:
        break;
    }

    char count[11] = {
        0,
    };
    memory::writeuint32(count, static_cast<uint32_t>(m_name.size()), 10);
    m_mangled_name = containers::string(_allocator);
    m_mangled_name += count;
    m_mangled_name += m_name;
  }

  bool is_builtin() {
    return m_builtin != builtin_type::not_builtin;
  }

  containers::deque<memory::unique_ptr<ast_declaration>>&
  initialized_structure_members(memory::allocator* _allocator) {
    if (m_structure_members.empty()) {
      m_structure_members =
          containers::deque<memory::unique_ptr<ast_declaration>>(_allocator);
    }
    return m_structure_members;
  }

  containers::deque<const ast_type*>& initialized_contained_types(
      memory::allocator* _allocator) {
    if (m_contained_types.empty()) {
      m_contained_types = containers::deque<const ast_type*>(_allocator);
    }
    return m_contained_types;
  }

  containers::deque<ast_function*>& initialized_member_functions(
      memory::allocator* _allocator) {
    if (m_member_functions.empty()) {
      m_member_functions = containers::deque<ast_function*>(_allocator);
    }
    return m_member_functions;
  }

  // is_arithmetic_type == true iff all normal arithmetic operations
  // are permitted on this type.
  bool m_is_arithmetic_type = false;
  bool m_is_comparable_type = false;
  uint32_t m_static_array_size = 0;
  ast_type_classification m_classification = ast_type_classification::primitive;
  builtin_type m_builtin = builtin_type::not_builtin;
  uint32_t m_bit_width = 0;
  source_location m_declared_at;
  containers::string m_name;
  containers::string m_mangled_name;
  containers::deque<const ast_type*> m_contained_types;
  containers::deque<memory::unique_ptr<ast_declaration>> m_structure_members;
  containers::deque<ast_function*> m_member_functions;
  ast_vtable* m_vtable = nullptr;
  uint32_t m_vtable_index = 0;
  bool m_struct_is_class = false;
  // m_overloaded_construction_child point to the
  // child/parent if allocated type looks
  // different than the script representation
  // E.g.
  //   WNScript     |  Looks like   | Allocated
  // struct A {     | struct A {    | struct _A {
  //   B b = B();   |   B* b;       |    B* b;
  // }              | }             |    B _b;
  //                |               | }
  ast_type* m_overloaded_construction_child = nullptr;
  ast_type* m_overloaded_construction_parent = nullptr;

  const ast_type* m_implicitly_contained_type = nullptr;
  ast_type* m_parent_type = nullptr;
  containers::deque<const ast_function*> m_declared_functions;
  const ast_function* m_constructor = nullptr;
  const ast_function* m_destructor = nullptr;
  const ast_function* m_assignment = nullptr;

  memory::unique_ptr<ast_type> clone(memory::allocator* _allocator) const {
    memory::unique_ptr<ast_type> other =
        memory::make_unique<ast_type>(_allocator);
    other->m_is_arithmetic_type = m_is_arithmetic_type;
    other->m_vtable = m_vtable;
    other->m_is_comparable_type = m_is_comparable_type;
    other->m_static_array_size = m_static_array_size;
    other->m_classification = m_classification;
    other->m_builtin = m_builtin;
    other->m_bit_width = m_bit_width;
    other->m_declared_at = m_declared_at;
    other->m_name = containers::string(_allocator, m_name);
    other->m_mangled_name = containers::string(_allocator, m_mangled_name);
    other->m_structure_members =
        containers::deque<memory::unique_ptr<ast_declaration>>(_allocator);
    for (const auto& m : m_structure_members) {
      other->m_structure_members.push_back(clone_node(_allocator, m.get()));
    }
    other->m_implicitly_contained_type = m_implicitly_contained_type;
    other->m_parent_type = m_parent_type;
    other->m_declared_functions =
        containers::deque<const ast_function*>(_allocator);
    other->m_declared_functions.insert(other->m_declared_functions.begin(),
        m_declared_functions.begin(), m_declared_functions.end());
    return core::move(other);
  }
};

enum class ast_binary_type {
  add,
  sub,
  div,
  mult,
  mod,
  lt,
  gt,
  lte,
  gte,
  eq,
  neq,
};

struct ast_node {
  ast_node(const node* _base_node, ast_node_type _type) : m_node_type(_type) {
    if (_base_node) {
      m_source_location = _base_node->get_start_location();
    } else {
      m_source_location.m_char_number = 0;
      m_source_location.m_end_index = 0;
      m_source_location.m_line_number = 0;
      m_source_location.m_line_start = 0;
      m_source_location.m_start_index = 0;
    }
  }

  // Writes the first line of this expression to the given log.
  void log_line(logging::log* _log, logging::log_level _level) const {
    size_t line_length = 1023;
    const char* c = memory::strnchr(
        reinterpret_cast<const char*>(m_source_location.m_line_start), '\n',
        1023);
    if (c == nullptr) {
      // If there is no \n then we simply print everything
      c = reinterpret_cast<const char*>(m_source_location.m_line_start) +
          memory::strnlen(
              reinterpret_cast<const char*>(m_source_location.m_line_start),
              1023);
    }
    line_length =
        (c - reinterpret_cast<const char*>(m_source_location.m_line_start));
    char* data_buffer = static_cast<char*>(WN_STACK_ALLOC(line_length + 1));
    memcpy(data_buffer, m_source_location.m_line_start, line_length);
    data_buffer[line_length] = '\0';
    _log->log_params(_level, 0, "Line: ", m_source_location.m_line_number, "\n",
        data_buffer);
  }

  virtual ~ast_node() {}
  virtual memory::unique_ptr<ast_node> clone(memory::allocator*) const {
    WN_RELEASE_ASSERT(false, "Unimplemented clone");
    return nullptr;
  }

  void copy_location_from(const ast_node* _other) {
    m_source_location = _other->m_source_location;
  }
  void copy_underlying_from(memory::allocator*, const ast_node* _other) {
    copy_location_from(_other);
    m_node_type = _other->m_node_type;
  }
  source_location m_source_location;
  ast_node_type m_node_type;
};

struct ast_vtable : public ast_node {
  ast_vtable() : ast_node(nullptr, ast_node_type::ast_vtable) {}

  containers::deque<ast_function*>& initialized_functions(
      memory::allocator* _allocator) {
    if (m_functions.empty()) {
      m_functions = containers::deque<ast_function*>(_allocator);
    }
    return m_functions;
  }

  ast_type* m_vtable_struct;
  containers::string m_name;
  containers::deque<ast_function*> m_functions;
};

struct ast_expression : public ast_node {
private:
  ast_expression(const ast_expression*);

public:
  ast_expression(const node* _base_node, ast_node_type _type)
    : ast_node(_base_node, _type) {}

  const ast_type* m_type;

  containers::deque<memory::unique_ptr<ast_declaration>>&
  initialized_setup_statements(memory::allocator* _allocator) {
    if (m_temporaries.empty()) {
      m_temporaries =
          containers::deque<memory::unique_ptr<ast_declaration>>(_allocator);
    }
    return m_temporaries;
  }

  containers::deque<memory::unique_ptr<ast_expression>>&
  initialized_destroy_expressions(memory::allocator* _allocator) {
    if (m_destroy_expressions.empty()) {
      m_destroy_expressions =
          containers::deque<memory::unique_ptr<ast_expression>>(_allocator);
    }
    return m_destroy_expressions;
  }

  containers::deque<memory::unique_ptr<ast_declaration>> m_temporaries;
  containers::deque<memory::unique_ptr<ast_expression>> m_destroy_expressions;
  void copy_underlying_from(
      memory::allocator* _alloc, const ast_expression* _other) {
    ast_node::copy_underlying_from(_alloc, _other);
    m_type = _other->m_type;
    for (auto& t : _other->m_temporaries) {
      initialized_setup_statements(_alloc).push_back(
          clone_node(_alloc, t.get()));
    }
    for (auto& t : _other->m_destroy_expressions) {
      initialized_destroy_expressions(_alloc).push_back(
          clone_node(_alloc, t.get()));
    }
  }
};

struct ast_statement : public ast_node {
  ast_statement(const node* _base_node, ast_node_type _type)
    : ast_node(_base_node, _type) {}
  void copy_underlying_from(
      memory::allocator* _alloc, const ast_statement* _other) {
    ast_node::copy_underlying_from(_alloc, _other);
  }
};

struct ast_declaration : public ast_statement {
  ast_declaration(const node* _base_node)
    : ast_statement(_base_node, ast_node_type::ast_declaration) {}
  const ast_type* m_type;
  containers::string m_name;
  bool is_ephemeral = false;
  memory::unique_ptr<ast_expression> m_initializer = nullptr;

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_declaration>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_type = m_type;
    d->m_name = containers::string(_allocator, m_name);
    d->is_ephemeral = is_ephemeral;
    d->m_initializer = clone_node(_allocator, m_initializer.get());
    return core::move(d);
  }
};

struct ast_binary_expression : public ast_expression {
  ast_binary_expression(const node* _base_node)
    : ast_expression(_base_node, ast_node_type::ast_binary_expression) {}

  ast_binary_type m_binary_type;
  memory::unique_ptr<ast_expression> m_lhs;
  memory::unique_ptr<ast_expression> m_rhs;

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_binary_expression>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_binary_type = this->m_binary_type;
    d->m_lhs = clone_node(_allocator, m_lhs.get());
    d->m_rhs = clone_node(_allocator, m_rhs.get());
    return core::move(d);
  }
};

struct ast_constant : public ast_expression {
  ast_constant(const node* _base_node)
    : ast_expression(_base_node, ast_node_type::ast_constant) {}

  union types {
    int32_t m_integer;
    float m_float;
    bool m_bool;
    uint8_t m_char;
    ast_vtable* m_vtable;
  } m_node_value;

  containers::string m_string_value;

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_constant>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_string_value = containers::string(_allocator, m_string_value);
    memory::memcpy(&d->m_node_value, &m_node_value, sizeof(m_node_value));
    return (core::move(d));
  }
};

struct ast_function : public ast_node {
  ast_function(const node* _base_node)
    : ast_node(_base_node, ast_node_type::ast_function) {}

  ~ast_function() {}
  struct parameter {
    ~parameter() {}

    parameter(containers::string _name, const ast_type* _type)
      : m_name(core::move(_name)), m_type(_type) {}

    parameter(parameter&& _other)
      : m_name(core::move(_other.m_name)), m_type(_other.m_type) {}
    containers::string m_name;
    const ast_type* m_type;
  };

  containers::deque<ast_function::parameter>& initialized_parameters(
      memory::allocator* _allocator) {
    if (m_parameters.empty()) {
      m_parameters = containers::deque<ast_function::parameter>(_allocator);
    }
    return m_parameters;
  }

  void calculate_mangled_name(memory::allocator* _allocator) {
    containers::string mangled_name(_allocator);

    containers::string name(_allocator);
    char count[11] = {
        0,
    };
    if (m_is_member_function) {
      memory::writeuint32(count,
          static_cast<uint32_t>(
              m_parameters[0]
                  .m_type->m_implicitly_contained_type->m_name.size()),
          10);
      name += count;
      name += m_parameters[0].m_type->m_implicitly_contained_type->m_name;
    }

    memory::writeuint32(count, static_cast<uint32_t>(m_name.size()), 10);
    mangled_name = containers::string(_allocator, "_ZN3wns");
    mangled_name += name;
    mangled_name += count;
    mangled_name += m_name;
    mangled_name += "E";
    mangled_name += m_return_type->m_mangled_name;

    for (auto& param : m_parameters) {
      mangled_name += param.m_type->m_mangled_name;
    }

    m_mangled_name = core::move(mangled_name);
  }

  containers::string m_name;
  containers::string m_mangled_name;
  containers::deque<ast_function::parameter> m_parameters;
  memory::unique_ptr<ast_scope_block> m_scope;
  const ast_type* m_return_type;
  const ast_type* m_function_pointer_type;
  bool m_is_member_function = false;
  bool m_is_virtual = false;
  bool m_is_override = false;
  uint32_t m_virtual_index = 0xFFFFFFFF;

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_function>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_name = containers::string(_allocator, m_name);
    d->m_mangled_name = containers::string(_allocator, m_mangled_name);
    d->m_scope = clone_node(_allocator, m_scope.get());
    d->m_return_type = m_return_type;
    auto& params = d->initialized_parameters(_allocator);
    for (auto& param : m_parameters) {
      params.emplace_back(
          containers::string(_allocator, param.m_name), param.m_type);
    }
    return core::move(d);
  }
};

struct ast_id : public ast_expression {
  ast_id(const node* _base_node)
    : ast_expression(_base_node, ast_node_type::ast_id) {}

  containers::string name() const {
    if (m_declaration) {
      return m_declaration->m_name;
    }
    if (m_function_parameter) {
      return m_function_parameter->m_name;
    }
    return m_function_name;
  }

  ast_declaration* m_declaration = nullptr;
  ast_function::parameter* m_function_parameter = nullptr;
  containers::string m_function_name;

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_id>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_declaration = m_declaration;
    d->m_function_parameter = m_function_parameter;
    return core::move(d);
  }
};

struct ast_array_access_expression : public ast_expression {
  ast_array_access_expression(const node* _base_node)
    : ast_expression(_base_node, ast_node_type::ast_array_access_expression) {}
};

struct ast_member_access_expression : public ast_expression {
  ast_member_access_expression(const node* _base_node)
    : ast_expression(_base_node, ast_node_type::ast_member_access_expression) {}

  containers::string m_member_name;
  memory::unique_ptr<ast_expression> m_base_expression;
  uint32_t m_member_offset = static_cast<uint32_t>(-1);

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d =
        memory::make_unique<ast_member_access_expression>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_member_name = containers::string(_allocator, m_member_name);
    d->m_base_expression = clone_node(_allocator, m_base_expression.get());
    d->m_member_offset = m_member_offset;
    return core::move(d);
  }
};

struct ast_unary_expression : public ast_expression {
  ast_unary_expression(const node* _base_node)
    : ast_expression(_base_node, ast_node_type::ast_unary_expression) {}
};

struct ast_cast_expression : public ast_expression {
  ast_cast_expression(const node* _base_node)
    : ast_expression(_base_node, ast_node_type::ast_cast_expression) {}
  memory::unique_ptr<ast_expression> m_base_expression;

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_cast_expression>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_base_expression = clone_node(_allocator, m_base_expression.get());
    return core::move(d);
  }
};

struct ast_function_pointer_expression : public ast_expression {
  ast_function_pointer_expression(const node* _base_node)
    : ast_expression(
          _base_node, ast_node_type::ast_function_pointer_expression) {}
  const ast_function* m_function;

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_function_pointer_expression>(
        _allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_function = m_function;
    return core::move(d);
  }
};

enum class builtin_statement_type {
  none,
  atomic_fence,
};

struct ast_builtin_statement : public ast_statement {
  ast_builtin_statement(const node* _base_node)
    : ast_statement(_base_node, ast_node_type::ast_builtin_statement) {}

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_builtin_statement>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_builtin_type = m_builtin_type;
    return core::move(d);
  }

  builtin_statement_type m_builtin_type;
};

enum class builtin_expression_type {
  none,
  pointer_to_shared,
  shared_to_pointer,
  size_of,
  align_of,
  atomic_inc,
  atomic_dec,
};

struct ast_builtin_expression : public ast_expression {
  ast_builtin_expression(const node* _base_node)
    : ast_expression(_base_node, ast_node_type::ast_builtin_expression) {}

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_builtin_expression>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    for (auto& e : m_extra_types) {
      d->initialized_extra_types(_allocator).push_back(e);
    }
    for (auto& e : m_expressions) {
      d->initialized_expressions(_allocator)
          .push_back(clone_node(_allocator, e.get()));
    }
    d->m_builtin_type = m_builtin_type;

    return core::move(d);
  }

  containers::deque<ast_type*>& initialized_extra_types(
      memory::allocator* m_allocator) {
    if (m_extra_types.empty()) {
      m_extra_types = containers::deque<ast_type*>(m_allocator);
    }
    return m_extra_types;
  }

  containers::deque<memory::unique_ptr<ast_expression>>&
  initialized_expressions(memory::allocator* m_allocator) {
    if (m_expressions.empty()) {
      m_expressions =
          containers::deque<memory::unique_ptr<ast_expression>>(m_allocator);
    }
    return m_expressions;
  }

  containers::deque<ast_type*> m_extra_types;
  containers::deque<memory::unique_ptr<ast_expression>> m_expressions;
  builtin_expression_type m_builtin_type = builtin_expression_type::none;
};

struct ast_scope_block : public ast_statement {
  ast_scope_block(const node* _base_node)
    : ast_statement(_base_node, ast_node_type::ast_scope_block) {}

  containers::deque<ast_declaration*>& initialized_declarations(
      memory::allocator* m_allocator) {
    if (m_declarations.empty()) {
      m_declarations = containers::deque<ast_declaration*>(m_allocator);
    }
    return m_declarations;
  }

  containers::deque<memory::unique_ptr<ast_statement>>& initialized_statements(
      memory::allocator* m_allocator) {
    if (m_statements.empty()) {
      m_statements =
          containers::deque<memory::unique_ptr<ast_statement>>(m_allocator);
    }
    return m_statements;
  }

  containers::deque<memory::unique_ptr<ast_expression>>& initialized_cleanup(
      memory::allocator* m_allocator) {
    if (m_cleanup_expressions.empty()) {
      m_cleanup_expressions =
          containers::deque<memory::unique_ptr<ast_expression>>(m_allocator);
    }
    return m_cleanup_expressions;
  }

  bool m_breaks = false;
  bool m_returns = false;
  containers::deque<ast_declaration*> m_declarations;
  containers::deque<memory::unique_ptr<ast_expression>> m_cleanup_expressions;
  containers::deque<memory::unique_ptr<ast_statement>> m_statements;
  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_scope_block>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_breaks = m_breaks;
    d->m_returns = m_returns;
    d->m_declarations =
        containers::deque<ast_declaration*>(_allocator, m_declarations);

    for (auto& st : m_statements) {
      d->initialized_statements(_allocator)
          .push_back(clone_node(_allocator, st.get()));
    }
    for (auto& clean : m_cleanup_expressions) {
      d->initialized_cleanup(_allocator)
          .push_back(clone_node(_allocator, clean.get()));
    }
    for (auto& decl : m_declarations) {
      d->initialized_declarations(_allocator).push_back(decl);
    }
    return core::move(d);
  }
};

struct ast_evaluate_expression : public ast_statement {
  ast_evaluate_expression(const node* _base_node)
    : ast_statement(_base_node, ast_node_type::ast_evaluate_expression) {}

  memory::unique_ptr<ast_expression> m_expr;
};

struct ast_function_call_expression : public ast_expression {
  ast_function_call_expression(const node* _base_node)
    : ast_expression(_base_node, ast_node_type::ast_function_call_expression) {}

  containers::deque<memory::unique_ptr<ast_expression>>& initialized_parameters(
      memory::allocator* _allocator) {
    if (m_parameters.empty()) {
      m_parameters =
          containers::deque<memory::unique_ptr<ast_expression>>(_allocator);
    }
    return m_parameters;
  }

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d =
        memory::make_unique<ast_function_call_expression>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_function = m_function;
    auto& params = d->initialized_parameters(_allocator);
    for (auto& p : m_parameters) {
      params.push_back(clone_node(_allocator, p.get()));
    }
    return core::move(d);
  }

  containers::deque<memory::unique_ptr<ast_expression>> m_parameters;
  const ast_function* m_function = nullptr;
  memory::unique_ptr<ast_expression> m_function_ptr;
};

struct array_initializer_function : public ast_expression {
  array_initializer_function(const node* _base_node)
    : ast_expression(_base_node, ast_node_type::array_initializer_function) {}
};

struct dynamic_array_initializer_function : public ast_expression {
  dynamic_array_initializer_function(const node* _base_node)
    : ast_expression(
          _base_node, ast_node_type::dynamic_array_initializer_function) {}
};

struct ast_assignment : public ast_statement {
  ast_assignment(const node* _base_node)
    : ast_statement(_base_node, ast_node_type::ast_assignment) {}

  memory::unique_ptr<ast_expression> m_lhs;
  memory::unique_ptr<ast_expression> m_rhs;

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_assignment>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_lhs = clone_node(_allocator, m_lhs.get());
    d->m_rhs = clone_node(_allocator, m_rhs.get());
    return core::move(d);
  }
};

struct ast_loop : public ast_statement {
  ast_loop(const node* _base_node)
    : ast_statement(_base_node, ast_node_type::ast_loop) {}

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_loop>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    return core::move(d);
  }

  memory::unique_ptr<ast_expression> m_pre_condition;
  memory::unique_ptr<ast_expression> m_post_condition;
  memory::unique_ptr<ast_scope_block> m_increment_statements;
  memory::unique_ptr<ast_scope_block> m_body;
};

struct ast_break : public ast_statement {
  ast_break(const node* _base_node)
    : ast_statement(_base_node, ast_node_type::ast_break) {}

  ast_loop* m_break_loop = nullptr;
};

struct ast_continue_instruction : public ast_statement {
  ast_continue_instruction(const node* _base_node)
    : ast_statement(_base_node, ast_node_type::ast_continue_instruction) {}
  ast_loop* m_continue_loop = nullptr;
};

struct ast_if_chain : public ast_statement {
  ast_if_chain(const node* _base_node)
    : ast_statement(_base_node, ast_node_type::ast_if_chain) {}

  struct conditional_block {
    conditional_block(memory::unique_ptr<ast_expression> _expr,
        memory::unique_ptr<ast_scope_block> _scope)
      : m_expr(core::move(_expr)), m_scope(core::move(_scope)) {}
    conditional_block(conditional_block&& _other)
      : m_expr(core::move(_other.m_expr)),
        m_scope(core::move(_other.m_scope)) {}
    memory::unique_ptr<ast_expression> m_expr;
    memory::unique_ptr<ast_scope_block> m_scope;
  };

  containers::deque<conditional_block>& initialized_conditionals(
      memory::allocator* _allocator) {
    if (m_conditionals.empty()) {
      m_conditionals = containers::deque<conditional_block>(_allocator);
    }
    return m_conditionals;
  }

  bool has_temporary() const {
    for (auto& c : m_conditionals) {
      if (!c.m_expr->m_temporaries.empty()) {
        return true;
      }
    }
    return false;
  }

  containers::deque<conditional_block> m_conditionals;

  memory::unique_ptr<ast_scope_block> m_else_block;

  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_if_chain>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_else_block = clone_node(_allocator, m_else_block.get());
    for (auto& a : m_conditionals) {
      auto s = clone_node(_allocator, a.m_scope.get());
      auto e = clone_node(_allocator, a.m_expr.get());
      d->initialized_conditionals(_allocator)
          .emplace_back(conditional_block{core::move(e), core::move(s)});
    }
    return core::move(d);
  }
};

struct ast_return_instruction : public ast_statement {
  ast_return_instruction(const node* _base_node)
    : ast_statement(_base_node, ast_node_type::ast_return_instruction) {}

  memory::unique_ptr<ast_expression> m_return_expr;
  memory::unique_ptr<ast_node> clone(
      memory::allocator* _allocator) const override {
    auto d = memory::make_unique<ast_return_instruction>(_allocator, nullptr);
    d->copy_underlying_from(_allocator, this);
    d->m_return_expr = clone_node(_allocator, m_return_expr.get());

    return core::move(d);
  }
};

struct ast_script_file : public ast_node {
  ast_script_file(const node* _base_node, memory::allocator* _allocator)
    : ast_node(_base_node, ast_node_type::ast_script_file),
      m_initialization_order(_allocator),
      m_all_vtables(_allocator),
      m_all_types(_allocator),
      m_functions(_allocator) {}
  containers::deque<ast_type*> m_initialization_order;
  containers::deque<memory::unique_ptr<ast_vtable>> m_all_vtables;
  containers::deque<memory::unique_ptr<ast_type>> m_all_types;
  containers::deque<memory::unique_ptr<ast_function>> m_functions;
};

}  // namespace scripting
}  // namespace wn

#endif  // __WN_SCRIPTING_AST_NODE_TYPES_H__