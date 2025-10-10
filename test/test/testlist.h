#ifndef TEST_LIST_H
#define TEST_LIST_H

#include <test/test.h>

#define TEST_REG(test_name) {.name = #test_name, .func = test_##test_name}

typedef struct test_t {
  const char *name;
  void (*func)();
} test_t;

TEST(add);
TEST(alloc);
TEST(get);
TEST(hash);
TEST(iter);
TEST(new);
TEST(put);
TEST(remove);
TEST(shrink);

const test_t tests[] = {
    TEST_REG(add),
    TEST_REG(alloc),
    TEST_REG(get),
    TEST_REG(hash),
    TEST_REG(iter),
    TEST_REG(new),
    TEST_REG(put),
    TEST_REG(remove),
    TEST_REG(shrink),
};

#endif