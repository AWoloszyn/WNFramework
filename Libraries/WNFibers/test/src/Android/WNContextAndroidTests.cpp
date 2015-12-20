// Copyright (c) 2015, WNProject Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

#include "WNTesting/inc/WNTestHarness.h"
#include "WNFibers/src/Android/WNContext.h"

int foo(volatile int* x, volatile int* y, ucontext_t* _ucontext) {
  EXPECT_LE(*x, 100);
  EXPECT_EQ(*x, *y + 1);

  return wn_setcontext(_ucontext);
}

TEST(context_android, basic) {
  ucontext_t ucontext;

  ::memset(&ucontext, 0, sizeof(ucontext_t));

  volatile int x = 0;
  volatile int y = 0;
  int result = wn_getcontext(&ucontext);

  y = x;
  x += 1;

  if (x <= 100) {
    result = foo(&x, &y, &ucontext);
  }

  EXPECT_EQ(x, 101);
}

void new_func(void* f) {
  int* y = reinterpret_cast<int*>(f);
  *y += 1;
}

TEST(context_android, make_context) {
  ucontext_t ucontext;
  ::memset(&ucontext, 0, sizeof(ucontext_t));

  ucontext_t my_context;
  ::memset(&my_context, 0, sizeof(ucontext_t));

  char stack[1024];
  memset(stack, 0, sizeof(stack));

  volatile int x = 0;
  int y = 0;

  // Set up new context to move to.
  wn_getcontext(&ucontext);
  ucontext.uc_stack.ss_sp = stack;
  ucontext.uc_stack.ss_size = 1024;

  // When this is done we want to my_context.
  ucontext.uc_link = &my_context;
  wn_makecontext(&ucontext, new_func, reinterpret_cast<void*>(&y));

  // We we are done, return to here.
  wn_getcontext(&my_context);
  x += 1;
  EXPECT_EQ(y, x - 1);
  if (x <= 100) {
    wn_setcontext(&ucontext);
  }
  EXPECT_EQ(101, x);
}
