#include <ohset.h>
#include <test/allocator.h>
#include <test/test.h>

static void str_destructor(
    void *alloc_ctx,
    void *(*alloc)(void *, void *, size_t),
    void *ptr) {

  alloc(alloc_ctx, ptr, 0);
}

static void test_remove_many() {

  ASSERT_FALSE(ohset_remove(NULL, NULL));

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

static void test_remove_destructor() {

  allocator_t allocator = {0};
  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .alloc = alloc,
      .alloc_ctx = &allocator,
      .item_size = sizeof(char *),
      .item_dtor = str_destructor,
  });

  ASSERT(ohset_add(set, alloc_strdup(&allocator, "foo")));
  ASSERT(ohset_remove(set, "foo"));
  ASSERT_FALSE(ohset_remove(set, "foo"));

  ohset_free(set);

  ASSERT_EQ(3, allocator.total);
  ASSERT_EQ(0, allocator.live);
}

TEST(remove) {
  test_remove_many();
  test_remove_destructor();
}