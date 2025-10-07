#include <set.h>

#include <assert.h>
#include <stdio.h>
#include <string.h> // memset

#include <app/log/log.h>
#include <app/util/magic.h>

#include "set.h"

/* === constants === */

/* === typedefs === */

/// @brief Enum determining a bucket's state
typedef enum { BUCKET_NULL,
               BUCKET_POPULATED,
               BUCKET_TOMBSTONED } BucketState;

/// @brief Returnable bucket with meta-data
typedef struct Bucket {
  byte_t *data;
  BucketState *state;
} Bucket;

/// @brief Iterator mode, determines behavious
typedef enum {
  /// @brief  Stop only at values & end of buckets
  ITERATOR_VALUES,
  /// @brief  Stop only at buckets & end of buckets
  ITERATOR_BUCKETS,
  /// @brief Set has been mutated; Set_iterator needs to be called again
  ITERATOR_INVALIDATED
} IteratorMode;

struct SetIterator {
  Set *set;
  size_t index;
  IteratorMode mode;
};

struct Set {
  MAGIC_DECL()
  bool destroyed;
  Allocator *alloc;
  size_t itemSize;
  Hasher hasher;
  Comparator comparator;
  Destructor destructor;

  size_t itemCount;
  byte_t *buckets;
  size_t bucketSize;
  size_t bucketCount;

  SetIterator iterator;
};

/* === private methods === */

/// @brief Resize the set's buckets, this should be called if the new load factor > 0.5
/// @param set instance
static void Set_rehash(Set *set) {

  size_t oldBucketCount = set->bucketCount;
  byte_t *oldBuckets = set->buckets;

  set->bucketCount = oldBucketCount == 0 ? 8 : set->bucketCount * 2;
  size_t newSize = set->bucketCount * set->bucketSize;
  set->buckets = MALLOC(set->alloc, newSize);
  set->itemCount = 0;
  memset(set->buckets, 0, newSize);

  for (size_t i = 0; i < oldBucketCount; ++i) {
    byte_t *bucket = oldBuckets + (i * set->bucketSize);
    BucketState flag = *((BucketState *)(bucket + set->itemSize));
    if (flag == BUCKET_POPULATED) {
      Set_add(set, bucket);
    }
  }

  FREE(set->alloc, oldBuckets);
}

/// @brief Retrieve the bucket for the provided value
/// @param set instance
/// @param value to lookup
/// @param writable indicate that we can include tombstones (for writing)
/// @return bucket with data or nulled fields if there's no buckets
static Bucket Set_getBucket(Set *set, const void *value, bool writable) {

  Bucket res = {NULL, NULL};

  const size_t bucketCount = set->bucketCount;

  if (bucketCount == 0) {
    return res;
  }

  const size_t bucketSize = set->bucketSize;
  const size_t itemSize = set->itemSize;
  size_t idx = set->hasher(value) % bucketCount;

  while (true) {

    res.data = set->buckets + (idx * bucketSize);
    res.state = (BucketState *)(res.data + itemSize);

    switch (*res.state) {
    case BUCKET_NULL:
      // Acceptable for either state
      return res;

    case BUCKET_POPULATED:
      if (set->comparator(res.data, value) == 0) {
        return res;
      }
      break;

    case BUCKET_TOMBSTONED:
      if (writable) {
        return res;
      }
      // Need to keep looking, could be another value
      break;

    default:
      assert(false);
      break;
    }

    // Linear search
    idx = (idx + 1) % bucketCount;
  }
}

/* === public methods === */

Set *Set_create(
    Allocator *alloc,
    size_t itemSize,
    Hasher hasher,
    Comparator comparator,
    Destructor destructor) {

  assert(alloc);
  assert(itemSize);
  assert(hasher);
  assert(comparator);

  Set *s;
  s = MALLOC(alloc, sizeof(Set));
  MAGIC_INIT(Set, s);

  s->destroyed = false;
  s->alloc = alloc;
  s->itemSize = itemSize;
  s->hasher = hasher;
  s->comparator = comparator;
  s->destructor = destructor;

  s->itemCount = 0;
  s->buckets = NULL;
  s->bucketCount = 0;
  s->bucketSize = itemSize + sizeof(BucketState);
  s->iterator.set = s;

  return s;
}

const void *Set_get(Set *s, const void *value) {

  MAGIC_ASSERT(Set, s);
  assert(value);
  assert(!s->destroyed);

  Bucket bucket = Set_getBucket(s, value, false);
  return (bucket.state != NULL && *bucket.state == BUCKET_POPULATED) ? bucket.data : NULL;
}

