#include "tack_test_embed.h"

#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
extern int setenv(const char *name, const char *value, int overwrite);
#endif

static int write_text_file(const char *path, const char *text) {
  FILE *f;
  if (!path || !text) return 1;
  f = fopen(path, "wb");
  if (!f) return 1;
  if (fputs(text, f) == EOF) { fclose(f); return 1; }
  if (fclose(f) != 0) return 1;
  return 0;
}

static int read_text_file(const char *path, char *buf, size_t cap) {
  FILE *f;
  size_t n;
  if (!path || !buf || cap == 0) return 1;
  f = fopen(path, "rb");
  if (!f) return 1;
  n = fread(buf, 1, cap - 1, f);
  if (ferror(f)) { fclose(f); return 1; }
  buf[n] = '\0';
  fclose(f);
  return 0;
}

static int ensure_clean_dir(const char *path) {
  if (!path) return 1;
  if (file_exists(path)) {
    if (rm_rf(path) != 0) return 1;
  }
  ensure_dir_recursive(path);
  return 0;
}

static int get_cwd_local(char *buf, size_t cap) {
#ifdef _WIN32
  if (_getcwd(buf, (int)cap) == 0) return 1;
#else
  if (getcwd(buf, cap) == 0) return 1;
#endif
  return 0;
}

static int change_dir_local(const char *path) {
#ifdef _WIN32
  return _chdir(path);
#else
  return chdir(path);
#endif
}

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

static int discover_default_target(TargetVec *tv, const Target **out) {
  const Target *t;
  if (!tv || !out) return 1;
  tv_init(tv);
  discover_targets(tv, 0);
  apply_tackfile_targets(tv);
  apply_ini_targets(tv);
  t = find_target(tv, default_target_name());
  if (!t) return 1;
  *out = t;
  return 0;
}

#ifndef _WIN32
static int write_cc_wrapper(const char *path) {
  return write_text_file(
    path,
    "#!/bin/sh\n"
    "realcc=\"${REAL_CC:-gcc}\"\n"
    "out=\"\"\n"
    "prev=\"\"\n"
    "for arg in \"$@\"; do\n"
    "  if [ \"$prev\" = \"-o\" ]; then out=\"$arg\"; fi\n"
    "  prev=\"$arg\"\n"
    "done\n"
    "\"$realcc\" \"$@\" || exit $?\n"
    "if [ -n \"$out\" ] && [ -n \"$TACK_TEST_TOUCH_EPOCH\" ]; then\n"
    "  touch -d \"@$TACK_TEST_TOUCH_EPOCH\" \"$out\" || exit $?\n"
    "fi\n"
  );
}
#endif

int main(void) {
  int failures = 0;
  char cwd[1024];
  const char *tmpdir = "build/test-incremental-link";
  TargetVec tv;
  const Target *t = 0;
  char exe[512];
  char result[64];
  char *runv[2];
  const char *real_cc;

#ifdef _WIN32
  puts("incremental_link_test: skipped (non-POSIX wrapper test)");
  return 0;
#else
  real_cc = get_cc();
  if (!fmt_exe_in_path(real_cc)) {
    puts("incremental_link_test: skipped (compiler not found)");
    return 0;
  }

  if (get_cwd_local(cwd, sizeof(cwd)) != 0) {
    fprintf(stderr, "getcwd failed\n");
    return 1;
  }

  if (ensure_clean_dir(tmpdir) != 0) {
    fprintf(stderr, "failed to prepare temp dir\n");
    return 1;
  }
  if (change_dir_local(tmpdir) != 0) {
    fprintf(stderr, "failed to change dir\n");
    return 1;
  }

  ensure_dir("src");
  ensure_dir("include");
  if (write_text_file("tack.ini",
      "[project]\n"
      "default_target = app\n"
      "disable_auto_tools = yes\n") != 0) {
    fprintf(stderr, "write tack.ini failed\n");
    return 1;
  }
  if (write_text_file("include/my.h", "#define MY_MAGIC 1\n") != 0) {
    fprintf(stderr, "write header failed\n");
    return 1;
  }
  if (write_text_file("src/main.c",
      "#include <stdio.h>\n"
      "#include \"my.h\"\n"
      "int main(void) {\n"
      "  FILE *f = fopen(\"result.txt\", \"wb\");\n"
      "  if (!f) return 1;\n"
      "  fprintf(f, \"%d\\n\", MY_MAGIC);\n"
      "  fclose(f);\n"
      "  return 0;\n"
      "}\n") != 0) {
    fprintf(stderr, "write main.c failed\n");
    return 1;
  }
  if (write_cc_wrapper("ccwrap.sh") != 0) {
    fprintf(stderr, "write wrapper failed\n");
    return 1;
  }
  if (chmod("ccwrap.sh", 0755) != 0) {
    fprintf(stderr, "chmod wrapper failed\n");
    return 1;
  }

  if (set_env_local("REAL_CC", real_cc) != 0) {
    fprintf(stderr, "set REAL_CC failed\n");
    return 1;
  }
  if (set_env_local("TACK_CC", "./ccwrap.sh") != 0) {
    fprintf(stderr, "set TACK_CC failed\n");
    return 1;
  }
  if (set_env_local("TACK_TEST_TOUCH_EPOCH", "946684800") != 0) {
    fprintf(stderr, "set touch epoch failed\n");
    return 1;
  }

  config_free();
  g_no_config = 0;
  g_no_code_config = 1;
  g_config_path_cli = "tack.ini";
  if (config_auto_load() != 0) {
    fprintf(stderr, "config_auto_load failed\n");
    return 1;
  }

  if (discover_default_target(&tv, &t) != 0) {
    fprintf(stderr, "discover target failed\n");
    return 1;
  }

  if (build_one_target(t, PROF_DEBUG, 0, 0, 0, 1, 0, 0) != 0) {
    fprintf(stderr, "first build failed\n");
    failures++;
  } else {
    exe_path(exe, sizeof(exe), t->id, PROF_DEBUG, t->bin_base);
    runv[0] = exe;
    runv[1] = 0;
    if (run_argv_wait(runv, 0) != 0) {
      fprintf(stderr, "first run failed\n");
      failures++;
    } else if (read_text_file("result.txt", result, sizeof(result)) != 0 || strstr(result, "1") == 0) {
      fprintf(stderr, "expected first result 1, got: %s\n", result);
      failures++;
    }
  }

  if (write_text_file("include/my.h", "#define MY_MAGIC 2\n") != 0) {
    fprintf(stderr, "rewrite header failed\n");
    failures++;
  }

  if (build_one_target(t, PROF_DEBUG, 0, 0, 0, 1, 0, 0) != 0) {
    fprintf(stderr, "second build failed\n");
    failures++;
  } else {
    exe_path(exe, sizeof(exe), t->id, PROF_DEBUG, t->bin_base);
    runv[0] = exe;
    runv[1] = 0;
    if (run_argv_wait(runv, 0) != 0) {
      fprintf(stderr, "second run failed\n");
      failures++;
    } else if (read_text_file("result.txt", result, sizeof(result)) != 0 || strstr(result, "2") == 0) {
      fprintf(stderr, "expected second result 2, got: %s\n", result);
      failures++;
    }
  }

  tv_free(&tv);
  config_free();
  if (change_dir_local(cwd) != 0) {
    fprintf(stderr, "restore cwd failed\n");
    return 1;
  }

  if (failures != 0) return 1;
  puts("incremental_link_test: ok");
  return 0;
#endif
}
