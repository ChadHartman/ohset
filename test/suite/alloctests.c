#include <ohset.h>
#include <test/allocator.h>
#include <test/test.h>

static void str_dtor(
    void *alloc_ctx,
    void *(*alloc)(void *, void *, size_t),
    void *ptr) {

  alloc(alloc_ctx, ptr, 0);
}

TEST(alloc) {

  allocator_t allocator = {0};
  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .alloc = allocator_alloc,
      .alloc_ctx = &allocator,
      .item_cmp = (int (*)(const void *, const void *))strcmp,
      .item_dtor = str_dtor,
      .item_hash = hash_str,
      .item_size = sizeof(const char *),
  });
  ASSERT_NON_NULL(set);

  ASSERT(ohset_add(set, allocator_strdup(&allocator, "alpha")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "beta")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "gamma")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "delta")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "epsilon")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "zeta")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "eta")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "iota")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "kappa")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "lambda")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "mu")));
  ASSERT_EQ(11, ohset_count(set));

  ASSERT(ohset_remove(set, "zeta"));
  ASSERT_EQ(10, ohset_count(set));

  ohset_clear(set);
  ASSERT_EQ(0, ohset_count(set));

  ASSERT(ohset_add(set, allocator_strdup(&allocator, "iota")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "kappa")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "lambda")));
  ASSERT(ohset_add(set, allocator_strdup(&allocator, "mu")));
  ASSERT_EQ(4, ohset_count(set));

  ohset_free(set);

  ASSERT_EQ(26, allocator.total);
  ASSERT_EQ(0, allocator.live);
}
