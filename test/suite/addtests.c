#include <ohset.h>
#include <test/allocator.h>
#include <test/test.h>

static void test_add_many() {

  ASSERT_FALSE(ohset_add(NULL, NULL));

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(size_t),
  });
  ASSERT_NON_NULL(set);
  ASSERT_FALSE(ohset_add(set, NULL));

  for (size_t i = 42; i < 52; ++i) {
    ASSERT(ohset_add(set, &i));
    ASSERT_FALSE(ohset_add(set, &i));
    ASSERT_EQ(i, *(size_t *)ohset_get(set, &i));
  }

  ohset_free(set);
}

static void test_add_alloc_failed_2nd_time() {

  const size_t item = 42;
  allocator_t allocator = {
      .max_times = 1,
  };
  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .alloc = alloc,
      .alloc_ctx = &allocator,
      .item_size = sizeof(size_t),
  });

  ASSERT_NON_NULL(set);
  ASSERT_FALSE(ohset_add(set, &item));

  ohset_free(set);
}

TEST(add) {
  test_add_many();
  test_add_alloc_failed_2nd_time();
}