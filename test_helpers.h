#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "q_strings.h"

typedef int (*test_fn_t)(void);

#define CHECK(cond)                                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "[CHECK FAILED] %s:%d %s\n", __FILE__, __LINE__,        \
              #cond);                                                          \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define REQUIRE(cond)                                                          \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "[REQUIRE FAILED] %s:%d %s\n", __FILE__, __LINE__,      \
              #cond);                                                          \
      return 2;                                                                \
    }                                                                          \
  } while (0)
