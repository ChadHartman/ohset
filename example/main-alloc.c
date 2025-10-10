#include <assert.h>
#include <ohset.h>
#include <stdlib.h> // malloc, free, realloc
#include <string.h> // strdup, strcmp

typedef char *string_t;
typedef struct allocator_t {
  int total;
  int live;
} allocator_t;

static void *allocator_alloc(void *alloc_ctx, void *ptr, size_t size) {
  allocator_t *restrict allocator = alloc_ctx;

  if (size == 0) {
    if (ptr == NULL) {
      return NULL;
    }
    --allocator->live;
    free(ptr);
    return NULL;
  }

  if (ptr == NULL) {
    ++allocator->live;
    ++allocator->total;
    return malloc(size);
  }

  return realloc(ptr, size);
}

static char *allocator_strdup(allocator_t *restrict allocator, const char *restrict src) {
  const size_t size = strlen(src) + 1;
  char *res = allocator_alloc(allocator, NULL, size);
  memcpy(res, src, size);
  return res;
}

static int string_cmp(const void *a, const void *b) {
  const string_t *lhs = a;
  const string_t *rhs = b;
  return strcmp(*lhs, *rhs);
}

static uint32_t string_hash(const void *ptr) {
  const string_t *str = ptr;
  return ohset_hash(*str, strlen(*str));
}

static void string_dtor(
    void *alloc_ctx,
    void *(*alloc)(void *, void *, size_t),
    void *ptr) {

  string_t *str = ptr;
  alloc(alloc_ctx, *str, 0);
}

int main() {

  allocator_t allocator = {0};
  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .alloc = allocator_alloc,
      .alloc_ctx = &allocator,
      .item_cmp = string_cmp,
      .item_dtor = string_dtor,
      .item_hash = string_hash,
      .item_size = sizeof(string_t),
  });

  string_t value = allocator_strdup(&allocator, "alpha");
  assert(NULL == ohset_get(set, &value));

  ohset_add(set, &value);

  assert(0 == strcmp("alpha", *(string_t *)ohset_get(set, &value)));

  ohset_free(set);
  assert(3 == allocator.total);
  assert(0 == allocator.live);

  return 0;
}