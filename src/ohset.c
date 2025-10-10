#include <inttypes.h> // PRIu32
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

/// @brief Bucket flag to indicate it has never been used
#define OHSET_BUCKET_NULL ((uint8_t)0U)

/// @brief Bucket flag indicating it is populated
#define OHSET_BUCKET_POPULATED ((uint8_t)1U)

/// @brief Bucket flag indicating that is was once populated; but now vacated.
///   This communicates that the bucket is available for writing but open
///   address chaining should continue
#define OHSET_BUCKET_TOMBSTONED ((uint8_t)2U)

/// @brief Indicate that there are no buckets available
#define OHSET_BUCKET_AT_CAPACITY ((uint8_t)3U)

struct ohset_iter_t {

  /// @brief Owning set
  ohset_t *set;

  /// @brief Current bucket offset
  uint32_t index;

  /// @brief Flag indicating no modifications occurred
  bool valid;
};

struct ohset_t {

  /// @brief Client-provided configuration
  ohset_config_t config;

  /// @brief Number of stored items
  uint32_t item_count;

  /// @brief Pointer to bucket raw bytes
  uint8_t *buckets;

  /// @brief Number of buckets currently allocated
  uint32_t bucket_count;

  /// @brief Preallocated iterator to return
  ohset_iter_t iter;
};

/// @brief Ephemeral bucket struct
typedef struct ohset_bucket_t {
  uint8_t *state;
  uint8_t *value;
} ohset_bucket_t;

/// @brief MurmurHash3 scramble function
static inline uint32_t ohset_hash_scramble(uint32_t k) {
  k *= 0xcc9e2d51;
  k = (k << 15) | (k >> 17);
  k *= 0x1b873593;
  return k;
}

/// @brief Default allocator to use when none was provided
/// @param ctx unused
/// @param ptr pointer to free
/// @param size to malloc
/// @return pointer to the malloc'd block or NULL if size was 0
static void *ohset_default_alloc(void *ctx, void *ptr, size_t size) {

  (void)ctx;

  if (size == 0) {
    free(ptr);
    return NULL;
  }

  // We never realloc
  return malloc(size);
}

/// @brief Set the value in the provided bucket
/// @param bucket to update
/// @param value to write; or NULL to tombstone
/// @param item_size size of the item to write
static void ohset_bucket_set(
    ohset_bucket_t *restrict bucket,
    const void *restrict value,
    size_t item_size) {

  if (value == NULL) {
    *bucket->state = OHSET_BUCKET_TOMBSTONED;
  } else {
    *bucket->state = OHSET_BUCKET_POPULATED;
    memcpy(bucket->value, value, item_size);
  }
}

/// @brief Retrieve a bucket by index; this will always return a populated bucket
/// @param buckets to search
/// @param item_size in bytes of a single item
/// @param idx bucket offset
/// @return the corresponding bucket
static ohset_bucket_t ohset_bucket_idx(
    const uint8_t *restrict buckets,
    size_t item_size,
    uint32_t idx) {

  const uint8_t *bucket = buckets + (idx * (item_size + sizeof(uint8_t)));

  return (ohset_bucket_t){
      .value = (uint8_t *)bucket,
      .state = (uint8_t *)(bucket + item_size),
  };
}

/// @brief Retrieve a bucket by value; possibly returning an invalid bucket if
///   there were none available
/// @param set instance
/// @param value to search
/// @param writable when true; NULL and TOMBSTONED buckets are returned; when
///   false NULL or POPULATED may be returned
/// @return Corresponding bucket (NOTE: it's fields may be NULL if there
///   weren't enough buckets)
static ohset_bucket_t ohset_bucket_val(
    const ohset_t *restrict set,
    const void *restrict value,
    bool writable) {

  static uint8_t at_capacity = OHSET_BUCKET_AT_CAPACITY;

  if (set->bucket_count == 0) {
    return (ohset_bucket_t){.state = &at_capacity};
  }

  const uint32_t digest = set->config.item_hash == NULL
                              ? ohset_hash(value, set->config.item_size)
                              : set->config.item_hash(value);
  uint32_t idx = digest % set->bucket_count;

  for (uint32_t i = 0; i < set->bucket_count; ++i) {

    ohset_bucket_t bucket = ohset_bucket_idx(set->buckets, set->config.item_size, idx);

    switch (*bucket.state) {
    case OHSET_BUCKET_NULL:
      // Acceptable for either state
      return bucket;

    case OHSET_BUCKET_POPULATED: {
      const int res = set->config.item_cmp == NULL
                          ? memcmp(value, bucket.value, set->config.item_size)
                          : set->config.item_cmp(value, bucket.value);
      if (res == 0) {
        return bucket;
      }
    } break;

    case OHSET_BUCKET_TOMBSTONED:
      if (writable) {
        return bucket;
      }
      // Need to keep looking, could be another open-addressed value
      break;

    default:
      OHSET_ABORT("Unreachable section reached");
      return (ohset_bucket_t){0};
    }

    // Linear search
    idx = (idx + 1) % set->bucket_count;
  }

  // Load factor must be 1; all buckets are filled
  return (ohset_bucket_t){.state = &at_capacity};
}

