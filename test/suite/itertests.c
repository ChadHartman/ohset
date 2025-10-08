#include <ohset.h>
#include <test/test.h>

static void test_iter_many() {

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

TEST(iter) {
  test_iter_many();
}