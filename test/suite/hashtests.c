#include <ohset.h>
#include <test/test.h>

static uint32_t test_hash_str(const char *restrict str) {
  return ohset_hash((uint8_t *)str, strlen(str));
}

TEST(hash) {
  ASSERT_EQ(0, ohset_hash(NULL, 0));
  ASSERT_EQ(0, test_hash_str(""));
  ASSERT_EQ(0xba6bd213, test_hash_str("test"));
  ASSERT_EQ(0x2e4ff723, test_hash_str("The quick brown fox jumps over the lazy dog"));
}
