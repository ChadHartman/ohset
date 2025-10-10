# Open-addressing Hash Set

`ohset` is a pure-c hash set implementation utilizing open addressing. 

## Sample Usages

### Integer Set

```c
#include <assert.h>
#include <ohset.h>

int main() {

  ohset_t *restrict set = ohset_new(&(ohset_config_t){
      .item_size = sizeof(int),
  });

  int value = 9;
  ohset_add(set, &value);
  assert(NULL != ohset_get(set, &value));

  value = 1;
  ohset_add(set, &value);
  assert(NULL != ohset_get(set, &value));

  value = 12;
  ohset_add(set, &value);
  assert(NULL != ohset_get(set, &value));

  value = 4;
  ohset_add(set, &value);
  assert(NULL != ohset_get(set, &value));

  size_t i = 0;
  const int expected[] = {1, 9, 12, 4};
  for (ohset_iter_t *iter = ohset_iter(set);
       iter != NULL;
       iter = ohset_iter_next(iter)) {

    const int actual = *(int *)ohset_iter_value(iter);
    assert(expected[i++] == actual);
  }

  return 0;
}
```

### Custom Allocator

```c
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
  char *res = allocator_alloc(allocator, NULL, strlen(src) + 1);
  strcpy(res, src);
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
```

### Dictionary Implementation

```c
```