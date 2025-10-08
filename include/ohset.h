#ifndef OHSET_H
#define OHSET_H

#include <stdbool.h>
#include <stddef.h> // size_t
#include <stdint.h> // uint8_t

/// @brief A collection of distinct elements implemented by an open-addressing hash set
typedef struct ohset_t ohset_t;

/// @brief A Set iterator
typedef struct ohset_iter_t ohset_iter_t;

typedef struct ohset_config_t {

  void *(*alloc)(void *, void *, size_t);

  void *alloc_ctx;

  int (*cmp)(const void *, const void *);

  void (*item_dtor)(void *, void *, void *(*alloc)(void *, void *, size_t));

  uint32_t (*hash)(const void *);

  uint32_t item_size;

  float load_factor;

} ohset_config_t;

ohset_t *ohset_new(const ohset_config_t *restrict config);

const void *ohset_get(const ohset_t *restrict set, const void *restrict value);

bool ohset_add(ohset_t *restrict set, const void *restrict value);

bool ohset_remove(ohset_t *s, const void *value);

void ohset_clear(ohset_t *s);

void ohset_free(ohset_t *restrict set);

ohset_iter_t *ohset_iter(ohset_t *s);

ohset_iter_t *ohset_iter_next(ohset_iter_t *restrict iter);

const void *ohset_iter_value(ohset_iter_t *restrict iter);

uint32_t ohset_hash(const void *restrict key, size_t len);

#endif