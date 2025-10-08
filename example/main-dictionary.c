#include <assert.h>
#include <ohset.h>
#include <stdlib.h> // EXIT_SUCCESS
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
  return EXIT_SUCCESS;
}