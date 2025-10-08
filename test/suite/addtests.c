#include <ohset.h>
#include <test/test.h>

TEST(add) {

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(size_t),
  });
  ASSERT_NON_NULL(set);

  for (size_t i = 42; i < 52; ++i) {
    ASSERT(ohset_add(set, &i));
    ASSERT_FALSE(ohset_add(set, &i));
    ASSERT_EQ(i, *(size_t *)ohset_get(set, &i));
  }

  ohset_free(set);
}