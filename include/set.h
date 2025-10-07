#ifndef SET_H
#define SET_H

#include <stdbool.h>

/// @brief A collection of distinct elements backed by a hashset
typedef struct set_t set_t;

/// @brief A Set iterator
typedef struct set_iter_t set_iter_t;

typedef struct set_config_t {

  void *(*alloc)(void *, void *, size_t);
  void *alloc_ctx;

  size_t item_size;

  int (*cmp)(const void *, const void *);

  size_t (*hash)(const void *);

  void (*destructor)(void *, void *, void *(*alloc)(void *, void *, size_t));

} set_config_t;

set_t *set_create(const set_config_t *restrict config);

const void *set_get(set_t *set, const void *value);

bool set_add(set_t *s, const void *value);

bool set_put(set_t *s, const void *value);

bool set_remove(set_t *s, const void *value);

void set_clear(set_t *s);

void set_free(set_t *s);

set_iter_t *set_iter(set_t *s);

set_iter_t *set_iter_next(set_iter_t *restrict iter);

const void *set_iter_value(set_iter_t *restrict iter);

#endif