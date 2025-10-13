#include <ohset.h>
#include <test/test.h>

static void test_ceil_pow_2(void) {

  for (uint32_t pow = 16; pow <= 2048; pow = pow * 2) {
    for (uint32_t i = ((pow / 2) + 1); i < pow; ++i) {

      uint32_t v = i; // The input number
      --v;
      v |= v >> 1;
      v |= v >> 2;
      v |= v >> 4;
      v |= v >> 8;
      v |= v >> 16;
      ++v;
      ASSERT_EQ(pow, v);
    }
  }
}

TEST(shrink) {

  test_ceil_pow_2();

  ASSERT_EQ(0, ohset_shrink(NULL));
  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(size_t),
  });

  ASSERT_NON_NULL(set);
  ASSERT_EQ(0, ohset_shrink(NULL));

  for (size_t i = 0; i < 100; ++i) {
    ASSERT(ohset_add(set, &i));
  }

  ASSERT_EQ(1152, ohset_shrink(set));

  ohset_free(set);
}
