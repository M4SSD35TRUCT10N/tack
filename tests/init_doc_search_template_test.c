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
  char template_html[4096];
  char template_css[4096];
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

  ensure_dir_recursive("build/tests/init-doc-search-repo");
  ensure_dir_recursive("build/tests/bin");

  path_join(repo_dir, sizeof(repo_dir), repo_root, "build/tests/init-doc-search-repo");
  path_join(tack_bin, sizeof(tack_bin), repo_root, "build/tests/bin/tack_init_doc_search");
  path_join(template_html, sizeof(template_html), repo_root, "build/tests/init-doc-search-repo/templates/tack_template_min.html");
  path_join(template_css, sizeof(template_css), repo_root, "build/tests/init-doc-search-repo/templates/tack_doc.css");
  path_join(index_html, sizeof(index_html), repo_root, "build/tests/init-doc-search-repo/build/doc/index.html");
  path_join(nested_html, sizeof(nested_html), repo_root, "build/tests/init-doc-search-repo/build/doc/docs/specs/alpha.html");
  path_join(js_out, sizeof(js_out), repo_root, "build/tests/init-doc-search-repo/build/doc/tack_doc_search.js");

  cc_argv[0] = host_cc;
  cc_argv[1] = "-std=c89";
  cc_argv[2] = "-pedantic";
  cc_argv[3] = "-Werror";
  cc_argv[4] = "-o";
  cc_argv[5] = tack_bin;
  cc_argv[6] = "src/tack.c";
  cc_argv[7] = 0;
  if (run_argv_wait_const(cc_argv, 0) != 0) {
    fprintf(stderr, "FAIL build-tack\n");
    return 1;
  }

  if (set_env_local("TACK_CC", host_cc) != 0) {
    fprintf(stderr, "FAIL setenv-tack-cc\n");
    return 1;
  }

  {
    const char *init_argv[3];
    init_argv[0] = tack_bin;
    init_argv[1] = "init";
    init_argv[2] = 0;
    if (run_in_repo(repo_dir, init_argv) != 0) {
      fprintf(stderr, "FAIL run-init\n");
      return 1;
    }
  }

  if (load_text_file(template_html, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-template-html\n");
    return 1;
  }
  if (!contains_text(buf, "{{TACK_HEADER_TOOLS_HTML}}")) {
    fprintf(stderr, "FAIL init-template-header-tools-placeholder\n");
    failures++;
  }

  if (load_text_file(template_css, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-template-css\n");
    return 1;
  }
  if (!contains_text(buf, ".header-tools")) {
    fprintf(stderr, "FAIL init-css-header-tools\n");
    failures++;
  }
  if (!contains_text(buf, ".doc-search-header")) {
    fprintf(stderr, "FAIL init-css-doc-search-header\n");
    failures++;
  }

  ensure_dir_recursive("build/tests/init-doc-search-repo/docs/specs");

  if (write_text_file("build/tests/init-doc-search-repo/README.md",
      "# Init Search Root\n\n"
      "Root page for optional search.\n") != 0) {
    fprintf(stderr, "FAIL write-readme\n");
    return 1;
  }

  if (write_text_file("build/tests/init-doc-search-repo/docs/specs/alpha.md",
      "# Alpha Init Spec\n\n"
      "Nested page for optional search.\n") != 0) {
    fprintf(stderr, "FAIL write-alpha\n");
    return 1;
  }

  if (write_text_file("build/tests/init-doc-search-repo/tack.ini",
      "[doc]\n"
      "template = templates/tack_template_min.html\n"
      "css = templates/tack_doc.css\n"
      "allow_js_search = yes\n") != 0) {
    fprintf(stderr, "FAIL write-ini\n");
    return 1;
  }

  {
    const char *doc_argv[4];
    doc_argv[0] = tack_bin;
    doc_argv[1] = "doc";
    doc_argv[2] = "debug";
    doc_argv[3] = 0;
    if (run_in_repo(repo_dir, doc_argv) != 0) {
      fprintf(stderr, "FAIL run-doc\n");
      return 1;
    }
  }

  if (load_text_file(index_html, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-index\n");
    return 1;
  }
  if (!contains_text(buf, "id=\"tack-search-input\"")) {
    fprintf(stderr, "FAIL index-search-input\n");
    failures++;
  }
  if (!contains_text(buf, "tack_doc_search.js")) {
    fprintf(stderr, "FAIL index-search-js\n");
    failures++;
  }

  if (load_text_file(nested_html, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-nested\n");
    return 1;
  }
  if (!contains_text(buf, "data-doc-prefix=\"../../\"")) {
    fprintf(stderr, "FAIL nested-doc-prefix\n");
    failures++;
  }
  if (!contains_text(buf, "../../tack_doc_search.js")) {
    fprintf(stderr, "FAIL nested-search-js-prefix\n");
    failures++;
  }

  if (load_text_file(js_out, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-search-js\n");
    return 1;
  }
  if (!contains_text(buf, "Init Search Root")) {
    fprintf(stderr, "FAIL js-root-title\n");
    failures++;
  }
  if (!contains_text(buf, "Alpha Init Spec")) {
    fprintf(stderr, "FAIL js-nested-title\n");
    failures++;
  }

  (void)set_env_local("TACK_CC", "");

  if (failures != 0) return 1;
  puts("init_doc_search_template_test: ok");
  return 0;
}
