#include "tack_test_embed.h"

int main(void) {
  if (strstr(TACK_INIT_DEFAULT_TACK_INI, "tack v" TACK_VERSION ")") == 0) {
    fprintf(stderr, "generated tack.ini header does not reference TACK_VERSION\n");
    return 1;
  }
  if (strstr(TACK_INIT_MAIN_C, "Hello from tack!") == 0) {
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