bool Set_add(Set *s, const void *value) {

  MAGIC_ASSERT(Set, s);
  assert(value);
  assert(!s->destroyed);

  Bucket bucket = Set_getBucket(s, value, true);
  if (bucket.state != NULL && *bucket.state == BUCKET_POPULATED) {
    return false;
  }

  s->iterator.mode = ITERATOR_INVALIDATED;
  const float loadFactor = s->bucketCount == 0 ? 1.0f : ((float)(s->itemCount + 1) / (float)(s->bucketCount));

  if (loadFactor > 0.5f) {
    Set_rehash(s);
    bucket = Set_getBucket(s, value, true);
  }

  memcpy(bucket.data, value, s->itemSize);
  *bucket.state = BUCKET_POPULATED;
  ++s->itemCount;
  return true;
}

bool Set_remove(Set *s, const void *value) {

  MAGIC_ASSERT(Set, s);
  assert(value);
  assert(!s->destroyed);

  Bucket bucket = Set_getBucket(s, value, false);

  if (bucket.state == NULL || *bucket.state == BUCKET_NULL || *bucket.state == BUCKET_TOMBSTONED) {
    // nothing to remove
    return false;
  }

  s->iterator.mode = ITERATOR_INVALIDATED;

  if (s->destructor) {
    s->destructor(s->alloc, bucket.data);
  }

  *bucket.state = BUCKET_TOMBSTONED;
  --s->itemCount;
  return true;
}

size_t Set_count(const Set *s) {
  MAGIC_ASSERT(Set, s);
  assert(!s->destroyed);
  return s->itemCount;
}

size_t Set_bucketCount(const Set *s) {
  MAGIC_ASSERT(Set, s);
  assert(!s->destroyed);
  return s->bucketCount;
}

SetIterator *Set_iterator(Set *s) {

  MAGIC_ASSERT(Set, s);
  assert(!s->destroyed);

  SetIterator *i = Set_bucketIterator(s);

  for (;
       i != NULL && SetIterator_value(i) == NULL;
       i = SetIterator_next(i)) {
    ; // advance
  }

  if (i) {
    i->mode = ITERATOR_VALUES;
  }

  return i;
}

SetIterator *Set_bucketIterator(Set *s) {

  MAGIC_ASSERT(Set, s);
  assert(!s->destroyed);

  if (s->bucketCount == 0) {
    return NULL;
  }

  s->iterator.index = 0;
  s->iterator.mode = ITERATOR_BUCKETS;
  return &s->iterator;
}

SetIterator *SetIterator_next(SetIterator *i) {

  assert(i);
  assert(i->mode != ITERATOR_INVALIDATED);

  if (++i->index == i->set->bucketCount) {
    // At end of bucket array
    return NULL;
  }

  if (i->mode == ITERATOR_BUCKETS) {
    return i;
  }

  // Value iterator
  byte_t *bucket = i->set->buckets + (i->index * i->set->bucketSize);
  BucketState flag = *((BucketState *)(bucket + i->set->itemSize));
  if (flag == BUCKET_POPULATED) {
    return i; // value found
  }

  return SetIterator_next(i);
}

const void *SetIterator_value(SetIterator *i) {

  assert(i);
  assert(i->mode != ITERATOR_INVALIDATED);

  byte_t *bucket = i->set->buckets + (i->index * i->set->bucketSize);
  BucketState flag = *((BucketState *)(bucket + i->set->itemSize));
  if (flag == BUCKET_POPULATED) {
    return bucket;
  }

  return NULL;
}

bool SetIterator_isTombstoned(SetIterator *i) {

  if (!i) {
    return false;
  }

  assert(i->mode != ITERATOR_INVALIDATED);

  byte_t *bucket = i->set->buckets + (i->index * i->set->bucketSize);
  BucketState flag = *((BucketState *)(bucket + i->set->itemSize));
  return flag == BUCKET_TOMBSTONED;
}

void Set_removeAll(Set *s) {

  MAGIC_ASSERT(Set, s);
  assert(!s->destroyed);

  s->iterator.mode = ITERATOR_INVALIDATED;

  for (size_t i = 0; i < s->bucketCount; ++i) {
    byte_t *bucket = s->buckets + (i * s->bucketSize);
    BucketState *state = (BucketState *)(bucket + s->itemSize);
    if (*state != BUCKET_NULL) {
      if (*state == BUCKET_POPULATED && s->destructor) {
        s->destructor(s->alloc, bucket);
      }
      *state = BUCKET_NULL;
    }
  }

  s->itemCount = 0;
}

void Set_clear(Set *s) {
  Set_removeAll(s);
  FREE(s->alloc, s->buckets);
  s->buckets = NULL;
  s->itemCount = 0;
  s->bucketCount = 0;
}

void Set_destroy(Set *s) {

  if (s) {
    Set_clear(s);
    s->destroyed = true;
  } else {
    return;
  }

  FREE(s->alloc, s);
}
