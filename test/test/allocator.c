#include <stdlib.h> // malloc, realloc, free
#include <string.h> // strlen

#include <test/test.h>

#include "allocator.h"

int alloc_record_cmp(const void *a, const void *b) {
  const uintptr_t lhs = (uintptr_t)((alloc_record_t *)a)->address;
  const uintptr_t rhs = (uintptr_t)((alloc_record_t *)b)->address;
  if (lhs == rhs) {
    return 0;
  }
  return lhs < rhs ? -1 : 1;
}

static void allocator_record(
    allocator_t *restrict allocator,
    void *restrict address,
    size_t size) {

  if (address == NULL) {
    return;
  }

  alloc_record_t key = {
      .address = address,
      .live = size != 0,
      .size = size,
  };

  alloc_record_t *found = bsearch(
      &key,
      allocator->records,
      allocator->record_count,
      sizeof(alloc_record_t),
      alloc_record_cmp);

  if (found == NULL) {
    allocator->records = realloc(allocator->records, sizeof(alloc_record_t) * ++allocator->record_count);
    allocator->records[allocator->record_count - 1] = key;
    qsort(allocator->records, allocator->record_count, sizeof(alloc_record_t), alloc_record_cmp);
    if (key.live) {
      TEST_LOG("Allocated %p sized %zu", address, size);
    } else {
      printf("Recorded free %p not previously present\n", address);
    }
  } else if (size == 0) {
    TEST_LOG("Freed %p", address);
    found->live = false;
  } else {
    TEST_LOG("Allocated %p sized %zu", address, size);
    found->live = true;
    found->size = size;
  }
}

void *allocator_alloc(void *ctx, void *ptr, size_t size) {

  allocator_t *restrict a = ctx;
  if (size == 0) {
    if (ptr == NULL) {
      return NULL;
    }

    --a->live;
    allocator_record(a, ptr, 0);
    free(ptr);
    return NULL;
  }

  if (ptr == NULL) {
    if (a->max_times != 0 && (a->total + 1) > a->max_times) {
      return NULL;
    }
    ++a->live;
    ++a->total;
    void *restrict res = malloc(size);
    allocator_record(a, res, size);
    return res;
  }

  allocator_record(a, ptr, 0);
  void *restrict res = realloc(ptr, size);
  allocator_record(a, res, size);
  return res;
}

char *allocator_strdup(allocator_t *restrict allocator, const char *restrict src) {

  if (allocator == NULL || src == NULL) {
    return NULL;
  }

  const size_t size = strlen(src) + 1;
  char *res = allocator_alloc(allocator, NULL, size);
  if (res == NULL) {
    return NULL;
  }

  memcpy(res, src, size);

  return res;
}

void allocator_dtor(allocator_t *restrict allocator) {

  for (size_t i = 0; i < allocator->record_count; ++i) {
    if (allocator->records[i].live) {
      printf("%p sized %zu leaked\n", allocator->records[i].address, allocator->records[i].size);
    }
  }

  free(allocator->records);
  memset(allocator, 0, sizeof(allocator_t));
}