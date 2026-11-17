#include "tests/tack_test_embed.h"

/* Keep failure diagnostics actionable on both Windows and POSIX. */
static int cov_expect_path(const char *label, const char *actual,
                           const char *expected) {
  if (streq(actual, expected)) return 0;
  fprintf(stderr, "coverage_ratchet_test: %s path mismatch: got '%s', expected '%s'\n",
          label, actual, expected);
  return 1;
}

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

  /* Coverage objects, notes and runtime data must share build/obj location.
   * This catches regressions to single-step GCC test linking, which caused
   * MinGW/TDM-GCC to write gcov notes/data into the project root. */
  {
    char obj[1024], notes[1024];
    test_coverage_artifact_paths("build/tests/coverage/obj", "sample_test.c",
                                 obj, sizeof(obj), notes, sizeof(notes));
    /* path_join emits the native separator: '\\' on Windows, '/' on POSIX.
     * Both checks must remain active on every platform, since both files
     * must stay under build/ rather than leaking into the repository root. */
#ifdef _WIN32
    failures += cov_expect_path("object (Windows)", obj,
                                "build/tests/coverage/obj\\sample_test.o");
    failures += cov_expect_path("gcno (Windows)", notes,
                                "build/tests/coverage/obj\\sample_test.gcno");
#else
    failures += cov_expect_path("object (POSIX)", obj,
                                "build/tests/coverage/obj/sample_test.o");
    failures += cov_expect_path("gcno (POSIX)", notes,
                                "build/tests/coverage/obj/sample_test.gcno");
#endif
    /* Verify that converting the native spelling produces one portable
     * path; never weaken the required build/ location assertion. */
    cov_test_canonical_path(obj);
    cov_test_canonical_path(notes);
    failures += cov_expect_path("object (canonical)", obj,
                                "build/tests/coverage/obj/sample_test.o");
    failures += cov_expect_path("gcno (canonical)", notes,
                                "build/tests/coverage/obj/sample_test.gcno");
  }

  /* The accepted Linux id must remain available when Windows has an
   * independent reference. The Windows override must win regardless of
   * INI key order, and clearing it must preserve legacy behavior. */
  {
    char *old_id = g_coverage_profile_id;
    char *old_win = g_coverage_profile_id_windows;
    g_coverage_profile_id = (char*)"linux-gcc-14.2.0";
    g_coverage_profile_id_windows = (char*)"windows-gcc-14.2.0";
    if (!streq(cov_id_for_os("linux"), "linux-gcc-14.2.0")) failures++;
    if (!streq(cov_id_for_os("windows"), "windows-gcc-14.2.0")) failures++;
    g_coverage_profile_id_windows = 0;
    if (!streq(cov_id_for_os("windows"), "linux-gcc-14.2.0")) failures++;
    g_coverage_profile_id = old_id;
    g_coverage_profile_id_windows = old_win;
  }

  ensure_dir_recursive("build/test-coverage-ratchet");
  /* Exercise real INI parsing, including order-independent Windows key
   * precedence. Never weaken accepted tests to raise measured coverage. */
  {
    char *old_id = g_coverage_profile_id;
    char *old_win = g_coverage_profile_id_windows;
    const char *conf = "build/test-coverage-ratchet/profile.ini";
    g_coverage_profile_id = 0;
    g_coverage_profile_id_windows = 0;
    f = fopen(conf, "wb");
    if (!f) return 1;
    fputs("[coverage]\nid_windows = windows-gcc-real\nid = linux-gcc-real\n", f);
    if (fclose(f) != 0 || ini_load_file(conf) != 0) return 1;
    if (!streq(cov_id_for_os("linux"), "linux-gcc-real")) failures++;
    if (!streq(cov_id_for_os("windows"), "windows-gcc-real")) failures++;
    free(g_coverage_profile_id); free(g_coverage_profile_id_windows);
    g_coverage_profile_id = old_id;
    g_coverage_profile_id_windows = old_win;
    if (remove(conf) != 0) failures++;
  }
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

  /* A malformed or incomplete measurement must never become an accepted
   * baseline. Exercise the on-disk record reader, not just cov_compare(). */
  {
    CovRecord parsed;
    const char *record = "build/test-coverage-ratchet/record.txt";
    f = fopen(record, "wb");
    if (!f) return 1;
    fputs("tack-coverage-v1\nprofile=linux-gcc14\nos=linux\ncompiler=gcc\n"
          "covered=7\ntotal=10\nsources_hash=0000007b\nsource_count=1\n"
          "source_state=00000001\ntests_state=00000002\nflags_state=0000002c\n"
          "tests_total=25\ntests_passed=24\ntests_skipped=1\n", f);
    if (fclose(f) != 0 || cov_read_record(record, &parsed) != 0) return 1;
    if (parsed.covered != 7ul || parsed.total != 10ul ||
        parsed.sources_hash != 123ul || parsed.tests_passed != 24) failures++;
    f = fopen(record, "wb");
    if (!f) return 1;
    fputs("tack-coverage-v1\nprofile=linux-gcc14\nos=linux\n"
          "covered=7\ntotal=10\nsources_hash=0000007b\n"
          "source_state=00000001\ntests_state=00000002\nflags_state=0000002c\n", f);
    if (fclose(f) != 0 || cov_read_record(record, &parsed) == 0) failures++;
    f = fopen(record, "wb");
    if (!f) return 1;
    fputs("tack-coverage-v1\nprofile=linux-gcc14\nos=linux\ncompiler=gcc\n"
          "covered=11\ntotal=10\nsources_hash=0000007b\nsource_count=1\n"
          "source_state=00000001\ntests_state=00000002\nflags_state=0000002c\n", f);
    if (fclose(f) != 0 || cov_read_record(record, &parsed) == 0) failures++;
    f = fopen(record, "wb");
    if (!f) return 1;
    fputs("tack-coverage-v1\nprofile=linux-gcc14\nos=linux\ncompiler=gcc\n"
          "covered=7\ntotal=10\nsources_hash=0000007b\nsource_count=1\n"
          "source_state=00000001\ntests_state=00000002\nflags_state=0000002c\n"
          "unknown_field=1\n", f);
    if (fclose(f) != 0 || cov_read_record(record, &parsed) == 0) failures++;
    if (remove(record) != 0) failures++;
  }

  /* Verify canonical inventory remains stable on Windows-style paths and
   * CRLF checkouts, while a real changed assertion is still rejected. */
  {
    char windows_path[] = "./tests\\cache_restore_after_clean_test.c";
    char unix_path[] = "tests/cache_restore_after_clean_test.c";
    unsigned long original_hash, lf_hash, crlf_hash, changed_hash;
    const char *temp = "build/test-coverage-ratchet/inventory-text.c";
    cov_test_canonical_path(windows_path);
    if (!streq(windows_path, unix_path)) failures++;
    if (cov_test_source_hash(unix_path, &original_hash) != 0) return 1;
    f = fopen(temp, "wb");
    if (!f) return 1;
    fputs("int main(void) {\n  return 0;\n}\n", f);
    if (fclose(f) != 0 || cov_test_source_hash(temp, &lf_hash) != 0) return 1;
    if (file_hash32_fnv1a(temp, &original_hash) != 0 || original_hash != lf_hash) failures++;
    f = fopen(temp, "wb");
    if (!f) return 1;
    fputs("int main(void) {\r\n  return 0;\r\n}\r\n", f);
    if (fclose(f) != 0 || cov_test_source_hash(temp, &crlf_hash) != 0) return 1;
    if (lf_hash != crlf_hash) failures++;
    f = fopen(temp, "wb");
    if (!f) return 1;
    fputs("int main(void) {\r\n  return 1;\r\n}\r\n", f);
    if (fclose(f) != 0 || cov_test_source_hash(temp, &changed_hash) != 0) return 1;
    if (crlf_hash == changed_hash) failures++;
    if (remove(temp) != 0) failures++;
  }

  if (failures) { fprintf(stderr, "coverage_ratchet_test: %d failures\n", failures); return 1; }
  puts("coverage_ratchet_test: ok");
  return 0;
}
