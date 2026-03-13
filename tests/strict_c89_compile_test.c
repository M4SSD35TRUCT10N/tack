#include "tack_test_embed.h"

int main(void) {
  char out_obj[512];
  const char *argv[12];
  int i = 0;
  const char *cc = get_cc();

  if (!fmt_exe_in_path(cc)) {
    puts("strict_c89_compile_test: skipped (compiler not found)");
    return 0;
  }
  if (compiler_name_is_tcc(cc)) {
    puts("strict_c89_compile_test: skipped (tcc path)");
    return 0;
  }

  ensure_dir("build");
  ensure_dir("build/test-strict-c89");
  path_join(out_obj, sizeof(out_obj), "build/test-strict-c89", "tack.o");
  if (file_exists(out_obj) && rm_rf(out_obj) != 0) {
    fprintf(stderr, "failed to remove old output\n");
    return 1;
  }

  argv[i++] = cc;
  argv[i++] = "-std=c89";
  argv[i++] = "-O2";
  argv[i++] = "-Wall";
  argv[i++] = "-Wextra";
  argv[i++] = "-pedantic";
  argv[i++] = "-Werror";
  argv[i++] = "-c";
  argv[i++] = "src/tack.c";
  argv[i++] = "-o";
  argv[i++] = out_obj;
  argv[i] = 0;

  if (run_argv_wait_const(argv, 0) != 0) {
    fprintf(stderr, "strict pedantic compile failed\n");
    return 1;
  }
  if (!file_exists(out_obj)) {
    fprintf(stderr, "strict pedantic output missing\n");
    return 1;
  }

  puts("strict_c89_compile_test: ok");
  return 0;
}
