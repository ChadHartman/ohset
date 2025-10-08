#include <ohset.h>
#include <stdlib.h> // EXIT_SUCCESS
#include <test/testlist.h>

int main() {

  const size_t test_count = sizeof(tests) / sizeof(test_t);
  for (size_t i = 0; i < test_count; ++i) {
    printf("+------------------+\n");
    printf("| " COLOR_MAGENTA "%-16s" COLOR_RESET " |\n", tests[i].name);
    printf("+------------------+\n\n");
    tests[i].func();
  }

  return EXIT_SUCCESS;
}