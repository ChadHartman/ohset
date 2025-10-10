#include <ohset.h>
#include <stdlib.h> // EXIT_SUCCESS
#include <test/testlist.h>

static void run_test(const test_t *restrict test) {
  printf("+------------------+\n");
  printf("| " COLOR_MAGENTA "%-16s" COLOR_RESET " |\n", test->name);
  printf("+------------------+\n\n");
  test->func();
}

int main(int argc, char **argv) {

  const size_t test_count = sizeof(tests) / sizeof(test_t);
  if (argc == 1) {
    for (size_t i = 0; i < test_count; ++i) {
      run_test(&tests[i]);
    }
    return EXIT_SUCCESS;
  }

  for (size_t i = 0; i < test_count; ++i) {
    if (strcmp(tests[i].name, argv[argc - 1]) == 0) {
      run_test(&tests[i]);
      return EXIT_SUCCESS;
    }
  }

  printf("Unknown test \"%s\"\n", argv[argc - 1]);
  return EXIT_FAILURE;
}