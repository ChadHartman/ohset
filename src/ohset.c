#include <ohset.h>
#include <stdio.h>  // printf
#include <stdlib.h> // malloc, realloc, free
#include <string.h> // memcpy

#ifdef OHSET_DEBUG
#define OHSET_LOG(...)                                        \
  printf("%s:%d - ", (strrchr(__FILE__, '/') + 1), __LINE__); \
  printf(__VA_ARGS__);                                        \
  printf("\n")
#else // OHSET_DEBUG
#define OHSET_LOG(...) (void)0
#endif // OHSET_DEBUG

#ifdef OHSET_NO_ABORT
#define OHSET_ABORT(...) OHSET_LOG(__VA_ARGS__)
#else // OHSET_NO_ABORT
#define OHSET_ABORT(...)  \
  OHSET_LOG(__VA_ARGS__); \
  abort()
#endif // OHSET_NO_ABORT

struct ohset_t {

  ohset_config_t config;

  uint8_t *buckets;
  size_t bucket_count;
};

static inline uint32_t ohset_hash_scramble(uint32_t k) {
  k *= 0xcc9e2d51;
  k = (k << 15) | (k >> 17);
  k *= 0x1b873593;
  return k;
}

static void *ohset_default_alloc(void *ctx, void *ptr, size_t size) {

  (void)ctx;

  if (size == 0) {
    free(ptr);
    return NULL;
  }

  if (ptr == NULL) {
    return malloc(size);
  }

  return realloc(ptr, size);
}

ohset_t *ohset_new(const ohset_config_t *restrict config) {

  if (config == NULL) {
    OHSET_ABORT("NULL config provided");
    return NULL;
  }

  if (config->item_size == 0) {
    OHSET_ABORT("%zu is not a valid item size", config->item_size);
    return NULL;
  }

  void *(*alloc)(void *, void *, size_t) = config->alloc ? config->alloc : ohset_default_alloc;

  ohset_t *restrict set = alloc(config->alloc_ctx, NULL, sizeof(ohset_t));
  if (set == NULL) {
    OHSET_ABORT("Failed to allocate %zu bytes; allocator returned NULL", sizeof(ohset_t));
    return NULL;
  }

  *set = (ohset_t){
      .config = *config,
  };

  set->config.alloc = alloc;

  return set;
}

void ohset_free(ohset_t *restrict set) {

  if (set == NULL) {
    return;
  }

  set->config.alloc(set->config.alloc_ctx, set, 0);
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