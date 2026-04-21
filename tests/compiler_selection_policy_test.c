#include "tack_test_embed.h"

#ifndef _WIN32
extern int setenv(const char *name, const char *value, int overwrite);
#endif

static int set_env_local(const char *key, const char *value) {
#ifdef _WIN32
  char buf[2048];
  tack_copy(buf, sizeof(buf), key);
  tack_cat(buf, sizeof(buf), "=");
  tack_cat(buf, sizeof(buf), value ? value : "");
  return _putenv(buf);
#else
  return setenv(key, value ? value : "", 1);
#endif
}

static void reset_compiler_cfg(void) {
  free(g_config_compiler);
  g_config_compiler = 0;
  free(g_config_compiler_policy);
  g_config_compiler_policy = 0;
  g_config_loaded = 0;
  cc_cache_reset();
  (void)set_env_local("TACK_CC", "");
}

static int expect_true(const char *label, int cond) {
  if (cond) return 0;
  fprintf(stderr, "FAIL %s\n", label);
  return 1;
}

static int expect_streq(const char *label, const char *got, const char *expected) {
  const char *lhs = got ? got : "";
  const char *rhs = expected ? expected : "";
  if (strcmp(lhs, rhs) == 0) return 0;
  fprintf(stderr, "FAIL %s: got \"%s\" expected \"%s\"\n", label, lhs, rhs);
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
  return expect_streq(label, got, expected);
}

int main(void) {
  int failures = 0;
  Argv av;

  reset_compiler_cfg();
  g_config_loaded = 1;
  g_config_compiler = xstrdup("clang");
  failures += expect_streq("config-compiler-selected", get_cc(), "clang");
  failures += expect_streq("config-compiler-source", compiler_source_label(), "config:[project] compiler");
  failures += expect_streq("config-policy-auto-detect", compiler_policy_name(compiler_policy_for_cc(get_cc())), "clang");

  reset_compiler_cfg();
  g_config_loaded = 1;
  g_config_compiler = xstrdup("clang");
  if (set_env_local("TACK_CC", "gcc") != 0) {
    fprintf(stderr, "FAIL setenv-env-override\n");
    return 1;
  }
  failures += expect_streq("env-override-selected", get_cc(), "gcc");
  failures += expect_streq("env-override-source", compiler_source_label(), "env:TACK_CC");
  failures += expect_streq("env-override-policy", compiler_policy_name(compiler_policy_for_cc(get_cc())), "gcc");

  reset_compiler_cfg();
  g_config_loaded = 1;
  g_config_compiler_policy = xstrdup("tcc");
  av_init(&av);
  push_profile_flags_for_cc(&av, PROF_DEBUG, "gcc");
  failures += expect_argv_eq("forced-tcc-debug-flags", &av, "-g -bt20 -DDEBUG=1");
  av_free(&av);
  av_init(&av);
  push_common_warnings_for_cc(&av, 0, "gcc");
  failures += expect_argv_eq("forced-tcc-warnings", &av,
      "-Wall -Werror -Wwrite-strings -Wimplicit-function-declaration -Wno-unsupported");
  av_free(&av);

  reset_compiler_cfg();
  g_config_loaded = 1;
  g_config_compiler_policy = xstrdup("generic");
  av_init(&av);
  push_profile_flags_for_cc(&av, PROF_DEBUG, "tcc");
  failures += expect_argv_eq("forced-generic-debug-flags", &av, "-g -DDEBUG=1");
  av_free(&av);
  av_init(&av);
  push_common_warnings_for_cc(&av, 1, "tcc");
  failures += expect_argv_eq("forced-generic-warnings", &av,
      "-Wall -Werror -Wwrite-strings -Wimplicit-function-declaration");
  av_free(&av);

  failures += expect_true("policy-name-valid-auto", compiler_policy_name_is_valid("auto"));
  failures += expect_true("policy-name-valid-generic", compiler_policy_name_is_valid("generic"));

  reset_compiler_cfg();
  if (failures != 0) return 1;
  puts("compiler_selection_policy_test: ok");
  return 0;
}
