// Copyright (c) 2017, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#ifndef __WN_SCRIPTING_AST_CONVERTOR_CONTEXT_H__
#define __WN_SCRIPTING_AST_CONVERTOR_CONTEXT_H__

#include "WNContainers/inc/WNHashMap.h"
#include "WNContainers/inc/WNHashSet.h"
#include "WNMemory/inc/WNStringUtility.h"
#include "WNScripting/inc/WNNodeTypes.h"
#include "WNScripting/inc/parse_ast_convertor.h"

namespace wn {
namespace scripting {

struct parse_ast_convertor::convertor_context {
  convertor_context(memory::allocator* _allocator, logging::log* _log,
      const parse_ast_convertor* _convertor);

  bool walk_script_file(const script_file* _file,
      const containers::contiguous_range<external_function>& _externals);
  ast_type* walk_struct_definition(const struct_definition* _def);
  bool create_constructor(
      const containers::deque<const struct_definition*>& _defs, ast_type* _type,
      ast_vtable* _vtable, uint32_t _vtable_index);
  bool create_destructor(
      const containers::deque<const struct_definition*>& _defs,
      ast_type* _type);
  bool create_struct_assign(const struct_definition* _def, ast_type* _type);
  // Types
  ast_type* resolve_type(const type* _type);
  ast_type* resolve_builtin_type(uint32_t _type_index);
  ast_type* resolve_static_array(const array_type* _type);
  ast_type* resolve_dynamic_array(const dynamic_array_type* _type);
  ast_type* resolve_reference_type(const type* _type);
  ast_type* get_reference_of(const node* _location, const ast_type* _type,
      ast_type_classification reference_kind);
  ast_type* resolve_function_ptr_type(
      containers::dynamic_array<const ast_type*> _types);
  ast_type* resolve_function_ptr_type(const ast_function* _function);

  // Functions
  // pre_resolve_function creates the function object, and intiializes
  // everything but the body. If _implicit_this is not nullptr, then
  // an implicit first parameter (_this) will be added to the function.
  // Furthermore if _implicit_this IS nullptr, (meaning this is not an
  // object method), then it will be added to the list of named functions.
  memory::unique_ptr<ast_function> pre_resolve_function(
      const function* _function, const ast_type* _implicit_this = nullptr);
  bool resolve_function(
      const function* _function, ast_function* _resolved_function);
  memory::unique_ptr<ast_function> resolve_external(
      const external_function _function);
  // Builtin Functions
  bool add_builtin_functions();
  void add_builtin_types();
  void add_allocate_shared();
  void add_release_shared();
  void add_assign_shared();
  void add_return_shared();

  // Statements
  bool resolve_statement(const instruction* _instruction);
  bool resolve_assignment(const assignment_instruction* _declaration);
  bool resolve_declaration(const declaration* _declaration);
  bool resolve_evaluate_expression(const expression_instruction* _inst);
  bool resolve_if(const if_instruction* _instruction);
  bool resolve_member_functions(
      containers::hash_set<const ast_type*>* _handled_types, ast_type* _type);
  bool resolve_return(const return_instruction* _instruction);
  bool resolve_while_loop(const while_instruction* _inst);
  bool resolve_do_loop(const do_instruction* _inst);
  bool resolve_for_loop(const for_instruction* _inst);
  bool resolve_break(const break_instruction* _inst);
  bool resolve_continue(const continue_instruction* _inst);

  // Expressions
  memory::unique_ptr<ast_expression> resolve_expression(
      const expression* _expression);
  memory::unique_ptr<ast_expression> resolve_id(
      const id_expression* _expression);
  memory::unique_ptr<ast_constant> resolve_constant(
      const constant_expression* _expression);
  memory::unique_ptr<ast_binary_expression> resolve_binary(
      const binary_expression* _expression);
  memory::unique_ptr<ast_function_call_expression> resolve_function_call(
      const function_call_expression* _expression);
  memory::unique_ptr<ast_expression> resolve_struct_allocation_expression(
      const struct_allocation_expression* _expression);
  memory::unique_ptr<ast_expression>
  resolve_shared_struct_allocation_expression(
      const struct_allocation_expression* _expression);
  memory::unique_ptr<ast_expression> resolve_member_access_expression(
      const member_access_expression* _expression);

  // Helpers
  memory::unique_ptr<ast_expression> get_id(const node* _location,
      const containers::string& _name, bool _can_fail = false);

