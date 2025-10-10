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
