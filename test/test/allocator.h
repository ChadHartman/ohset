#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h> // size_t
#include <stdint.h> // int32_t

typedef struct alloc_record_t {
  void *address;
  size_t size;
  bool live;
} alloc_record_t;

typedef struct allocator_t {

  /// @brief Max number of times this allocator will return non-null
  int32_t max_times;

  /// @brief Number of live allocations
  int32_t live;

  /// @brief Number of total allocations
  int32_t total;

  alloc_record_t *records;
  size_t record_count;

} allocator_t;

void *allocator_alloc(void *ctx, void *ptr, size_t size);

char *allocator_strdup(allocator_t *restrict allocator, const char *restrict src);

void allocator_dtor(allocator_t *restrict allocator);

#endif
