#include "tack_test_embed.h"

static int expect_rc(const char *label, int got, int want) {
  if (got == want) return 0;
  fprintf(stderr, "%s: got %d want %d\n", label, got, want);
  return 1;
}

int main(void) {
  int failures = 0;
  TargetVec tv;
  TargetDef d;

  tv_init(&tv);
  tv_push(&tv, "app", "src", "app");

  g_allow_unsafe_paths_cli = 0;
  g_config_loaded = 0;
  g_config_allow_unsafe_paths = 0;

  memset(&d, 0, sizeof(d));
  d.name = "app";
  d.id = "../pwned";
  d.enabled = 1;
  failures += expect_rc("reject-id-traversal", tv_apply_targetdef(&tv, &d), 2);

  memset(&d, 0, sizeof(d));
  d.name = "app";
  d.bin_base = "../../evil";
  d.enabled = 1;
  failures += expect_rc("reject-bin-traversal", tv_apply_targetdef(&tv, &d), 2);

  memset(&d, 0, sizeof(d));
  d.name = "app";
  d.src_dir = "../src";
  d.enabled = 1;
  failures += expect_rc("reject-src-traversal", tv_apply_targetdef(&tv, &d), 2);

  memset(&d, 0, sizeof(d));
  d.name = "app";
  d.id = "app_dbg-1";
  d.bin_base = "my.app";
  d.src_dir = "src/app";
  d.enabled = 1;
  failures += expect_rc("accept-safe-values", tv_apply_targetdef(&tv, &d), 0);

  g_allow_unsafe_paths_cli = 1;
  memset(&d, 0, sizeof(d));
  d.name = "app";
  d.id = "../pwned";
  d.enabled = 1;
  failures += expect_rc("unsafe-opt-in-allows-id", tv_apply_targetdef(&tv, &d), 0);
  g_allow_unsafe_paths_cli = 0;

  tv_free(&tv);
  if (failures != 0) return 1;
  puts("path_safety_test: ok");
  return 0;
}
