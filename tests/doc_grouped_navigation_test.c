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
  char index_html[4096];
  char docs_readme_html[4096];
  char spec_html[4096];
  char adr_html[4096];
  char buf[65536];
  const char *cc_argv[8];
  const char *doc_argv[4];
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

  ensure_dir_recursive("build/tests/doc-group-repo/src");
  ensure_dir_recursive("build/tests/doc-group-repo/docs/specs");
  ensure_dir_recursive("build/tests/doc-group-repo/docs/adr");
  ensure_dir_recursive("build/tests/doc-group-repo/templates");
  ensure_dir_recursive("build/tests/bin");

  if (write_text_file("build/tests/doc-group-repo/src/main.c", "int main(void) { return 0; }\n") != 0) {
    fprintf(stderr, "FAIL write-main\n");
    return 1;
  }
  if (write_text_file("build/tests/doc-group-repo/README.md", "# Root\n\nRoot file.\n") != 0) {
    fprintf(stderr, "FAIL write-root-readme\n");
    return 1;
  }
  if (write_text_file("build/tests/doc-group-repo/docs/README.md", "# Docs Root\n\nDocs root file.\n") != 0) {
    fprintf(stderr, "FAIL write-docs-readme\n");
    return 1;
  }
  if (write_text_file("build/tests/doc-group-repo/docs/specs/alpha.md", "# Alpha\n\nSpec file.\n") != 0) {
    fprintf(stderr, "FAIL write-spec\n");
    return 1;
  }
  if (write_text_file("build/tests/doc-group-repo/docs/adr/decision.md", "# Decision\n\nADR file.\n") != 0) {
    fprintf(stderr, "FAIL write-adr\n");
    return 1;
  }
  if (copy_file("templates/tack_doc.css", "build/tests/doc-group-repo/templates/tack_doc.css") != 0) {
    fprintf(stderr, "FAIL copy-css\n");
    return 1;
  }

  path_join(repo_dir, sizeof(repo_dir), repo_root, "build/tests/doc-group-repo");
  path_join(tack_bin, sizeof(tack_bin), repo_root, "build/tests/bin/tack_doc_group_nav");
  path_join(index_html, sizeof(index_html), repo_root, "build/tests/doc-group-repo/build/doc/index.html");
  path_join(docs_readme_html, sizeof(docs_readme_html), repo_root, "build/tests/doc-group-repo/build/doc/docs/README.html");
  path_join(spec_html, sizeof(spec_html), repo_root, "build/tests/doc-group-repo/build/doc/docs/specs/alpha.html");
  path_join(adr_html, sizeof(adr_html), repo_root, "build/tests/doc-group-repo/build/doc/docs/adr/decision.html");

  cc_argv[0] = host_cc;
  cc_argv[1] = "-std=c89";
  cc_argv[2] = "-pedantic";
  cc_argv[3] = "-Werror";
  cc_argv[4] = "-o";
  cc_argv[5] = tack_bin;
  cc_argv[6] = "src/tack.c";
  cc_argv[7] = 0;
  if (run_argv_wait_const(cc_argv, 0) != 0) {
    fprintf(stderr, "FAIL build-tack-doc-group\n");
    return 1;
  }

  if (set_env_local("TACK_CC", host_cc) != 0) {
    fprintf(stderr, "FAIL setenv-tack-cc\n");
    return 1;
  }

  doc_argv[0] = tack_bin;
  doc_argv[1] = "doc";
  doc_argv[2] = "debug";
  doc_argv[3] = 0;
  if (run_in_repo(repo_dir, doc_argv) != 0) {
    fprintf(stderr, "FAIL run-doc\n");
    return 1;
  }

  if (load_text_file(index_html, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-index\n");
    return 1;
  }
  if (!contains_text(buf, "<section class=\"doc-group\"><h2>Docs</h2>")) {
    fprintf(stderr, "FAIL index-group-docs\n");
    failures++;
  }
  if (!contains_text(buf, "<section class=\"doc-group\"><h2>ADR</h2>")) {
    fprintf(stderr, "FAIL index-group-adr\n");
    failures++;
  }
  if (!contains_text(buf, "<section class=\"doc-group\"><h2>SPEC</h2>")) {
    fprintf(stderr, "FAIL index-group-spec\n");
    failures++;
  }
  if (!contains_text(buf, ">README</a>")) {
    fprintf(stderr, "FAIL index-docs-readme-label\n");
    failures++;
  }
  if (!contains_text(buf, ">alpha</a>")) {
    fprintf(stderr, "FAIL index-spec-label\n");
    failures++;
  }
  if (!contains_text(buf, ">decision</a>")) {
    fprintf(stderr, "FAIL index-adr-label\n");
    failures++;
  }

  if (load_text_file(docs_readme_html, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-docs-readme-html\n");
    return 1;
  }
  if (!contains_text(buf, "<h2>Docs</h2>")) {
    fprintf(stderr, "FAIL nav-docs-group\n");
    failures++;
  }
  if (!contains_text(buf, "aria-current=\"page\">README</a>")) {
    fprintf(stderr, "FAIL nav-docs-current\n");
    failures++;
  }

  if (load_text_file(spec_html, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-spec-html\n");
    return 1;
  }
  if (!contains_text(buf, "<h2>SPEC</h2>")) {
    fprintf(stderr, "FAIL nav-spec-group\n");
    failures++;
  }
  if (!contains_text(buf, "aria-current=\"page\">alpha</a>")) {
    fprintf(stderr, "FAIL nav-spec-current\n");
    failures++;
  }

  if (load_text_file(adr_html, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-adr-html\n");
    return 1;
  }
  if (!contains_text(buf, "<h2>ADR</h2>")) {
    fprintf(stderr, "FAIL nav-adr-group\n");
    failures++;
  }
  if (!contains_text(buf, "aria-current=\"page\">decision</a>")) {
    fprintf(stderr, "FAIL nav-adr-current\n");
    failures++;
  }

  (void)set_env_local("TACK_CC", "");

  if (failures != 0) return 1;
  puts("doc_grouped_navigation_test: ok");
  return 0;
}
