# Open-addressing Hash Set

`ohset` is a pure-c hash set implementation utilizing open addressing with the following features:

* Custom allocator support
* c99 minimum standard
* No dependencies
* Simple import
    * Drag and drop `ohset.h` & `ohset.c`
    * Meson import
* Robust test coverage
    * Lines: 99.3%
    * Functions: 100.0%
    * Branches: 100.0%

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
#include <assert.h>
#include <ohset.h>
#include <stdlib.h> // malloc, free
#include <string.h> // strlen

typedef struct dict_item_t {
  const char *key;
  const char *value;
} dict_item_t;

typedef struct dict_t {
  ohset_t *items;
} dict_t;

static int dict_item_cmp(const void *a, const void *b) {
  const dict_item_t *restrict lhs = a;
  const dict_item_t *restrict rhs = b;
  return strcmp(lhs->key, rhs->key);
}

static void dict_item_dtor(void *alloc_ctx, void *(*alloc)(void *, void *, size_t), void *ptr) {

  (void)alloc_ctx;
  (void)alloc;

  dict_item_t *restrict item = ptr;
  free((void *)item->key);
  free((void *)item->value);
}

static uint32_t dict_item_hash(const void *ptr) {
  const dict_item_t *restrict item = ptr;
  return ohset_hash(item->key, strlen(item->key));
}

static dict_t *dict_new() {

  dict_t *dict = malloc(sizeof(dict_t));
  assert(dict);
  dict->items = ohset_new(&(ohset_config_t){
      .item_cmp = dict_item_cmp,
      .item_dtor = dict_item_dtor,
      .item_hash = dict_item_hash,
      .item_size = sizeof(dict_item_t),
  });
  return dict;
}

static void dict_free(dict_t *restrict dict) {
  ohset_free(dict->items);
  free(dict);
}

static const char *dict_get(const dict_t *restrict dict, const char *restrict key) {
  const dict_item_t item = {.key = key};
  const dict_item_t *restrict found = ohset_get(dict->items, &item);
  return found == NULL ? NULL : found->value;
}

static void dict_put(
    dict_t *restrict dict,
    const char *restrict key,
    const char *restrict value) {

  const dict_item_t item = {
      .key = strdup(key),
      .value = strdup(value),
  };

  ohset_put(dict->items, &item);
}

int main() {

  dict_t *restrict dict = dict_new();

  assert(NULL == dict_get(dict, "alpha"));
  dict_put(dict, "alpha", "beta");
  assert(0 == strcmp("beta", dict_get(dict, "alpha")));
  dict_put(dict, "alpha", "gamma");
  assert(0 == strcmp("gamma", dict_get(dict, "alpha")));

  dict_free(dict);
  return 0;
}
```

## Integration

### Drag & Drop

`./includes/ohset.h` & `./src/ohset.c` may be dropped into a project with no special configuration required.

### Meson

Sample `ohset.wrap`:

```
[wrap-git]
url = https://github.com/ChadHartman/ohset.git
revision = 011943aae66ceb8bf45b1883df27b771f1c631f9
depth = 1

[provide]
ohset = ohset_dep
```

NOTE: Be sure to target your desired revision

## Changelog

* 1.0.2
    * Updated `ohset_clear` to tombstone buckets directly instead of calling `ohset_remove` (and consequently re-finding the bucket)
* 1.0.1
    * Added compilation compliance with `-Wpedantic`
    * Added magic to ensure pointers provided to APIs were authored by the APIs in the first place
    * Converted modulo operations to bitwise operators for improved performance