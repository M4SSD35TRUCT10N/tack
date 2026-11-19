#include "tests/tack_test_embed.h"

/* M2 bounded compile/link pool, with shared-fixture test programs executed
 * sequentially. All generated sources and artifacts live under build/. */
static int m2_write_fixture(const char *path, int invalid) {
  FILE *f = fopen(path, "wb");
  if (!f) return 1;
  if (invalid) fputs("int main(void) { this is invalid C; }\n", f);
  else fputs("int main(void) { return 0; }\n", f);
  return fclose(f) != 0;
}

int main(void) {
  const char *fixtures = "build/m2-parallel-fixtures";
  const char *saved_tests = g_tests_dir;
  const char *sources[] = {
    "build/m2-parallel-fixtures/m2_pool_a_test.c",
    "build/m2-parallel-fixtures/m2_pool_b_test.c",
    "build/m2-parallel-fixtures/m2_pool_c_test.c",
    "build/m2-parallel-fixtures/m2_pool_d_test.c"
  };
  TestRunStats serial, parallel, cached, fail;
  int i, failures = 0;

  if (config_auto_load() != 0) {
    fprintf(stderr, "parallel_test_build_test: cannot read tack.ini\n");
    return 1;
  }
  if (file_exists(fixtures) && rm_rf(fixtures) != 0) return 1;
  ensure_dir_recursive(fixtures);
  for (i = 0; i < 4; i++) {
    if (m2_write_fixture(sources[i], 0) != 0) return 1;
  }

  g_tests_dir = fixtures;
  if (build_and_run_tests(default_target_name(), PROF_DEBUG,
                          0, 1, 1, 0, &serial) != 0 ||
      serial.total != 4 || serial.compiled != 4 ||
      serial.passed != 4 || serial.failed != 0 ||
      serial.skipped != 0 || serial.not_run != 0) {
    fprintf(stderr, "parallel_test_build_test: serial baseline failed\n");
    failures++;
  }
  if (build_and_run_tests(default_target_name(), PROF_DEBUG,
                          0, 1, 4, 0, &parallel) != 0 ||
      parallel.total != 4 || parallel.compiled != 4 ||
      parallel.passed != 4 || parallel.failed != 0 ||
      parallel.skipped != 0 || parallel.not_run != 0) {
    fprintf(stderr, "parallel_test_build_test: parallel run failed\n");
    failures++;
  }
  /* The pool must really start multiple compiler processes before waiting,
   * not just accept -j 4 and run all compilers one at a time. */
  if (g_test_compile_peak_pending < 2) {
    fprintf(stderr, "parallel_test_build_test: -j ignored (peak=%d)\n",
            g_test_compile_peak_pending);
    failures++;
  }
  if (build_and_run_tests(default_target_name(), PROF_DEBUG,
                          0, 0, 4, 0, &cached) != 0 ||
      cached.total != 4 || cached.compiled != 0 ||
      cached.passed != 4 || cached.failed != 0 ||
      cached.skipped != 0 || cached.not_run != 0) {
    fprintf(stderr, "parallel_test_build_test: incremental result failed\n");
    failures++;
  }
  /* A compile failure must stop the run, drain all started children and
   * report remaining tests as NOT RUN, never pretend the suite passed. */
  if (m2_write_fixture("build/m2-parallel-fixtures/m2_pool_bad_test.c", 1) != 0)
    failures++;
  else if (build_and_run_tests(default_target_name(), PROF_DEBUG,
                               0, 1, 4, 0, &fail) == 0 ||
           fail.total != 5 || fail.failed != 1 ||
           fail.passed != 0 || fail.not_run != 4) {
    fprintf(stderr, "parallel_test_build_test: fail-fast/drain failed\n");
    failures++;
  }
  /* Exercise the two-stage GCC coverage builder with four independent
   * objects and notes under build/. No gcov/ratchet is invoked here. */
  if (fmt_exe_in_path("gcc")) {
    StrVec cov_sources;
    EffectiveOverride cov_ov;
    const char *incs[] = { "include", "tests", "src", 0 };
    int built[4] = {0, 0, 0, 0}, compiled = 0, failed_index = 0;
    int stage_rc, k;
    const char *saved_target = g_active_compiler_target;
    Profile saved_profile = g_active_compiler_profile;
    char exe[1024];
    ensure_dir_recursive("build/m2-parallel-fixtures/coverage-obj");
    ensure_dir_recursive("build/m2-parallel-fixtures/coverage-bin");
    sv_init(&cov_sources);
    for (k = 0; k < 4; k++) sv_push(&cov_sources, sources[k]);
    set_active_compiler_target(default_target_name(), PROF_COVERAGE);
    get_effective_override(default_target_name(), PROF_COVERAGE, &cov_ov);
    stage_rc = test_build_parallel(&cov_sources, "gcc", PROF_COVERAGE,
                                   &cov_ov, incs,
                                   "build/m2-parallel-fixtures/coverage-bin",
                                   "build/m2-parallel-fixtures/coverage-obj",
                                   1, 0, 0, 4, built, &compiled, &failed_index);
    g_active_compiler_target = saved_target;
    g_active_compiler_profile = saved_profile;
    if (stage_rc != 0 || compiled != 4) {
      fprintf(stderr, "parallel_test_build_test: coverage link pool failed\n");
      failures++;
    } else {
      for (k = 0; k < 4; k++) {
        test_build_exe_path(exe, sizeof(exe),
                            "build/m2-parallel-fixtures/coverage-bin",
                            sources[k]);
        if (!file_exists(exe)) failures++;
      }
    }
    sv_free(&cov_sources);
  }
  /* A fake compiler that returns 0 but writes no output MUST fail closed,
   * and must never execute an old binary from the prior successful build. */
  {
    StrVec fake_sources;
    EffectiveOverride dbg_ov;
    const char *incs[] = { "include", "tests", "src", 0 };
    char fake_cc[1024];
    int built[2] = {0, 0}, count = 0, failed_index = 0;
    int fake_rc;
    ensure_dir_recursive("build/m2-parallel-fixtures/fake-bin");
    test_build_exe_path(fake_cc, sizeof(fake_cc),
                        "build/tests/debug/bin", sources[0]);
    sv_init(&fake_sources);
    sv_push(&fake_sources, sources[0]);
    sv_push(&fake_sources, sources[1]);
    get_effective_override(default_target_name(), PROF_DEBUG, &dbg_ov);
    fake_rc = test_build_parallel(&fake_sources, fake_cc, PROF_DEBUG,
                                  &dbg_ov, incs,
                                  "build/m2-parallel-fixtures/fake-bin",
                                  "build/m2-parallel-fixtures/coverage-obj",
                                  1, 0, 0, 2, built, &count, &failed_index);
    if (fake_rc == 0 || count != 2 || failed_index != 0) {
      fprintf(stderr, "parallel_test_build_test: zero-exit missing output accepted\n");
      failures++;
    }
    sv_free(&fake_sources);
  }
  g_tests_dir = saved_tests;
  if (file_exists(fixtures) && rm_rf(fixtures) != 0) failures++;
  config_free();
  if (failures) {
    fprintf(stderr, "parallel_test_build_test: %d failures\n", failures);
    return 1;
  }
  puts("parallel_test_build_test: ok");
  return 0;
}
