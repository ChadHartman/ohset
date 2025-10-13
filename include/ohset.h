// MIT License
// Copyright (c) 2025 Chad Hartman
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef OHSET_H
#define OHSET_H

#define OHSET_VERSION_MAJOR 1
#define OHSET_VERSION_MINOR 0
#define OHSET_VERSION_PATCH 0

#define OHSET_VERSION_STRING "1.0.0"

#if (defined(_WIN32) || defined(__CYGWIN__)) && !defined(OHSET_TEST)
#if defined(OHSET_EXPORTS)
#define OHSET_API __declspec(dllexport)

#else
#define OHSET_API __declspec(dllimport)

#endif
#elif defined(__GNUC__)
#define OHSET_API __attribute__((visibility("default")))

#else
#define OHSET_API

#endif

#include <stdbool.h>
#include <stddef.h> // size_t
#include <stdint.h> // uint32_t

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

  /// @brief OPTIONAL hash function; by default @see ohset_hash is used
  uint32_t (*item_hash)(const void *);

  /// @brief REQUIRED size of a hash item
  size_t item_size;

  /// @brief OPTIONAL override load factor > 0.0f and <= 1.0f; 0.5f is used by
  ///   default.
  float load_factor;

} ohset_config_t;

/// @brief Construct a new open-addressing hash set instance
/// @param config used to create the set
/// @return set instance
OHSET_API ohset_t *ohset_new(const ohset_config_t *restrict config);

/// @brief Number of items stored in the set
/// @details uint32_t was chosen (UINT32_MAX number of rows max) because
///   @see ohset_hash returns uint32_t
/// @param set instance
/// @return the number of items stored
OHSET_API uint32_t ohset_count(const ohset_t *restrict set);

/// @brief Retrieve the item from the set whose @see ohset_config_t::item_hash
///   computes the same and @see ohset_config_t::item_cmp
///   returns 0 as the provided value
/// @param set instance
/// @param value to find
/// @return Pointer to the internal item or NULL if not found
OHSET_API const void *ohset_get(const ohset_t *restrict set, const void *restrict value);

/// @brief Add the item to the set; doing so invalidates any provided
///   @see ohset_iter_t
/// @param set instance
/// @param value to add
/// @return true if the item was added or false if
///   @see ohset_config_t::item_hash computes the same and
///   @see ohset_config_t::item_cmp returns 0 as the provided value
OHSET_API bool ohset_add(ohset_t *restrict set, const void *restrict value);

/// @brief Add or clobber an existing item; doing so invalidates any provided
///   @see ohset_iter_t
/// @param set instance
/// @param value to put
OHSET_API void ohset_put(ohset_t *restrict set, const void *restrict value);

/// @brief Remove the item from the set; calling @see ohset_config_t::item_dtor
///   if provided and found; doing so invalidates any provided @see ohset_iter_t
/// @param set instance
/// @param value to remove
/// @return true if the item was removed or false if not found
OHSET_API bool ohset_remove(ohset_t *restrict set, const void *restrict value);

/// @brief Remove all entried (calling @see ohset_config_t::item_dtor) for each
///   one; doing so invalidates any provided @see ohset_iter_t
/// @param set instance
OHSET_API void ohset_clear(ohset_t *restrict set);

/// @brief Destroy the set instance and free all of the utilized memory
/// @param set
OHSET_API void ohset_free(ohset_t *restrict set);

/// @brief Densely compacts the set's contents (malloc only what is needed and
///   freeing the rest)
/// @param set instance
/// @return number of bytes freed
OHSET_API size_t ohset_shrink(ohset_t *restrict set);

/// @brief Retrieve an iterator at the start of the set
/// @param set instance
/// @return an iterator at the first position or NULL if empty
OHSET_API ohset_iter_t *ohset_iter(const ohset_t *restrict set);

/// @brief Advance the iterator to the next item
/// @param iter instance
/// @return an iterator advanced to the next entry or NULL if reached the end
OHSET_API ohset_iter_t *ohset_iter_next(ohset_iter_t *restrict iter);

/// @brief Retrieve the value from the iterator at its current position
/// @param iter instance
/// @return the current value
OHSET_API const void *ohset_iter_value(ohset_iter_t *restrict iter);

/// @brief Utility implementation of Austin Appleby's MurmurHash3
/// @param key value to hash
/// @param len size of the value to hash
/// @return hash digest
OHSET_API uint32_t ohset_hash(const void *restrict key, size_t len);

#endif
