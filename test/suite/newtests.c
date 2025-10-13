#include <ohset.h>
#include <test/test.h>

static void *test_null_alloc(void *ctx, void *ptr, size_t size) {
  (void)ctx;
  (void)ptr;
  (void)size;
  return NULL;
}

TEST(new) {

  ASSERT_EQ(0, ohset_count(NULL));
  ASSERT_NULL(ohset_new(NULL));
  ASSERT_NULL(ohset_new(&(ohset_config_t){0}));
  ASSERT_NULL(ohset_new(&(ohset_config_t){
      .alloc = test_null_alloc,
      .item_size = sizeof(size_t),
  }));

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(size_t),
      .load_factor = 0.75, // Why not?
  });

  ASSERT_NON_NULL(set);
  ASSERT_EQ(0, ohset_count(set));

  ohset_free(set);
  // Ensure noop
  ohset_free(NULL);
}
