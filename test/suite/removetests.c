#include <ohset.h>
#include <test/allocator.h>
#include <test/test.h>

static void test_remove_many(void) {

  ASSERT_FALSE(ohset_remove(NULL, NULL));
  ASSERT_FALSE(ohset_remove((ohset_t *)&(ohset_config_t){0}, NULL));
  ohset_clear(NULL); // Verify noop

  const size_t item = 42;
  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(size_t),
      .load_factor = 1.0f,
  });
  ASSERT_NON_NULL(set);
  ASSERT_FALSE(ohset_remove(set, NULL));
  ASSERT_FALSE(ohset_remove(set, &item));

  // Populate
  for (size_t i = 42; i < 52; ++i) {
    ASSERT(ohset_add(set, &i));
  }

  // Remove
  for (size_t i = 42; i < 52; ++i) {
    ASSERT(ohset_remove(set, &i));
    ASSERT_NULL(ohset_get(set, &i));
    ASSERT_FALSE(ohset_remove(set, &i));
  }

  // Repopulate
  for (size_t i = 42; i < 52; ++i) {
    ASSERT(ohset_add(set, &i));
  }

  // Re-remove
  for (size_t i = 42; i < 52; ++i) {
    ASSERT(ohset_remove(set, &i));
    ASSERT_NULL(ohset_get(set, &i));
    ASSERT_FALSE(ohset_remove(set, &i));
  }

  ohset_free(set);
}

TEST(remove) {
  test_remove_many();
}
