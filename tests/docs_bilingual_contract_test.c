/* Regression guard for SPEC 0001: paired DE and EN sections in normative docs. */
/* Clang targeting the MSVC UCRT: use ISO C89 CRT APIs without Microsoft's
 * vendor-only 'unsafe' deprecation annotations. This opt-out must precede
 * ALL CRT/Windows headers; it does not disable -Werror or other diagnostics.
 * fopen_s/getenv_s/strerror_s are not ISO C89 replacements. */
#if defined(_WIN32) && defined(_MSC_VER) && defined(__clang__)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#include <stdio.h>
#include <string.h>

static const char * const document_paths[] = {
  "README.md",
  "FAQ.md",
  "ROADMAP.md",
  "RELEASENOTES.md",
  "CHANGELOG.md",
  "SECURITY.md",
  "AGENTS.md",
  "CLAUDE.md",
  "templates/README.md",
  "docs/README.md",
  "docs/adr/0001-open-v0.8-series.md",
  "docs/adr/0002-profile-compiler-ownership.md",
  "docs/agents/STATE.md",
  "docs/agents/TASK-TEMPLATE.md",
  "docs/security/README.md",
  "docs/releases/0.8.2-dev.md",
  "docs/releases/0.8.3-dev.md",
  "docs/specs/0001-bilingual-documentation-policy.md",
  "docs/specs/0002-ini-first-toolchain-policy-foundation.md",
  "docs/specs/0003-compiler-selection-and-policy-split.md",
  "docs/specs/0004-ci-mode-and-summary-foundation.md",
  "docs/specs/0005-events-jsonl-foundation.md",
  "docs/specs/0006-tap-report-foundation.md",
  "docs/specs/0007-junit-xml-foundation.md",
  "docs/specs/0008-doc-ux-foundation.md",
  "docs/specs/0009-optional-js-search-foundation.md",
  "docs/specs/0010-profile-compiler-selection.md",
  "docs/specs/0011-ba-std-c89-ratchet-adoption.md",
  "docs/guides/github-runner-install-linux-arm64.md",
  "docs/guides/github-runner-install-windows.md",
  "docs/guides/github-self-hosted-runners.md",
  "docs/guides/init-new-and-ba-std-c89.md",
  0
};

/* Accept a Markdown H1/H2 language heading, not an incidental text mention. */
static int language_heading(const char *line, const char *language) {
  const char *p = line;
  size_t n;
  int hashes = 0;
  while (*p == '#' && hashes < 3) { hashes++; p++; }
  if (hashes < 1 || hashes > 2 || *p != ' ') return 0;
  p++;
  n = strlen(language);
  if (strncmp(p, language, n) != 0) return 0;
  p += n;
  return *p == '\n' || *p == '\r' || *p == '\0' || *p == ' ' || *p == '(';
}

static int verify_doc(const char *path) {
  FILE *f;
  char line[4096];
  int state = 0;
  int de_count = 0;
  int en_count = 0;
  int bad = 0;
  f = fopen(path, "rb");
  if (!f) { fprintf(stderr, "FAIL docs-missing: %s\n", path); return 1; }
  while (fgets(line, sizeof(line), f)) {
    if (language_heading(line, "Deutsch")) {
      if (state != 0) bad = 1;
      state = 1;
      de_count++;
    } else if (language_heading(line, "English")) {
      if (state != 1) bad = 1;
      state = 2;
      en_count++;
    } else if (strncmp(line, "DE:", 3) == 0 || strncmp(line, "EN:", 3) == 0) {
      /* Avoid future append-only mixed-language tails. */
      bad = 1;
    }
  }
  if (ferror(f)) bad = 1;
  fclose(f);
  if (bad || de_count != 1 || en_count != 1 || state != 2) {
    fprintf(stderr, "FAIL docs-bilingual: %s (de=%d en=%d order=%d)\n",
            path, de_count, en_count, state);
    return 1;
  }
  return 0;
}

int main(void) {
  int i;
  int failed = 0;
  for (i = 0; document_paths[i]; ++i) {
    if (verify_doc(document_paths[i]) != 0) failed++;
  }
  if (failed != 0) return 1;
  puts("docs_bilingual_contract_test: ok");
  return 0;
}
