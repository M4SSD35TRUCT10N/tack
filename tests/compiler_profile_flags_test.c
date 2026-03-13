#include "tack_test_embed.h"

static int expect_true(const char *label, int cond) {
  if (cond) return 0;
  fprintf(stderr, "FAIL %s\n", label);
  return 1;
}

static int expect_false(const char *label, int cond) {
  if (!cond) return 0;
  fprintf(stderr, "FAIL %s\n", label);
  return 1;
}

static int expect_argv_eq(const char *label, Argv *av, const char *expected) {
  char got[256];
  int i;
  got[0] = '\0';
  for (i = 0; i < av->n; i++) {
    if (i) tack_cat(got, sizeof(got), " ");
    tack_cat(got, sizeof(got), av->a[i]);
  }
  if (strcmp(got, expected) == 0) return 0;
  fprintf(stderr, "FAIL %s: got \"%s\" expected \"%s\"\n", label, got, expected);
  return 1;
}

int main(void) {
  int failures = 0;
  Argv av;

  failures += expect_true("tcc-name", compiler_name_is_tcc("tcc"));
  failures += expect_true("tcc-path", compiler_name_is_tcc("/opt/tinycc/bin/tcc"));
  failures += expect_true("tinycc-name", compiler_name_is_tcc("tinycc"));
#ifdef _WIN32
  failures += expect_true("tcc-exe", compiler_name_is_tcc("C:\\tcc\\tcc.exe"));
#endif
  failures += expect_false("gcc-name", compiler_name_is_tcc("gcc"));
  failures += expect_false("clang-name", compiler_name_is_tcc("clang"));
  failures += expect_false("cc-name", compiler_name_is_tcc("cc"));

  av_init(&av);
  push_profile_flags_for_cc(&av, PROF_DEBUG, "tcc");
  failures += expect_argv_eq("debug-tcc-flags", &av, "-g -bt20 -DDEBUG=1");
  av_free(&av);

  av_init(&av);
  push_profile_flags_for_cc(&av, PROF_DEBUG, "gcc");
  failures += expect_argv_eq("debug-gcc-flags", &av, "-g -DDEBUG=1");
  av_free(&av);

  av_init(&av);
  push_profile_flags_for_cc(&av, PROF_RELEASE, "gcc");
  failures += expect_argv_eq("release-flags", &av, "-O2 -DNDEBUG=1");
  av_free(&av);

  av_init(&av);
  push_common_warnings_for_cc(&av, 0, "gcc");
  failures += expect_argv_eq("warn-gcc-default", &av,
      "-Wall -Werror -Wwrite-strings -Wimplicit-function-declaration");
  av_free(&av);

  av_init(&av);
  push_common_warnings_for_cc(&av, 0, "clang");
  failures += expect_argv_eq("warn-clang-default", &av,
      "-Wall -Werror -Wwrite-strings -Wimplicit-function-declaration");
  av_free(&av);

  av_init(&av);
  push_common_warnings_for_cc(&av, 0, "tcc");
  failures += expect_argv_eq("warn-tcc-default", &av,
      "-Wall -Werror -Wwrite-strings -Wimplicit-function-declaration -Wno-unsupported");
  av_free(&av);

  av_init(&av);
  push_common_warnings_for_cc(&av, 1, "tcc");
  failures += expect_argv_eq("warn-tcc-strict", &av,
      "-Wall -Werror -Wwrite-strings -Wimplicit-function-declaration -Wunsupported");
  av_free(&av);

  if (failures != 0) return 1;
  puts("compiler_profile_flags_test: ok");
  return 0;
}
