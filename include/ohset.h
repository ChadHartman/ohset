#ifndef OHSET_H
#define OHSET_H

#include <stdbool.h>
#include <stddef.h> // size_t
#include <stdint.h> // uint8_t

/// @brief A collection of distinct elements implemented by an open-addressing hash set
typedef struct ohset_t ohset_t;

/// @brief A Set iterator
typedef struct ohset_iter_t ohset_iter_t;

/// @brief Set of configuration for a set instance
typedef struct ohset_config_t {

  /// @brief OPTIONAL Custom allocator function
  /// @param alloc_ctx the provided alloc_ctx
  /// @param ptr pointer to free or reallocate (or NULL to malloc)
  /// @param size the size of the pointer to malloc or realloc; when 0 the
  ///   provided ptr should be freed
  /// @return the malloc or realloc'd block or NULL if size was zero
  void *(*alloc)(void *, void *, size_t);

  /// @brief OPTIONAL Allocator context to pass into the alloc function
  void *alloc_ctx;

  /// @brief OPTIONAL Item comparator; when absent @see memcmp is utilized
  int (*item_cmp)(const void *, const void *);

  /// @brief OPTIONAL Item destructor; called before item removal
  /// @param alloc_ctx the provided alloc_ctx
  /// @param alloc the provided alloc function or the default one if none was
  ///   provided
  /// @param ptr the pointer to destruct NOTE: the pointer itself is a row in
  ///   the set; and itself should not be freed as it is managed by the ohset
  void (*item_dtor)(void *, void *(*alloc)(void *, void *, size_t), void *);

  uint32_t (*item_hash)(const void *);

  uint32_t item_size;

  float load_factor;

} ohset_config_t;

ohset_t *ohset_new(const ohset_config_t *restrict config);

uint32_t ohset_count(const ohset_t *restrict set);

const void *ohset_get(const ohset_t *restrict set, const void *restrict value);

bool ohset_add(ohset_t *restrict set, const void *restrict value);

void ohset_put(ohset_t *restrict set, const void *restrict value);

bool ohset_remove(ohset_t *restrict set, const void *restrict value);

void ohset_clear(ohset_t *restrict set);

void ohset_free(ohset_t *restrict set);

uint32_t ohset_shrink(ohset_t *restrict set);

ohset_iter_t *ohset_iter(const ohset_t *restrict set);

ohset_iter_t *ohset_iter_next(ohset_iter_t *restrict iter);

const void *ohset_iter_value(ohset_iter_t *restrict iter);

uint32_t ohset_hash(const void *restrict key, size_t len);

#endif