#include "tack_test_embed.h"

#ifdef _WIN32
#define PATH_LIST_SEP ';'
#else
#define PATH_LIST_SEP ':'
#endif

static int write_text_file(const char *path, const char *text) {
  FILE *f;
  if (!path || !text) return 1;
  f = fopen(path, "wb");
  if (!f) return 1;
  fputs(text, f);
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

static int get_cwd(char *buf, size_t cap) {
#ifdef _WIN32
  if (_getcwd(buf, (int)cap) == 0) return 1;
#else
  if (getcwd(buf, cap) == 0) return 1;
#endif
  return 0;
}

static int change_dir(const char *path) {
#ifdef _WIN32
  if (_chdir(path) != 0) return 1;
#else
  if (chdir(path) != 0) return 1;
#endif
  return 0;
}

static int has_path_sep(const char *s) {
  if (!s) return 0;
  while (*s) {
    if (*s == '/' || *s == '\\') return 1;
    s++;
  }
  return 0;
}

static int path_has_executable(const char *name) {
  const char *path_env;
  const char *p;
  char dir[512];
  char full[1024];
  size_t i;

  if (!name || !name[0]) return 0;
  if (has_path_sep(name)) return file_exists(name);

  path_env = getenv("PATH");
  if (!path_env) return 0;

  p = path_env;
  while (*p) {
    i = 0;
    while (p[i] && p[i] != PATH_LIST_SEP && i + 1 < sizeof(dir)) {
      dir[i] = p[i];
      i++;
    }
    dir[i] = '\0';
    if (dir[0]) {
      path_join(full, sizeof(full), dir, name);
      if (file_exists(full)) return 1;
#ifdef _WIN32
      tack_cat(full, sizeof(full), ".exe");
      if (file_exists(full)) return 1;
#endif
    }
    p += i;
    if (*p == PATH_LIST_SEP) p++;
  }
  return 0;
}

static int compiler_available(void) {
  const char *cc = get_cc();
  return path_has_executable(cc);
}

static int setup_fixture(void) {
  if (write_text_file("README.md", "# Test README\n\nHello.\n") != 0) return 1;
  if (write_text_file("FAQ.md", "# Test FAQ\n") != 0) return 1;
  if (write_text_file("ROADMAP.md", "# Test Roadmap\n") != 0) return 1;
  if (write_text_file("RELEASENOTES.md", "# Test Release Notes\n") != 0) return 1;
  if (write_text_file("tack.ini",
    "[project]\n"
    "default_target = app\n"
    "disable_auto_tools = yes\n"
  ) != 0) return 1;
  if (write_text_file("tackfile.c", "/* no-op */\n") != 0) return 1;
  return 0;
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

int main(void) {
  char cwd[512];
  int failures = 0;
  int can_build = 0;
  TargetVec tv;
  const Target *t = 0;
  const char *tmpdir = "build/test-fixture";

  if (get_cwd(cwd, sizeof(cwd)) != 0) {
    fprintf(stderr, "failed to get cwd\n");
    return 1;
  }

  if (ensure_clean_dir(tmpdir) != 0) {
    fprintf(stderr, "failed to prepare temp dir\n");
    return 1;
  }
  if (change_dir(tmpdir) != 0) {
    fprintf(stderr, "failed to chdir to temp dir\n");
    return 1;
  }

  if (setup_fixture() != 0) {
    fprintf(stderr, "failed to setup fixture\n");
    return 1;
  }

  if (cmd_init() != 0) {
    fprintf(stderr, "cmd_init failed\n");
    return 1;
  }

  /* tack new: create project dir and run init inside it */
  if (ensure_clean_dir("newcase") != 0) {
    fprintf(stderr, "failed to prepare newcase dir\n");
    return 1;
  }
  if (change_dir("newcase") != 0) {
    fprintf(stderr, "failed to chdir to newcase\n");
    return 1;
  }

  if (cmd_new("hello") != 0) {
    fprintf(stderr, "cmd_new failed\n");
    failures++;
  } else {
    if (!file_exists("hello/tack.ini")) { fprintf(stderr, "new: tack.ini missing\n"); failures++; }
    if (!file_exists("hello/templates/tack_doc.css")) { fprintf(stderr, "new: templates css missing\n"); failures++; }
    if (!file_exists("hello/src/main.c") && !file_exists("hello/src/app/main.c")) { fprintf(stderr, "new: main.c missing\n"); failures++; }
  }

  if (change_dir("..") != 0) {
    fprintf(stderr, "failed to return from newcase\n");
    return 1;
  }

  config_free();
  g_no_config = 1;
  if (config_auto_load() != 0 || g_config_loaded) {
    fprintf(stderr, "expected config to be disabled\n");
    failures++;
  }
  config_free();

  g_no_config = 0;
  g_no_code_config = 1;
  g_config_path_cli = 0;
  if (config_auto_load() != 0 || !g_config_loaded || !g_config_disable_auto_tools) {
    fprintf(stderr, "expected config to load with disable_auto_tools\n");
    failures++;
  }
  config_free();

  g_no_config = 0;
  g_no_code_config = 1;
  g_config_path_cli = "tack.ini";
  if (config_auto_load() != 0 || !g_config_loaded || !streq(g_config_path, "tack.ini")) {
    fprintf(stderr, "expected --config path to be used\n");
    failures++;
  }

  if (discover_default_target(&tv, &t) != 0) {
    fprintf(stderr, "failed to discover default target\n");
    failures++;
  }

  if (cmd_doc(&tv, t, 0, 0, 0, "build/doc", PROF_DEBUG) != 0) {
    fprintf(stderr, "cmd_doc failed\n");
    failures++;
  } else if (!file_exists("build/doc/readme.html")) {
    fprintf(stderr, "doc output missing\n");
    failures++;
  }

  if (cmd_bom(PROF_DEBUG, &tv, t, 0, 0, 0, "build") != 0) {
    fprintf(stderr, "cmd_bom failed\n");
    failures++;
  } else if (!file_exists("build/bom.md")) {
    fprintf(stderr, "bom output missing\n");
    failures++;
  }

  if (cmd_sbom(PROF_DEBUG, &tv, t, 0, 0, 0, "build") != 0) {
    fprintf(stderr, "cmd_sbom failed\n");
    failures++;
  } else if (!file_exists("build/sbom.json")) {
    fprintf(stderr, "sbom output missing\n");
    failures++;
  }

  can_build = compiler_available();
  if (can_build) {
    if (build_one_target(t, PROF_DEBUG, 0, 0, 1, 1, 0, 0) != 0) {
      fprintf(stderr, "build_one_target failed\n");
      failures++;
    } else {
      char exe[512];
      char *runv[2];
      exe_path(exe, sizeof(exe), t->id, PROF_DEBUG, t->bin_base);
      runv[0] = exe;
      runv[1] = 0;
      if (run_argv_wait(runv, 0) != 0) {
        fprintf(stderr, "run failed\n");
        failures++;
      }
    }

    g_no_cache = 0;
    if (file_exists(g_cache_dir)) rm_rf(g_cache_dir);
    if (build_one_target(t, PROF_DEBUG, 0, 0, 0, 1, 0, 0) != 0) {
      fprintf(stderr, "build for cache failed\n");
      failures++;
    } else if (!file_exists(g_cache_dir)) {
      fprintf(stderr, "cache dir missing\n");
      failures++;
    }

    g_no_cache = 1;
    if (file_exists(g_cache_dir)) rm_rf(g_cache_dir);
    if (build_one_target(t, PROF_DEBUG, 0, 0, 1, 1, 0, 0) != 0) {
      fprintf(stderr, "build with no-cache failed\n");
      failures++;
    } else if (file_exists(g_cache_dir)) {
      fprintf(stderr, "cache dir should not exist\n");
      failures++;
    }

    if (build_and_run_tests(PROF_DEBUG, 0, 1, 0) != 0) {
      fprintf(stderr, "build_and_run_tests failed\n");
      failures++;
    }
  } else {
    puts("compiler not found; skipping build/run/cache/tests");
  }

  tv_free(&tv);
  config_free();

  if (change_dir(cwd) != 0) {
    fprintf(stderr, "failed to restore cwd\n");
    return 1;
  }

  if (failures != 0) return 1;
  puts("functional_smoke_test: ok");
  return 0;
}
