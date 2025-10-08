#include <ohset.h>
#include <test/allocator.h>
#include <test/test.h>

static void str_destructor(
    void *alloc_ctx,
    void *(*alloc)(void *, void *, size_t),
    void *ptr) {

  char **str = ptr;

  alloc(alloc_ctx, *str, 0);
}

static int custom_strcmp(const void *a, const void *b) {
  const char *const *lhs = a;
  const char *const *rhs = b;
  return strcmp(*lhs, *rhs);
}

static uint32_t custom_strhash(const void *ptr) {
  const char *const *str = ptr;
  return ohset_hash(*str, strlen(*str));
}

static void test_remove_many() {

  ASSERT_FALSE(ohset_remove(NULL, NULL));
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