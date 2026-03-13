#include "tack_test_embed.h"

int main(void) {
  if (strstr(TACK_INIT_DEFAULT_TACK_INI, TACK_VERSION) == 0) {
    fprintf(stderr, "generated tack.ini header does not reference TACK_VERSION\n");
    return 1;
  }
  puts("init_scaffold_version_test: ok");
  return 0;
}
