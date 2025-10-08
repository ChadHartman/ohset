#include <stdlib.h> // malloc, realloc, free
#include <string.h> // strlen

#include "allocator.h"

void *alloc(void *ctx, void *ptr, size_t size) {

  allocator_t *restrict a = ctx;
  if (size == 0) {
    --a->live;
    free(ptr);
    return NULL;
  }

  if (ptr == NULL) {
    if (a->max_times != 0 && (a->total + 1) > a->max_times) {
      return NULL;
    }
    ++a->live;
    ++a->total;
    return malloc(size);
  }

  return realloc(ptr, size);
}

char *alloc_strdup(allocator_t *restrict allocator, const char *restrict src) {

  if (allocator == NULL || src == NULL) {
    return NULL;
  }

  char *res = malloc(strlen(src) + 1);
  if (res == NULL) {
    return NULL;
  }

  strcpy(res, src);
  return res;
}