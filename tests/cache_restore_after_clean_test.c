#define TACK_TEST 1
#include "../src/tack.c"

static int write_text_file(const char *path, const char *text) {
  FILE *f = fopen(path, "wb");
  if (!f) return 1;
  if (fputs(text, f) == EOF) { fclose(f); return 1; }
  if (fclose(f) != 0) return 1;
  return 0;
}

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

static void wait_for_timestamp_tick(void) {
  time_t start = time((time_t*)0);
  while (time((time_t*)0) == start) {
    /* busy wait: portable C89 and stable for mtime granularity tests */
  }
}

static int change_dir_local(const char *path) {
#ifdef _WIN32
  return _chdir(path);
#else
  return chdir(path);
#endif
}

int main(void) {
  int failures = 0;
  char cwd[1024];
  const char *tmpdir = "build/test-cache-restore-after-clean";
  const char *obj_path = "build/app/debug/obj/main.o";
  const char *dep_path = "build/app/debug/dep/main.d";
  const char *header_path = "include/common.h";
  const char *key = "cache-clean-test";

  if (!getcwd(cwd, (int)sizeof(cwd))) {
    fprintf(stderr, "getcwd failed\n");
    return 1;
  }

  ensure_dir("build");
  if (file_exists(tmpdir) && rm_rf(tmpdir) != 0) {
    fprintf(stderr, "rm_rf tmpdir failed\n");
    return 1;
  }
  ensure_dir(tmpdir);
  if (change_dir_local(tmpdir) != 0) {
    fprintf(stderr, "change_dir failed\n");
    return 1;
  }

  ensure_dir("include");
  ensure_dir("build");
  ensure_dir("build/app");
  ensure_dir("build/app/debug");
  ensure_dir("build/app/debug/obj");
  ensure_dir("build/app/debug/dep");

  if (write_text_file(header_path, "#define VALUE 1\n") != 0) {
    fprintf(stderr, "header write failed\n");
    failures++;
  }
  if (write_text_file(obj_path, "OBJ\n") != 0) {
    fprintf(stderr, "object write failed\n");
    failures++;
  }
  if (write_text_file(dep_path, "# tack-deps-v1\ninclude/common.h\n") != 0) {
    fprintf(stderr, "depfile write failed\n");
    failures++;
  }

  cache_store(key, obj_path, dep_path);
  failures += expect_true("cache-dir-created", file_exists(g_cache_dir));

  wait_for_timestamp_tick();
  if (rm_rf("build") != 0) {
    fprintf(stderr, "rm_rf build failed\n");
    failures++;
  }
  ensure_dir("build");
  ensure_dir("build/app");
  ensure_dir("build/app/debug");
  ensure_dir("build/app/debug/obj");
  ensure_dir("build/app/debug/dep");
  if (write_text_file(dep_path, "# tack-deps-v1\ninclude/common.h\n") != 0) {
    fprintf(stderr, "depfile rewrite failed\n");
    failures++;
  }

  failures += expect_true("restore-after-clean", cache_restore(key, obj_path, dep_path));
  failures += expect_true("restored-object", file_exists(obj_path));
  failures += expect_true("restored-depfile", file_exists(dep_path));

  wait_for_timestamp_tick();
  if (write_text_file(dep_path, "# tack-deps-v1\ninclude/common.h\ninclude/other.h\n") != 0) {
    fprintf(stderr, "depfile graph change write failed\n");
    failures++;
  }
  failures += expect_false("graph-change-invalidates-cache", cache_restore(key, obj_path, dep_path));

  if (change_dir_local(cwd) != 0) {
    fprintf(stderr, "restore cwd failed\n");
    return 1;
  }

  if (failures != 0) return 1;
  puts("cache_restore_after_clean_test: ok");
  return 0;
}
