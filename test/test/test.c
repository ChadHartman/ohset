#include <ohset.h>

#include "test.h"

uint32_t hash_str(const void *ptr) {
  const char *str = ptr;
  return ohset_hash(str, strlen(str));
}
