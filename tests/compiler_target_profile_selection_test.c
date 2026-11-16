#include "tests/tack_test_embed.h"

#ifndef _WIN32
extern int setenv(const char *name, const char *value, int overwrite);
#endif

static int env_set(const char *value) {
#ifdef _WIN32
  char buf[128];
  tack_copy(buf, sizeof(buf), "TACK_CC=");
  tack_cat(buf, sizeof(buf), value ? value : "");
  return _putenv(buf);
#else
  return setenv("TACK_CC", value ? value : "", 1);
#endif
}

static int check_equal(const char *what, const char *got, const char *want) {
  if (strcmp(got ? got : "", want) == 0) return 0;
  fprintf(stderr, "FAIL %s: got '%s', expected '%s'\n", what, got ? got : "", want);
  return 1;
}

int main(void) {
  IniProfileCfg *release;
  int failed = 0;

  if (env_set("") != 0) return 1;
  ini_profile_targets_init();
  g_config_loaded = 1;
  g_config_compiler = xstrdup("tcc");
  g_config_compiler_policy = xstrdup("auto");

  release = ini_get_or_add_profile_target("app", PROF_RELEASE);
  release->compiler = xstrdup("gcc");
  release->compiler_policy = xstrdup("gcc");

  set_active_compiler_target("app", PROF_DEBUG);
  failed += check_equal("debug compiler", get_cc(), "tcc");
  failed += check_equal("debug source", compiler_source_label(), "config:[project] compiler");
  failed += check_equal("debug policy", compiler_policy_name(compiler_policy_for_cc(get_cc())), "tcc");

  set_active_compiler_target("app", PROF_RELEASE);
  failed += check_equal("release compiler", get_cc(), "gcc");
  failed += check_equal("release source", compiler_source_label(), "config:[target profile] compiler");
  failed += check_equal("release policy", compiler_policy_name(compiler_policy_for_cc(get_cc())), "gcc");

  set_active_compiler_target("tool:other", PROF_RELEASE);
  failed += check_equal("other target fallback", get_cc(), "tcc");

  set_active_compiler_target("app", PROF_RELEASE);
  if (env_set("clang") != 0) failed++;
  cc_cache_reset();
  failed += check_equal("environment compiler override", get_cc(), "clang");
  failed += check_equal("environment compiler source", compiler_source_label(), "env:TACK_CC");
  /* Explicit target policy is independent of TACK_CC until overridden. */
  failed += check_equal("explicit policy remains", compiler_policy_name(compiler_policy_for_cc(get_cc())), "gcc");

  if (env_set("") != 0) failed++;
  set_active_compiler_target(0, PROF_DEBUG);
  ini_profile_targets_free();
  free(g_config_compiler); g_config_compiler = 0;
  free(g_config_compiler_policy); g_config_compiler_policy = 0;
  g_config_loaded = 0;
  cc_cache_reset();

  if (failed) return 1;
  puts("compiler_target_profile_selection_test: ok");
  return 0;
}
