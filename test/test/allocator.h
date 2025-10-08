#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h> // size_t
#include <stdint.h> // int32_t

typedef struct allocator_t {

  /// @brief Max number of times this allocator will return non-null
  int32_t max_times;

  /// @brief Number of live allocations
  int32_t live;

  /// @brief Number of total allocations
  int32_t total;

} allocator_t;

void *alloc(void *ctx, void *ptr, size_t size);

#endif