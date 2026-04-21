#include "tack_test_embed.h"

#ifdef _WIN32
#include <io.h>
#define DUP_FN _dup
#define DUP2_FN _dup2
#define FILENO_FN _fileno
#else
#include <unistd.h>
#define DUP_FN dup
#define DUP2_FN dup2
#define FILENO_FN fileno
#endif

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

static int capture_stdout_to_file(const char *path, void (*fn)(void)) {
  FILE *out;
  int saved_fd;
  int out_fd;

  if (!path || !fn) return 1;

  fflush(stdout);
  saved_fd = DUP_FN(FILENO_FN(stdout));
  if (saved_fd < 0) return 1;

  out = fopen(path, "wb");
  if (!out) {
    close(saved_fd);
    return 1;
  }

  out_fd = FILENO_FN(out);
  if (DUP2_FN(out_fd, FILENO_FN(stdout)) < 0) {
    fclose(out);
    close(saved_fd);
    return 1;
  }

  fn();
  fflush(stdout);

  if (DUP2_FN(saved_fd, FILENO_FN(stdout)) < 0) {
    fclose(out);
    close(saved_fd);
    return 1;
  }

  close(saved_fd);
  fclose(out);
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

static int contains_once(const char *hay, const char *needle) {
  const char *p;
  const char *q;
  if (!hay || !needle || !needle[0]) return 0;
  p = strstr(hay, needle);
  if (!p) return 0;
  q = strstr(p + strlen(needle), needle);
  return q == 0;
}

static int contains_text(const char *hay, const char *needle) {
  return hay && needle && strstr(hay, needle) != 0;
}

int main(void) {
  char buf[16384];
  int failures = 0;
  const char *help_path = "build/help-output.txt";
  const char *doctor_path = "build/doctor-output.txt";

  ensure_dir_recursive("build");

  if (capture_stdout_to_file(help_path, print_help) != 0) {
    fprintf(stderr, "FAIL capture-help\n");
    return 1;
  }
  if (load_text_file(help_path, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-help\n");
    return 1;
  }
  if (!contains_once(buf, "Global options (must come before the command):")) {
    fprintf(stderr, "FAIL help-global-options-once\n");
    failures++;
  }
  if (!contains_once(buf, "Notes:")) {
    fprintf(stderr, "FAIL help-notes-once\n");
    failures++;
  }
  if (!contains_text(buf, "Compiler selection:")) {
    fprintf(stderr, "FAIL help-compiler-selection-section\n");
    failures++;
  }
  if (!contains_text(buf, "tack doctor prints the compiler, source, policy and policy source")) {
    fprintf(stderr, "FAIL help-doctor-note\n");
    failures++;
  }

  reset_compiler_cfg();
  g_config_loaded = 1;
  g_config_compiler = xstrdup("clang");
  g_config_compiler_policy = xstrdup("generic");

  if (capture_stdout_to_file(doctor_path, cmd_doctor) != 0) {
    fprintf(stderr, "FAIL capture-doctor\n");
    return 1;
  }
  if (load_text_file(doctor_path, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-doctor\n");
    return 1;
  }
  if (!contains_text(buf, "Compiler source: config:[project] compiler")) {
    fprintf(stderr, "FAIL doctor-compiler-source\n");
    failures++;
  }
  if (!contains_text(buf, "Compiler policy: generic")) {
    fprintf(stderr, "FAIL doctor-compiler-policy\n");
    failures++;
  }
  if (!contains_text(buf, "Policy source: config:[project] compiler_policy")) {
    fprintf(stderr, "FAIL doctor-policy-source\n");
    failures++;
  }

  reset_compiler_cfg();
  if (failures != 0) return 1;
  puts("help_doctor_output_test: ok");
  return 0;
}
