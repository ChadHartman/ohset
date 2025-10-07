#ifndef TEST_LIST_H
#define TEST_LIST_H

#include <test/test.h>

#define TEST_REG(test_name) {.name = #test_name, .func = test_##test_name}

typedef struct test_t {
  const char *name;
  void (*func)();
} test_t;

TEST(hash);
TEST(new);

const test_t tests[] = {
    TEST_REG(hash),
    TEST_REG(new),
};

#endif