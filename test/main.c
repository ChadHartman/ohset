#include <ohset.h>
#include <stdlib.h> // EXIT_SUCCESS
#include <test/testlist.h>

int main() {

  const size_t test_count = sizeof(tests) / sizeof(test_t);
  for (size_t i = 0; i < test_count; ++i) {
    printf("| %s |\n", tests[i].name);
    tests[i].func();
  }

  return EXIT_SUCCESS;
}