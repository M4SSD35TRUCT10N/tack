#include "tests/tack_test_embed.h"

int main(void) {
  CovRecord hwm, cur;
  CovSet set;
  unsigned long covered, total;
  FILE *f;
  int failures = 0;

  memset(&hwm, 0, sizeof(hwm));
  strcpy(hwm.profile, "linux-gcc14");
  strcpy(hwm.os, "linux");
  strcpy(hwm.compiler, "gcc");
  hwm.covered = 7ul; hwm.total = 10ul;
  hwm.sources_hash = 123ul; hwm.source_count = 1; hwm.flags_state = 44ul;
  cur = hwm;
  if (cov_compare(&hwm, &cur) != 0) failures++;
  cur.covered = 6ul;
  if (cov_compare(&hwm, &cur) != 1) failures++;
  cur.covered = 8ul;
  if (cov_compare(&hwm, &cur) != 2) failures++;
  cur = hwm;
  strcpy(cur.profile, "windows-gcc14");
  if (cov_compare(&hwm, &cur) != 2) failures++;
  cur = hwm;
  cur.flags_state++;
  if (cov_compare(&hwm, &cur) != 2) failures++;

  ensure_dir_recursive("build/test-coverage-ratchet");
  f = fopen("build/test-coverage-ratchet/source.gcov", "wb");
  if (!f) return 1;
  fputs("        -:    0:Source:src/tack.c\n", f);
  fputs("        3:    1:int a = 0;\n", f);
  fputs("    #####:    2:const char *s = \":Source:literal\";\n", f);
  fputs("        8:    3:int b = 1;\n", f);
  fputs("        -:    4:// non-executable\n", f);
  if (fclose(f) != 0) return 1;
  cov_set_init(&set);
  if (cov_read_gcov("build/test-coverage-ratchet/source.gcov", ".", &set) != 0) return 1;
  if (set.count != 1 || cov_count_lines(&set, &covered, &total) != 0 || covered != 2ul || total != 3ul) failures++;
  cov_set_free(&set);
  if (remove("build/test-coverage-ratchet/source.gcov") != 0) failures++;

  if (failures) { fprintf(stderr, "coverage_ratchet_test: %d failures\n", failures); return 1; }
  puts("coverage_ratchet_test: ok");
  return 0;
}
