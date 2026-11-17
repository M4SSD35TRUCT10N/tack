#include "tests/tack_test_embed.h"

static int fmt_expect(int condition, const char *what) {
  if (condition) return 0;
  fprintf(stderr, "fmt_ini_parser_test: FAIL %s\n", what);
  return 1;
}

static int fmt_write_fixture(const char *path, const char *body) {
  FILE *f = fopen(path, "wb");
  if (!f) return 1;
  if (fputs(body, f) == EOF) { fclose(f); return 1; }
  return fclose(f) != 0;
}

int main(void) {
  const char *good = "build/test-fmt-ini-parser/good.ini";
  const char *bad = "build/test-fmt-ini-parser/bad.ini";
  const char *invalid = "build/test-fmt-ini-parser/invalid.ini";
  char *args[] = { (char*)"tack", (char*)"fmt", (char*)"--list",
                   (char*)"--strict", (char*)"--", (char*)"src/tack.c" };
  FmtRuleCfg *r;
  TargetVec targets;
  StrVec selected, candidates, files;
  int failures = 0;

  ensure_dir_recursive("build/test-fmt-ini-parser");
  fmt_cfg_init();
  if (fmt_write_fixture(good,
      "[sbom]\nformat = spdx\n"
      "[fmt]\nenabled = yes\nfail_fast = yes\ndefaults = off\n"
      "rules = c\nexclude = vendor/**\n"
      "[fmt \"c\"]\ntool = clang-format\nmode = file\n"
      "include = src/**/*.c\nexclude = src/generated/**\n"
      "cmd = clang-format -i @FILE@\n"
      "check_cmd = clang-format --dry-run @FILE@\n"
      "diff_cmd = clang-format --diff @FILE@\n"
      "requires = clang-format\nworkdir = .\nenv.LANG = C\n") != 0)
    return 1;

  failures += fmt_expect(ini_load_file(good) == 0, "parse valid INI");
  failures += fmt_expect(g_config_sbom_format != 0 &&
                         streq(g_config_sbom_format, "spdx"), "SBOM remains separate");
  failures += fmt_expect(g_fmt.enabled_set && g_fmt.enabled &&
                         g_fmt.fail_fast_set && g_fmt.fail_fast &&
                         g_fmt.defaults_set && !g_fmt.defaults_auto, "global formatter options");
  failures += fmt_expect(g_fmt.rules.count == 1 &&
                         streq(g_fmt.rules.items[0], "c"), "rule selection");
  failures += fmt_expect(g_fmt.exclude.count == 1, "global exclude");
  failures += fmt_expect(g_fmt_rules.count == 1, "one explicit rule");
  r = fmt_get_or_add_rule("c");
  failures += fmt_expect(r != 0 && r->mode_set && !r->mode &&
                         r->cmd && streq(r->cmd, "clang-format -i @FILE@") &&
                         r->check_cmd && r->diff_cmd &&
                         r->include.count == 1 && r->exclude.count == 1 &&
                         r->requires.count == 1 && r->workdir &&
                         r->env_keys.count == 1 && r->env_vals.count == 1,
                         "all documented rule fields parsed");

  sv_init(&selected);
  sv_push(&selected, "c");
  failures += fmt_expect(fmt_validate_strict(&selected) == 0,
                         "strict valid rule");
  tv_init(&targets);
  failures += fmt_expect(cmd_fmt(&targets, 6, args, 2) == 0,
                         "fmt --list --strict CLI");
  tv_free(&targets);

  sv_init(&candidates);
  sv_init(&files);
  sv_push(&candidates, "src/dir/with spaces.c");
  sv_push(&candidates, "vendor/external.c");
  sv_push(&candidates, "src/generated/skip.c");
  fmt_select_files_for_rule(&files, r, &candidates);
  failures += fmt_expect(files.count == 1 &&
                         streq(files.items[0], "src/dir/with spaces.c"),
                         "file names with spaces and excludes");
  sv_free(&files);
  sv_free(&candidates);
  sv_free(&selected);

  if (fmt_write_fixture(bad, "[fmt]\nunknown_option = yes\n") != 0) return 1;
  failures += fmt_expect(ini_load_file(bad) == 0, "load invalid global field");
  sv_init(&selected);
  sv_push(&selected, "c");
  failures += fmt_expect(fmt_validate_strict(&selected) != 0,
                         "strict rejects unknown global key");
  sv_free(&selected);

  fmt_cfg_reset();
  if (fmt_write_fixture(invalid,
       "[fmt]\nrules = c\n"
       "[fmt \"c\"]\nmode = impossible\n"
       "include = src/**/*.c\ncmd = clang-format -i @FILE@\n") != 0) return 1;
  failures += fmt_expect(ini_load_file(invalid) == 0, "load invalid rule field");
  r = fmt_get_or_add_rule("c");
  failures += fmt_expect(r->unknown_keys.count == 1,
                         "invalid rule mode recorded");
  sv_init(&selected);
  sv_push(&selected, "c");
  failures += fmt_expect(fmt_validate_strict(&selected) != 0,
                         "strict rejects invalid mode");
  sv_free(&selected);

  if (remove(good) != 0 || remove(bad) != 0 || remove(invalid) != 0)
    failures++;
  fmt_cfg_free();
  free(g_config_sbom_format); g_config_sbom_format = 0;

  if (failures) {
    fprintf(stderr, "fmt_ini_parser_test: %d failures\n", failures);
    return 1;
  }
  puts("fmt_ini_parser_test: ok");
  return 0;
}
