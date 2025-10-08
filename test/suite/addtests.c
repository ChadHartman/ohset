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

static void test_add_str() {

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_cmp = (int (*)(const void *, const void *))strcmp,
      .item_hash = hash_str,
      .item_size = sizeof(size_t),
  });
  ASSERT_NON_NULL(set);

  ASSERT(ohset_add(set, "alpha"));
  ASSERT_FALSE(ohset_add(set, "alpha"));
  ASSERT_STR_EQ("alpha", ohset_get(set, "alpha"));

  ASSERT(ohset_add(set, "beta"));
  ASSERT_FALSE(ohset_add(set, "beta"));
  ASSERT_STR_EQ("beta", ohset_get(set, "beta"));

  ASSERT(ohset_add(set, "gamma"));
  ASSERT_FALSE(ohset_add(set, "gamma"));
  ASSERT_STR_EQ("gamma", ohset_get(set, "gamma"));

  ASSERT(ohset_add(set, "delta"));
  ASSERT_FALSE(ohset_add(set, "delta"));
  ASSERT_STR_EQ("delta", ohset_get(set, "delta"));

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

static void test_add_load_factor_1() {

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
    ASSERT_EQ(i, *(size_t *)ohset_get(set, &i));
  }

  ohset_free(set);
}

TEST(add) {
  test_add_load_factor_1();
  test_add_many();
  test_add_str();
  test_add_alloc_failed_2nd_time();
}