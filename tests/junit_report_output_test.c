#include "tack_test_embed.h"

#ifdef _WIN32
#include <direct.h>
#define CHDIR_FN _chdir
#define GETCWD_FN _getcwd
#else
#include <unistd.h>
#define CHDIR_FN chdir
#define GETCWD_FN getcwd
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

static const char *pick_host_cc(void) {
  static const char *cands[] = { "gcc", "clang", "cc", "tcc", 0 };
  int i;
  for (i = 0; cands[i]; i++) {
    if (fmt_exe_in_path(cands[i])) return cands[i];
  }
  return 0;
}

static int write_text_file(const char *path, const char *text) {
  FILE *f = fopen(path, "wb");
  if (!f) return 1;
  if (text && text[0]) fwrite(text, 1u, strlen(text), f);
  fclose(f);
  return 0;
}

static int load_text_file(const char *path, char *buf, size_t cap) {
  FILE *f;
  size_t n;
  if (!path || !buf || cap == 0u) return 1;
  f = fopen(path, "rb");
  if (!f) return 1;
  n = fread(buf, 1u, cap - 1u, f);
  fclose(f);
  buf[n] = '\0';
  return 0;
}

static int contains_text(const char *hay, const char *needle) {
  return hay && needle && strstr(hay, needle) != 0;
}

static int run_in_repo(const char *cwd, const char * const *argv) {
  char saved_cwd[4096];
  int rc;
  if (!cwd || !argv || !argv[0]) return 1;
  if (!GETCWD_FN(saved_cwd, sizeof(saved_cwd))) return 1;
  if (CHDIR_FN(cwd) != 0) return 1;
  rc = run_argv_wait_const(argv, 0);
  if (CHDIR_FN(saved_cwd) != 0) return 1;
  return rc;
}

int main(void) {
  char repo_root[4096];
  char pass_repo[4096];
  char fail_repo[4096];
  char tack_bin[4096];
  char pass_xml[4096];
  char fail_xml[4096];
  char buf[32768];
  const char *cc_argv[8];
  const char *pass_argv[6];
  const char *fail_argv[6];
  const char *host_cc;
  int failures = 0;

  if (!GETCWD_FN(repo_root, sizeof(repo_root))) {
    fprintf(stderr, "FAIL getcwd\n");
    return 1;
  }

  host_cc = pick_host_cc();
  if (!host_cc) {
    fprintf(stderr, "FAIL no-host-cc\n");
    return 1;
  }

  ensure_dir_recursive("build/tests/junit-pass-repo/src");
  ensure_dir_recursive("build/tests/junit-pass-repo/tests");
  ensure_dir_recursive("build/tests/junit-fail-repo/src");
  ensure_dir_recursive("build/tests/junit-fail-repo/tests");
  ensure_dir_recursive("build/tests/bin");

  if (write_text_file("build/tests/junit-pass-repo/src/main.c",
      "int main(void) { return 0; }\n") != 0) {
    fprintf(stderr, "FAIL write-pass-main\n");
    return 1;
  }
  if (write_text_file("build/tests/junit-pass-repo/tests/pass_test.c",
      "#include <stdio.h>\nint main(void) { puts(\"pass_test: ok\"); return 0; }\n") != 0) {
    fprintf(stderr, "FAIL write-pass-test\n");
    return 1;
  }
  if (write_text_file("build/tests/junit-fail-repo/src/main.c",
      "int main(void) { return 0; }\n") != 0) {
    fprintf(stderr, "FAIL write-fail-main\n");
    return 1;
  }
  if (write_text_file("build/tests/junit-fail-repo/tests/fail_test.c",
      "int main(void) { return 1; }\n") != 0) {
    fprintf(stderr, "FAIL write-fail-test\n");
    return 1;
  }

  path_join(pass_repo, sizeof(pass_repo), repo_root, "build/tests/junit-pass-repo");
  path_join(fail_repo, sizeof(fail_repo), repo_root, "build/tests/junit-fail-repo");
  path_join(tack_bin, sizeof(tack_bin), repo_root, "build/tests/bin/tack_junit");
  path_join(pass_xml, sizeof(pass_xml), repo_root, "build/tests/pass-junit.xml");
  path_join(fail_xml, sizeof(fail_xml), repo_root, "build/tests/fail-junit.xml");

  cc_argv[0] = host_cc;
  cc_argv[1] = "-std=c89";
  cc_argv[2] = "-pedantic";
  cc_argv[3] = "-Werror";
  cc_argv[4] = "-o";
  cc_argv[5] = tack_bin;
  cc_argv[6] = "src/tack.c";
  cc_argv[7] = 0;
  if (run_argv_wait_const(cc_argv, 0) != 0) {
    fprintf(stderr, "FAIL build-tack-junit\n");
    return 1;
  }

  if (set_env_local("TACK_CC", host_cc) != 0) {
    fprintf(stderr, "FAIL setenv-tack-cc\n");
    return 1;
  }

  pass_argv[0] = tack_bin;
  pass_argv[1] = "test";
  pass_argv[2] = "debug";
  pass_argv[3] = "--report-junit";
  pass_argv[4] = pass_xml;
  pass_argv[5] = 0;
  if (run_in_repo(pass_repo, pass_argv) != 0) {
    fprintf(stderr, "FAIL run-pass-junit\n");
    return 1;
  }
  if (load_text_file(pass_xml, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-pass-junit\n");
    return 1;
  }
  if (!contains_text(buf, "<testsuite name=\"tack.test.debug\" tests=\"1\" failures=\"0\"")) {
    fprintf(stderr, "FAIL junit-suite-pass\n");
    failures++;
  }
  if (!contains_text(buf, "<testcase classname=\"tack.tests\" name=\"pass_test.c\"")) {
    fprintf(stderr, "FAIL junit-case-pass\n");
    failures++;
  }
  if (!contains_text(buf, "<system-out>compiled: true")) {
    fprintf(stderr, "FAIL junit-system-out-pass\n");
    failures++;
  }
  if (contains_text(buf, "<failure message=")) {
    fprintf(stderr, "FAIL junit-unexpected-failure-pass\n");
    failures++;
  }

  fail_argv[0] = tack_bin;
  fail_argv[1] = "test";
  fail_argv[2] = "debug";
  fail_argv[3] = "--report-junit";
  fail_argv[4] = fail_xml;
  fail_argv[5] = 0;
  if (run_in_repo(fail_repo, fail_argv) == 0) {
    fprintf(stderr, "FAIL fail-junit-expected-error\n");
    return 1;
  }
  if (load_text_file(fail_xml, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-fail-junit\n");
    return 1;
  }
  if (!contains_text(buf, "<testsuite name=\"tack.test.debug\" tests=\"1\" failures=\"1\"")) {
    fprintf(stderr, "FAIL junit-suite-fail\n");
    failures++;
  }
  if (!contains_text(buf, "<failure message=\"test failed\">test failed</failure>")) {
    fprintf(stderr, "FAIL junit-failure-node\n");
    failures++;
  }
  if (!contains_text(buf, "name=\"fail_test.c\"")) {
    fprintf(stderr, "FAIL junit-case-fail\n");
    failures++;
  }

  (void)set_env_local("TACK_CC", "");

  if (failures != 0) return 1;
  puts("junit_report_output_test: ok");
  return 0;
}
