#include <ohset.h>
#include <test/allocator.h>
#include <test/test.h>

static void test_add_many(void) {

  ASSERT_FALSE(ohset_add((ohset_t *)&(ohset_config_t){0}, NULL));
  ASSERT_FALSE(ohset_add(NULL, NULL));

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(size_t),
  });
  ASSERT_NON_NULL(set);
  ASSERT_FALSE(ohset_add(set, NULL));

  for (size_t i = 42; i < 52; ++i) {
    ASSERT_FALSE(ohset_get(set, &i));
    ASSERT(ohset_add(set, &i));
    ASSERT_FALSE(ohset_add(set, &i));
    ASSERT_EQ(i, *(size_t *)ohset_get(set, &i));
  }

  // Ensure all items are present
  for (size_t i = 42; i < 52; ++i) {
    ASSERT_EQ(i, *(size_t *)ohset_get(set, &i));
  }

  ohset_free(set);
}

static void test_add_alloc_failed_2nd_time(void) {

  const size_t item = 42;
  allocator_t allocator = {
      .max_times = 1,
  };
  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .alloc = allocator_alloc,
      .alloc_ctx = &allocator,
      .item_size = sizeof(size_t),
  });

  ASSERT_NON_NULL(set);
  ASSERT_FALSE(ohset_add(set, &item));

  ohset_free(set);
}

static void test_add_load_factor_1(void) {

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(size_t),
      .load_factor = 2.0f, // Will get maxed to 1.0
  });

  ASSERT_NON_NULL(set);

  for (size_t i = 42; i < 52; ++i) {
    ASSERT_FALSE(ohset_get(set, &i));
    ASSERT(ohset_add(set, &i));
    ASSERT_FALSE(ohset_add(set, &i));
    ASSERT_EQ(i, *(size_t *)ohset_get(set, &i));
  }

  // Ensure all items are present
  for (size_t i = 42; i < 52; ++i) {
    const size_t *value = ohset_get(set, &i);
    ASSERT_NON_NULL(value);
    ASSERT_EQ(i, *value);
  }

  ohset_free(set);
}

TEST(add) {

  test_add_load_factor_1();
  test_add_many();
  test_add_alloc_failed_2nd_time();
}
