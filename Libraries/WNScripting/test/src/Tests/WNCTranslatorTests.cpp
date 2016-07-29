// Copyright (c) 2016, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include "WNScripting/inc/WNCTranslator.h"
#include "WNFileSystem/inc/WNFactory.h"
#include "WNFileSystem/inc/WNMapping.h"
#include "WNLogging/inc/WNBufferLogger.h"
#include "WNScripting/test/inc/Common.h"
#include "WNScripting/test/inc/Common.h"
#include "WNTesting/inc/WNTestHarness.h"

void flush_buffer(void* v, const char* bytes, size_t length,
    const std::vector<WNLogging::WNLogColorElement>&) {
  wn::containers::string* s = static_cast<wn::containers::string*>(v);
  s->append(bytes, length);
}

using buffer_logger = WNLogging::WNBufferLogger<flush_buffer>;
using log_buff = wn::containers::string;

std::string get_file_data(wn::file_system::mapping_ptr& mapping,
    wn::containers::string_view file_name) {
  wn::file_system::result res;

  wn::file_system::file_ptr pt = mapping->open_file(file_name, res);
  if (!pt) {
    return "";
  }
  return std::string(pt->typed_data<char>(), pt->size());
}

TEST(c_translator, simple_c_translation) {
  wn::testing::allocator allocator;
  wn::scripting::type_validator validator(&allocator);
  wn::file_system::mapping_ptr mapping =
      wn::file_system::factory().make_mapping(
          wn::file_system::mapping_type::memory_backed, &allocator);

  mapping->initialize_files({{"file.wns", "Void main() { return; }"}});

  wn::scripting::c_translator translator(
      &validator, &allocator, mapping.get(), WNLogging::get_null_logger());
  EXPECT_EQ(
      wn::scripting::parse_error::ok, translator.translate_file("file.wns"));

  EXPECT_EQ(
      "void _Z3wns4mainEv() {\n"
      "return;\n"
      "}\n",
      get_file_data(mapping, "file.wns.c"));
}

struct source_pair {
  const char* first;
  const char* second;
};

using c_translator_direct_translation_test =
    ::testing::TestWithParam<std::vector<source_pair>>;

// The format of these tests is a vector of pairs of strings.
// This is entirely so that the test can be written as
// { "Int main() {", "int32_t main() {"},
// { "  return 4;",  "return 4;"},
// ...
// A \n in automatically appended to each string.
// If an empty string is encountered, no new-line is appended.
// If you want a line that is only a newline, you can insert a newline,
// a second one will not be added.
TEST_P(c_translator_direct_translation_test, translations) {
  wn::testing::allocator allocator;
  wn::scripting::type_validator validator(&allocator);
  wn::containers::string input_str(&allocator);
  wn::containers::string expected_output(&allocator);

  for (auto& a : GetParam()) {
    if (wn::memory::strlen(a.first) != 0) {
      input_str += a.first;
      if (a.first[0] != '\n') {
        input_str += '\n';
      }
    }
    if (wn::memory::strlen(a.second) != 0) {
      expected_output += a.second;
      if (a.second[0] != '\n') {
        expected_output += '\n';
      }
    }
  }
  log_buff buff(&allocator);
  buffer_logger logger(&buff);
  WNLogging::WNLog log(&logger);

  wn::file_system::mapping_ptr mapping =
      wn::file_system::factory().make_mapping(
          wn::file_system::mapping_type::memory_backed, &allocator);

  mapping->initialize_files({{"file.wns", input_str}});
  wn::scripting::c_translator translator(
      &validator, &allocator, mapping.get(), &log);
  EXPECT_EQ(
      wn::scripting::parse_error::ok, translator.translate_file("file.wns"))
      << (log.Flush(), buff.c_str());
  EXPECT_EQ(std::string(expected_output.c_str()),
      get_file_data(mapping, "file.wns.c"));
}

