#include <ohset.h>
#include <test/test.h>

TEST(iter) {

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(char),
  });
  ASSERT_NON_NULL(set);

  for (char c = 'a'; c <= 'e'; ++c) {
    ASSERT(ohset_add(set, &c));
  }

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