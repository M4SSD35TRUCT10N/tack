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
  for (i = 0; cands[i]; i++) if (fmt_exe_in_path(cands[i])) return cands[i];
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
  char root_html[4096];
  char docs_html[4096];
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

  ensure_dir_recursive("build/tests/doc-link-repo/src");
  ensure_dir_recursive("build/tests/doc-link-repo/docs/adr");
  ensure_dir_recursive("build/tests/doc-link-repo/docs/specs");
  ensure_dir_recursive("build/tests/doc-link-repo/docs/guides");
  ensure_dir_recursive("build/tests/bin");

  if (write_text_file("build/tests/doc-link-repo/src/main.c", "int main(void){return 0;}\n") != 0) return 1;
  if (write_text_file("build/tests/doc-link-repo/README.md",
      "# Root\n\n"
      "See the [docs overview](docs/README.md).\n") != 0) return 1;
  if (write_text_file("build/tests/doc-link-repo/docs/README.md",
      "# Docs\n\n"
      "- [ADR](adr/0001-open-v0.8-series.md)\n"
      "- [SPEC](specs/0008-doc-ux-foundation.md)\n"
      "- [Guide](guides/github-self-hosted-runners.md)\n") != 0) return 1;
  if (write_text_file("build/tests/doc-link-repo/docs/adr/0001-open-v0.8-series.md", "# ADR\n") != 0) return 1;
  if (write_text_file("build/tests/doc-link-repo/docs/specs/0008-doc-ux-foundation.md", "# SPEC\n") != 0) return 1;
  if (write_text_file("build/tests/doc-link-repo/docs/guides/github-self-hosted-runners.md", "# Guide\n") != 0) return 1;

  path_join(repo_dir, sizeof(repo_dir), repo_root, "build/tests/doc-link-repo");
  path_join(tack_bin, sizeof(tack_bin), repo_root, "build/tests/bin/tack_doc_links");
  path_join(root_html, sizeof(root_html), repo_root, "build/tests/doc-link-repo/build/doc/readme.html");
  path_join(docs_html, sizeof(docs_html), repo_root, "build/tests/doc-link-repo/build/doc/docs/README.html");

  cc_argv[0] = host_cc;
  cc_argv[1] = "-std=c89";
  cc_argv[2] = "-pedantic";
  cc_argv[3] = "-Werror";
  cc_argv[4] = "-o";
  cc_argv[5] = tack_bin;
  cc_argv[6] = "src/tack.c";
  cc_argv[7] = 0;
  if (run_argv_wait_const(cc_argv, 0) != 0) {
    fprintf(stderr, "FAIL build-tack-doc-links\n");
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

  if (load_text_file(root_html, buf, sizeof(buf)) != 0) return 1;
  if (!contains_text(buf, "href=\"docs/README.html\"")) {
    fprintf(stderr, "FAIL root-readme-link-rewrite\n");
    failures++;
  }
  if (contains_text(buf, "href=\"docs/README.md\"")) {
    fprintf(stderr, "FAIL root-readme-md-leak\n");
    failures++;
  }

  if (load_text_file(docs_html, buf, sizeof(buf)) != 0) return 1;
  if (!contains_text(buf, "href=\"adr/0001-open-v0.8-series.html\"")) {
    fprintf(stderr, "FAIL docs-adr-link-rewrite\n");
    failures++;
  }
  if (!contains_text(buf, "href=\"specs/0008-doc-ux-foundation.html\"")) {
    fprintf(stderr, "FAIL docs-spec-link-rewrite\n");
    failures++;
  }
  if (!contains_text(buf, "href=\"guides/github-self-hosted-runners.html\"")) {
    fprintf(stderr, "FAIL docs-guide-link-rewrite\n");
    failures++;
  }
  if (contains_text(buf, "href=\"specs/0008-doc-ux-foundation.md\"")) {
    fprintf(stderr, "FAIL docs-spec-md-leak\n");
    failures++;
  }

  (void)set_env_local("TACK_CC", "");
  if (failures != 0) return 1;
  puts("doc_inline_link_rewrite_test: ok");
  return 0;
}
