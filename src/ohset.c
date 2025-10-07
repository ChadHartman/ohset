#include <ohset.h>
#include <string.h> // memcpy

static inline uint32_t ohset_hash_scramble(uint32_t k) {
  k *= 0xcc9e2d51;
  k = (k << 15) | (k >> 17);
  k *= 0x1b873593;
  return k;
}

uint32_t ohset_hash(const uint8_t *restrict key, size_t len) {

  if (key == NULL) {
    return 0;
  }

  uint32_t h = 0;
  uint32_t k;

  for (size_t i = len >> 2; i; i--) {
    memcpy(&k, key, sizeof(uint32_t));
    key += sizeof(uint32_t);
    h ^= ohset_hash_scramble(k);
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;
  }

  k = 0;
  for (size_t i = len & 3; i; i--) {
    k <<= 8;
    k |= key[i - 1];
  }

  h ^= ohset_hash_scramble(k);
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}