  memory::unique_ptr<ast_expression> make_cast(
      memory::unique_ptr<ast_expression> from, const ast_type* _type);
  // Make implicit cast will take a reference pointer or a shared
  // pointer, and cast it to a parent class of the same type.
  // If the cast cannot be performed, or the types are the same
  // the the original expression will be returned.
  memory::unique_ptr<ast_expression> make_implicit_cast(
      memory::unique_ptr<ast_expression> from, const ast_type* _type);
  memory::unique_ptr<ast_function_call_expression> call_function(
      const node* _base_node, const ast_function* _func,
      containers::contiguous_range<memory::unique_ptr<ast_expression>>
          _expressions);
  containers::string get_next_temporary_name() {
    char buff[11] = {
        0,
    };
    memory::writeuint32(buff, m_temporary_number++, 11);
    containers::string str = containers::string(m_allocator, "_wns_temp");
    str += buff;
    return core::move(str);
  }
  memory::unique_ptr<ast_statement> evaluate_expression(
      memory::unique_ptr<ast_expression> _expr);

  // Helpers
  void transfer_temporaries(ast_expression* to, ast_expression* from);

  // Scopes
  void push_scope(ast_scope_block* _scope);
  void pop_scope();
  // Runs all destructors in this scope block up to _end_scope_block.
  // If _end_scope_block == nullptr, then cleans all scopes.
  void clean_scopes(ast_scope_block* _end_scope_block = nullptr);

  template <typename K, typename V>
  struct pair_hasher {
    size_t operator()(const core::pair<K, V>& v) const {
      return std::hash<K>{}(v.first) ^ std::hash<V>{}(v.second);
    }
  };

  template <typename K, typename V>
  struct pair_equality {
    bool operator()(
        const core::pair<K, V>& a, const core::pair<K, V>& b) const {
      return std::equal_to<K>{}(a.first, b.first) &&
             std::equal_to<V>{}(a.second, b.second);
    }
  };

  memory::allocator* m_allocator;
  const parse_ast_convertor* m_convertor;
  containers::hash_map<uint32_t, memory::unique_ptr<ast_type>> m_integral_types;
  containers::hash_map<uint32_t, memory::unique_ptr<ast_type>> m_float_types;
  containers::hash_map<containers::string, memory::unique_ptr<ast_type>>
      m_structure_types;
  containers::hash_map<containers::string, const struct_definition*>
      m_struct_definitions;
  containers::hash_map<core::pair<uint32_t, ast_type*>,
      memory::unique_ptr<ast_type>, pair_hasher<uint32_t, ast_type*>,
      pair_equality<uint32_t, ast_type*>>
      m_static_array_types;
  containers::hash_map<ast_type*, memory::unique_ptr<ast_type>>
      m_dynamic_array_types;
  containers::hash_set<const struct_definition*> m_handled_definitions;
  containers::deque<ast_type*> m_ordered_type_definitions;

  containers::deque<ast_scope_block*> m_nested_scopes;
  containers::deque<memory::unique_ptr<ast_statement>>* m_current_statements;
  containers::deque<memory::unique_ptr<ast_function>> m_constructor_destructors;
  ast_function* m_current_function;
  ast_declaration* m_return_decl;

  // Externals
  containers::hash_map<containers::string, ast_function*> m_external_functions;

  // Reference types
  containers::hash_map<const ast_type*, memory::unique_ptr<ast_type>>
      m_reference_types;
  containers::hash_map<const ast_type*, memory::unique_ptr<ast_type>>
      m_shared_types;
  containers::hash_map<const ast_type*, memory::unique_ptr<ast_type>>
      m_weak_types;

  containers::hash_map<containers::dynamic_array<const ast_type*>,
      memory::unique_ptr<ast_type>>
      m_function_pointer_types;

  // Unmangled functions
  containers::hash_map<containers::string, containers::deque<ast_function*>>
      m_named_functions;

  memory::unique_ptr<ast_script_file> m_script_file;

  memory::unique_ptr<ast_type> m_void_t;
  memory::unique_ptr<ast_type> m_void_ptr_t;
  memory::unique_ptr<ast_type> m_bool_t;
  memory::unique_ptr<ast_type> m_size_t;
  memory::unique_ptr<ast_type> m_function_t;
  memory::unique_ptr<ast_type> m_shared_object_header;
  memory::unique_ptr<ast_type> m_nullptr_t;
  memory::unique_ptr<ast_type> m_vtable_t;
  ast_type* m_destructor_fn_ptr_t;

  ast_function* m_allocate_shared;
  ast_function* m_release_shared;
  ast_function* m_assign_shared;
  ast_function* m_return_shared;
  ast_loop* m_current_loop;

  uint32_t m_temporary_number;
  logging::log* m_log;
};
}  // namespace scripting
}  // namespace wn

#endif  // __WN_SCRIPTING_AST_CONVERTOR_CONTEXT_H__