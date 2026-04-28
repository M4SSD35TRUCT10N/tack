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
  char repo_dir[4096];
  char tack_bin[4096];
  char tack_ini[4096];
  char index_html[4096];
  char nested_html[4096];
  char js_out[4096];
  char buf[131072];
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

  ensure_dir_recursive("build/tests/doc-js-search-repo/src");
  ensure_dir_recursive("build/tests/doc-js-search-repo/docs/specs");
  ensure_dir_recursive("build/tests/doc-js-search-repo/templates");
  ensure_dir_recursive("build/tests/bin");

  if (write_text_file("build/tests/doc-js-search-repo/src/main.c",
      "int main(void) { return 0; }\n") != 0) {
    fprintf(stderr, "FAIL write-main\n");
    return 1;
  }

  if (write_text_file("build/tests/doc-js-search-repo/README.md",
      "# Hello Search\n\n"
      "This document mentions alpha and beta.\n") != 0) {
    fprintf(stderr, "FAIL write-readme\n");
    return 1;
  }

  if (write_text_file("build/tests/doc-js-search-repo/docs/specs/alpha.md",
      "# Alpha Spec\n\n"
      "Nested alpha topic.\n") != 0) {
    fprintf(stderr, "FAIL write-alpha\n");
    return 1;
  }

  if (write_text_file("build/tests/doc-js-search-repo/tack.ini",
      "[doc]\n"
      "template = templates/tack_template_min.html\n"
      "css = templates/tack_doc.css\n"
      "allow_js_search = yes\n") != 0) {
    fprintf(stderr, "FAIL write-ini\n");
    return 1;
  }

  if (copy_file("templates/tack_doc.css",
      "build/tests/doc-js-search-repo/templates/tack_doc.css") != 0) {
    fprintf(stderr, "FAIL copy-css\n");
    return 1;
  }
  if (copy_file("templates/tack_template_min.html",
      "build/tests/doc-js-search-repo/templates/tack_template_min.html") != 0) {
    fprintf(stderr, "FAIL copy-template\n");
    return 1;
  }

  path_join(repo_dir, sizeof(repo_dir), repo_root, "build/tests/doc-js-search-repo");
  path_join(tack_bin, sizeof(tack_bin), repo_root, "build/tests/bin/tack_doc_js_search");
  path_join(tack_ini, sizeof(tack_ini), repo_root, "build/tests/doc-js-search-repo/tack.ini");
  path_join(index_html, sizeof(index_html), repo_root, "build/tests/doc-js-search-repo/build/doc/index.html");
  path_join(nested_html, sizeof(nested_html), repo_root, "build/tests/doc-js-search-repo/build/doc/docs/specs/alpha.html");
  path_join(js_out, sizeof(js_out), repo_root, "build/tests/doc-js-search-repo/build/doc/tack_doc_search.js");

  cc_argv[0] = host_cc;
  cc_argv[1] = "-std=c89";
  cc_argv[2] = "-pedantic";
  cc_argv[3] = "-Werror";
  cc_argv[4] = "-o";
  cc_argv[5] = tack_bin;
  cc_argv[6] = "src/tack.c";
  cc_argv[7] = 0;
  if (run_argv_wait_const(cc_argv, 0) != 0) {
    fprintf(stderr, "FAIL build-tack-doc-search\n");
    return 1;
  }

  if (set_env_local("TACK_CC", host_cc) != 0) {
    fprintf(stderr, "FAIL setenv-tack-cc\n");
    return 1;
  }

  {
    const char *doc2_argv[6];
    doc2_argv[0] = tack_bin;
    doc2_argv[1] = "--config";
    doc2_argv[2] = tack_ini;
    doc2_argv[3] = "doc";
    doc2_argv[4] = "debug";
    doc2_argv[5] = 0;
    if (run_in_repo(repo_dir, doc2_argv) != 0) {
      fprintf(stderr, "FAIL run-doc\n");
      return 1;
    }
  }

  if (load_text_file(index_html, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-index\n");
    return 1;
  }
  if (!contains_text(buf, "tack_doc_search.js")) {
    fprintf(stderr, "FAIL index-script\n");
    failures++;
  }
  if (!contains_text(buf, "id=\"tack-search-input\"")) {
    fprintf(stderr, "FAIL index-search-input\n");
    failures++;
  }

  if (load_text_file(nested_html, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-nested\n");
    return 1;
  }
  if (!contains_text(buf, "../../tack_doc_search.js")) {
    fprintf(stderr, "FAIL nested-script-prefix\n");
    failures++;
  }
  if (!contains_text(buf, "data-doc-prefix=\"../../\"")) {
    fprintf(stderr, "FAIL nested-doc-prefix\n");
    failures++;
  }

  if (load_text_file(js_out, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-js\n");
    return 1;
  }
  if (!contains_text(buf, "Hello Search")) {
    fprintf(stderr, "FAIL js-title-root\n");
    failures++;
  }
  if (!contains_text(buf, "Alpha Spec")) {
    fprintf(stderr, "FAIL js-title-doc\n");
    failures++;
  }
  if (!contains_text(buf, "group")) {
    fprintf(stderr, "FAIL js-group\n");
    failures++;
  }

  (void)set_env_local("TACK_CC", "");

  if (failures != 0) return 1;
  puts("doc_js_search_test: ok");
  return 0;
}
