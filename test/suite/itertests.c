#include <ohset.h>
#include <test/test.h>

static void test_iter_many(void) {

  ASSERT_NULL(ohset_iter(NULL));
  ASSERT_NULL(ohset_iter_next(NULL));
  ASSERT_NULL(ohset_iter_value(NULL));

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(char),
  });
  ASSERT_NON_NULL(set);
  ASSERT_NULL(ohset_iter(set));

  for (char c = 'a'; c <= 'e'; ++c) {
    ASSERT(ohset_add(set, &c));
  }

  ASSERT_NULL(ohset_iter(NULL));
  ohset_iter_t *iter = ohset_iter(set);

  ASSERT_NON_NULL(iter);
  ASSERT_EQ('a', *(char *)ohset_iter_value(iter));
  iter = ohset_iter_next(iter);
  ASSERT_NON_NULL(iter);

  ASSERT_EQ('b', *(char *)ohset_iter_value(iter));
  iter = ohset_iter_next(iter);
  ASSERT_NON_NULL(iter);

  ASSERT_EQ('d', *(char *)ohset_iter_value(iter));
  iter = ohset_iter_next(iter);
  ASSERT_NON_NULL(iter);

  ASSERT_EQ('e', *(char *)ohset_iter_value(iter));
  iter = ohset_iter_next(iter);
  ASSERT_NON_NULL(iter);

  ASSERT_EQ('c', *(char *)ohset_iter_value(iter));
  ASSERT_NULL(ohset_iter_next(iter));

  ohset_free(set);
}

static void test_iter_next_invalid(void) {

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(size_t),
  });
  ASSERT_NON_NULL(set);

  size_t item = 42;
  ASSERT(ohset_add(set, &item));

  ohset_iter_t *iter = ohset_iter(set);
  ASSERT_NON_NULL(iter);

  item = 43;
  ASSERT(ohset_add(set, &item));

  ASSERT_NULL(ohset_iter_next(iter));

  ohset_free(set);
}

static void test_iter_value_invalid(void) {

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(size_t),
  });
  ASSERT_NON_NULL(set);

  size_t item = 42;
  ASSERT(ohset_add(set, &item));

  ohset_iter_t *iter = ohset_iter(set);
  ASSERT_NON_NULL(iter);

  item = 43;
  ASSERT(ohset_add(set, &item));

  ASSERT_NULL(ohset_iter_value(iter));

  ohset_free(set);
}

TEST(iter) {
  test_iter_many();
  test_iter_next_invalid();
  test_iter_value_invalid();
}
