#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
/* Deterministic Win32 directory-enumeration fault injection. The Windows
 * API calls in tack.c are redirected only in this embedded test translation
 * unit. Normal calls pass through; armed calls fail with ACCESS_DENIED. */
#include <windows.h>
static int fs_fail_find_first = 0;
static int fs_fail_find_next = 0;
static int fs_enum_fault_hits = 0;

static HANDLE WINAPI fs_test_find_first(LPCSTR pattern, LPWIN32_FIND_DATAA data) {
  if (fs_fail_find_first) {
    fs_fail_find_first = 0;
    fs_enum_fault_hits++;
    SetLastError(ERROR_ACCESS_DENIED);
    return INVALID_HANDLE_VALUE;
  }
  return FindFirstFileA(pattern, data);
}

static BOOL WINAPI fs_test_find_next(HANDLE handle, LPWIN32_FIND_DATAA data) {
  if (fs_fail_find_next) {
    fs_fail_find_next = 0;
    fs_enum_fault_hits++;
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
  }
  return FindNextFileA(handle, data);
}

#define FindFirstFileA fs_test_find_first
#define FindNextFileA fs_test_find_next
#endif
#include "tests/tack_test_embed.h"
#ifdef _WIN32
#undef FindFirstFileA
#undef FindNextFileA
#endif

#ifndef _WIN32
/* Strict C89 headers may hide symlink() under POSIX feature macros. */
extern int symlink(const char *target, const char *linkpath);
#endif

static int fs_expect_int(const char *label, int got, int want) {
  if (got == want) return 0;
  fprintf(stderr, "path_filesystem_safety_test: %s got=%d expected=%d\n", label, got, want);
  return 1;
}

static int fs_expect_string(const char *label, const char *got, const char *want) {
  if (strcmp(got, want) == 0) return 0;
  fprintf(stderr, "path_filesystem_safety_test: %s got='%s' expected='%s'\n", label, got, want);
  return 1;
}

static int fs_file(const char *path, const char *content) {
  FILE *f = fopen(path, "wb");
  if (!f) { fprintf(stderr, "cannot create fixture: %s\n", path); return 1; }
  if (fputs(content, f) < 0) { fclose(f); return 1; }
  return fclose(f) == 0 ? 0 : 1;
}

static int fs_test_child(const char *exe, const char *mode, const char *arg) {
  char *argv[4];
  int rc;
  argv[0] = (char*)exe;
  argv[1] = (char*)mode;
  argv[2] = (char*)arg;
  argv[3] = 0;
  rc = run_argv_wait(argv, 0);
  if (rc != 2) {
    fprintf(stderr, "path_filesystem_safety_test: expected fail-fast exit 2: %s got=%d\n", mode, rc);
    return 1;
  }
  return 0;
}

#ifdef _WIN32
/* Some TinyCC Windows headers omit CreateSymbolicLinkA. Resolve the API
 * at runtime rather than depending on a header declaration/import stub.
 * This also makes the absence of the API an explicit fixture SKIP. */
typedef BOOLEAN (WINAPI *FsCreateSymbolicLinkFn)(LPCSTR, LPCSTR, DWORD);

static int fs_create_symlink(const char *target, const char *link_name,
                             DWORD flags) {
  HMODULE kernel = GetModuleHandleA("kernel32.dll");
  FARPROC proc;
  FsCreateSymbolicLinkFn create_link;
  if (!kernel) return 1;
  proc = GetProcAddress(kernel, "CreateSymbolicLinkA");
  if (!proc) return 1;
  /* Win32 function-pointer representations must agree. Copy object bytes
   * instead of casting incompatible function types: GCC -Wcast-function-type
   * rejects FARPROC -> the typed CreateSymbolicLinkA signature under -Werror.
   * No pointer-to-object conversion, no non-C89 extension. */
  if (sizeof(create_link) != sizeof(proc)) return 1;
  memcpy(&create_link, &proc, sizeof(create_link));
  return create_link(link_name, target, flags) != 0 ? 0 : 1;
}
#endif

static int fs_link_dir(const char *target, const char *link_name) {
#ifdef _WIN32
  /* Developer Mode permits the unprivileged-creation flag on modern Windows. */
  if (fs_create_symlink(target, link_name, 3u) == 0) return 0;
  if (fs_create_symlink(target, link_name, 1u) == 0) return 0;
  return 1;
#else
  return symlink(target, link_name) == 0 ? 0 : 1;
#endif
}

