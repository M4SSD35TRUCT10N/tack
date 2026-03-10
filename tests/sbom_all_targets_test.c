#include "tack_test_embed.h"

static int write_text_file(const char *path, const char *text) {
  FILE *f;
  if (!path || !text) return 1;
  f = fopen(path, "wb");
  if (!f) return 1;
  if (fputs(text, f) == EOF) { fclose(f); return 1; }
  if (fclose(f) != 0) return 1;
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
  if (_chdir(path) != 0) return 1;
#else
  if (chdir(path) != 0) return 1;
#endif
  return 0;
}

static int file_contains_text(const char *path, const char *needle) {
  FILE *f;
  char buf[4096];
  size_t n;
  size_t needle_len;
  if (!path || !needle) return 0;
  f = fopen(path, "rb");
  if (!f) return 0;
  n = fread(buf, 1, sizeof(buf) - 1, f);
  fclose(f);
  buf[n] = '\0';
  needle_len = strlen(needle);
  if (needle_len == 0) return 1;
  return strstr(buf, needle) != 0;
}

int main(void) {
  char cwd[512];
  const char *tmpdir = "build/test-sbom-all-targets";
  TargetVec tv;
  int failures = 0;

  if (get_cwd_local(cwd, sizeof(cwd)) != 0) {
    fprintf(stderr, "getcwd failed\n");
    return 1;
  }

  if (ensure_clean_dir(tmpdir) != 0) {
    fprintf(stderr, "prepare tmpdir failed\n");
    return 1;
  }
  if (change_dir_local(tmpdir) != 0) {
    fprintf(stderr, "chdir tmpdir failed\n");
    return 1;
  }

  if (write_text_file("tack.ini",
      "[project]\n"
      "default_target = app\n"
      "disable_auto_tools = no\n") != 0) {
    fprintf(stderr, "write tack.ini failed\n");
    return 1;
  }

  if (cmd_init() != 0) {
    fprintf(stderr, "cmd_init failed\n");
    return 1;
  }

  ensure_dir_recursive("tools/pack");
  if (write_text_file("tools/pack/main.c", "int main(void) { return 0; }\n") != 0) {
    fprintf(stderr, "write tool source failed\n");
    return 1;
  }

  config_free();
  g_no_config = 0;
  g_no_code_config = 1;
  g_config_path_cli = "tack.ini";
  if (config_auto_load() != 0 || !g_config_loaded) {
    fprintf(stderr, "config load failed\n");
    return 1;
  }

  tv_init(&tv);
  discover_targets(&tv, 0);
  apply_tackfile_targets(&tv);
  apply_ini_targets(&tv);

  if (cmd_sbom_all(PROF_DEBUG, &tv, 0, 0, 0, "build") != 0) {
    fprintf(stderr, "cmd_sbom_all failed\n");
    failures++;
  }

  if (!file_exists("build/sbom.app.json")) {
    fprintf(stderr, "missing app sbom\n");
    failures++;
  }
  if (!file_exists("build/sbom.tool_pack.json")) {
    fprintf(stderr, "missing tool sbom\n");
    failures++;
  }
  if (!file_contains_text("build/sbom.app.json", "\"name\": \"app\"")) {
    fprintf(stderr, "app sbom target mismatch\n");
    failures++;
  }
  if (!file_contains_text("build/sbom.tool_pack.json", "\"name\": \"tool:pack\"")) {
    fprintf(stderr, "tool sbom target mismatch\n");
    failures++;
  }

  tv_free(&tv);
  config_free();
  if (change_dir_local(cwd) != 0) {
    fprintf(stderr, "restore cwd failed\n");
    return 1;
  }

  if (failures != 0) return 1;
  puts("sbom_all_targets_test: ok");
  return 0;
}
