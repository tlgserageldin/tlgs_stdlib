#include "test_runner.h"

void run_tests(test_case *tests, size_t n) {
  for (size_t i = 0; i < n; i++) {
    tests[i].result = tests[i].fn();
  }
}

void results(test_case *tests, size_t n) {
  str ok = {.data = "PASS", .len = 4};
  str fail = {.data = "FAIL", .len = 4};
  str *s = NULL;
  for (size_t i = 0; i < n; i++) {
    test_case t = tests[i];
    s = t.result ? &fail : &ok;
    fprintf(stdout, "[%4.4s], %s\n", s->data, tests[i].name.data);
  }
  fprintf(stdout, "\n");
}