// clang-format off
INSTANTIATE_TEST_CASE_P(
    if_statement_tests, c_translator_direct_translation_test,
    ::testing::ValuesIn(
        std::vector<std::vector<source_pair>>({
            {
              {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"  },
              {"",                      "bool __wns_if_temp0 = false;"   },
              {"",                      "{"                              },
              {"",                      "__wns_if_temp0 = (x == 3);"     },
              {"",                      "}"                              },
              {"  if (x == 3) {",       "if (__wns_if_temp0) {"          },
              {"    return 7;",         "return 7;"                      },
              {"  }",                   "}"                              },
              {"  return 9;",           "return 9;"                      },
              {"}",                     "}"                              },
            },
            {
              {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"  },
              {"",                      "bool __wns_if_temp0 = false;"   },
              {"",                      "{"                              },
              {"",                      "__wns_if_temp0 = (x == 3);"     },
              {"",                      "}"                              },
              {"  if (x == 3) {",       "if (__wns_if_temp0) {"          },
              {"    return 7;",         "return 7;"                      },
              {"  } else {",            "} else {"                       },
              {"    return 10;",        "return 10;"                     },
              {"  }",                   "}"                              },
              {"  return 9;",           ""                               },
              {"}",                     "}"                              },
            },
            {
              {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"  },
              {"",                      "bool __wns_if_temp1 = false;"   },
              {"",                      "{"                              },
              {"",                      "__wns_if_temp1 = (x == 4);"     },
              {"",                      "}"                              },
              {"  if (x == 4) {",       "if (__wns_if_temp1) {"          },
              {"",                      "bool __wns_if_temp0 = false;"   },
              {"",                      "{"                              },
              {"",                      "__wns_if_temp0 = (x > 3);"      },
              {"",                      "}"                              },
              {"    if (x > 3) {",      "if (__wns_if_temp0) {"          },
              {"      return 9;",       "return 9;"                      },
              {"    }",                 "}"                              },
              {"    return 7;",         "return 7;"                      },
              {"  } else {",            "} else {"                       },
              {"    return 10;",        "return 10;"                     },
              {"  }",                   "}"                              },
              {"  return 9;",           "}"                              },
              {"}",                     ""                               },
            },
            {
              {"Int main(Int x) {",        "int32_t _Z3wns4mainEll(int32_t x) {" },
              {"",                         "bool __wns_if_temp2 = false;"   },
              {"",                         "{"                              },
              {"",                         "__wns_if_temp2 = (x == 4);"     },
              {"",                         "}"                              },
              {"  if (x == 4) {",          "if (__wns_if_temp2) {"          },
              {"",                         "bool __wns_if_temp0 = false;"   },
              {"",                         "bool __wns_if_temp1 = true;"    },
              {"",                         "{"                              },
              {"",                         "__wns_if_temp0 = (x > 3);"      },
              {"",                         "}"                              },
              {"    if (x > 3) {",         "if (__wns_if_temp0) {"          },
              {"",                         "__wns_if_temp1 = false;"        },
              {"      return 9;",          "return 9;"                      },
              {"    } else if (x < 4) {",  "}"                              },
              {"",                         "__wns_if_temp0 = __wns_if_temp1;"},
              {"",                         "if (__wns_if_temp0) {"           },
              {"",                         "__wns_if_temp0 = (x < 4);"      },
              {"",                         "}"                              },
              {"",                         "if (__wns_if_temp0) {"          },
              {"",                         "__wns_if_temp1 = false;"        },
              {"      return 32;",         "return 32;"                     },
              {"    } else {",             "}"                              },
              {"",                         "if (__wns_if_temp1) {"          },
              {"      return 9;",          "return 9;"                      },
              {"    }",                    "}"                              },
              {"    return 7;",            "} else {"                       },
              {"  } else {",               "return 10;"                     },
              {"    return 10;",           "}"                              },
              {"  }",                      "}"                              },
              {"  return 9;",              ""                               },
              {"}",                        ""                               },
            }
})));
// clang-format on

// clang-format off
INSTANTIATE_TEST_CASE_P(
    declaration_tests, c_translator_direct_translation_test,
    ::testing::ValuesIn(
        std::vector<std::vector<source_pair>>({
          {
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"  },
            {"  Int y = x;",          "int32_t y = x;"              },
            {"  Bool b = y == 4;",    "bool b = (y == 4);"          },
            {"",                      "bool __wns_if_temp0 = false;"},
            {"",                      "{"                           },
            {"",                      "__wns_if_temp0 = b;"         },
            {"",                      "}"                           },
            {"  if (b) {    ",        "if (__wns_if_temp0) {"       },
            {"     return 3;",        "return 3;"                   },
            {"  }",                   "}"                           },
            {"  return 4;",           "return 4;"                   },
            {"}",                     "}"                           },
          }
})));
// clang-format on


// clang-format off
INSTANTIATE_TEST_CASE_P(
    empty_expressions, c_translator_direct_translation_test,
    ::testing::ValuesIn(
        std::vector<std::vector<source_pair>>({
          {
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"  },
            {"  x + 3;",              "(x + 3);"                     },
            {"  return x;",           "return x;"                    },
            {"}",                     "}"                            },
          },
})));
// clang-format on

// clang-format off
INSTANTIATE_TEST_CASE_P(
    scope_tests, c_translator_direct_translation_test,
    ::testing::ValuesIn(
        std::vector<std::vector<source_pair>>({
          {
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"  },
            {"  Int y = x;",          "int32_t y = x;"               },
            {"  {",                   "{"                            },
            {"    y = 4;",            "y = 4;"                       },
            {"  }",                   "}"                            },
            {"}",                     "}"                            },
          },
          {
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"  },
            {"  Int y = x;",          "int32_t y = x;"               },
            {"  {",                   "{"                            },
            {"    {",                 "{"                            },
            {"      y = 4;",          "y = 4;"                       },
            {"    }",                 "}"                            },
            {"    return 3; ",        "return 3;"                    },
            {"  }",                   "}"                            },
            {"return 4; ",            ""                             },
            {"}",                     "}"                            },
          },
          {
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"  },
            {"  Int y = x;",          "int32_t y = x;"               },
            {"  {",                   "{"                            },
            {"     return 3; ",       "return 3;"                    },
            {"  }",                   "}"                            },
            {"return 4; ",            ""                             },
            {"}",                     "}"                            },
          }
})));
// clang-format on


// clang-format off
INSTANTIATE_TEST_CASE_P(
    assignment_tests, c_translator_direct_translation_test,
    ::testing::ValuesIn(
        std::vector<std::vector<source_pair>>({
          {
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"  },
            {"  Int y = 0;",          "int32_t y = 0;"              },
            {"  x = y;",              "x = y;"                       },
            {"  return x;",           "return x;"                    },
            {"}",                     "}"                            },
          },
          {
            {"Bool main(Int x) {",    "bool _Z3wns4mainEbl(int32_t x) {"   },
            {"  Bool b = false;",     "bool b = false;"           },
            {"  b = x == 4;",         "b = (x == 4);"                },
            {"  return b;",           "return b;"                    },
            {"}",                     "}"                            },
          },
          {
            {"Int main(Bool x) {",    "int32_t _Z3wns4mainElb(bool x) {"   },
            {"  Int y = 4;",          "int32_t y = 4;"               },
            {"",                      "bool __wns_if_temp0 = false;" },
            {"",                      "{"                            },
            {"",                      "__wns_if_temp0 = x;"          },
            {"",                      "}"                            },
            {"  if (x) {",            "if (__wns_if_temp0) {"        },
            {"    y = 10;",           "y = 10;"                      },
            {"  } else {",            "} else {"                     },
            {"    y = 4;",            "y = 4;"                       },
            {"  }",                   "}"                            },
            {"  return y;",           "return y;"                    },
            {"}",                     "}"                            },
          },
})));
// clang-format on

// clang-format off
INSTANTIATE_TEST_CASE_P(
    struct_definition_tests, c_translator_direct_translation_test,
    ::testing::ValuesIn(
        std::vector<std::vector<source_pair>>({
          {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {"   },
            {"",                      "_this->x = 0;"                },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            }

          },
          {
            {"struct Bar {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"  Int y = 0;",          "  int32_t y;"                 },
            {"  Float z = 0.0;",      "  float z;"                   },
            {"}",                     "} Bar;"                       },
            {"",                      "\n"                           },
            {"",                      "Bar* _Z3wns14_construct_BarENR3BarENR3BarE(Bar* _this) {"   },
            {"",                      "_this->x = 0;"                },
            {"",                      "_this->y = 0;"                },
            {"",                      "_this->z = 0.0f;"             },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_BarEvNR3BarE(Bar* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            }
          },
          {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"  Int y = 0;",          "  int32_t y;"                 },
            {"  Float z = 0.0;",      "  float z;"                   },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"struct Bar {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"  Int y = 0;",          "  int32_t y;"                 },
            {"  Float z = 0.0;",      "  float z;"                   },
            {"}",                     "} Bar;"                       },
            {"",                      "\n"                           },
            {"",                      "Bar* _Z3wns14_construct_BarENR3BarENR3BarE(Bar* _this) {"   },
            {"",                      "_this->x = 0;"                },
            {"",                      "_this->y = 0;"                },
            {"",                      "_this->z = 0.0f;"             },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_BarEvNR3BarE(Bar* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {"   },
            {"",                      "_this->x = 0;"                },
            {"",                      "_this->y = 0;"                },
            {"",                      "_this->z = 0.0f;"             },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            }
          },
          {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"  Int y = 0;",          "  int32_t y;"                 },
            {"  Float z = 0.0;",      "  float z;"                   },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"struct Bar {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"  Int y = 0;",          "  int32_t y;"                 },
            {"  Float z = 0.0;",      "  float z;"                   },
            {"}",                     "} Bar;"                       },
            {"",                      "\n"                           },
            {"",                      "Bar* _Z3wns14_construct_BarENR3BarENR3BarE(Bar* _this) {"   },
            {"",                      "_this->x = 0;"                },
            {"",                      "_this->y = 0;"                },
            {"",                      "_this->z = 0.0f;"             },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_BarEvNR3BarE(Bar* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {"   },
            {"",                      "_this->x = 0;"                },
            {"",                      "_this->y = 0;"                },
            {"",                      "_this->z = 0.0f;"             },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"  },
            {"",                      "bool __wns_if_temp0 = false;" },
            {"",                      "{"                            },
            {"",                      "__wns_if_temp0 = (x == 3);"   },
            {"",                      "}"                            },
            {"  if (x == 3) {",       "if (__wns_if_temp0) {"        },
            {"    return 7;",         "return 7;"                    },
            {"  }",                   "}"                            },
            {"  return 9;",           "return 9;"                    },
            {"}",                     "}"                            }
          },
})));
// clang-format on

// clang-format off
INSTANTIATE_TEST_CASE_P(
    struct_explicit_declaration, c_translator_direct_translation_test,
    ::testing::ValuesIn(
        std::vector<std::vector<source_pair>>({
          {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {"   },
            {"",                      "_this->x = 0;"                },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {" Foo f = Foo();",       "Foo __wns_temp_expression0;"             },
            {"",                      "Foo* f = _Z3wns14_construct_FooENR3FooENR3FooE(&__wns_temp_expression0);"       },
            {"",                      "{"                            },
            {"",                      "int32_t __wns_ret_temp0 = 4;" },
            {"",                      "_Z3wns13_destruct_FooEvNR3FooE(&__wns_temp_expression0);"},
            {" return 4;",            "return __wns_ret_temp0;"      },
            {"",                      "}"},
            {"}",                     "}"                            }
          },
})));
// clang-format on

// clang-format off
INSTANTIATE_TEST_CASE_P(
    struct_usage, c_translator_direct_translation_test,
    ::testing::ValuesIn(
       std::vector<std::vector<source_pair>>({
          {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {" },
            {"",                      "_this->x = 0;"                },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {" },
            {" Foo f = Foo();",       "Foo __wns_temp_expression0;"             },
            {"",                      "Foo* f = _Z3wns14_construct_FooENR3FooENR3FooE(&__wns_temp_expression0);" },
            {" f.x = x;",             "f->x = x;"                    },
            {"",                      "{"                            },
            {"",                      "int32_t __wns_ret_temp0 = f->x;" },
            {"",                      "_Z3wns13_destruct_FooEvNR3FooE(&__wns_temp_expression0);"},
            {" return f.x;",          "return __wns_ret_temp0;"      },
            {"",                      "}"                            },
            {"}",                     "}"                            }
          },
          {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"  Int y = 0;",          "  int32_t y;"                 },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {" },
            {"",                      "_this->x = 0;"                },
            {"",                      "_this->y = 0;"                },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {" Foo f = Foo();",       "Foo __wns_temp_expression0;"             },
            {"",                      "Foo* f = _Z3wns14_construct_FooENR3FooENR3FooE(&__wns_temp_expression0);" },
            {" f.x = x;",             "f->x = x;"                    },
            {" f.y = 2 * x;",         "f->y = (2 * x);"              },
            {"",                      "{"                            },
            {"",                      "int32_t __wns_ret_temp0 = (f->x + f->y);" },
            {"",                      "_Z3wns13_destruct_FooEvNR3FooE(&__wns_temp_expression0);"},
            {" return f.x + f.y;",    "return __wns_ret_temp0;"      },
            {"",                      "}"                            },
            {"}",                     "}"                            }
          },
         {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 32;",         "  int32_t x;"                 },
            {"  Int y = 7;",          "  int32_t y;"                 },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {" },
            {"",                      "_this->x = 32;"               },
            {"",                      "_this->y = 7;"                },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {" Foo f = Foo();",       "Foo __wns_temp_expression0;"             },
            {"",                      "Foo* f = _Z3wns14_construct_FooENR3FooENR3FooE(&__wns_temp_expression0);" },
            {"",                      "{"                            },
            {"",                      "int32_t __wns_ret_temp0 = (f->x + f->y);" },
            {"",                      "_Z3wns13_destruct_FooEvNR3FooE(&__wns_temp_expression0);"},
            {" return f.x + f.y;",    "return __wns_ret_temp0;"      },
            {"",                      "}"                            },
            {"}",                     "}"                            }
          },
          {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 32;",         "  int32_t x;"                 },
            {"  Int y = 7;",          "  int32_t y;"                 },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {" },
            {"",                      "_this->x = 32;"               },
            {"",                      "_this->y = 7;"                },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {"Int r = 0;",            "int32_t r = 0;"               },
            {"{",                     "{"                            },
            {" Foo f = Foo();",       "Foo __wns_temp_expression0;"             },
            {"",                      "Foo* f = _Z3wns14_construct_FooENR3FooENR3FooE(&__wns_temp_expression0);" },
            {" r = f.x;",             "r = f->x;"                    },
            {"",                      "_Z3wns13_destruct_FooEvNR3FooE(&__wns_temp_expression0);"},
            {"}",                     "}"                            },
            {" return r + x;",        "return (r + x);"              },
            {"}",                     "}"                            }
          },
})));
// clang-format on

// clang-format off
INSTANTIATE_TEST_CASE_P(
    function_calls, c_translator_direct_translation_test,
    ::testing::ValuesIn(
       std::vector<std::vector<source_pair>>({
          {
            {"Int foo() {",           "int32_t _Z3wns3fooEl() {"     },
            {"  return 4;",           "return 4;"                    },
            {"}",                     "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {"  return foo() + x;",   "return (_Z3wns3fooEl() + x);" },
            {"}",                     "}"                            }
          },
          {
            {"Int foo(Int x) {",      "int32_t _Z3wns3fooEll(int32_t x) {"    },
            {"  return x + 4;",       "return (x + 4);"              },
            {"}",                     "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {"  return foo(x);",      "return _Z3wns3fooEll(x);"     },
            {"}",                     "}"                            }
          },
         {
            {"Int foo(Int x, Int y){","int32_t _Z3wns3fooElll(int32_t x, int32_t y) {"    },
            {"  return x + y;",       "return (x + y);"              },
            {"}",                     "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {"  return foo(x, x);",   "return _Z3wns3fooElll(x, x);" },
            {"}",                     "}"                            }
          },
})));
// clang-format on

// clang-format off
INSTANTIATE_TEST_CASE_P(
    temporary_objects, c_translator_direct_translation_test,
    ::testing::ValuesIn(
        std::vector<std::vector<source_pair>>({
          {
            {"struct Foo {",          "typedef struct {"  },
            {"  Int x = 4;",          "  int32_t x;"      },
            {"}",                     "} Foo;"             },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {" },
            {"",                      "_this->x = 4;"               },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"  },
            {" return Foo().x;",      "Foo __wns_temp_expression0;"             },
            {"",                      "{"                            },
            {"",                      "int32_t __wns_ret_temp0 = _Z3wns14_construct_FooENR3FooENR3FooE(&__wns_temp_expression0)->x;"},
            {"",                      "_Z3wns13_destruct_FooEvNR3FooE(&__wns_temp_expression0);"    },
            {"}",                     "return __wns_ret_temp0;"  },
            {"",                      "}"  },
            {"",                      "}"  },
          }
})));
// clang-format on


// clang-format off
INSTANTIATE_TEST_CASE_P(
    shared_objects, c_translator_direct_translation_test,
    ::testing::ValuesIn(
        std::vector<std::vector<source_pair>>({
          {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {"   },
            {"",                      "_this->x = 0;"                },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {" shared Foo f = shared Foo();", "Foo* f = (Foo*)_Z3wns14_assign_sharedEvpvpvp((void*)_Z3wns14_construct_FooENR3FooENR3FooE((Foo*)_Z3wns16_allocate_sharedEvpsfp(sizeof(Foo), &_Z3wns13_destruct_FooEvNR3FooE)), NULL);"       },
            {"",                      "{"                            },
            {"",                      "int32_t __wns_ret_temp0 = 4;" },
            {"",                      "_Z3wns13_deref_sharedEvvp((void*)f);"},
            {" return 4;",            "return __wns_ret_temp0;"      },
            {"",                      "}"},
            {"}",                     "}"                            }
          },
          {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 42;",          "  int32_t x;"                 },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {"   },
            {"",                      "_this->x = 42;"                },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {"",                      "Foo* __wns_temp_expression0 = (Foo*)_Z3wns14_assign_sharedEvpvpvp((void*)_Z3wns14_construct_FooENR3FooENR3FooE((Foo*)_Z3wns16_allocate_sharedEvpsfp(sizeof(Foo), &_Z3wns13_destruct_FooEvNR3FooE)), NULL);"},
            {"",                      "{"},
            {"",                      "int32_t __wns_ret_temp0 = __wns_temp_expression0->x;" },
            {"",                      "_Z3wns13_deref_sharedEvvp((void*)__wns_temp_expression0);" },
            {" return (shared Foo()).x;", "return __wns_ret_temp0;"      },
            {"",                      "}"},
            {"}",                     "}"                            }
          },
          {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {"   },
            {"",                      "_this->x = 0;"                },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {" shared Foo f = shared Foo();", "Foo* f = (Foo*)_Z3wns14_assign_sharedEvpvpvp((void*)_Z3wns14_construct_FooENR3FooENR3FooE((Foo*)_Z3wns16_allocate_sharedEvpsfp(sizeof(Foo), &_Z3wns13_destruct_FooEvNR3FooE)), NULL);" },
            {" f = shared Foo();",     "f = (Foo*)_Z3wns14_assign_sharedEvpvpvp((void*)_Z3wns14_construct_FooENR3FooENR3FooE((Foo*)_Z3wns16_allocate_sharedEvpsfp(sizeof(Foo), &_Z3wns13_destruct_FooEvNR3FooE)), (void*)f);" },
            {"",                      "{"                            },
            {"",                      "int32_t __wns_ret_temp0 = 4;" },
            {"",                      "_Z3wns13_deref_sharedEvvp((void*)f);"},
            {" return 4;",            "return __wns_ret_temp0;"      },
            {"",                      "}"},
            {"}",                     "}"                            }
          },
          {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {"   },
            {"",                      "_this->x = 0;"                },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"shared Foo blah() {",   "Foo* _Z3wns4blahENRR3FooE() {"},
            {" return shared Foo();", "return _Z3wns14_construct_FooENR3FooENR3FooE((Foo*)_Z3wns16_allocate_sharedEvpsfp(sizeof(Foo), &_Z3wns13_destruct_FooEvNR3FooE));"},
            {"}",                     "}"},
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {" shared Foo f = blah();", "Foo* f = (Foo*)_Z3wns14_assign_sharedEvpvpvp((void*)_Z3wns4blahENRR3FooE(), NULL);"       },
            {"",                      "{"                            },
            {"",                      "int32_t __wns_ret_temp0 = 4;" },
            {"",                      "_Z3wns13_deref_sharedEvvp((void*)f);"},
            {" return 4;",            "return __wns_ret_temp0;"      },
            {"",                      "}"},
            {"}",                     "}"                            }
          },
          {
            {"struct Foo {",          "typedef struct {"             },
            {"  Int x = 0;",          "  int32_t x;"                 },
            {"}",                     "} Foo;"                       },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {"   },
            {"",                      "_this->x = 0;"                },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"shared Foo blah() {",   "Foo* _Z3wns4blahENRR3FooE() {"},
            {" shared Foo f = shared Foo();", "Foo* f = (Foo*)_Z3wns14_assign_sharedEvpvpvp((void*)_Z3wns14_construct_FooENR3FooENR3FooE((Foo*)_Z3wns16_allocate_sharedEvpsfp(sizeof(Foo), &_Z3wns13_destruct_FooEvNR3FooE)), NULL);"},
            {"",                      "{"                            },
            {"",                      "Foo* __wns_ret_temp0 = (Foo*)_Z3wns14_assign_sharedEvpvpvp((void*)f, NULL);" },
            {"",                      "_Z3wns13_deref_sharedEvvp((void*)f);"},
            {" return f;",            "return (Foo*)_Z3wns14_return_sharedEvpvp((void*)__wns_ret_temp0);"      },
            {"",                      "}"},
            {"}",                     "}"},
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {" shared Foo f = blah();", "Foo* f = (Foo*)_Z3wns14_assign_sharedEvpvpvp((void*)_Z3wns4blahENRR3FooE(), NULL);"       },
            {"",                      "{"                            },
            {"",                      "int32_t __wns_ret_temp1 = 4;" },
            {"",                      "_Z3wns13_deref_sharedEvvp((void*)f);"},
            {" return 4;",            "return __wns_ret_temp1;"      },
            {"",                      "}"},
            {"}",                     "}"                            }
          },
})));

// clang-format on

// clang-format off
INSTANTIATE_TEST_CASE_P(
    function_taking_struct, c_translator_direct_translation_test,
    ::testing::ValuesIn(
        std::vector<std::vector<source_pair>>({
          {
            {"struct Foo {",          "typedef struct {"  },
            {"  Int x = 4;",          "  int32_t x;"      },
            {"}",                     "} Foo;"             },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {" },
            {"",                      "_this->x = 4;"               },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"Int blah(Foo f) {",     "int32_t _Z3wns4blahElNP3FooE(Foo* f) {"},
            {" return f.x;",          "return f->x;"  },
            {"}",                     "}"},
            {"",                      "\n"},
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {" Foo f = Foo();",       "Foo __wns_temp_expression0;"             },
            {"",                      "Foo* f = _Z3wns14_construct_FooENR3FooENR3FooE(&__wns_temp_expression0);"       },
            {"",                      "{"                            },
            {"",                      "int32_t __wns_ret_temp0 = _Z3wns4blahElNP3FooE(f);" },
            {"",                      "_Z3wns13_destruct_FooEvNR3FooE(&__wns_temp_expression0);"},
            {" return blah(f);",       "return __wns_ret_temp0;"      },
            {"",                      "}"},
            {"}",                     "}"                            }
          }
})));

// clang-format off
INSTANTIATE_TEST_CASE_P(
    struct_in_struct, c_translator_direct_translation_test,
    ::testing::ValuesIn(
        std::vector<std::vector<source_pair>>({
          {
            {"struct Foo {",          "typedef struct {"  },
            {"  Int x = 4;",          "  int32_t x;"      },
            {"}",                     "} Foo;"            },
            {"",                      "\n"                },
            {"struct Bar {",          "typedef struct {"  },
            {"  Foo f = Foo();",      "  Foo* f;"         },
            {"",                      "  Foo __f;"        },
            {"}",                     "} Bar;"            },
            {"",                      "\n"                },
            {"",                      "Bar* _Z3wns14_construct_BarENR3BarENR3BarE(Bar* _this) {" },
            {"",                      "_this->f = _Z3wns14_construct_FooENR3FooENR3FooE(&_this->__f);"},
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_BarEvNR3BarE(Bar* _this) {"   },
            {"",                      "_Z3wns13_destruct_FooEvNR3FooE(&_this->__f);"           },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {" },
            {"",                      "_this->x = 4;"               },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            }
          },
          {
            {"struct Foo {",          "typedef struct {"  },
            {"  Int x = 4;",          "  int32_t x;"      },
            {"}",                     "} Foo;"            },
            {"",                      "\n"                },
            {"struct Bar {",          "typedef struct {"  },
            {"  Foo f = Foo();",      "  Foo* f;"         },
            {"",                      "  Foo __f;"        },
            {"}",                     "} Bar;"            },
            {"",                      "\n"                },
            {"",                      "Bar* _Z3wns14_construct_BarENR3BarENR3BarE(Bar* _this) {" },
            {"",                      "_this->f = _Z3wns14_construct_FooENR3FooENR3FooE(&_this->__f);"},
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_BarEvNR3BarE(Bar* _this) {"   },
            {"",                      "_Z3wns13_destruct_FooEvNR3FooE(&_this->__f);"           },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "Foo* _Z3wns14_construct_FooENR3FooENR3FooE(Foo* _this) {" },
            {"",                      "_this->x = 4;"               },
            {"",                      "return _this;"                },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"",                      "void _Z3wns13_destruct_FooEvNR3FooE(Foo* _this) {"   },
            {"",                      "return;"                      },
            {"",                      "}"                            },
            {"",                      "\n"                           },
            {"Int main(Int x) {",     "int32_t _Z3wns4mainEll(int32_t x) {"   },
            {" Bar b = Bar();",       "Bar __wns_temp_expression0;"             },
            {"",                      "Bar* b = _Z3wns14_construct_BarENR3BarENR3BarE(&__wns_temp_expression0);"       },
            {"",                      "{"                            },
            {"",                      "int32_t __wns_ret_temp0 = b->f->x;" },
            {"",                      "_Z3wns13_destruct_BarEvNR3BarE(&__wns_temp_expression0);"},
            {" return b.f.x;",        "return __wns_ret_temp0;"      },
            {"",                      "}"},
            {"}",                     "}"                            }
          }
})));
// clang-format on

using c_translator_function_params =
    ::testing::TestWithParam<std::tuple<const char*, const char*, const char*>>;
TEST_P(c_translator_function_params, single_parameter) {
  wn::testing::allocator allocator;
  wn::scripting::type_validator validator(&allocator);
  wn::containers::string str(&allocator);
  str += std::get<0>(GetParam());
  str = str + " main(" + std::get<0>(GetParam()) + " x) { return x; }";

  wn::containers::string expected_str(&allocator);
  expected_str += std::get<1>(GetParam());
  expected_str = expected_str + " _Z3wns4mainE" + std::get<2>(GetParam()) +
                 std::get<2>(GetParam()) + "(" + std::get<1>(GetParam()) +
                 " x) {\nreturn x;\n}\n";

  wn::file_system::mapping_ptr mapping =
      wn::file_system::factory().make_mapping(
          wn::file_system::mapping_type::memory_backed, &allocator);

  mapping->initialize_files({{"file.wns", str}});

  wn::scripting::c_translator translator(
      &validator, &allocator, mapping.get(), WNLogging::get_null_logger());
  EXPECT_EQ(
      wn::scripting::parse_error::ok, translator.translate_file("file.wns"));
  EXPECT_EQ(
      std::string(expected_str.c_str()), get_file_data(mapping, "file.wns.c"));
}

INSTANTIATE_TEST_CASE_P(
    parameter_tests, c_translator_function_params,
    ::testing::ValuesIn(
        std::vector<std::tuple<const char*, const char*, const char*>>(
            {std::make_tuple("Int", "int32_t", "l"),
                std::make_tuple("Float", "float", "f"),
                std::make_tuple("Bool", "bool", "b"),
                std::make_tuple("Char", "char", "c")})));

TEST(c_translator, multiple_c_functions) {
  wn::testing::allocator allocator;
  wn::scripting::type_validator validator(&allocator);
  wn::file_system::mapping_ptr mapping =
      wn::file_system::factory().make_mapping(
          wn::file_system::mapping_type::memory_backed, &allocator);

  mapping->initialize_files({{"file.wns",
      "Void main() { return; }\n"
      "Void foo() { return; }\n"}});

  wn::scripting::c_translator translator(
      &validator, &allocator, mapping.get(), WNLogging::get_null_logger());
  EXPECT_EQ(
      wn::scripting::parse_error::ok, translator.translate_file("file.wns"));
  EXPECT_EQ(
      "void _Z3wns4mainEv() {\n"
      "return;\n"
      "}\n"
      "\n"
      "void _Z3wns3fooEv() {\n"
      "return;\n"
      "}\n",
      get_file_data(mapping, "file.wns.c"));
}

// Make sure multiple returns get combined into just 1.
TEST(c_translator, multiple_returns) {
  wn::testing::allocator allocator;
  wn::scripting::type_validator validator(&allocator);

  wn::file_system::mapping_ptr mapping =
      wn::file_system::factory().make_mapping(
          wn::file_system::mapping_type::memory_backed, &allocator);

  mapping->initialize_files({{"file.wns", "Void main() { return; return; }"}});

  wn::scripting::c_translator translator(
      &validator, &allocator, mapping.get(), WNLogging::get_null_logger());
  EXPECT_EQ(
      wn::scripting::parse_error::ok, translator.translate_file("file.wns"));
  EXPECT_EQ(
      "void _Z3wns4mainEv() {\n"
      "return;\n"
      "}\n",
      get_file_data(mapping, "file.wns.c"));
}

class c_int_params : public ::testing::TestWithParam<const char*> {};

TEST_P(c_int_params, int_return) {
  wn::testing::allocator allocator;
  wn::scripting::type_validator validator(&allocator);
  wn::containers::string str(&allocator);
  str += "Int main() { return ";
  str += GetParam();
  str += "; } ";

  wn::containers::string expected(&allocator);
  expected +=
      "int32_t _Z3wns4mainEl() {\n"
      "return ";
  expected += GetParam();
  expected += ";\n}\n";
  wn::file_system::mapping_ptr mapping =
      wn::file_system::factory().make_mapping(
          wn::file_system::mapping_type::memory_backed, &allocator);

  mapping->initialize_files({{"file.wns", str}});

  wn::scripting::c_translator translator(
      &validator, &allocator, mapping.get(), WNLogging::get_null_logger());
  EXPECT_EQ(
      wn::scripting::parse_error::ok, translator.translate_file("file.wns"));
  EXPECT_EQ(
      std::string(expected.c_str()), get_file_data(mapping, "file.wns.c"));
}

INSTANTIATE_TEST_CASE_P(int_tests, c_int_params,
    ::testing::Values("0", "-1", "-32", "-4096", "2147483647", "-2147483648"));

struct arithmetic_operations {
  const char* source;
  const char* dest;
};

using c_arith_params = ::testing::TestWithParam<arithmetic_operations>;

TEST_P(c_arith_params, binary_arithmetic) {
  wn::testing::allocator allocator;
  wn::scripting::type_validator validator(&allocator);
  wn::containers::string str(&allocator);
  str += "Int main() { return ";
  str += GetParam().source;
  str += "; } ";

  wn::containers::string expected(&allocator);
  expected +=
      "int32_t _Z3wns4mainEl() {\n"
      "return ";
  expected += GetParam().dest;
  expected += ";\n}\n";
  wn::file_system::mapping_ptr mapping =
      wn::file_system::factory().make_mapping(
          wn::file_system::mapping_type::memory_backed, &allocator);

  mapping->initialize_files({{"file.wns", str}});

  wn::scripting::c_translator translator(
      &validator, &allocator, mapping.get(), WNLogging::get_null_logger());
  EXPECT_EQ(
      wn::scripting::parse_error::ok, translator.translate_file("file.wns"));
  EXPECT_EQ(
      std::string(expected.c_str()), get_file_data(mapping, "file.wns.c"));
}

INSTANTIATE_TEST_CASE_P(int_int_tests, c_arith_params,
    ::testing::ValuesIn(std::vector<arithmetic_operations>(
        {{"1 + 2", "(1 + 2)"}, {"10 - 20", "(10 - 20)"},
            {"-32 * 0", "(-32 * 0)"}, {"-32 % 22", "(-32 % 22)"},
            {"-32 + 4 * 10", "(-32 + (4 * 10))"},
            {"27 / 4 + 8 * 3", "((27 / 4) + (8 * 3))"}})));

using c_bool_params = ::testing::TestWithParam<arithmetic_operations>;

TEST_P(c_bool_params, boolean_arithmetic) {
  wn::testing::allocator allocator;
  wn::scripting::type_validator validator(&allocator);
  wn::containers::string str(&allocator);
  str += "Bool wn_main(Bool b) { return ";
  str += GetParam().source;
  str += "; } ";

  wn::containers::string expected(&allocator);
  expected +=
      "bool _Z3wns7wn_mainEbb(bool b) {\n"
      "return ";
  expected += GetParam().dest;
  expected += ";\n}\n";
  wn::file_system::mapping_ptr mapping =
      wn::file_system::factory().make_mapping(
          wn::file_system::mapping_type::memory_backed, &allocator);

  mapping->initialize_files({{"file.wns", str}});

  wn::scripting::c_translator translator(
      &validator, &allocator, mapping.get(), WNLogging::get_null_logger());
  EXPECT_EQ(
      wn::scripting::parse_error::ok, translator.translate_file("file.wns"));
  EXPECT_EQ(
      std::string(expected.c_str()), get_file_data(mapping, "file.wns.c"));
}

INSTANTIATE_TEST_CASE_P(int_int_tests, c_bool_params,
    ::testing::ValuesIn(std::vector<arithmetic_operations>(
        {{"true", "true"}, {"true == true", "(true == true)"},
            {"false", "false"}, {"2 == 3", "(2 == 3)"},
            {"3 == 4 != b", "((3 == 4) != b)"}, {"b == true", "(b == true)"},
            {"b == b", "(b == b)"}, {"1 >= 3", "(1 >= 3)"},
            {"1 < 1", "(1 < 1)"}, {"1 > 1", "(1 > 1)"}, {"1 >= 1", "(1 >= 1)"},
            {"1 <= 1", "(1 <= 1)"}, {"1 > (3 + 2)", "(1 > (3 + 2))"},
            {"(1 < 2) == (4 > 10)", "((1 < 2) == (4 > 10))"},
            {"(1 <= 2) == (b == false)", "((1 <= 2) == (b == false))"}})));
