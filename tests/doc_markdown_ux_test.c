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
  char readme_html[4096];
  char nested_html[4096];
  char css_out[4096];
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

  ensure_dir_recursive("build/tests/doc-ux-repo/src");
  ensure_dir_recursive("build/tests/doc-ux-repo/docs/specs");
  ensure_dir_recursive("build/tests/doc-ux-repo/templates");
  ensure_dir_recursive("build/tests/bin");

  if (write_text_file("build/tests/doc-ux-repo/src/main.c",
      "int main(void) { return 0; }\n") != 0) {
    fprintf(stderr, "FAIL write-main\n");
    return 1;
  }

  if (write_text_file("build/tests/doc-ux-repo/README.md",
      "# Hello World\n\n"
      "First line\n"
      "second line with [link](https://example.com) and `code`.\n\n"
      "## Features\n\n"
      "- item one\n"
      "- item two\n\n"
      "### Steps\n\n"
      "1. first\n"
      "2. second\n\n"
      "```c\n"
      "int x = 1;\n"
      "```\n") != 0) {
    fprintf(stderr, "FAIL write-readme\n");
    return 1;
  }

  if (write_text_file("build/tests/doc-ux-repo/docs/specs/alpha.md",
      "# Alpha\n\n"
      "A nested document.\n") != 0) {
    fprintf(stderr, "FAIL write-alpha\n");
    return 1;
  }

  if (copy_file("templates/tack_doc.css",
      "build/tests/doc-ux-repo/templates/tack_doc.css") != 0) {
    fprintf(stderr, "FAIL copy-css\n");
    return 1;
  }

  path_join(repo_dir, sizeof(repo_dir), repo_root, "build/tests/doc-ux-repo");
  path_join(tack_bin, sizeof(tack_bin), repo_root, "build/tests/bin/tack_doc_ux");
  path_join(readme_html, sizeof(readme_html), repo_root, "build/tests/doc-ux-repo/build/doc/readme.html");
  path_join(nested_html, sizeof(nested_html), repo_root, "build/tests/doc-ux-repo/build/doc/docs/specs/alpha.html");
  path_join(css_out, sizeof(css_out), repo_root, "build/tests/doc-ux-repo/build/doc/tack_doc.css");

  cc_argv[0] = host_cc;
  cc_argv[1] = "-std=c89";
  cc_argv[2] = "-pedantic";
  cc_argv[3] = "-Werror";
  cc_argv[4] = "-o";
  cc_argv[5] = tack_bin;
  cc_argv[6] = "src/tack.c";
  cc_argv[7] = 0;
  if (run_argv_wait_const(cc_argv, 0) != 0) {
    fprintf(stderr, "FAIL build-tack-doc-ux\n");
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

  if (load_text_file(readme_html, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-readme-html\n");
    return 1;
  }
  if (!contains_text(buf, "<h1 id=\"hello-world-1\">Hello World</h1>")) {
    fprintf(stderr, "FAIL heading-h1\n");
    failures++;
  }
  if (!contains_text(buf, "<p>First line second line with <a href=\"https://example.com\">link</a> and <code>code</code>.</p>")) {
    fprintf(stderr, "FAIL paragraph-inline\n");
    failures++;
  }
  if (!contains_text(buf, "<ul>\n<li>item one</li>\n<li>item two</li>\n</ul>")) {
    fprintf(stderr, "FAIL unordered-list\n");
    failures++;
  }
  if (!contains_text(buf, "<ol>\n<li>first</li>\n<li>second</li>\n</ol>")) {
    fprintf(stderr, "FAIL ordered-list\n");
    failures++;
  }
  if (!contains_text(buf, "<pre><code>int x = 1;\n</code></pre>")) {
    fprintf(stderr, "FAIL code-block\n");
    failures++;
  }
  if (!contains_text(buf, "<aside class=\"toc\" id=\"tack-toc\">")) {
    fprintf(stderr, "FAIL toc-block\n");
    failures++;
  }
  if (!contains_text(buf, "href=\"#features-2\"")) {
    fprintf(stderr, "FAIL toc-link\n");
    failures++;
  }
  if (!contains_text(buf, "aria-current=\"page\">README</a>")) {
    fprintf(stderr, "FAIL current-root-nav\n");
    failures++;
  }

  if (load_text_file(nested_html, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-nested-html\n");
    return 1;
  }
  if (!contains_text(buf, "aria-current=\"page\">specs / alpha</a>")) {
    fprintf(stderr, "FAIL current-docs-nav\n");
    failures++;
  }

  if (load_text_file(css_out, buf, sizeof(buf)) != 0) {
    fprintf(stderr, "FAIL read-css\n");
    return 1;
  }
  if (!contains_text(buf, "overflow-wrap:anywhere")) {
    fprintf(stderr, "FAIL css-wrap\n");
    failures++;
  }
  if (!contains_text(buf, ".toc-list .toc-l2")) {
    fprintf(stderr, "FAIL css-toc-indent\n");
    failures++;
  }

  (void)set_env_local("TACK_CC", "");

  if (failures != 0) return 1;
  puts("doc_markdown_ux_test: ok");
  return 0;
}