/// @brief Allocate the number of bucket provided and migrate existing items
///   to the new bucket collection
/// @param set instance
/// @param new_bucket_count number of buckets to allocate
/// @return true on success; false on allocation failure
static bool ohset_rehash(ohset_t *restrict set, uint32_t new_bucket_count) {

  const size_t new_size = new_bucket_count * (set->config.item_size + sizeof(uint8_t));
  uint8_t *restrict new_buckets = set->config.alloc(set->config.alloc_ctx, NULL, new_size);

  if (new_buckets == NULL) {
    OHSET_ABORT("Failed to reallocate buckets; allocator returned NULL");
    return false;
  }

  const uint32_t old_bucket_count = set->bucket_count;
  uint8_t *restrict old_buckets = set->buckets;
  set->bucket_count = new_bucket_count;
  set->buckets = new_buckets;
  memset(set->buckets, 0, new_size);

  for (uint32_t i = 0; i < old_bucket_count; ++i) {
    const ohset_bucket_t src = ohset_bucket_idx(old_buckets, set->config.item_size, i);
    if (*src.state == OHSET_BUCKET_POPULATED) {
      ohset_bucket_t dst = ohset_bucket_val(set, src.value, true);
      ohset_bucket_set(&dst, src.value, set->config.item_size);
    }
  }

  set->config.alloc(set->config.alloc_ctx, old_buckets, 0);
  return true;
}

OHSET_API ohset_t *ohset_new(const ohset_config_t *restrict config) {

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
      .iter.set = set,
  };

  set->config.alloc = alloc;

  if (config->load_factor <= 0.0f) {
    set->config.load_factor = 0.5f;
  } else if (config->load_factor > 1.0f) {
    set->config.load_factor = 1.0f;
  }

  return set;
}

OHSET_API uint32_t ohset_count(const ohset_t *restrict set) {

  if (set == NULL) {
    OHSET_ABORT("ohset_count(NULL) was called");
    return 0;
  }

  return set->item_count;
}

OHSET_API const void *ohset_get(const ohset_t *restrict set, const void *restrict value) {

  if (set == NULL) {
    OHSET_ABORT("ohset_get(NULL, ...) was called");
    return false;
  }

  if (value == NULL) {
    OHSET_ABORT("ohset_get(ohset_t@%p, NULL) was called", set);
    return false;
  }

  ohset_bucket_t bucket = ohset_bucket_val(set, value, false);

  return *bucket.state == OHSET_BUCKET_POPULATED ? bucket.value : NULL;
}

OHSET_API bool ohset_add(ohset_t *restrict set, const void *restrict value) {

  if (set == NULL) {
    OHSET_ABORT("ohset_add(NULL, ...) was called");
    return false;
  }

  if (value == NULL) {
    OHSET_ABORT("ohset_add(ohset_t@%p, NULL) was called", set);
    return false;
  }

  ohset_bucket_t bucket = ohset_bucket_val(set, value, true);
  if (OHSET_BUCKET_POPULATED == *bucket.state) {
    return false;
  }

  set->iter.valid = false;
  const float load_factor = set->bucket_count == 0
                                ? 2.0f
                                // +1 for the new item
                                : ((float)(set->item_count + 1) / (float)(set->bucket_count));

  if (load_factor > set->config.load_factor) {
    const uint32_t new_bucket_count = set->bucket_count == 0 ? 8 : set->bucket_count * 2;
    if (!ohset_rehash(set, new_bucket_count)) {
      return false;
    }
    bucket = ohset_bucket_val(set, value, true);
  }

  ohset_bucket_set(&bucket, value, set->config.item_size);
  ++set->item_count;
  return true;
}

