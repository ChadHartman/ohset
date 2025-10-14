#include <ohset.h>
#include <test/test.h>

TEST(get) {
  ASSERT_EQ(0, ohset_get((ohset_t *)&(ohset_config_t){0}, NULL));
  ASSERT_NULL(ohset_get(NULL, NULL));

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(size_t),
  });
  ASSERT_NON_NULL(set);
  ASSERT_NULL(ohset_get(set, NULL));

  ohset_free(set);
}
