#include "tack_test_embed.h"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define CHDIR_FN _chdir
#define GETCWD_FN _getcwd
#define DUP_FN _dup
#define DUP2_FN _dup2
#define FILENO_FN _fileno
#else
#include <unistd.h>
#define CHDIR_FN chdir
#define GETCWD_FN getcwd
#define DUP_FN dup
#define DUP2_FN dup2
#define FILENO_FN fileno
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
  FILE *f;
  f = fopen(path, "wb");
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

static int capture_run_to_file(const char *cwd, const char *path, const char * const *argv) {
  FILE *out;
  int saved_fd;
  int out_fd;
  char saved_cwd[4096];
  int rc;

  if (!cwd || !path || !argv || !argv[0]) return 1;
  if (!GETCWD_FN(saved_cwd, sizeof(saved_cwd))) return 1;

  fflush(stdout);
  saved_fd = DUP_FN(FILENO_FN(stdout));
  if (saved_fd < 0) return 1;

  out = fopen(path, "wb");
  if (!out) {
#ifdef _WIN32
    _close(saved_fd);
#else
    close(saved_fd);
#endif
    return 1;
  }

  out_fd = FILENO_FN(out);
  if (DUP2_FN(out_fd, FILENO_FN(stdout)) < 0) {
    fclose(out);
#ifdef _WIN32
    _close(saved_fd);
#else
    close(saved_fd);
#endif
    return 1;
  }

  if (CHDIR_FN(cwd) != 0) {
    fclose(out);
#ifdef _WIN32
    _close(saved_fd);
#else
    close(saved_fd);
#endif
    return 1;
  }

  rc = run_argv_wait_const(argv, 0);
  fflush(stdout);

  if (CHDIR_FN(saved_cwd) != 0) {
    fclose(out);
#ifdef _WIN32
    _close(saved_fd);
#else
    close(saved_fd);
#endif
    return 1;
  }

  if (DUP2_FN(saved_fd, FILENO_FN(stdout)) < 0) {
    fclose(out);
#ifdef _WIN32
    _close(saved_fd);
#else
    close(saved_fd);
#endif
    return 1;
  }

#ifdef _WIN32
  _close(saved_fd);
#else
  close(saved_fd);
#endif
  fclose(out);
  return rc;
}

static int contains_text(const char *hay, const char *needle) {
  return hay && needle && strstr(hay, needle) != 0;
}

int main(void) {
  char repo_root[4096];
  char repo_dir[4096];
  char tack_bin[4096];
  char build_out[4096];
  char test_out[4096];
  char buf[32768];
  const char *build_argv[5];
  const char *test_argv[5];
  const char *cc_argv[8];
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

  ensure_dir_recursive("build/tests/ci-summary-repo/src");
  ensure_dir_recursive("build/tests/ci-summary-repo/tests");
  ensure_dir_recursive("build/tests/bin");

  if (write_text_file("build/tests/ci-summary-repo/src/main.c",
      "int main(void) { return 0; }\n") != 0) {
    fprintf(stderr, "FAIL write-main\n");
    return 1;
  }
  if (write_text_file("build/tests/ci-summary-repo/tests/smoke_test.c",
      "#include <stdio.h>\nint main(void) { puts(\"smoke_test: ok\"); return 0; }\n") != 0) {
    fprintf(stderr, "FAIL write-test\n");
    return 1;
  }

  path_join(repo_dir, sizeof(repo_dir), repo_root, "build/tests/ci-summary-repo");
  path_join(tack_bin, sizeof(tack_bin), repo_root, "build/tests/bin/tack_ci");
  path_join(build_out, sizeof(build_out), repo_root, "build/tests/ci-build-output.txt");
  path_join(test_out, sizeof(test_out), repo_root, "build/tests/ci-test-output.txt");

  cc_argv[0] = (char*)host_cc;
  cc_argv[1] = "-std=c89";
  cc_argv[2] = "-pedantic";
  cc_argv[3] = "-Werror";
  cc_argv[4] = "-o";
  cc_argv[5] = tack_bin;
  cc_argv[6] = "src/tack.c";
  cc_argv[7] = 0;
  if (run_argv_wait_const(cc_argv, 0) != 0) {
    fprintf(stderr, "FAIL build-tack-ci\n");
    return 1;
  }

  if (set_env_local("TACK_CC", host_cc) != 0) {
    fprintf(stderr, "FAIL setenv-tack-cc\n");
    return 1;
  }

  build_argv[0] = tack_bin;
  build_argv[1] = "build";
  build_argv[2] = "debug";
  build_argv[3] = "--ci";
  build_argv[4] = 0;
  if (capture_run_to_file(repo_dir, build_out, build_argv) != 0) {
    fprintf(stderr, "FAIL run-build-ci\n");
    return 1;
  }
  if (load_text_file(build_out, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-build-output\n");
    return 1;
  }
  if (!contains_text(buf, "TACK_SUMMARY version=1 mode=build status=ok profile=debug target=app")) {
    fprintf(stderr, "FAIL build-summary\n");
    failures++;
  }
  if (!contains_text(buf, "duration_ms=")) {
    fprintf(stderr, "FAIL build-duration\n");
    failures++;
  }

  test_argv[0] = tack_bin;
  test_argv[1] = "test";
  test_argv[2] = "debug";
  test_argv[3] = "--ci";
  test_argv[4] = 0;
  if (capture_run_to_file(repo_dir, test_out, test_argv) != 0) {
    fprintf(stderr, "FAIL run-test-ci\n");
    return 1;
  }
  if (load_text_file(test_out, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-test-output\n");
    return 1;
  }
  if (!contains_text(buf, "smoke_test: ok")) {
    fprintf(stderr, "FAIL test-program-output\n");
    failures++;
  }
  if (!contains_text(buf, "TACK_SUMMARY version=1 mode=test status=ok profile=debug total=1")) {
    fprintf(stderr, "FAIL test-summary-total\n");
    failures++;
  }
  if (!contains_text(buf, "passed=1 failed=0 skipped=0 not_run=0")) {
    fprintf(stderr, "FAIL test-summary-counts\n");
    failures++;
  }

  (void)set_env_local("TACK_CC", "");

  if (failures != 0) return 1;
  puts("ci_summary_output_test: ok");
  return 0;
}
