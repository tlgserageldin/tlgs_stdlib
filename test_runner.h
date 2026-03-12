#pragma once

#include "q_strings.h"
#include "test_helpers.h"

typedef struct {
  str name;
  test_fn_t fn;
  int result;
} test_case;

// calls and stores the result of tests
void run_tests(test_case *tests, size_t n);

// prints the names and results of the tests
void results(test_case *tests, size_t n);
