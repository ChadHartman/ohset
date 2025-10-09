#include <ohset.h>
#include <test/test.h>

typedef struct item_t {
  uint32_t key;
  size_t value;
} item_t;

static int item_cmp(const void *a, const void *b) {
  const item_t *restrict lhs = a;
  const item_t *restrict rhs = b;
  if (lhs->key == rhs->key) {
    return 0;
  }
  return lhs->key < rhs->key ? -1 : 1;
}

static uint32_t item_hash(const void *ptr) {
  const item_t *restrict item = ptr;
  return item->key;
}

TEST(put) {

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_cmp = item_cmp,
      .item_hash = item_hash,
      .item_size = sizeof(item_t),
  });

  item_t item = {.key = 42, .value = 52};

  ASSERT_NON_NULL(set);
  ASSERT_NULL(ohset_get(set, &item));

  ohset_put(set, &item);
  ASSERT_EQ(52, ((const item_t *)ohset_get(set, &item))->value);

  item.value = 62;
  ohset_put(set, &item);
  ASSERT_EQ(62, ((const item_t *)ohset_get(set, &item))->value);

  ohset_free(set);
}