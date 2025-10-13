#include <ohset.h>
#include <test/test.h>

TEST(shrink) {

  ASSERT_EQ(0, ohset_shrink(NULL));
  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(size_t),
  });

  ASSERT_NON_NULL(set);
  ASSERT_EQ(0, ohset_shrink(NULL));

  for (size_t i = 42; i < 52; ++i) {
    ASSERT(ohset_add(set, &i));
  }

  ASSERT_EQ(54, ohset_shrink(set));

  ohset_free(set);
}
