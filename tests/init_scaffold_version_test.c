#include "tack_test_embed.h"

static int lines_contain(const char * const *lines, const char *needle) {
  int i;
  if (!lines || !needle) return 0;
  for (i = 0; lines[i]; i++) {
    if (strstr(lines[i], needle) != 0) return 1;
  }
  return 0;
}

int main(void) {
  if (!lines_contain(TACK_INIT_DEFAULT_TACK_INI_LINES, "tack v" TACK_VERSION ")")) {
    fprintf(stderr, "generated tack.ini header does not reference TACK_VERSION\n");
    return 1;
  }
  if (!strstr(TACK_INIT_MAIN_C, "Hello from tack!")) {
    fprintf(stderr, "generated src/main.c scaffold text mismatch\n");
    return 1;
  }
  if (strstr(TACK_INIT_MAIN_C, "v0.4.0") != 0) {
    fprintf(stderr, "generated src/main.c still contains stale v0.4.0 text\n");
    return 1;
  }
  puts("init_scaffold_version_test: ok");
  return 0;
}
