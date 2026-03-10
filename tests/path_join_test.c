#include "tack_test_embed.h"

#ifdef _WIN32
static int expect_eq(const char *label, const char *got, const char *expected) {
  if (strcmp(got, expected) == 0) return 0;
  fprintf(stderr, "FAIL %s: got \"%s\" expected \"%s\"\n", label, got, expected);
  return 1;
}
#endif

int main(void) {
#ifdef _WIN32
  char out[256];
  int failures = 0;

  path_join(out, sizeof(out), "C:\\path\\", "file.c");
  failures += expect_eq("backslash", out, "C:\\path\\file.c");

  path_join(out, sizeof(out), "C:/path/", "file.c");
  failures += expect_eq("forward-slash", out, "C:/path/file.c");

  {
    char *joined = path_join_alloc("C:/path/", "file.c");
    failures += expect_eq("alloc-forward-slash", joined, "C:/path/file.c");
    free(joined);
  }

  if (failures != 0) return 1;
  puts("path_join_test: ok");
  return 0;
#else
  puts("path_join_test: skipped (non-Windows)");
  return 0;
#endif
}