static int fs_link_file(const char *target, const char *link_name) {
#ifdef _WIN32
  if (fs_create_symlink(target, link_name, 2u) == 0) return 0;
  if (fs_create_symlink(target, link_name, 0u) == 0) return 0;
  return 1;
#else
  return symlink(target, link_name) == 0 ? 0 : 1;
#endif
}

int main(int argc, char **argv) {
  int errors = 0;
  char a[512], b[512], c[512], d[512], e[512], root[512], outside[512];
  char file[512], marker[512], link[512], rootlink[512], dangling[512];
  char outside_abs[1024], marker_abs[1024], cwd[512], pid_text[32];
  StrVec files;
  int created_dir_link = 0;
  int created_file_link = 0;
  const char *invalid_src[] = {
    "", ".", "..", "../escape", "sub/../escape", "sub\\..\\escape",
    "sub/..\\escape", "sub\\../escape", "/tmp/code", "\\\\host\\share",
    "//host/share", "C:\\code", "c:/code", "C:relative", "x//y",
    "x\\\\y", "x/", "x\\", "x/./y", "x\\.\\y", "x/:stream",
    "x/file:stream", "x/file?", "x/file*", "x/file|x", "x/<file>",
    "x/file\"x", "x/a. ", "x/a.", "x/CON", "x/CON.txt",
    "x/aux", "x/NUL.txt", "x/com1", "x/LPT9.txt", "x/a\tbad", 0
  };
  const char *valid_src[] = {
    "src", "src/app", "src\\app", "some folder/sub-dir", "x/...y",
    "x/a..b", "x/.hidden", "x/com10", "x/console.c", 0
  };
  const char *invalid_id[] = {
    "", ".", "..", "../x", "x/y", "C:foo", "name..part",
    "CON", "Con.txt", "aux", "NUL", "com1", "lpt9", "x.", 0
  };
  const char *valid_id[] = {
    "app", "app_dbg-1", "my.app", "a.b", "com10", "console", ".hidden", 0
  };
  int i;

  if (argc >= 2) {
    if (streq(argv[1], "--join-overflow")) {
      path_join(a, 4u, "abc", "d");
      return 0;
    }
    if (streq(argv[1], "--scan-limit")) {
      sv_init(&files);
      scan_dir_recursive_suffix_skip_depth(&files, ".", ".c", 0, TACK_MAX_SCAN_DEPTH + 1);
      return 0;
    }
    if (streq(argv[1], "--rm-limit")) {
      return rm_rf_depth("does-not-exist", TACK_MAX_RM_DEPTH + 1);
    }
    if (streq(argv[1], "--scan-root-link") && argc >= 3) {
      sv_init(&files);
      scan_dir_recursive_suffix(&files, argv[2], ".c");
      sv_free(&files);
      return 0;
    }
#ifdef _WIN32
    if ((streq(argv[1], "--scan-enum-first") ||
         streq(argv[1], "--scan-enum-next")) && argc >= 3) {
      sv_init(&files);
      if (streq(argv[1], "--scan-enum-first")) fs_fail_find_first = 1;
      else fs_fail_find_next = 1;
      scan_dir_recursive_suffix(&files, argv[2], ".c");
      sv_free(&files);
      /* Injected scan failure must terminate with tack_die / exit 2. */
      return 4;
    }
#endif
    return 3;
  }

  /* M1 S01-S09: validated paths remain portable in safe mode. */
  for (i = 0; invalid_src[i]; i++) {
    if (is_safe_relative_path(invalid_src[i])) {
      fprintf(stderr, "accepted unsafe src: '%s'\n", invalid_src[i]); errors++;
    }
  }
  for (i = 0; valid_src[i]; i++) {
    if (!is_safe_relative_path(valid_src[i])) {
      fprintf(stderr, "rejected safe src: '%s'\n", valid_src[i]); errors++;
    }
  }
  for (i = 0; invalid_id[i]; i++) {
    if (is_safe_simple_token(invalid_id[i])) {
      fprintf(stderr, "accepted unsafe id: '%s'\n", invalid_id[i]); errors++;
    }
  }
  for (i = 0; valid_id[i]; i++) {
    if (!is_safe_simple_token(valid_id[i])) {
      fprintf(stderr, "rejected safe id: '%s'\n", valid_id[i]); errors++;
    }
  }
  g_allow_unsafe_paths_cli = 0;
  g_config_loaded = 0;
  g_config_allow_unsafe_paths = 0;
  errors += fs_expect_int("default-id-traversal", validate_target_path_value("id", "../wrong"), 2);
  errors += fs_expect_int("default-bin-traversal", validate_target_path_value("bin", "../wrong"), 2);
  errors += fs_expect_int("default-src-traversal", validate_target_path_value("src", "a/../wrong"), 2);
  g_config_loaded = 1; g_config_allow_unsafe_paths = 1;
  errors += fs_expect_int("ini-explicit-opt-in", validate_target_path_value("src", "../outside"), 0);
  g_config_loaded = 0; g_config_allow_unsafe_paths = 0;
  {
    TargetVec tv;
    TargetDef d;
    tv_init(&tv);
    tv_push(&tv, "app", "src", "app");
    memset(&d, 0, sizeof(d));
    d.name = "app"; d.id = "Con.txt"; d.enabled = 1;
    errors += fs_expect_int("target-id-device-name", tv_apply_targetdef(&tv, &d), 2);
    d.id = 0; d.bin_base = "LPT1";
    errors += fs_expect_int("target-bin-device-name", tv_apply_targetdef(&tv, &d), 2);
    d.bin_base = 0; d.src_dir = "safe/../escape";
    errors += fs_expect_int("target-mixed-traversal", tv_apply_targetdef(&tv, &d), 2);
    tv_free(&tv);
  }
  g_allow_unsafe_paths_cli = 1;
  errors += fs_expect_int("cli-explicit-opt-in", validate_target_path_value("id", "../outside"), 0);
  g_allow_unsafe_paths_cli = 0;
  errors += fs_expect_int("safe-restored", validate_target_path_value("src", "../outside"), 2);

  /* M1 S10/S11: exact fit including NUL and separator, never truncate. */
  path_join(a, 8u, "a", "12345");
#ifdef _WIN32
  errors += fs_expect_string("join-exact-capacity", a, "a\\12345");
  path_join(a, 8u, "a\\", "12345");
  errors += fs_expect_string("join-existing-separator", a, "a\\12345");
#else
  errors += fs_expect_string("join-exact-capacity", a, "a/12345");
  path_join(a, 8u, "a/", "12345");
  errors += fs_expect_string("join-existing-separator", a, "a/12345");
#endif
  errors += fs_expect_int("cleanup-refuse-dot", is_dangerous_cleanup_root("."), 1);
  errors += fs_expect_int("cleanup-refuse-dotdot", is_dangerous_cleanup_root(".."), 1);
  errors += fs_expect_int("cleanup-refuse-posix-root", is_dangerous_cleanup_root("/"), 1);
  errors += fs_expect_int("cleanup-refuse-drive-root", is_dangerous_cleanup_root("C:\\"), 1);
  errors += fs_expect_int("cleanup-refuse-relative-root", is_dangerous_cleanup_root("./"), 1);
  errors += fs_expect_int("cleanup-refuse-parent-escape", is_dangerous_cleanup_root("build/../outside"), 1);
  errors += fs_expect_int("cleanup-allow-build", is_dangerous_cleanup_root("build"), 0);
  errors += fs_expect_int("cleanup-allow-cache", is_dangerous_cleanup_root(".tack-cache"), 0);
  errors += fs_test_child(argv[0], "--join-overflow", "");
  errors += fs_test_child(argv[0], "--scan-limit", "");
  errors += fs_test_child(argv[0], "--rm-limit", "");
  {
    char *j = path_join_alloc("", "abc");
    errors += fs_expect_string("alloc-empty-left", j, "abc");
    free(j);
    j = path_join_alloc("abc", "");
#ifdef _WIN32
    errors += fs_expect_string("alloc-empty-right", j, "abc\\");
#else
    errors += fs_expect_string("alloc-empty-right", j, "abc/");
#endif
    free(j);
    j = path_join_alloc(0, "abc");
    errors += fs_expect_string("alloc-null-left", j, "abc");
    free(j);
  }

  /* Isolated non-production fixture: two sibling trees with a sentinel. */
#ifdef _WIN32
  sprintf(pid_text, "%lu", (unsigned long)_getpid());
#else
  sprintf(pid_text, "%lu", (unsigned long)getpid());
#endif
  tack_snprintf(root, sizeof(root), "build/m1_fs_inside_%s", pid_text);
  tack_snprintf(outside, sizeof(outside), "build/m1_fs_outside_%s", pid_text);
  if (file_exists(root) || file_exists(outside)) {
    fprintf(stderr, "fixture path unexpectedly exists\n"); return 1;
  }
  ensure_dir_recursive("build");
  ensure_dir_recursive(root);
  ensure_dir_recursive(outside);
  path_join(marker, sizeof(marker), outside, "DO_NOT_DELETE.c");
  path_join(file, sizeof(file), root, "safe.c");
  path_join(a, sizeof(a), root, "deep");
  ensure_dir_recursive(a);
  path_join(b, sizeof(b), a, "inner.c");
  if (fs_file(marker, "/* external sentinel */\n") ||
      fs_file(file, "/* internal file */\n") ||
      fs_file(b, "/* internal nested file */\n")) return 1;
#ifdef _WIN32
  if (!_getcwd(cwd, (int)sizeof(cwd))) return 1;
#else
  if (!getcwd(cwd, sizeof(cwd))) return 1;
#endif
  path_join(outside_abs, sizeof(outside_abs), cwd, outside);
  path_join(marker_abs, sizeof(marker_abs), cwd, marker);
  path_join(link, sizeof(link), root, "linked-dir");
  if (fs_link_dir(outside_abs, link) == 0) created_dir_link = 1;
  else fprintf(stderr, "path_filesystem_safety_test: Windows directory symlink fixture SKIP (permission)\n");
  path_join(c, sizeof(c), root, "linked-file.c");
  if (fs_link_file(marker_abs, c) == 0) created_file_link = 1;
  else fprintf(stderr, "path_filesystem_safety_test: Windows file symlink fixture SKIP (permission)\n");
  tack_snprintf(rootlink, sizeof(rootlink), "build/m1_fs_rootlink_%s", pid_text);
  if (path_exists_nofollow(rootlink)) { fprintf(stderr, "rootlink fixture collision\n"); return 1; }
  if (created_dir_link && fs_link_dir(outside_abs, rootlink) == 0) {
    errors += fs_test_child(argv[0], "--scan-root-link", rootlink);
    path_join(d, sizeof(d), outside, "nested-dir");
    ensure_dir_recursive(d);
    path_join(e, sizeof(e), link, "nested-dir");
    errors += fs_test_child(argv[0], "--scan-root-link", e);
  } else {
    fprintf(stderr, "path_filesystem_safety_test: root-link probe SKIP (permission)\n");
  }
  sv_init(&files);
  scan_dir_recursive_suffix(&files, root, ".c");
  errors += fs_expect_int("scanner-only-real-files", files.count, 2);
  sv_free(&files);
  errors += fs_expect_int("outside-sentinel-preserved-before-rm", file_exists(marker), 1);
  if (created_file_link) {
    errors += fs_expect_int("file-link-exists", path_exists_nofollow(c), 1);
  }

  /* A root symlink must never be traversed by rm_rf_contents. */
  if (path_exists_nofollow(rootlink)) {
    errors += fs_expect_int("contents-refuse-symlink-root", rm_rf_contents(rootlink), 1);
    errors += fs_expect_int("outside-sentinel-after-refusal", file_exists(marker), 1);
  }
  /* M1 S13: remove links and in-tree content, without touching sibling. */
  errors += fs_expect_int("remove-contents", rm_rf_contents(root), 0);
  errors += fs_expect_int("outside-sentinel-after-rm", file_exists(marker), 1);
  sv_init(&files);
  scan_dir_recursive_suffix(&files, root, ".c");
  errors += fs_expect_int("inside-now-empty", files.count, 0);
  sv_free(&files);

#ifdef _WIN32
  /* M1 S15: FindFirst/FindNext failures must never masquerade as an empty
   * source or a successful recursive cleanup. Only isolated build/ fixtures
   * are touched; the external sentinel is checked after injected errors. */
  {
    char enumdir[512], enumfile[512];
    path_join(enumdir, sizeof(enumdir), root, "enum-fault");
    ensure_dir_recursive(enumdir);
    path_join(enumfile, sizeof(enumfile), enumdir, "probe.c");
    if (fs_file(enumfile, "/* enumeration fixture */\n")) return 1;

    errors += fs_test_child(argv[0], "--scan-enum-first", enumdir);
    errors += fs_test_child(argv[0], "--scan-enum-next", enumdir);

    fs_enum_fault_hits = 0;
    fs_fail_find_first = 1;
    errors += fs_expect_int("rm-contents-enum-first-fails", rm_rf_contents(enumdir), 1);
    errors += fs_expect_int("rm-contents-first-fault-fired", fs_enum_fault_hits, 1);
    errors += fs_expect_int("rm-contents-first-keeps-file", file_exists(enumfile), 1);

    fs_enum_fault_hits = 0;
    fs_fail_find_next = 1;
    errors += fs_expect_int("rm-contents-enum-next-fails", rm_rf_contents(enumdir), 1);
    errors += fs_expect_int("rm-contents-next-fault-fired", fs_enum_fault_hits, 1);
    errors += fs_expect_int("rm-contents-next-keeps-dir", path_exists_nofollow(enumdir), 1);

    /* Crucial regression: an empty directory with a failed FindFirstFileA
     * must NOT be removed and reported as successful. */
    fs_enum_fault_hits = 0;
    fs_fail_find_first = 1;
    errors += fs_expect_int("rm-depth-enum-first-fails", rm_rf(enumdir), 1);
    errors += fs_expect_int("rm-depth-first-fault-fired", fs_enum_fault_hits, 1);
    errors += fs_expect_int("rm-depth-first-keeps-dir", path_exists_nofollow(enumdir), 1);

    if (fs_file(enumfile, "/* enumerate next */\n")) return 1;
    fs_enum_fault_hits = 0;
    fs_fail_find_next = 1;
    errors += fs_expect_int("rm-depth-enum-next-fails", rm_rf(enumdir), 1);
    errors += fs_expect_int("rm-depth-next-fault-fired", fs_enum_fault_hits, 1);
    errors += fs_expect_int("rm-depth-next-keeps-dir", path_exists_nofollow(enumdir), 1);
    errors += fs_expect_int("enum-fault-external-sentinel", file_exists(marker), 1);
    errors += fs_expect_int("enum-fault-cleanup", rm_rf(enumdir), 0);
  }
#endif

  path_join(dangling, sizeof(dangling), root, "dangling-link");
  if (fs_link_file("definitely-nonexistent-m1", dangling) == 0) {
    errors += fs_expect_int("dangling-link-detected", path_exists_nofollow(dangling), 1);
    errors += fs_expect_int("dangling-link-removed", rm_rf(dangling), 0);
    errors += fs_expect_int("dangling-link-gone", path_exists_nofollow(dangling), 0);
  } else fprintf(stderr, "path_filesystem_safety_test: dangling symlink fixture SKIP (permission)\n");

#ifdef _WIN32
  /* Windows: an exclusively opened file must yield a deletion failure. */
  {
    HANDLE lock;
    path_join(d, sizeof(d), root, "locked-file.c");
    if (fs_file(d, "/* locked */\n")) return 1;
    lock = CreateFileA(d, GENERIC_READ, 0u, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (lock == INVALID_HANDLE_VALUE) {
      fprintf(stderr, "path_filesystem_safety_test: Windows lock probe SKIP (cannot open)\n");
    } else {
      errors += fs_expect_int("locked-file-removal-fails", rm_rf(d), 1);
      errors += fs_expect_int("locked-file-still-there", path_exists_nofollow(d), 1);
      CloseHandle(lock);
    }
    errors += fs_expect_int("locked-file-removal-after-close", rm_rf(d), 0);
  }
#else
  /* A directory scan must not treat EACCES as an empty source tree. */
  if (geteuid() != 0) {
    path_join(d, sizeof(d), outside, "scan-denied");
    ensure_dir_recursive(d);
    if (chmod(d, 0000) != 0) errors++;
    else {
      errors += fs_test_child(argv[0], "--scan-root-link", d);
      if (chmod(d, 0700) != 0) errors++;
    }
    errors += fs_expect_int("remove-scan-denied-fixture", rm_rf(d), 0);
  }
  /* Permission-denied unlink must fail, not be reported as success. */
  if (geteuid() != 0) {
    path_join(d, sizeof(d), root, "locked");
    ensure_dir_recursive(d);
    path_join(e, sizeof(e), d, "keep.c");
    if (fs_file(e, "/* inaccessible */\n")) return 1;
    if (chmod(d, 0500) != 0) errors++;
    else {
      errors += fs_expect_int("rm-permission-denied", rm_rf(d), 1);
      errors += fs_expect_int("locked-sentinel-preserved", file_exists(e), 1);
      if (chmod(d, 0700) != 0) errors++;
    }
    errors += fs_expect_int("rm-after-permission-restored", rm_rf(d), 0);
  } else fprintf(stderr, "path_filesystem_safety_test: permission probe SKIP (root user)\n");
#endif
  /* All fixture cleanup is restricted to paths constructed in build/. */
  errors += fs_expect_int("remove-root", rm_rf(root), 0);
  if (path_exists_nofollow(rootlink)) errors += fs_expect_int("remove-rootlink", rm_rf(rootlink), 0);
  errors += fs_expect_int("remove-outside-fixture", rm_rf(outside), 0);
  errors += fs_expect_int("root-gone", path_exists_nofollow(root), 0);
  errors += fs_expect_int("outside-gone", path_exists_nofollow(outside), 0);

  if (errors) {
    fprintf(stderr, "path_filesystem_safety_test: %d failure(s)\n", errors);
    return 1;
  }
  puts("path_filesystem_safety_test: ok");
  return 0;
}