OHSET_API void ohset_put(ohset_t *restrict set, const void *restrict value) {

  if (ohset_add(set, value)) {
    return;
  }

  ohset_remove(set, value);
  ohset_add(set, value);
}

OHSET_API bool ohset_remove(ohset_t *restrict set, const void *restrict value) {

  if (set == NULL) {
    OHSET_ABORT("ohset_remove(NULL, ...) was called");
    return false;
  }

  if (value == NULL) {
    OHSET_ABORT("ohset_remove(ohset_t@%p, NULL) was called", set);
    return false;
  }

  ohset_bucket_t bucket = ohset_bucket_val(set, value, false);

  if (*bucket.state != OHSET_BUCKET_POPULATED) {
    // nothing to remove
    return false;
  }

  set->iter.valid = false;
  if (set->config.item_dtor) {
    set->config.item_dtor(
        set->config.alloc_ctx,
        set->config.alloc,
        bucket.value);
  }

  ohset_bucket_set(&bucket, NULL, set->config.item_size);
  --set->item_count;
  return true;
}

OHSET_API ohset_iter_t *ohset_iter(const ohset_t *restrict set) {

  if (set == NULL) {
    OHSET_ABORT("ohset_iter(NULL) was called");
    return false;
  }

  ohset_iter_t *restrict iter = (ohset_iter_t *)&set->iter;
  for (iter->index = 0; iter->index < set->bucket_count; ++iter->index) {
    ohset_bucket_t bucket = ohset_bucket_idx(set->buckets, set->config.item_size, iter->index);
    if (*bucket.state == OHSET_BUCKET_POPULATED) {
      iter->valid = true;
      return iter;
    }
  }

  iter->valid = false;
  return NULL;
}

OHSET_API ohset_iter_t *ohset_iter_next(ohset_iter_t *restrict iter) {

  if (iter == NULL) {
    OHSET_ABORT("ohset_iter_next(NULL) was called");
    return NULL;
  }

  if (!iter->valid) {
    OHSET_ABORT("Invalid iterator provided");
    return NULL;
  }

  for (++iter->index; iter->index < iter->set->bucket_count; ++iter->index) {
    ohset_bucket_t bucket = ohset_bucket_idx(iter->set->buckets, iter->set->config.item_size, iter->index);
    if (*bucket.state == OHSET_BUCKET_POPULATED) {
      return iter;
    }
  }

  iter->valid = false;
  return NULL;
}

OHSET_API const void *ohset_iter_value(ohset_iter_t *restrict iter) {

  if (iter == NULL) {
    OHSET_ABORT("ohset_iter_value(NULL) was called");
    return NULL;
  }

  if (!iter->valid) {
    OHSET_ABORT("Invalid iterator provided");
    return NULL;
  }

  return ohset_bucket_idx(iter->set->buckets, iter->set->config.item_size, iter->index).value;
}

OHSET_API void ohset_clear(ohset_t *restrict set) {

  if (set == NULL) {
    OHSET_ABORT("ohset_clear(NULL) was called");
    return;
  }

  for (uint32_t i = 0; i < set->bucket_count; ++i) {
    ohset_bucket_t bucket = ohset_bucket_idx(set->buckets, set->config.item_size, i);
    if (*bucket.state == OHSET_BUCKET_POPULATED) {
      ohset_remove(set, bucket.value);
    }
  }
}

OHSET_API size_t ohset_shrink(ohset_t *restrict set) {

  if (set == NULL) {
    OHSET_ABORT("ohset_shrink(NULL) was called");
    return 0;
  }

  const size_t bucket_size = sizeof(uint8_t) + set->config.item_size;
  const size_t current_size = set->bucket_count * bucket_size;
  ohset_rehash(set, set->item_count);
  const size_t shrink_size = set->bucket_count * bucket_size;
  return current_size - shrink_size;
}

OHSET_API void ohset_free(ohset_t *restrict set) {

  if (set == NULL) {
    return;
  }

  ohset_clear(set);

  set->config.alloc(set->config.alloc_ctx, set->buckets, 0);
  set->config.alloc(set->config.alloc_ctx, set, 0);
}

OHSET_API uint32_t ohset_hash(const void *restrict ptr, size_t len) {

  if (ptr == NULL) {
    return 0;
  }

  const uint8_t *restrict key = ptr;
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