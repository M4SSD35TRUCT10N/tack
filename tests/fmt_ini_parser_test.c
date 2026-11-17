#include "tests/tack_test_embed.h"

static int fmt_expect(int condition, const char *what) {
  if (condition) return 0;
  fprintf(stderr, "fmt_ini_parser_test: FAIL %s\n", what);
  return 1;
}

/* Verify @FILE@ expansion through the same path used for formatter argv.
 * In particular, tokens ending in a placeholder must be NUL-terminated. */
static int fmt_expect_expansion(const char *token, const char *file,
                                const char *expected, const char *what) {
  char *result = fmt_expand_token(token, file, 0, 0, 0);
  int failed = fmt_expect(result != 0 && streq(result, expected), what);
  free(result);
  return failed;
}

#ifdef _WIN32
static int fmt_expect_windows_cmd_quote(const char *arg, const char *expected,
                                          const char *what) {
  char *actual = fmt_windows_quote_cmd_arg(arg);
  int failed = fmt_expect(streq(actual, expected), what);
  free(actual);
  return failed;
}
#endif

static int fmt_write_fixture(const char *path, const char *body) {
  FILE *f = fopen(path, "wb");
  if (!f) return 1;
  if (fputs(body, f) == EOF) { fclose(f); return 1; }
  return fclose(f) != 0;
}

int main(int argc, char **argv) {
  const char *good = "build/test-fmt-ini-parser/good.ini";
  const char *bad = "build/test-fmt-ini-parser/bad.ini";
  const char *invalid = "build/test-fmt-ini-parser/invalid.ini";
  char *args[] = { (char*)"tack", (char*)"fmt", (char*)"--list",
                   (char*)"--strict", (char*)"--", (char*)"src/tack.c" };
  FmtRuleCfg *r;
  TargetVec targets;
  StrVec selected, candidates, files;
  int failures = 0;

#ifdef _WIN32
  /* A real child process checks the exact argv after CreateProcess, not
   * merely the quoted command-line string assembled by the parent. */
  if (argc >= 2 && streq(argv[1], "--fmt-child-probe")) {
    if (argc != 3) return 3;
    return streq(argv[2], "C:\\fmt proof\\nested file.c") ||
           streq(argv[2], "C:\\fmt proof\\tail\\") ||
           streq(argv[2], "C:\\fmt proof\\quoted\"file.c") ? 0 : 4;
  }
#else
  (void)argc;
  (void)argv;
#endif

  /* End-of-token placeholders previously left the allocated string without
   * a terminating NUL; the subsequent expansion pass read garbage suffixes. */
  failures += fmt_expect_expansion("@FILE@", "C:\\format proof\\tail\\",
                                   "C:\\format proof\\tail\\",
                                   "terminate trailing backslash file expansion");
  failures += fmt_expect_expansion("@FILE@", "C:\\format proof\\quoted\"file.c",
                                   "C:\\format proof\\quoted\"file.c",
                                   "terminate embedded quote file expansion");
  failures += fmt_expect_expansion("prefix/@FILE@", "nested file.c",
                                   "prefix/nested file.c",
                                   "terminate file expansion with prefix");
  failures += fmt_expect_expansion("@FILE@.tmp", "nested file.c",
                                   "nested file.c.tmp",
                                   "preserve suffix after expansion");
  failures += fmt_expect_expansion("@FILE@@FILE@", "file.c",
                                   "file.cfile.c",
                                   "terminate repeated file expansion");
  failures += fmt_expect_expansion("@FILE@", "", "",
                                   "terminate empty file expansion");

  /* Windows must preserve a single argv item when Windows paths contain
   * spaces, quotes, or a trailing backslash. */
#ifdef _WIN32
  failures += fmt_expect_windows_cmd_quote("plain.c", "plain.c", "plain spawn arg");
  failures += fmt_expect_windows_cmd_quote("", "\"\"", "empty spawn arg");
  failures += fmt_expect_windows_cmd_quote("C:\\with spaces\\file.c",
                                             "\"C:\\with spaces\\file.c\"",
                                             "Windows path with spaces");
  failures += fmt_expect_windows_cmd_quote("quoted\"file",
                                             "\"quoted\\\"file\"",
                                             "embedded quote escape");
  failures += fmt_expect_windows_cmd_quote("C:\\with spaces\\",
                                             "\"C:\\with spaces\\\\\"",
                                             "trailing slash escape");
  {
    char self[MAX_PATH];
    DWORD sl = GetModuleFileNameA(NULL, self, (DWORD)sizeof(self));
    const char *child = "build/test-fmt-ini-parser/space dir/fmt child probe.exe";
    const char *command = "\"build/test-fmt-ini-parser/space dir/fmt child probe.exe\" --fmt-child-probe @FILE@";
    if (!sl || sl >= (DWORD)sizeof(self)) {
      failures += fmt_expect(0, "query child executable path");
    } else {
      ensure_dir_recursive("build/test-fmt-ini-parser/space dir");
      if (!CopyFileA(self, child, FALSE)) {
        failures += fmt_expect(0, "copy child executable to spaced path");
      } else {
        failures += fmt_expect(fmt_exec_cmd(command, "C:\\fmt proof\\nested file.c",
                                             0, 0, 0, 0, 0) == 0,
                               "spawn child with spaced executable and file path");
        failures += fmt_expect(fmt_exec_cmd(command, "C:\\fmt proof\\tail\\",
                                             0, 0, 0, 0, 0) == 0,
                               "spawn child with trailing backslash");
        failures += fmt_expect(fmt_exec_cmd(command, "C:\\fmt proof\\quoted\"file.c",
                                             0, 0, 0, 0, 0) == 0,
                               "spawn child with embedded quote");
        if (!DeleteFileA(child))
          failures += fmt_expect(0, "remove child executable fixture");
      }
    }
  }
#endif

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
