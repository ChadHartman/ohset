#include <ohset.h>
#include <test/allocator.h>
#include <test/test.h>

typedef struct str_wrapper_t {
  char *value;
} str_wrapper_t;

static int str_wrapper_cmp(const void *a, const void *b) {
  const str_wrapper_t *restrict lhs = a;
  const str_wrapper_t *restrict rhs = b;
  return strcmp(lhs->value, rhs->value);
}

static void str_wrapper_dtor(
    void *alloc_ctx,
    void *(*alloc)(void *, void *, size_t),
    void *ptr) {

  str_wrapper_t *sw = ptr;
  TEST_LOG("DTOR \"%s\"", sw->value);

  alloc(alloc_ctx, sw->value, 0);
}

static uint32_t str_wrapper_hash(const void *ptr) {
  const str_wrapper_t *restrict sw = ptr;
  return ohset_hash(sw->value, strlen(sw->value));
}

TEST(alloc) {

  allocator_t allocator = {0};
  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .alloc = allocator_alloc,
      .alloc_ctx = &allocator,
      .item_cmp = str_wrapper_cmp,
      .item_dtor = str_wrapper_dtor,
      .item_hash = str_wrapper_hash,
      .item_size = sizeof(str_wrapper_t),
  });
  ASSERT_NON_NULL(set);

  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "alpha")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "beta")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "gamma")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "delta")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "epsilon")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "zeta")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "eta")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "iota")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "kappa")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "lambda")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "mu")}));
  ASSERT_EQ(11, ohset_count(set));

  ASSERT(ohset_remove(set, &(str_wrapper_t){.value = "zeta"}));
  ASSERT_EQ(10, ohset_count(set));

  ohset_clear(set);
  ASSERT_EQ(0, ohset_count(set));

  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "iota")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "kappa")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "lambda")}));
  ASSERT(ohset_add(set, &(str_wrapper_t){.value = allocator_strdup(&allocator, "mu")}));
  ASSERT_EQ(4, ohset_count(set));

  ohset_free(set);

  ASSERT_EQ(19, allocator.total);
  ASSERT_EQ(0, allocator.live);
}
