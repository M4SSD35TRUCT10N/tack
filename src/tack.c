
/* tack.c - Tiny ANSI-C Kit
 * v0.7.5
 *
 * Adds:
 * - Runtime config via tack.ini (data-only)
 * - Optional code config via tackfile.c: on-the-fly compile to generated INI (fail-fast)
 * - Real per-target configuration overrides (includes/defines/cflags/ldflags/libs, core yes/no)
 * - Shared "core" code (src/core/...) built once per profile and linked into targets
 * - Keeps: recursive scanning, optional tool discovery, -j parallel compile, robust process execution
 *
 * Conventions:
 *   app            : sources under src/ (or src/app/ if exists)
 *   shared core    : sources under src/core/
 *   tools          : sources under tools/<name>/
 *   tests          : sources under tests/ (recursive _test.c files)
 *
 * Features:
 * - single file build driver (C89)
 * - no make/cmake/ninja
 * - recursive source scanning
 * - target discovery: app + tools/<name>
 * - list targets: tack list
 * - robust process execution (no system() for builds)
 * - parallel compilation: -j N
 * - strict mode: --strict enables -Wunsupported (default suppresses it)
 *
 * Env:
 *   TACK_CC: override compiler (default "tcc")
 *
 * Quickstart (Windows):
 *   tcc -run src/tack.c init
 *   tcc -run src/tack.c list
 *   tcc -run src/tack.c build debug -v -j 8
 *   tcc -run src/tack.c run debug -- --hello "world"
 *   tcc -run src/tack.c build release --target tool:foo
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
  #include <windows.h>
  #include <direct.h>
  #include <process.h>
  #define PATH_SEP '\\'
  #define STAT_FN _stat
  #define STAT_ST struct _stat
#else
  #include <dirent.h>
  #include <unistd.h>
  #include <sys/wait.h>
  #define PATH_SEP '/'
  #define STAT_FN stat
  #define STAT_ST struct stat
#endif

#define TACK_VERSION "0.7.5"

/* Hard limits for untrusted inputs (fail-fast) */
#define TACK_MAX_LINE        8192
#define TACK_MAX_TOKEN       4096
#define TACK_MAX_LIST_ITEMS   512

/* FS recursion limits (fail-fast) */
#define TACK_MAX_SCAN_DEPTH    64
#define TACK_MAX_RM_DEPTH      64
#define TACK_MAX_NAME         512
#define TACK_MAX_CC          1024
#define TACK_MAX_CONFIG_PATH 4096


/* --------------------------- runtime config (globals) --------------------------- */
/* Optional project configuration:
 * - tack.ini (data-only) auto-loads if present (or via --config)
 * - tackfile.c (code) is optional; if present, tack compiles a tiny generator that emits
 *   a generated INI layer under build/_tackfile/ (fail-fast on errors)
 * - disable config loading entirely with --no-config
 * - disable code config (tackfile.c) with --no-code-config (INI still loaded)
 *
 * Global options (must appear before the command):
 *   --no-config        ignore tack.ini and tackfile.c
 *   --no-code-config    ignore tackfile.c (still use tack.ini / --config)
 *   --config <path>    use explicit INI file (highest priority)
 *   --no-auto-tools    disable tool discovery at runtime
 *   --no-cache         disable compile cache
 */
static int g_no_config = 0;
static int g_no_code_config = 0; /* ignore tackfile.c but still load INI */
static const char *g_config_path_cli = 0;
static int g_no_auto_tools_cli = 0;
static int g_no_cache = 0;

static int g_config_loaded = 0;
static char g_config_path[TACK_MAX_CONFIG_PATH + 1] = {0};
static char *g_config_default_target = 0; /* owned; freed at exit */
static int g_config_disable_auto_tools = 0;
static char *g_config_doc_template = 0; /* owned */
static char *g_config_doc_css = 0;      /* owned */
static char *g_config_bom_template = 0; /* owned */
static char *g_config_bom_css = 0;      /* owned */
static char *g_config_sbom_format = 0;       /* owned */
static char *g_config_sbom_spec_version = 0; /* owned */
static char *g_config_sbom_output = 0;       /* owned 

static const char *g_cc_default = "tcc";
static const char *g_build_dir  = "build";
static const char *g_cache_dir  = ".tack-cache";

static const char *g_src_dir    = "src";
static const char *g_inc_dir    = "include";
static const char *g_tests_dir  = "tests";
static const char *g_tools_dir  = "tools";
static const char *g_core_dir   = "src/core";
static const char *g_app_dir    = "src/app";

static const char *g_default_target = "app";

static const char *default_target_name(void) {
  if (g_config_default_target) return g_config_default_target;
#ifdef TACKFILE_DEFAULT_TARGET
  return TACKFILE_DEFAULT_TARGET;
#else
  return g_default_target;
#endif
}

static const char *sbom_format_effective(void) {
  if (g_config_sbom_format && g_config_sbom_format[0]) return g_config_sbom_format;
  return "tack";
}

/* Warnings: keep strict, but avoid killing builds due to GCC attributes in system headers */
static const char *g_warn_flags_base[] = {
  "-Wall",
  "-Werror",
  "-Wwrite-strings",
  "-Wimplicit-function-declaration",
  "-Wno-unsupported",
  0
};
/* Optional strict: re-enable unsupported warnings */
static const char *g_warn_flags_strict_add[] = { "-Wunsupported", 0 };

/* Profiles */
typedef enum { PROF_DEBUG = 0, PROF_RELEASE = 1 } Profile;
static const char *profile_name(Profile p) { return (p == PROF_RELEASE) ? "release" : "debug"; }

/* Depfiles */
#define USE_DEPFILES 1

/* --------------------------- utilities --------------------------- */

static int streq(const char *a, const char *b) { return strcmp(a, b) == 0; }

static void *xmalloc(size_t n) {
  void *p = malloc(n);
  if (!p) { fprintf(stderr, "tack: out of memory\n"); exit(1); }
  return p;
}

static void *xrealloc(void *p, size_t n) {
  void *q = realloc(p, n);
  if (!q) { fprintf(stderr, "tack: out of memory\n"); exit(1); }
  return q;
}

static char *xstrdup(const char *s) {
  size_t n = strlen(s);
  char *p = (char*)xmalloc(n + 1);
  memcpy(p, s, n + 1);
  return p;
}

/* --------------------------- safe strings (fail-fast) --------------------------- */

static void tack_die(const char *msg) {
  fprintf(stderr, "tack: %s\n", msg);
  exit(2);
}

static void tack_copy(char *dst, size_t cap, const char *src) {
  size_t n;
  if (!dst || cap == 0) tack_die("internal error: bad buffer");
  if (!src) src = "";
  n = strlen(src);
  if (n >= cap) tack_die("string too long");
  memcpy(dst, src, n + 1);
}

static void tack_cat(char *dst, size_t cap, const char *src) {
  size_t d, s;
  if (!dst || cap == 0) tack_die("internal error: bad buffer");
  if (!src) src = "";
  d = strlen(dst);
  s = strlen(src);
  if (d + s >= cap) tack_die("string too long");
  memcpy(dst + d, src, s + 1);
}

static void tack_check_len(const char *what, const char *s, size_t maxlen) {
  size_t n;
  if (!s) return;
  n = strlen(s);
  if (n > maxlen) {
    fprintf(stderr, "tack: %s too long (max %lu)\n", what, (unsigned long)maxlen);
    exit(2);
  }
}

static const char *get_cc(void) {
  static int inited = 0;
  static char ccbuf[TACK_MAX_CC + 1];
  const char *v;

  if (inited) return ccbuf;

  v = getenv("TACK_CC");
  if (!v || !v[0]) v = g_cc_default;
  tack_check_len("TACK_CC", v, TACK_MAX_CC);

  /* trim leading whitespace */
  while (*v && isspace((unsigned char)*v)) v++;

  {
    size_t n = strlen(v);

    /* trim trailing whitespace */
    while (n > 0 && isspace((unsigned char)v[n - 1])) n--;

    if (n == 0) tack_die("TACK_CC is empty");

    /* strip surrounding quotes if present */
    if (n >= 2 && v[0] == '"' && v[n - 1] == '"') {
      v++;
      n -= 2;
      while (n > 0 && isspace((unsigned char)v[0])) { v++; n--; }
      while (n > 0 && isspace((unsigned char)v[n - 1])) n--;
      if (n == 0) tack_die("TACK_CC is empty");
    }

    if (n > TACK_MAX_CC) tack_die("TACK_CC too long");
    memcpy(ccbuf, v, n);
    ccbuf[n] = '\0';
  }

  /* CC policy: allow spaces only when this clearly looks like a path.
     Disallow "clang -std=c89" style values (use cflags/ldflags in tack.ini instead). */
  {
    int has_ws = 0, has_sep = 0, has_drive = 0;
    const char *p;
    for (p = ccbuf; *p; p++) {
      unsigned char ch = (unsigned char)*p;
      if (isspace(ch)) has_ws = 1;
      if (*p == '/' || *p == '\\') has_sep = 1;
      if (*p == ':') has_drive = 1;
    }
    if (has_ws && !has_sep && !has_drive) {
      tack_die("TACK_CC must be an executable name or path, not a command line (put flags into tack.ini)");
    }
  }

  inited = 1;
  return ccbuf;
}

static int file_exists(const char *path) {
  STAT_ST st;
  return STAT_FN(path, &st) == 0;
}

/* --------------------------- small file helpers --------------------------- */

static char *read_entire_file(const char *path, long *out_len) {
  FILE *f;
  long n;
  char *buf;

  f = fopen(path, "rb");
  if (!f) return 0;
  if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
  n = ftell(f);
  if (n < 0) { fclose(f); return 0; }
  if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 0; }

  /* sanity limit: ignore files should never be huge */
  if (n > (1024 * 1024)) { fclose(f); return 0; }

  buf = (char*)xmalloc((size_t)n + 1);
  if (n > 0) {
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(buf); return 0; }
  }
  buf[n] = '\0';
  fclose(f);

  if (out_len) *out_len = n;
  return buf;
}

static int copy_file(const char *src, const char *dst) {
  FILE *in, *out;
  unsigned char buf[65536];
  size_t n;

  in = fopen(src, "rb");
  if (!in) return 1;
  out = fopen(dst, "wb");
  if (!out) { fclose(in); return 1; }

  while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
    if (fwrite(buf, 1, n, out) != n) { fclose(in); fclose(out); return 1; }
  }

  fclose(in);
  fclose(out);
  return 0;
}

static int rename_replace(const char *src, const char *dst) {
  if (rename(src, dst) == 0) return 0;
#ifdef _WIN32
  (void)remove(dst);
  if (rename(src, dst) == 0) return 0;
#endif
  if (copy_file(src, dst) == 0) {
    (void)remove(src);
    return 0;
  }
  return 1;
}

static int file_contains_substr(const char *path, const char *needle) {
  long n = 0;
  char *s;
  int ok;

  s = read_entire_file(path, &n);
  (void)n;
  if (!s) return 0;

  ok = (strstr(s, needle) != 0);
  free(s);
  return ok;
}

static void fputs_lines(FILE *f, const char **lines) {
  int i;
  if (!f || !lines) return;
  for (i = 0; lines[i]; i++) fputs(lines[i], f);
}

static int write_file_if_missing(const char *path, const char *content) {
  FILE *f;
  if (file_exists(path)) return 0;
  f = fopen(path, "wb");
  if (!f) return 1;
  fputs(content, f);
  fclose(f);
  return 0;
}

static int append_block_if_missing(const char *path, const char *marker, const char *block) {
  FILE *f;

  if (!file_exists(path)) return write_file_if_missing(path, block);

  if (file_contains_substr(path, marker)) return 0;

  f = fopen(path, "ab");
  if (!f) return 1;

  /* separate with a newline for cleanliness */
  fputs("\n", f);
  fputs(block, f);
  fclose(f);
  return 0;
}


static int write_file_if_missing_lines(const char *path, const char **lines) {
  FILE *f;
  if (file_exists(path)) return 0;
  f = fopen(path, "wb");
  if (!f) return 1;
  fputs_lines(f, lines);
  fclose(f);
  return 0;
}

static int append_block_if_missing_lines(const char *path, const char *marker, const char **lines) {
  FILE *f;

  if (!file_exists(path)) return write_file_if_missing_lines(path, lines);
  if (file_contains_substr(path, marker)) return 0;

  f = fopen(path, "ab");
  if (!f) return 1;

  /* separate with newline for readability */
  fputs("\n", f);
  fputs_lines(f, lines);
  fclose(f);
  return 0;
}

static long file_mtime(const char *path) {
  STAT_ST st;
  if (STAT_FN(path, &st) != 0) return -1;
  return (long)st.st_mtime;
}

static long file_size(const char *path) {
  STAT_ST st;
  if (STAT_FN(path, &st) != 0) return -1;
  return (long)st.st_size;
}

static unsigned long fnv1a_update(unsigned long h, const unsigned char *data, size_t n);

static int file_hash32_fnv1a(const char *path, unsigned long *out_hash) {
  FILE *f;
  unsigned char buf[4096];
  size_t n;
  unsigned long h = 2166136261ul;

  if (!out_hash) return 1;
  *out_hash = 0;

  f = fopen(path, "rb");
  if (!f) return 1;

  while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
    h = fnv1a_update(h, buf, n);
  }

  if (ferror(f)) { fclose(f); return 1; }
  fclose(f);

  *out_hash = (h & 0xfffffffful);
  return 0;
}

static int is_dir_path(const char *path) {
  STAT_ST st;
  if (STAT_FN(path, &st) != 0) return 0;
#ifdef _WIN32
  return (st.st_mode & _S_IFDIR) != 0;
#else
  return S_ISDIR(st.st_mode);
#endif
}

#ifndef _WIN32
/* POSIX: lstat() may be hidden under feature macros in strict C89; declare it. */
extern int lstat(const char *path, struct stat *buf);
#endif

/* Directory check that does NOT follow symlinks/junctions (loop defense) */
static int is_dir_path_nofollow(const char *path) {
#ifdef _WIN32
  DWORD attr = GetFileAttributesA(path);
  if (attr == INVALID_FILE_ATTRIBUTES) return 0;
  if (attr & FILE_ATTRIBUTE_REPARSE_POINT) return 0;
  return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
  struct stat st;
  if (lstat(path, &st) != 0) return 0;
  if (S_ISLNK(st.st_mode)) return 0;
  return S_ISDIR(st.st_mode);
#endif
}

static void ensure_dir(const char *path) {
#ifdef _WIN32
  _mkdir(path);
#else
  mkdir(path, 0777);
#endif
}

static void ensure_dir_recursive(const char *path) {
  char buf[512];
  size_t i;
  size_t len;
  if (!path || !path[0]) return;
  len = strlen(path);
  if (len >= sizeof(buf)) tack_die("path too long");
  tack_copy(buf, sizeof(buf), path);
  for (i = 1; buf[i]; i++) {
    if (buf[i] == '/' || buf[i] == '\\') {
      char saved = buf[i];
      buf[i] = '\0';
      if (!(i == 2 && buf[1] == ':')) ensure_dir(buf);
      buf[i] = saved;
    }
  }
  if (!(len == 2 && buf[1] == ':')) ensure_dir(buf);
}

static void ensure_parent_dir_recursive(const char *path) {
  const char *slash = 0;
  const char *p;
  char dir[512];
  size_t len;

  if (!path || !path[0]) return;
  for (p = path; *p; p++) {
    if (*p == '/' || *p == '\\') slash = p;
  }
  if (!slash) return;
  len = (size_t)(slash - path);
  if (len == 0) return;
  if (len >= sizeof(dir)) tack_die("path too long");
  memcpy(dir, path, len);
  dir[len] = '\0';
  ensure_dir_recursive(dir);
}

static void path_join(char *out, size_t cap, const char *a, const char *b) {
  size_t la, lb, need;
  int need_sep;

  if (!out || cap == 0) tack_die("internal error: bad buffer");
  if (!a) a = "";
  if (!b) b = "";

  la = strlen(a);
  lb = strlen(b);
  need_sep = (la > 0 && a[la - 1] != PATH_SEP) ? 1 : 0;
  need = la + (size_t)need_sep + lb + 1;

  if (need > cap) tack_die("path too long");

  tack_copy(out, cap, a);
  if (need_sep) {
    size_t l = strlen(out);
    out[l] = PATH_SEP;
    out[l + 1] = '\0';
  }
  tack_cat(out, cap, b);
}

/* Allocate joined path (supports long paths; caller frees). */
static char *path_join_alloc(const char *a, const char *b) {
  size_t la, lb, need_sep;
  char *out;
  la = strlen(a);
  lb = strlen(b);
#ifdef _WIN32
  need_sep = (la > 0 && a[la - 1] != '\\' && a[la - 1] != '/') ? 1 : 0;
#else
  need_sep = (la > 0 && a[la - 1] != PATH_SEP) ? 1 : 0;
#endif
  out = (char*)xmalloc(la + need_sep + lb + 1);
  memcpy(out, a, la);
  if (need_sep) out[la] = PATH_SEP;
  memcpy(out + la + need_sep, b, lb + 1);
  return out;
}

static const char *path_base(const char *p) {
  const char *s1 = strrchr(p, '/');
  const char *s2 = strrchr(p, '\\');
  const char *s = s1 > s2 ? s1 : s2;
  return s ? (s + 1) : p;
}

static int ends_with(const char *s, const char *suffix) {
  size_t ls = strlen(s), lf = strlen(suffix);
  if (lf > ls) return 0;
  return memcmp(s + (ls - lf), suffix, lf) == 0;
}

static int is_abs_path(const char *p) {
  if (!p || !p[0]) return 0;
#ifdef _WIN32
  if (p[0] == '/' || p[0] == '\\') return 1;
  if (isalpha((unsigned char)p[0]) && p[1] == ':') return 1;
  return 0;
#else
  return p[0] == '/';
#endif
}

/* Make safe id from display name (filesystem-friendly) */
static void sanitize_name_to_id(char *out, size_t cap, const char *name) {
  size_t i = 0;
  while (*name && i + 1 < cap) {
    char c = *name++;
    if (!(isalnum((unsigned char)c) || c == '_' || c == '-')) c = '_';
    out[i++] = c;
  }
  out[i] = '\0';
}

/* Make unique-ish object id from relative source path */
static void sanitize_path_to_id(char *out, size_t cap, const char *path) {
  size_t i = 0;
  while (*path && i + 1 < cap) {
    char c = *path++;
    if (c == '/' || c == '\\' || c == '.' || c == ':' ) c = '_';
    out[i++] = c;
  }
  out[i] = '\0';
}

/* --------------------------- vectors --------------------------- */

typedef struct {
  char **items;
  int count;
  int cap;
} StrVec;

static void sv_init(StrVec *v) { v->items = 0; v->count = 0; v->cap = 0; }

static void sv_push(StrVec *v, const char *s) {
  if (v->count + 1 > v->cap) {
    int ncap = v->cap ? v->cap * 2 : 16;
    v->items = (char**)xrealloc(v->items, (size_t)ncap * sizeof(char*));
    v->cap = ncap;
  }
  v->items[v->count++] = xstrdup(s);
}

static void sv_push_own(StrVec *v, char *s) {
  if (v->count + 1 > v->cap) {
    int ncap = v->cap ? v->cap * 2 : 16;
    v->items = (char**)xrealloc(v->items, (size_t)ncap * sizeof(char*));
    v->cap = ncap;
  }
  v->items[v->count++] = s;
}


static void sv_free(StrVec *v) {
  int i;
  for (i = 0; i < v->count; i++) free(v->items[i]);
  free(v->items);
  v->items = 0; v->count = 0; v->cap = 0;
}

/* --------------------------- recursive scanning --------------------------- */

static void scan_dir_recursive_suffix_skip_depth(StrVec *out, const char *dir, const char *suffix,
                                                 const char *skip_dirname, int depth) {
#ifdef _WIN32
  WIN32_FIND_DATAA fd;
  HANDLE h;
  char *pattern;

  if (depth > TACK_MAX_SCAN_DEPTH) tack_die("directory recursion too deep");

  pattern = path_join_alloc(dir, "*");
  h = FindFirstFileA(pattern, &fd);
  free(pattern);
  if (h == INVALID_HANDLE_VALUE) return;

  do {
    char *full;

    if (streq(fd.cFileName, ".") || streq(fd.cFileName, "..")) continue;

    if (skip_dirname && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      if (streq(fd.cFileName, skip_dirname)) continue;
      if (streq(fd.cFileName, "build")) continue;
    }

    /* loop defense: do not recurse into reparse points (junctions/symlinks) */
    if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
      continue;
    }

    full = path_join_alloc(dir, fd.cFileName);

    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      scan_dir_recursive_suffix_skip_depth(out, full, suffix, skip_dirname, depth + 1);
      free(full);
    } else {
      if (ends_with(fd.cFileName, suffix)) {
        sv_push_own(out, full);
      } else {
        free(full);
      }
    }
  } while (FindNextFileA(h, &fd));

  FindClose(h);
#else
  DIR *d;
  struct dirent *e;

  if (depth > TACK_MAX_SCAN_DEPTH) tack_die("directory recursion too deep");

  d = opendir(dir);
  if (!d) return;

  while ((e = readdir(d)) != 0) {
    char *full;

    if (streq(e->d_name, ".") || streq(e->d_name, "..")) continue;

    if (skip_dirname && streq(e->d_name, skip_dirname)) continue;
    if (streq(e->d_name, "build")) continue;

    full = path_join_alloc(dir, e->d_name);

    if (is_dir_path_nofollow(full)) {
      scan_dir_recursive_suffix_skip_depth(out, full, suffix, skip_dirname, depth + 1);
      free(full);
    } else {
      if (ends_with(e->d_name, suffix)) {
        sv_push_own(out, full);
      } else {
        free(full);
}
    }
  }
  closedir(d);
#endif
}

static void scan_dir_recursive_suffix_skip(StrVec *out, const char *dir, const char *suffix,
                                           const char *skip_dirname) {
  scan_dir_recursive_suffix_skip_depth(out, dir, suffix, skip_dirname, 0);
}

static void scan_dir_recursive_suffix(StrVec *out, const char *dir, const char *suffix) {
  scan_dir_recursive_suffix_skip(out, dir, suffix, 0);
}

/* --------------------------- rm -rf --------------------------- */

static int rm_rf(const char *path);
static int rm_rf_contents(const char *dir);

/* Optional: collect rm failures for `tack clean -v` / `tack clobber -v` */
static int g_rm_collect = 0;
static StrVec g_rm_failed;
static int g_rm_failed_inited = 0;

static void rm_collect_begin(int enable) {
  g_rm_collect = enable ? 1 : 0;
  if (!g_rm_collect) return;
  if (g_rm_failed_inited) sv_free(&g_rm_failed);
  sv_init(&g_rm_failed);
  g_rm_failed_inited = 1;
}

static void rm_collect_add_own(char *s) {
  if (!g_rm_collect) { free(s); return; }
  if (!g_rm_failed_inited) { sv_init(&g_rm_failed); g_rm_failed_inited = 1; }
  sv_push_own(&g_rm_failed, s);
}

static void rm_collect_end_report(const char *ctx) {
  int i;
  if (!g_rm_collect) return;
  if (g_rm_failed_inited && g_rm_failed.count > 0) {
    fprintf(stderr, "tack: %s: could not remove %d path(s):\n", ctx, g_rm_failed.count);
    for (i = 0; i < g_rm_failed.count; i++) fprintf(stderr, "  %s\n", g_rm_failed.items[i]);
  }
  if (g_rm_failed_inited) sv_free(&g_rm_failed);
  g_rm_failed_inited = 0;
  g_rm_collect = 0;
}

#ifdef _WIN32
/* Windows can fail deleting read-only/hidden/system files or when a scanner briefly
 * holds a handle. Make rm -rf more tolerant by clearing attributes + retrying.
 * Also: print Win32 error names/messages to make failures actionable.
 */

static const char *win32_err_name(DWORD e) {
  switch (e) {
    case ERROR_ACCESS_DENIED: return "ERROR_ACCESS_DENIED";
    case ERROR_SHARING_VIOLATION: return "ERROR_SHARING_VIOLATION";
    case ERROR_LOCK_VIOLATION: return "ERROR_LOCK_VIOLATION";
    case ERROR_DIR_NOT_EMPTY: return "ERROR_DIR_NOT_EMPTY";
    case ERROR_PATH_NOT_FOUND: return "ERROR_PATH_NOT_FOUND";
    case ERROR_FILE_NOT_FOUND: return "ERROR_FILE_NOT_FOUND";
#ifdef ERROR_FILENAME_EXCED_RANGE
    case ERROR_FILENAME_EXCED_RANGE: return "ERROR_FILENAME_EXCED_RANGE";
#endif
#ifdef ERROR_INVALID_NAME
    case ERROR_INVALID_NAME: return "ERROR_INVALID_NAME";
#endif
#ifdef ERROR_ALREADY_EXISTS
    case ERROR_ALREADY_EXISTS: return "ERROR_ALREADY_EXISTS";
#endif
    default: break;
  }
  return 0;
}

static void win32_err_text(DWORD e, char *buf, size_t cap) {
  DWORD n;
  if (!buf || cap == 0) return;
  buf[0] = '\0';
  n = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        0, e, 0, buf, (DWORD)(cap - 1), 0
      );
  if (n == 0) { buf[0] = '\0'; return; }
  buf[n] = '\0';
  /* trim trailing CR/LF/spaces */
  while (n > 0) {
    char c = buf[n - 1];
    if (c == '\r' || c == '\n' || c == ' ' || c == '\t') { buf[n - 1] = '\0'; n--; }
    else break;
  }
}

static void win32_record_failure(const char *kind, const char *path, DWORD e) {
  const char *name;
  char msg[256];
  size_t need;
  char *s;

  if (!g_rm_collect) return;

  name = win32_err_name(e);
  win32_err_text(e, msg, sizeof(msg));

  /* kind + path + codes + names/messages */
  need = strlen(kind) + strlen(path) + 64;
  if (name) need += strlen(name) + 1;
  if (msg[0]) need += strlen(msg) + 2;

  s = (char*)xmalloc(need + 1);
  s[0] = '\0';

  strcat(s, kind);
  strcat(s, ": ");
  strcat(s, path);
  strcat(s, " (winerr=");
  {
    char num[32];
    sprintf(num, "%lu", (unsigned long)e);
    strcat(s, num);
  }
  if (name) { strcat(s, " "); strcat(s, name); }
  if (msg[0]) { strcat(s, ": "); strcat(s, msg); }
  strcat(s, ")");

  rm_collect_add_own(s);
}

static void win32_clear_attrs(const char *path) {
  DWORD a, na;
  a = GetFileAttributesA(path);
  if (a == INVALID_FILE_ATTRIBUTES) return;
  na = a & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
  if (na == 0) na = FILE_ATTRIBUTE_NORMAL;
  SetFileAttributesA(path, na);
}

static int win32_delete_file_retry(const char *path) {
  int i;
  for (i = 0; i < 20; i++) {
    win32_clear_attrs(path);
    if (DeleteFileA(path)) return 0;
    {
      DWORD e = GetLastError();
      if (e == ERROR_FILE_NOT_FOUND || e == ERROR_PATH_NOT_FOUND) return 0;
      if (e == ERROR_ACCESS_DENIED || e == ERROR_SHARING_VIOLATION || e == ERROR_LOCK_VIOLATION) {
        Sleep((DWORD)(10 * (i + 1)));
        continue;
      }
      break;
    }
  }
  {
    DWORD e = GetLastError();
    const char *name = win32_err_name(e);
    char msg[256];
    win32_err_text(e, msg, sizeof(msg));

    fprintf(stderr, "tack: rm: cannot delete file: %s (winerr=%lu", path, (unsigned long)e);
    if (name) fprintf(stderr, " %s", name);
    if (msg[0]) fprintf(stderr, ": %s", msg);
    fprintf(stderr, ")\n");

    win32_record_failure("file", path, e);
  }
  return 1;
}

static int win32_remove_dir_retry(const char *path) {
  int i;
  for (i = 0; i < 30; i++) {
    win32_clear_attrs(path);
    if (RemoveDirectoryA(path)) return 0;
    {
      DWORD e = GetLastError();
      if (e == ERROR_FILE_NOT_FOUND || e == ERROR_PATH_NOT_FOUND) return 0;
      if (e == ERROR_ACCESS_DENIED || e == ERROR_SHARING_VIOLATION || e == ERROR_LOCK_VIOLATION || e == ERROR_DIR_NOT_EMPTY) {
        Sleep((DWORD)(10 * (i + 1)));
        continue;
      }
      break;
    }
  }
  {
    DWORD e = GetLastError();
    const char *name = win32_err_name(e);
    char msg[256];
    win32_err_text(e, msg, sizeof(msg));

    fprintf(stderr, "tack: rm: cannot remove dir: %s (winerr=%lu", path, (unsigned long)e);
    if (name) fprintf(stderr, " %s", name);
    if (msg[0]) fprintf(stderr, ": %s", msg);
    fprintf(stderr, ")\n");

    win32_record_failure("dir", path, e);
  }
  return 1;
}
#endif


#ifndef _WIN32
static void posix_record_failure(const char *kind, const char *path, int err) {
  const char *msg;
  size_t need;
  char *s;

  if (!g_rm_collect) return;

  msg = strerror(err);
  if (!msg) msg = "unknown";

  need = strlen(kind) + strlen(path) + strlen(msg) + 64;
  s = (char*)xmalloc(need + 1);

  sprintf(s, "%s: %s (errno=%d: %s)", kind, path, err, msg);
  rm_collect_add_own(s);
}
#endif

static int rm_rf_depth(const char *path, int depth) {
  if (depth > TACK_MAX_RM_DEPTH) tack_die("rm recursion too deep");
  if (!file_exists(path)) return 0;

  if (!is_dir_path_nofollow(path)) {
#ifdef _WIN32
    return win32_delete_file_retry(path);
#else
    if (unlink(path) == 0) return 0;
#ifndef _WIN32
    posix_record_failure("file", path, errno);
#endif
    return 1;
#endif
  }

#ifdef _WIN32
  {
    WIN32_FIND_DATAA fd;
    HANDLE h;
    char *pattern;

    pattern = path_join_alloc(path, "*");
    h = FindFirstFileA(pattern, &fd);
    free(pattern);

    if (h != INVALID_HANDLE_VALUE) {
      do {
        char *child;

        if (streq(fd.cFileName, ".") || streq(fd.cFileName, "..")) continue;
        child = path_join_alloc(path, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          if (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            /* remove junction/symlink without following */
            if (win32_remove_dir_retry(child) != 0) { free(child); FindClose(h); return 1; }
            free(child);
          } else {
            if (rm_rf_depth(child, depth + 1) != 0) { free(child); FindClose(h); return 1; }
            free(child);
          }
        } else {
          if (win32_delete_file_retry(child) != 0) { free(child); FindClose(h); return 1; }
          free(child);
        }
      } while (FindNextFileA(h, &fd));
      FindClose(h);
    }
    return win32_remove_dir_retry(path);
  }
#else
  {
    DIR *d;
    struct dirent *e;

    d = opendir(path);
    if (!d) {
#ifndef _WIN32
      posix_record_failure("dir", path, errno);
#endif
      return 1;
    }

    while ((e = readdir(d)) != 0) {
      char *child;
      if (streq(e->d_name, ".") || streq(e->d_name, "..")) continue;

      child = path_join_alloc(path, e->d_name);
      if (rm_rf_depth(child, depth + 1) != 0) { free(child); closedir(d); return 1; }
      free(child);
    }

    closedir(d);

    if (rmdir(path) == 0) return 0;
#ifndef _WIN32
    posix_record_failure("dir", path, errno);
#endif
    return 1;
  }
#endif
}

static int rm_rf(const char *path) {
  return rm_rf_depth(path, 0);
}

static int rm_rf_contents_depth(const char *dir, int depth) {
  if (depth > TACK_MAX_RM_DEPTH) tack_die("rm recursion too deep");
  if (!file_exists(dir)) return 0;
  if (!is_dir_path_nofollow(dir)) return 1;

#ifdef _WIN32
  {
    WIN32_FIND_DATAA fd;
    HANDLE h;
    char *pattern;

    pattern = path_join_alloc(dir, "*");
    h = FindFirstFileA(pattern, &fd);
    free(pattern);

    if (h == INVALID_HANDLE_VALUE) return 0;

    do {
      char *child;

      if (streq(fd.cFileName, ".") || streq(fd.cFileName, "..")) continue;
      child = path_join_alloc(dir, fd.cFileName);

      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
          if (win32_remove_dir_retry(child) != 0) { free(child); FindClose(h); return 1; }
          free(child);
        } else {
          if (rm_rf_depth(child, depth + 1) != 0) { free(child); FindClose(h); return 1; }
          free(child);
        }
      } else {
        if (win32_delete_file_retry(child) != 0) { free(child); FindClose(h); return 1; }
        free(child);
      }
    } while (FindNextFileA(h, &fd));

    FindClose(h);
    return 0;
  }
#else
  {
    DIR *d;
    struct dirent *e;

    d = opendir(dir);
    if (!d) {
#ifndef _WIN32
      posix_record_failure("dir", dir, errno);
#endif
      return 1;
    }

    while ((e = readdir(d)) != 0) {
      char *child;
      if (streq(e->d_name, ".") || streq(e->d_name, "..")) continue;
      child = path_join_alloc(dir, e->d_name);
      if (rm_rf_depth(child, depth + 1) != 0) { free(child); closedir(d); return 1; }
      free(child);
    }
    closedir(d);
    return 0;
  }
#endif
}

static int rm_rf_contents(const char *dir) {
  return rm_rf_contents_depth(dir, 0);
}

/* --------------------------- process execution --------------------------- */

static void print_argv(char **argv) {
  int i;
  i = 0;
  while (argv[i]) {
    const char *a;
    int needq;
    const char *p;

    a = argv[i];
    needq = 0;
    for (p = a; *p; p++) {
      if (isspace((unsigned char)*p) || *p == '"') { needq = 1; break; }
    }

    if (i) fputc(' ', stdout);
    if (!needq) {
      fputs(a, stdout);
    } else {
      fputc('"', stdout);
      for (p = a; *p; p++) {
        if (*p == '"') fputs("\\\"", stdout);
        else fputc(*p, stdout);
      }
      fputc('"', stdout);
    }
    i++;
  }
  fputc('\n', stdout);
}

#ifdef _WIN32
typedef struct { intptr_t pid; } Proc;
static int proc_spawn_nowait(char **argv, Proc *out) {
  intptr_t pid = _spawnvp(_P_NOWAIT, argv[0], (const char * const *)argv);
  if (pid == -1) return 1;
  out->pid = pid;
  return 0;
}
static int proc_wait(Proc *p) {
  int status = 0;
  intptr_t r = _cwait(&status, p->pid, 0);
  if (r == -1) return 1;
  return status;
}
#else
typedef struct { pid_t pid; } Proc;
static int proc_spawn_nowait(char **argv, Proc *out) {
  pid_t pid = fork();
  if (pid < 0) return 1;
  if (pid == 0) {
    execvp(argv[0], argv);
    _exit(127);
  }
  out->pid = pid;
  return 0;
}
static int proc_wait(Proc *p) {
  int status = 0;
  if (waitpid(p->pid, &status, 0) < 0) return 1;
  if (WIFEXITED(status)) return WEXITSTATUS(status);
  return 1;
}
#endif

static int run_argv_wait(char **argv, int verbose) {
  Proc p;
  if (verbose) print_argv(argv);
  if (proc_spawn_nowait(argv, &p) != 0) {
    const char *cmd0 = (argv && argv[0]) ? argv[0] : "(null)";
    fprintf(stderr, "tack: spawn failed: %s\n", cmd0);
    fprintf(stderr, "tack: errno: %d (%s)\n", errno, strerror(errno));
    return 1;
  }
  return proc_wait(&p);
}

/* --------------------------- argv builder --------------------------- */

typedef struct {
  char **a;
  int n;
  int cap;
} Argv;

static void av_init(Argv *v) { v->a = 0; v->n = 0; v->cap = 0; }
static void av_free(Argv *v) { free(v->a); v->a = 0; v->n = 0; v->cap = 0; }

static void av_push(Argv *v, const char *s) {
  if (v->n + 1 > v->cap) {
    int ncap = v->cap ? v->cap * 2 : 32;
    v->a = (char**)xrealloc(v->a, (size_t)ncap * sizeof(char*));
    v->cap = ncap;
  }
  v->a[v->n++] = (char*)s;
}

static void av_push_list(Argv *v, const char * const *lst) {
  int i;
  if (!lst) return;
  for (i = 0; lst[i]; i++) av_push(v, lst[i]);
}

static void av_terminate(Argv *v) {
  av_push(v, 0);
}

/* --------------------------- cache helpers --------------------------- */

static unsigned long fnv1a_update(unsigned long h, const unsigned char *data, size_t n) {
  /* Force 32-bit semantics even when unsigned long is 64-bit. */
  size_t i;
  for (i = 0; i < n; i++) {
    h ^= (unsigned long)data[i];
    h *= 16777619ul;
    h &= 0xfffffffful;
  }
  return h;
}

static unsigned long fnv1a_str(unsigned long h, const char *s) {
  if (!s) return h;
  return fnv1a_update(h, (const unsigned char*)s, strlen(s));
}

static void u32_to_hex8(char *out, size_t out_cap, unsigned long v) {
  static const char *hex = "0123456789abcdef";
  unsigned long x = v & 0xfffffffful;
  int i;

  if (!out || out_cap == 0) return;
  if (out_cap < 9) { out[0] = '\0'; return; }

  for (i = 7; i >= 0; i--) {
    out[i] = hex[x & 0xful];
    x >>= 4;
  }
  out[8] = '\0';
}

static void cache_key_from_argv(char *out, size_t out_cap, char **argv) {
  unsigned long h = 2166136261ul;
  int i;

  h = fnv1a_str(h, "tack-cache-v2");

  for (i = 0; argv && argv[i]; i++) {
    /* Exclude paths that change per build invocation. */
    if (streq(argv[i], "-o") || streq(argv[i], "-MF")) { i++; continue; }
    if (streq(argv[i], "-MD")) continue;

    h = fnv1a_str(h, argv[i]);
    h = fnv1a_str(h, "\n");
  }

  /* 8 hex chars (stable 32-bit key). */
  u32_to_hex8(out, out_cap, h);
}

/* --------------------------- dep parsing --------------------------- */

/* "Why rebuild" diagnostics (optional)
 *
 * The build engine is intentionally simple: it rebuilds when
 *   - output is missing
 *   - input is newer than output
 *   - depfile or any listed dependency is missing/newer
 *   - forced via --rebuild
 */

static void tack_snprintf(char *dst, size_t dst_sz, const char *fmt, ...) {
  /* Tiny, C89-friendly formatter: supports only %s and %%. */
  va_list ap;
  size_t pos = 0;
  const char *p;

  if (!dst || dst_sz == 0) return;
  dst[0] = '\0';
  if (!fmt) return;

  va_start(ap, fmt);
  p = fmt;

  while (*p && pos + 1 < dst_sz) {
    if (p[0] == '%' && p[1] == 's') {
      const char *s = va_arg(ap, const char*);
      if (!s) s = "(null)";
      while (*s && pos + 1 < dst_sz) dst[pos++] = *s++;
      p += 2;
      continue;
    }
    if (p[0] == '%' && p[1] == '%') {
      dst[pos++] = '%';
      p += 2;
      continue;
    }

    dst[pos++] = *p++;
  }

  dst[pos] = '\0';
  va_end(ap);
}

static int depfile_needs_rebuild_explain(const char *obj_path, const char *dep_path,
                                        char *why, size_t why_sz) {
#if USE_DEPFILES
  FILE *f;
  long obj_t;
  int c;
  char tok[2048];
  int ti;
  int seen_colon;

  obj_t = file_mtime(obj_path);
  if (obj_t < 0) {
    tack_snprintf(why, why_sz, "output missing or unreadable");
    return 1;
  }

  f = fopen(dep_path, "rb");
  if (!f) {
    tack_snprintf(why, why_sz, "depfile missing: %s", dep_path);
    return 1;
  }

  ti = 0;
  seen_colon = 0;

  while ((c = fgetc(f)) != EOF) {
    if (c == '\\') {
      int n = fgetc(f);
      if (n == '\n' || n == '\r') continue;
      if (n == EOF) break;

      /* In make-style depfiles, backslash is used for line continuations and to
         escape whitespace. On Windows, dep generators may also emit backslashes
         as path separators. Preserve them unless they clearly escape whitespace
         or another backslash. */
      if (n == ' ' || n == '\t' || n == '\\') {
      if (ti < (int)sizeof(tok) - 1) tok[ti++] = (char)n;
      } else {
        if (ti < (int)sizeof(tok) - 1) tok[ti++] = '\\';
        if (ti < (int)sizeof(tok) - 1) tok[ti++] = (char)n;
      }
      continue;
    }

    if (c == ':' && !seen_colon) {
      int n = fgetc(f);

      /* Windows depfiles may contain absolute paths like C:\path\file.h.
         In that case the first ':' is part of the drive letter. Treat ':' as the
         rule separator only if it is followed by whitespace. */
      if (n == EOF) {
      tok[ti] = '\0';
      ti = 0;
      seen_colon = 1;
        break;
      }

      if (isspace((unsigned char)n)) {
        tok[ti] = '\0';
        ti = 0;
        seen_colon = 1;
        continue;
      }

      /* Not a rule separator: keep ':' and the next char as part of the token. */
      if (ti < (int)sizeof(tok) - 1) tok[ti++] = ':';
      if (ti < (int)sizeof(tok) - 1) tok[ti++] = (char)n;
      continue;
    }

    if (isspace((unsigned char)c)) {
      if (ti > 0) {
        tok[ti] = '\0';
        ti = 0;
        if (seen_colon) {
          long dt = file_mtime(tok);
          if (dt < 0) {
            tack_snprintf(why, why_sz, "dependency missing: %s", tok);
            fclose(f);
            return 1;
          }
          if (dt > obj_t) {
            tack_snprintf(why, why_sz, "dependency newer than output: %s", tok);
            fclose(f);
            return 1;
          }
        }
      }
      continue;
    }

    if (ti < (int)sizeof(tok) - 1) tok[ti++] = (char)c;
  }

  if (ti > 0 && seen_colon) {
    tok[ti] = '\0';
    {
      long dt = file_mtime(tok);
      if (dt < 0) {
        tack_snprintf(why, why_sz, "dependency missing: %s", tok);
        fclose(f);
        return 1;
      }
      if (dt > obj_t) {
        tack_snprintf(why, why_sz, "dependency newer than output: %s", tok);
        fclose(f);
        return 1;
      }
    }
  }

  fclose(f);
  return 0;
#else
  (void)obj_path; (void)dep_path;
  tack_snprintf(why, why_sz, "depfiles disabled (conservative rebuild)");
  return 1;
#endif
}

static int depfile_collect_deps(const char *dep_path, StrVec *deps) {
#if USE_DEPFILES
  FILE *f;
  int c;
  char tok[2048];
  int ti;
  int seen_colon;

  f = fopen(dep_path, "rb");
  if (!f) return 1;

  ti = 0;
  seen_colon = 0;

  while ((c = fgetc(f)) != EOF) {
    if (c == '\\') {
      int n = fgetc(f);
      if (n == '\n' || n == '\r') continue;
      if (n == EOF) break;
      if (n == ' ' || n == '\t' || n == '\\') {
        if (ti < (int)sizeof(tok) - 1) tok[ti++] = (char)n;
      } else {
        if (ti < (int)sizeof(tok) - 1) tok[ti++] = '\\';
        if (ti < (int)sizeof(tok) - 1) tok[ti++] = (char)n;
      }
      continue;
    }

    if (c == ':' && !seen_colon) {
      int n = fgetc(f);
      if (n == EOF) {
        tok[ti] = '\0';
        ti = 0;
        seen_colon = 1;
        break;
      }
      if (isspace((unsigned char)n)) {
        tok[ti] = '\0';
        ti = 0;
        seen_colon = 1;
        continue;
      }
      if (ti < (int)sizeof(tok) - 1) tok[ti++] = ':';
      if (ti < (int)sizeof(tok) - 1) tok[ti++] = (char)n;
      continue;
    }

    if (isspace((unsigned char)c)) {
      if (ti > 0) {
        tok[ti] = '\0';
        ti = 0;
        if (seen_colon) sv_push(deps, tok);
      }
      continue;
    }

    if (ti < (int)sizeof(tok) - 1) tok[ti++] = (char)c;
  }

  if (ti > 0 && seen_colon) {
    tok[ti] = '\0';
    sv_push(deps, tok);
  }

  fclose(f);
  return 0;
#else
  (void)dep_path; (void)deps;
  return 1;
#endif
}

static void cache_entry_paths(char *obj_path, size_t obj_cap,
                              char *dep_path, size_t dep_cap,
                              char *meta_path, size_t meta_cap,
                              const char *key) {
  char objd[512];
  char depd[512];
  char metad[512];
  char obj_name[128];
  char dep_name[128];
  char meta_name[128];

  path_join(objd, sizeof(objd), g_cache_dir, "obj");
  path_join(depd, sizeof(depd), g_cache_dir, "dep");
  path_join(metad, sizeof(metad), g_cache_dir, "meta");

  tack_copy(obj_name, sizeof(obj_name), key);
  tack_cat(obj_name, sizeof(obj_name), ".o");
  tack_copy(dep_name, sizeof(dep_name), key);
  tack_cat(dep_name, sizeof(dep_name), ".d");
  tack_copy(meta_name, sizeof(meta_name), key);
  tack_cat(meta_name, sizeof(meta_name), ".meta");

  path_join(obj_path, obj_cap, objd, obj_name);
  path_join(dep_path, dep_cap, depd, dep_name);
  path_join(meta_path, meta_cap, metad, meta_name);
}

static int cache_meta_valid(const char *meta_path) {
  FILE *f;
  char line[4096];

  f = fopen(meta_path, "rb");
  if (!f) return 0;

  while (fgets(line, sizeof(line), f)) {
    char *t1 = strchr(line, '\t');
    char *t2;
    char *t3;
    char *path;
    char *endptr;
    long mt_rec;
    long sz_rec;
    unsigned long h_rec;
    long mt_cur;
    long sz_cur;
    unsigned long h_cur;
    size_t len;

    if (!t1) { fclose(f); return 0; }
    *t1 = '\0';
    t2 = strchr(t1 + 1, '\t');
    if (!t2) { fclose(f); return 0; }
    *t2 = '\0';
    t3 = strchr(t2 + 1, '\t');
    if (!t3) { fclose(f); return 0; }
    *t3 = '\0';

    mt_rec = strtol(line, &endptr, 10);
    if (endptr == line) { fclose(f); return 0; }

    sz_rec = strtol(t1 + 1, &endptr, 10);
    if (endptr == (t1 + 1)) { fclose(f); return 0; }

    h_rec = strtoul(t2 + 1, &endptr, 16);
    if (endptr == (t2 + 1)) { fclose(f); return 0; }
    h_rec &= 0xfffffffful;

    path = t3 + 1;
    len = strlen(path);
    while (len > 0 && (path[len - 1] == '\n' || path[len - 1] == '\r')) {
      path[--len] = '\0';
    }

    mt_cur = file_mtime(path);
    sz_cur = file_size(path);
    if (mt_cur < 0 || sz_cur < 0) { fclose(f); return 0; }
    if (mt_cur != mt_rec || sz_cur != sz_rec) { fclose(f); return 0; }

    if (file_hash32_fnv1a(path, &h_cur) != 0) { fclose(f); return 0; }
    h_cur &= 0xfffffffful;
    if (h_cur != h_rec) { fclose(f); return 0; }
  }

  fclose(f);
  return 1;
}

static void cache_ensure_dirs(void) {
  char objd[512];
  char depd[512];
  char metad[512];

  ensure_dir(g_cache_dir);
  path_join(objd, sizeof(objd), g_cache_dir, "obj");
  path_join(depd, sizeof(depd), g_cache_dir, "dep");
  path_join(metad, sizeof(metad), g_cache_dir, "meta");
  ensure_dir(objd);
  ensure_dir(depd);
  ensure_dir(metad);
}

static int cache_restore(const char *key, const char *obj_path, const char *dep_path) {
  char cache_obj[512];
  char cache_dep[512];
  char cache_meta[512];

  if (!file_exists(g_cache_dir)) return 0;

  cache_entry_paths(cache_obj, sizeof(cache_obj),
                    cache_dep, sizeof(cache_dep),
                    cache_meta, sizeof(cache_meta),
                    key);

  if (!file_exists(cache_obj) || !file_exists(cache_dep) || !file_exists(cache_meta)) return 0;
  if (!cache_meta_valid(cache_meta)) return 0;

  if (copy_file(cache_obj, obj_path) != 0) return 0;
  if (copy_file(cache_dep, dep_path) != 0) return 0;

  return 1;
}

static int cache_write_meta(const char *dep_path, const char *meta_path) {
  FILE *f;
  StrVec deps;
  int i;

  sv_init(&deps);
  if (depfile_collect_deps(dep_path, &deps) != 0) { sv_free(&deps); return 1; }

  f = fopen(meta_path, "wb");
  if (!f) { sv_free(&deps); return 1; }

  /* Format (tab-separated):
     mtime <tab> size <tab> hash32hex <tab> path */
  for (i = 0; i < deps.count; i++) {
    long mt = file_mtime(deps.items[i]);
    long sz = file_size(deps.items[i]);
    unsigned long hh;
    char hex[9];

    if (mt < 0 || sz < 0) { fclose(f); sv_free(&deps); return 1; }
    if (file_hash32_fnv1a(deps.items[i], &hh) != 0) { fclose(f); sv_free(&deps); return 1; }

    u32_to_hex8(hex, sizeof(hex), hh);
    fprintf(f, "%ld\t%ld\t%s\t%s\n", mt, sz, hex, deps.items[i]);
  }

  fclose(f);
  sv_free(&deps);
  return 0;
}

static void cache_store(const char *key, const char *obj_path, const char *dep_path) {
  char cache_obj[512];
  char cache_dep[512];
  char cache_meta[512];

  char tmp_obj[520];
  char tmp_dep[520];
  char tmp_meta[520];

  if (!file_exists(obj_path) || !file_exists(dep_path)) return;

  cache_ensure_dirs();
  cache_entry_paths(cache_obj, sizeof(cache_obj),
                    cache_dep, sizeof(cache_dep),
                    cache_meta, sizeof(cache_meta),
                    key);

  tack_copy(tmp_obj, sizeof(tmp_obj), cache_obj);
  tack_cat(tmp_obj, sizeof(tmp_obj), ".tmp");
  tack_copy(tmp_dep, sizeof(tmp_dep), cache_dep);
  tack_cat(tmp_dep, sizeof(tmp_dep), ".tmp");
  tack_copy(tmp_meta, sizeof(tmp_meta), cache_meta);
  tack_cat(tmp_meta, sizeof(tmp_meta), ".tmp");

  if (copy_file(obj_path, tmp_obj) != 0) return;
  if (copy_file(dep_path, tmp_dep) != 0) { (void)remove(tmp_obj); return; }
  if (cache_write_meta(dep_path, tmp_meta) != 0) { (void)remove(tmp_obj); (void)remove(tmp_dep); return; }

  if (rename_replace(tmp_obj, cache_obj) != 0)   { (void)remove(tmp_obj); }
  if (rename_replace(tmp_dep, cache_dep) != 0)   { (void)remove(tmp_dep); }
  if (rename_replace(tmp_meta, cache_meta) != 0) { (void)remove(tmp_meta); }
}

static int obj_needs_rebuild_explain(const char *obj_path, const char *src_path,
                                    const char *dep_path, int force,
                                    char *why, size_t why_sz) {
  long obj_t, src_t;

  if (force) {
    tack_snprintf(why, why_sz, "forced (--rebuild)");
    return 1;
  }

  obj_t = file_mtime(obj_path);
  if (obj_t < 0) {
    tack_snprintf(why, why_sz, "output missing: %s", obj_path);
    return 1;
  }

  src_t = file_mtime(src_path);
  if (src_t < 0) {
    tack_snprintf(why, why_sz, "source missing: %s", src_path);
    return 1;
  }

  if (src_t > obj_t) {
    tack_snprintf(why, why_sz, "source newer than output: %s", src_path);
    return 1;
  }

#if USE_DEPFILES
  if (depfile_needs_rebuild_explain(obj_path, dep_path, why, why_sz)) return 1;
#else
  (void)dep_path;
  tack_snprintf(why, why_sz, "depfiles disabled (conservative rebuild)");
  return 1;
#endif

  tack_snprintf(why, why_sz, "up to date");
  return 0;
}

static int exe_needs_relink_explain(const char *out_exe, StrVec *inputs, int force,
                                   char *why, size_t why_sz) {
  int i;
  long exe_t;

  if (force) {
    tack_snprintf(why, why_sz, "forced (--rebuild)");
    return 1;
  }

  if (!file_exists(out_exe)) {
    tack_snprintf(why, why_sz, "output missing: %s", out_exe);
    return 1;
  }

  exe_t = file_mtime(out_exe);
  if (exe_t < 0) {
    tack_snprintf(why, why_sz, "output unreadable: %s", out_exe);
    return 1;
  }

  for (i = 0; i < inputs->count; i++) {
    const char *in = inputs->items[i];
    long it = file_mtime(in);
    if (it < 0) {
      tack_snprintf(why, why_sz, "input missing: %s", in);
      return 1;
    }
    if (it > exe_t) {
      tack_snprintf(why, why_sz, "input newer than output: %s", in);
      return 1;
    }
  }

  tack_snprintf(why, why_sz, "up to date");
  return 0;
}

/* --------------------------- target configuration --------------------------- */

typedef struct {
  const char *name;                 /* matches CLI target name (e.g. "app" or "tool:foo") */
  const char * const *includes;     /* extra -I dirs */
  const char * const *defines;      /* extra -D... */
  const char * const *cflags;       /* extra compile flags */
  const char * const *ldflags;      /* extra link flags */
  const char * const *libs;         /* extra libs/flags, e.g. "-lws2_32" */
  int use_core;                     /* 1 = link src/core into this target */
} TargetOverride;

/* runtime INI overrides (higher priority than tackfile/built-ins) */
static const TargetOverride *find_ini_override(const char *name);

typedef struct {
  const char *name;      /* CLI name (e.g. "app" or "tool:foo") */
  const char *src_dir;   /* directory to scan recursively for .c files (upsert if non-0) */
  const char *bin_base;  /* output executable base name (no extension; upsert if non-0) */
  const char *id;        /* optional filesystem-safe id (upsert if non-0) */
  int enabled;           /* action-only: if src_dir/bin_base/id are all 0 -> 0 disable, 1 enable */
  int remove;            /* action: 1 remove target from graph */
} TargetDef;

/* Optional external configuration:
 *
 * tack supports two ways to keep project-specific build setup out of tack.c:
 *
 *  A) Runtime tackfile.c (recommended):
 *     - If a file named "tackfile.c" exists in the project root, tack compiles a tiny
 *       generator (with your chosen compiler) and runs it to produce:
 *         build/_tackfile/tackfile.generated.ini
 *     - That generated INI is loaded as a low-priority config layer (below tack.ini / --config).
 *     - If compilation or execution fails, tack exits with an error (fail-fast).
 *
 *  B) Compile-time include (legacy / locked-down environments):
 *     - Build tack with -DTACK_USE_TACKFILE to #include "tackfile.c" directly:
 *         tcc -DTACK_USE_TACKFILE src/tack.c -o tack.exe
 *     - When this mode is enabled, the runtime generator is not used.
 *
 * In tackfile.c you may define:
 *
 *   1) Overrides (includes/defines/cflags/ldflags/libs, core):
 *   #define TACKFILE_OVERRIDES my_overrides
 *      static const TargetOverride my_overrides[] = { ... , { 0,0,0,0,0,0,0 } };
 *
 *   2) Targets (add/modify/disable/remove):
 *      #define TACKFILE_TARGETS my_targets
 *      static const TargetDef my_targets[] = { ... , { 0,0,0,0,0,0 } };
 *
 *   3) Default target:
 *      #define TACKFILE_DEFAULT_TARGET "app"
 *
 *   4) Disable auto tool discovery:
 *      #define TACKFILE_DISABLE_AUTO_TOOLS 1
 *
 * Layering (highest wins):
 *   tack.ini / --config  >  generated tackfile.ini  >  built-ins
 */

#ifdef TACK_USE_TACKFILE
#include "tackfile.c"
#endif

/* Example overrides (edit as needed) */
static const char *app_includes[] = { "src", 0 };
static const char *app_defines[]  = { 0 };
static const char *app_cflags[]   = { 0 };
static const char *app_ldflags[]  = { 0 };
static const char *app_libs[]     = { 0 };

static const TargetOverride g_overrides[] = {
  /* app: use shared core by default */
  { "app", app_includes, app_defines, app_cflags, app_ldflags, app_libs, 1 },

  /* Example tool override (uncomment when you have tools/foo):
   * static const char *foo_defines[] = { "TOOL_FOO=1", 0 };
   * { "tool:foo", 0, foo_defines, 0, 0, 0, 1 },
   */

  { 0, 0, 0, 0, 0, 0, 0 }
};

static const TargetOverride *find_override(const char *name) {
  int i;
  {
    const TargetOverride *io = find_ini_override(name);
    if (io) return io;
  }

#ifdef TACKFILE_OVERRIDES
  /* user overrides (from tackfile.c) take precedence */
  for (i = 0; TACKFILE_OVERRIDES[i].name; i++) {
    if (streq(TACKFILE_OVERRIDES[i].name, name)) return &TACKFILE_OVERRIDES[i];
  }
#endif

  for (i = 0; g_overrides[i].name; i++) {
    if (streq(g_overrides[i].name, name)) return &g_overrides[i];
  }
  return 0;
}

/* --------------------------- discovered targets --------------------------- */

typedef struct {
  char *name;     /* CLI name (may contain ':') */
  char *id;       /* filesystem-safe id */
  char *src_dir;  /* directory to scan */
  char *bin_base; /* output base name */
  int enabled;    /* 1=active, 0=disabled */
} Target;

typedef struct {
  Target *items;
  int count;
  int cap;
} TargetVec;

static void tv_init(TargetVec *v) { v->items = 0; v->count = 0; v->cap = 0; }

static void tv_free(TargetVec *v) {
  int i;
  for (i = 0; i < v->count; i++) {
    free(v->items[i].name);
    free(v->items[i].id);
    free(v->items[i].src_dir);
    free(v->items[i].bin_base);
  }
  free(v->items);
  v->items = 0; v->count = 0; v->cap = 0;
}

static void tv_push(TargetVec *v, const char *name, const char *src_dir, const char *bin_base) {
  Target *t;
  char idbuf[256];

  if (v->count + 1 > v->cap) {
    int ncap = v->cap ? v->cap * 2 : 8;
    v->items = (Target*)xrealloc(v->items, (size_t)ncap * sizeof(Target));
    v->cap = ncap;
  }

  sanitize_name_to_id(idbuf, sizeof(idbuf), name);

  t = &v->items[v->count++];
  t->name = xstrdup(name);
  t->id = xstrdup(idbuf);
  t->src_dir = xstrdup(src_dir);
  t->bin_base = xstrdup(bin_base);
  t->enabled = 1;
}

/* --------------------------- target graph helpers --------------------------- */
/* Used by tackfile targets (compile-time) and tack.ini targets (runtime). */

static int tv_find_index_by_name(TargetVec *v, const char *name) {
  int i;
  for (i = 0; i < v->count; i++) {
    if (streq(v->items[i].name, name)) return i;
  }
  return -1;
}

static void tv_remove_at(TargetVec *v, int idx) {
  int i;
  if (idx < 0 || idx >= v->count) return;

  free(v->items[idx].name);
  free(v->items[idx].id);
  free(v->items[idx].src_dir);
  free(v->items[idx].bin_base);

  for (i = idx + 1; i < v->count; i++) v->items[i - 1] = v->items[i];
  v->count--;
}

static void tv_update_at_fields(TargetVec *v, int idx, const TargetDef *d) {
  if (idx < 0 || idx >= v->count) return;

  if (d->src_dir) {
    free(v->items[idx].src_dir);
    v->items[idx].src_dir = xstrdup(d->src_dir);
  }
  if (d->bin_base) {
    free(v->items[idx].bin_base);
    v->items[idx].bin_base = xstrdup(d->bin_base);
  }
  if (d->id) {
    free(v->items[idx].id);
    v->items[idx].id = xstrdup(d->id);
  }
}

static void tv_apply_targetdef(TargetVec *v, const TargetDef *d) {
  int idx;

  if (!d || !d->name) return;

    idx = tv_find_index_by_name(v, d->name);

  /* remove wins */
  if (d->remove) {
    if (idx >= 0) tv_remove_at(v, idx);
    return;
  }

  /* action-only: enable/disable existing */
  if (!d->src_dir && !d->bin_base && !d->id) {
    if (idx >= 0) v->items[idx].enabled = d->enabled ? 1 : 0;
    return;
  }

  /* upsert */
  if (idx < 0) {
    /* create with best-effort defaults, then apply upserts */
    tv_push(v, d->name, d->src_dir ? d->src_dir : "src", d->bin_base ? d->bin_base : "app");
    idx = v->count - 1;
  }

    tv_update_at_fields(v, idx, d);

  /* enabled defaults to 1; if explicitly 0, allow disabling */
  v->items[idx].enabled = (d->enabled ? 1 : 0);
}

/* --------------------------- tackfile.c targets --------------------------- */
#ifdef TACKFILE_TARGETS
static void apply_tackfile_targets(TargetVec *out) {
  int i;
  for (i = 0; TACKFILE_TARGETS[i].name; i++) {
    tv_apply_targetdef(out, &TACKFILE_TARGETS[i]);
  }
}
#else
static void apply_tackfile_targets(TargetVec *out) { (void)out; }
#endif

/* --------------------------- tack.ini (INI config) --------------------------- */

typedef struct {
  char *name; /* target name key */
  char *src_dir;
  char *bin_base;
  char *id;
  int enabled_set, enabled;
  int remove_set, remove;
  int core_set, core;

  StrVec includes;
  StrVec defines;
  StrVec cflags;
  StrVec ldflags;
  StrVec libs;
} IniTargetCfg;

typedef struct {
  IniTargetCfg *items;
  int count;
  int cap;
} IniTargetVec;

typedef struct {
  TargetOverride *items;
  int count;
  int cap;
} IniOverrideVec;

static IniTargetVec g_ini_targets;
static IniOverrideVec g_ini_overrides;

static void ini_targets_init(void) { g_ini_targets.items = 0; g_ini_targets.count = 0; g_ini_targets.cap = 0; }
static void ini_overrides_init(void) { g_ini_overrides.items = 0; g_ini_overrides.count = 0; g_ini_overrides.cap = 0; }

static void free_strlist(char **lst) {
  int i;
  if (!lst) return;
  for (i = 0; lst[i]; i++) free(lst[i]);
  free(lst);
}

static void ini_targets_free(void) {
  int i;
  for (i = 0; i < g_ini_targets.count; i++) {
    IniTargetCfg *t = &g_ini_targets.items[i];
    free(t->name);
    free(t->src_dir);
    free(t->bin_base);
    free(t->id);
    sv_free(&t->includes);
    sv_free(&t->defines);
    sv_free(&t->cflags);
    sv_free(&t->ldflags);
    sv_free(&t->libs);
  }
  free(g_ini_targets.items);
  g_ini_targets.items = 0; g_ini_targets.count = 0; g_ini_targets.cap = 0;
}

static void ini_overrides_free(void) {
  int i;
  for (i = 0; i < g_ini_overrides.count; i++) {
    TargetOverride *ov = &g_ini_overrides.items[i];
    free((char*)ov->name);
    free_strlist((char**)ov->includes);
    free_strlist((char**)ov->defines);
    free_strlist((char**)ov->cflags);
    free_strlist((char**)ov->ldflags);
    free_strlist((char**)ov->libs);
  }
  free(g_ini_overrides.items);
  g_ini_overrides.items = 0; g_ini_overrides.count = 0; g_ini_overrides.cap = 0;
}

/* tiny trimming helpers */
static char *ltrim(char *s) { while (*s && isspace((unsigned char)*s)) s++; return s; }
static void rtrim_inplace(char *s) {
  size_t n = strlen(s);
  while (n > 0 && isspace((unsigned char)s[n - 1])) { s[n - 1] = '\0'; n--; }
}
static char *trim(char *s) { s = ltrim(s); rtrim_inplace(s); return s; }

static int strieq(const char *a, const char *b) {
  unsigned char ca, cb;
  while (*a && *b) {
    ca = (unsigned char)*a++;
    cb = (unsigned char)*b++;
    if (tolower(ca) != tolower(cb)) return 0;
  }
  return *a == '\0' && *b == '\0';
}

static int parse_bool(const char *v, int *out) {
  if (!v) return 0;
  if (strieq(v, "1") || strieq(v, "yes") || strieq(v, "true") || strieq(v, "on")) { *out = 1; return 1; }
  if (strieq(v, "0") || strieq(v, "no")  || strieq(v, "false")|| strieq(v, "off")) { *out = 0; return 1; }
  return 0;
}

static void split_list_tokens(StrVec *out, const char *v, int ws_sep) {
  const char *p = v;

  while (p && *p) {
    char *tmp;
    size_t n;

    /* skip separators and leading whitespace */
    while (*p && (*p == ';' || isspace((unsigned char)*p))) p++;
    if (!*p) break;

    /* quoted token: " ... " */
    if (*p == '"') {
      const char *q;
      p++; /* skip opening quote */
    q = p;
      while (*q && *q != '"') q++;
      if (!*q) tack_die("ini: unterminated quote in list");

    n = (size_t)(q - p);
    if (n > TACK_MAX_TOKEN) tack_die("ini token too long");

    tmp = (char*)xmalloc(n + 1);
    memcpy(tmp, p, n);
    tmp[n] = '\0';

      if (tmp[0]) {
        if (out->count >= TACK_MAX_LIST_ITEMS) { free(tmp); tack_die("ini list too long"); }
        sv_push_own(out, tmp); /* already owned */
      } else {
    free(tmp);
      }

      p = q + 1; /* skip closing quote */
      continue;
    }

    /* unquoted token */
    {
      const char *q = p;
      while (*q) {
        if (*q == ';') break;
        if (ws_sep && isspace((unsigned char)*q)) break;
        q++;
      }

      n = (size_t)(q - p);
      if (n > TACK_MAX_TOKEN) tack_die("ini token too long");

      tmp = (char*)xmalloc(n + 1);
      memcpy(tmp, p, n);
      tmp[n] = '\0';

      {
        char *t = trim(tmp);
        if (t[0]) {
          if (out->count >= TACK_MAX_LIST_ITEMS) { free(tmp); tack_die("ini list too long"); }
          if (t != tmp) {
            char *own = xstrdup(t);
            free(tmp);
            tmp = own;
          }
          sv_push_own(out, tmp);
        } else {
          free(tmp);
        }
      }

    p = q;
      continue;
  }
}
}

static IniTargetCfg *ini_get_or_add_target(const char *name) {
  int i;
  for (i = 0; i < g_ini_targets.count; i++) {
    if (streq(g_ini_targets.items[i].name, name)) return &g_ini_targets.items[i];
  }
  if (g_ini_targets.count + 1 > g_ini_targets.cap) {
    int ncap = g_ini_targets.cap ? g_ini_targets.cap * 2 : 8;
    g_ini_targets.items = (IniTargetCfg*)xrealloc(g_ini_targets.items, (size_t)ncap * sizeof(IniTargetCfg));
    g_ini_targets.cap = ncap;
  }
  {
    IniTargetCfg *t = &g_ini_targets.items[g_ini_targets.count++];
    memset(t, 0, sizeof(*t));
    t->name = xstrdup(name);
    sv_init(&t->includes);
    sv_init(&t->defines);
    sv_init(&t->cflags);
    sv_init(&t->ldflags);
    sv_init(&t->libs);
    t->enabled_set = 0; t->enabled = 1;
    t->remove_set = 0; t->remove = 0;
    t->core_set = 0; t->core = 0;
    return t;
  }
}

static char **sv_to_strlist_own(StrVec *v) {
  char **lst;
  int i;
  lst = (char**)xmalloc((size_t)(v->count + 1) * sizeof(char*));
  for (i = 0; i < v->count; i++) lst[i] = v->items[i]; /* transfer */
  lst[v->count] = 0;
  free(v->items);
  v->items = 0; v->count = 0; v->cap = 0;
  return lst;
}

static TargetOverride *ini_get_or_add_override(const char *name) {
  int i;
  for (i = 0; i < g_ini_overrides.count; i++) {
    if (streq(g_ini_overrides.items[i].name, name)) return &g_ini_overrides.items[i];
  }
  if (g_ini_overrides.count + 1 > g_ini_overrides.cap) {
    int ncap = g_ini_overrides.cap ? g_ini_overrides.cap * 2 : 8;
    g_ini_overrides.items = (TargetOverride*)xrealloc(g_ini_overrides.items, (size_t)ncap * sizeof(TargetOverride));
    g_ini_overrides.cap = ncap;
  }
  {
    TargetOverride *ov = &g_ini_overrides.items[g_ini_overrides.count++];
    memset(ov, 0, sizeof(*ov));
    ov->name = xstrdup(name);
    ov->includes = 0;
    ov->defines = 0;
    ov->cflags = 0;
    ov->ldflags = 0;
    ov->libs = 0;
    ov->use_core = 0;
    return ov;
  }
}

/* INI override lookup (used by find_override) */
static const TargetOverride *find_ini_override(const char *name) {
  int i;
  for (i = 0; i < g_ini_overrides.count; i++) {
    if (streq(g_ini_overrides.items[i].name, name)) return &g_ini_overrides.items[i];
  }
  return 0;
}

/* read tack.ini into g_ini_targets/g_ini_overrides + project globals */
static int ini_load_file(const char *path) {
  FILE *f;
  char line[TACK_MAX_LINE];
  int lineno = 0;

  enum { SEC_NONE = 0, SEC_PROJECT = 1, SEC_TARGET = 2, SEC_DOC = 3, SEC_BOM = 4, SEC_SBOM = 5 } sec = SEC_NONE;
  IniTargetCfg *cur_t = 0;

  f = fopen(path, "rb");
  if (!f) return 1;

  while (fgets(line, sizeof(line), f)) {
    char *s, *eq;
    lineno++;

    /* fail-fast on truncated lines */
    if (!strchr(line, '\n') && !feof(f)) {
      int ch;
      while ((ch = fgetc(f)) != EOF && ch != '\n') { /* discard */ }
      fclose(f);
      tack_die("ini line too long");
    }

    s = line;
    s = trim(s);
    if (!s[0]) continue;
    if (s[0] == ';' || s[0] == '#') continue;

    if (s[0] == '[') {
      char *end = strchr(s, ']');
      if (!end) continue;
      *end = '\0';
      s++;
      s = trim(s);

      cur_t = 0;
      sec = SEC_NONE;

      if (strieq(s, "project")) {
        sec = SEC_PROJECT;
        continue;
      }

      if (strieq(s, "doc")) { sec = SEC_DOC; continue; }
      if (strieq(s, "bom")) { sec = SEC_BOM; continue; }
      if (strieq(s, "sbom")) { sec = SEC_SBOM; continue; }

      if ((tolower((unsigned char)s[0])=='t') && (tolower((unsigned char)s[1])=='a') && (tolower((unsigned char)s[2])=='r') && (tolower((unsigned char)s[3])=='g') && (tolower((unsigned char)s[4])=='e') && (tolower((unsigned char)s[5])=='t')) {
        char *p;
        char *tname;
        size_t tn;

        p = s + 6;
        tname = 0;
        tn = 0;
        p = trim(p);
        if (*p == '"') {
          char *q = strchr(p + 1, '"');
          if (!q) continue;
          tn = (size_t)(q - (p + 1));
          if (tn > TACK_MAX_NAME) tack_die("target name too long");
          tname = (char*)xmalloc(tn + 1);
          memcpy(tname, p + 1, tn);
          tname[tn] = '\0';
        } else {
          tn = strlen(p);
          if (tn > TACK_MAX_NAME) tack_die("target name too long");
          tname = xstrdup(p);
        }
        cur_t = ini_get_or_add_target(tname);
        sec = SEC_TARGET;
        free(tname);
        continue;
      }

      continue;
    }

    eq = strchr(s, '=');
    if (!eq) continue;
    *eq = '\0';
    {
      char *key = trim(s);
      char *val = trim(eq + 1);

      if (sec == SEC_PROJECT) {
        if (strieq(key, "default_target")) {
          free(g_config_default_target);
          tack_check_len("default_target", val, TACK_MAX_NAME);
          g_config_default_target = xstrdup(val);
        } else if (strieq(key, "disable_auto_tools")) {
          int b;
          if (parse_bool(val, &b)) g_config_disable_auto_tools = b;
        }
        continue;
      }

      if (sec == SEC_DOC) {
        if (strieq(key, "template")) { free(g_config_doc_template); g_config_doc_template = xstrdup(val); }
        else if (strieq(key, "css")) { free(g_config_doc_css); g_config_doc_css = xstrdup(val); }
        continue;
      }

      if (sec == SEC_BOM) {
        if (strieq(key, "template")) { free(g_config_bom_template); g_config_bom_template = xstrdup(val); }
        else if (strieq(key, "css")) { free(g_config_bom_css); g_config_bom_css = xstrdup(val); }
        continue;
      }

      if (sec == SEC_SBOM) {
        if (strieq(key, "format")) {
          free(g_config_sbom_format);
          tack_check_len("sbom format", val, TACK_MAX_NAME);
          g_config_sbom_format = xstrdup(val);
        } else if (strieq(key, "spec_version")) {
          free(g_config_sbom_spec_version);
          tack_check_len("sbom spec_version", val, TACK_MAX_NAME);
          g_config_sbom_spec_version = xstrdup(val);
        } else if (strieq(key, "output")) {
          free(g_config_sbom_output);
          tack_check_len("sbom output", val, TACK_MAX_CONFIG_PATH);
          g_config_sbom_output = xstrdup(val);
          if (strieq(val, "tack")) {
            free(g_config_sbom_format);
            g_config_sbom_format = xstrdup("tack");
          } else if (strieq(val, "cyclonedx")) {
            free(g_config_sbom_format);
            g_config_sbom_format = xstrdup("cyclonedx");
          } else if (strieq(val, "spdx")) {
            free(g_config_sbom_format);
            g_config_sbom_format = xstrdup("spdx");
          } else {
            fprintf(stderr, "tack: ini: invalid sbom.format: %s\n", val);
            exit(2);
          }
        } else if (strieq(key, "spec_version")) {
          free(g_config_sbom_spec_version);
          g_config_sbom_spec_version = 0;
          if (val[0]) {
            tack_check_len("sbom.spec_version", val, TACK_MAX_TOKEN);
            g_config_sbom_spec_version = xstrdup(val);
          }
        } else if (strieq(key, "output")) {
          free(g_config_sbom_output);
          g_config_sbom_output = 0;
          if (val[0]) {
            tack_check_len("sbom.output", val, TACK_MAX_CONFIG_PATH);
            g_config_sbom_output = xstrdup(val);
          }
          free(g_config_sbom_format);
          g_config_sbom_format = xstrdup(val);
        }
        continue;
      }

      if (sec == SEC_TARGET && cur_t) {
        if (strieq(key, "src")) {
          free(cur_t->src_dir);
          cur_t->src_dir = xstrdup(val);
        } else if (strieq(key, "bin")) {
          free(cur_t->bin_base);
          cur_t->bin_base = xstrdup(val);
        } else if (strieq(key, "id")) {
          free(cur_t->id);
          cur_t->id = xstrdup(val);
        } else if (strieq(key, "enabled")) {
          int b;
          if (parse_bool(val, &b)) { cur_t->enabled_set = 1; cur_t->enabled = b; }
        } else if (strieq(key, "remove")) {
          int b;
          if (parse_bool(val, &b)) { cur_t->remove_set = 1; cur_t->remove = b; }
        } else if (strieq(key, "core")) {
          int b;
          if (parse_bool(val, &b)) { cur_t->core_set = 1; cur_t->core = b; }
        } else if (strieq(key, "includes")) {
          sv_free(&cur_t->includes); sv_init(&cur_t->includes); split_list_tokens(&cur_t->includes, val, 0);
        } else if (strieq(key, "defines")) {
          sv_free(&cur_t->defines); sv_init(&cur_t->defines); split_list_tokens(&cur_t->defines, val, 1);
        } else if (strieq(key, "cflags")) {
          sv_free(&cur_t->cflags); sv_init(&cur_t->cflags); split_list_tokens(&cur_t->cflags, val, 1);
        } else if (strieq(key, "ldflags")) {
          sv_free(&cur_t->ldflags); sv_init(&cur_t->ldflags); split_list_tokens(&cur_t->ldflags, val, 1);
        } else if (strieq(key, "libs")) {
          sv_free(&cur_t->libs); sv_init(&cur_t->libs); split_list_tokens(&cur_t->libs, val, 1);
        }
      }
    }
  }

  fclose(f);
  return 0;
}

/* materialize override arrays from ini target cfg lists */
static void ini_materialize_overrides(void) {
  int i;
  for (i = 0; i < g_ini_targets.count; i++) {
    IniTargetCfg *t = &g_ini_targets.items[i];
    int need = 0;
    if (t->includes.count) need = 1;
    if (t->defines.count) need = 1;
    if (t->cflags.count) need = 1;
    if (t->ldflags.count) need = 1;
    if (t->libs.count) need = 1;
    if (t->core_set) need = 1;

    if (need) {
      TargetOverride *ov = ini_get_or_add_override(t->name);
      if (t->includes.count) ov->includes = (const char * const *)sv_to_strlist_own(&t->includes);
      if (t->defines.count) ov->defines = (const char * const *)sv_to_strlist_own(&t->defines);
      if (t->cflags.count) ov->cflags = (const char * const *)sv_to_strlist_own(&t->cflags);
      if (t->ldflags.count) ov->ldflags = (const char * const *)sv_to_strlist_own(&t->ldflags);
      if (t->libs.count) ov->libs = (const char * const *)sv_to_strlist_own(&t->libs);
      if (t->core_set) ov->use_core = t->core ? 1 : 0;
    }
  }
}

static void apply_ini_targets(TargetVec *out) {
  int i;
  if (!g_config_loaded) return;

  /* ensure overrides are ready */
  ini_materialize_overrides();

  for (i = 0; i < g_ini_targets.count; i++) {
    IniTargetCfg *t = &g_ini_targets.items[i];
    TargetDef d;
    memset(&d, 0, sizeof(d));
    d.name = t->name;

    if (t->remove_set && t->remove) {
      d.remove = 1;
      tv_apply_targetdef(out, &d);
      continue;
    }

    /* action-only enable/disable */
    if (!t->src_dir && !t->bin_base && !t->id && t->enabled_set) {
      d.enabled = t->enabled ? 1 : 0;
      tv_apply_targetdef(out, &d);
      continue;
    }

    /* upsert */
    d.src_dir = t->src_dir;
    d.bin_base = t->bin_base;
    d.id = t->id;

    /* if enabled not specified, default to 1 for created/updated targets */
    d.enabled = t->enabled_set ? (t->enabled ? 1 : 0) : 1;
    d.remove = 0;

    tv_apply_targetdef(out, &d);

    /* If target exists and enabled_set was specified, we already applied it.
     * If enabled_set was not specified and target existed disabled, leave it as-is?
     * For safety, we keep the existing enabled state in that case.
     */
    if (!t->enabled_set) {
      int idx = tv_find_index_by_name(out, t->name);
      if (idx >= 0) {
        /* keep existing enabled state if it was disabled by other config */
        /* (do nothing) */
      }
    }
  }
}

/* config glue (called from main) */

/* --------------------------- tackfile.c auto-config (v0.6.0) ---------------------------
 *
 * If a file named "tackfile.c" exists in the project root, tack will (by default):
 *   1) compile a tiny generator into build/_tackfile/
 *   2) run it to emit build/_tackfile/tackfile.generated.ini
 *   3) load that INI as a low-priority config layer
 *
 * Then tack loads tack.ini (or --config <path>) on top, so INI can override tackfile.c.
 *
 * Important:
 * - If tackfile.c exists but cannot be compiled/executed, tack fails (unless --no-config).
 * - If you compile tack with -DTACK_USE_TACKFILE, this runtime step is skipped.
 */

#ifndef TACK_USE_TACKFILE
static char g_tackfile_generated_ini[TACK_MAX_CONFIG_PATH + 1] = {0};


static void tackfile_gen_paths(char *dir, size_t dir_cap,
                               char *gen_c, size_t gen_c_cap,
                               char *gen_exe, size_t gen_exe_cap,
                               char *gen_ini, size_t gen_ini_cap) {
  path_join(dir, dir_cap, g_build_dir, "_tackfile");
  path_join(gen_c, gen_c_cap, dir, "tackfile_gen.c");
#ifdef _WIN32
  path_join(gen_exe, gen_exe_cap, dir, "tackfile_gen.exe");
#else
  path_join(gen_exe, gen_exe_cap, dir, "tackfile_gen");
#endif
  path_join(gen_ini, gen_ini_cap, dir, "tackfile.generated.ini");
}

static int tackfile_write_generator_source(const char *gen_c_path) {
  /* Keep generator self-contained and C89-friendly. */
  static const char *lines[] = {
    "/* auto-generated by tack " TACK_VERSION " */\n",
    "#include <stdio.h>\n",
    "#include <stdlib.h>\n",
    "#include <string.h>\n",
    "\n",
    "typedef struct {\n",
    "  const char *name;\n",
    "  const char * const *includes;\n",
    "  const char * const *defines;\n",
    "  const char * const *cflags;\n",
    "  const char * const *ldflags;\n",
    "  const char * const *libs;\n",
    "  int use_core;\n",
    "} TargetOverride;\n",
    "\n",
    "typedef struct {\n",
    "  const char *name;\n",
    "  const char *src_dir;\n",
    "  const char *bin_base;\n",
    "  const char *id;\n",
    "  int enabled;\n",
    "  int remove;\n",
    "} TargetDef;\n",
    "\n",
    "/* Pull in project config */\n",
    "#include \"tackfile.c\"\n",
    "\n",
    "static void emit_list(FILE *f, const char *key, const char * const *lst) {\n",
    "  int i;\n",
    "  if (!lst || !lst[0]) return;\n",
    "  fputs(key, f); fputs(\" = \", f);\n",
    "  for (i = 0; lst[i]; i++) {\n",
    "    if (i) fputc(';', f);\n",
    "    fputs(lst[i], f);\n",
    "  }\n",
    "  fputc('\\n', f);\n",
    "}\n",
    "\n",
    "int main(int argc, char **argv) {\n",
    "  const char *out = (argc > 1) ? argv[1] : \"tackfile.generated.ini\";\n",
    "  FILE *f = fopen(out, \"wb\");\n",
    "  if (!f) return 1;\n",
    "\n",
    "  fputs(\"# generated from tackfile.c\\n\\n\", f);\n",
    "\n",
    "  /* project */\n",
    "  fputs(\"[project]\\n\", f);\n",
    "#ifdef TACKFILE_DEFAULT_TARGET\n",
    "  fprintf(f, \"default_target = %s\\n\", TACKFILE_DEFAULT_TARGET);\n",
    "#endif\n",
    "#ifdef TACKFILE_DISABLE_AUTO_TOOLS\n",
    "#if TACKFILE_DISABLE_AUTO_TOOLS\n",
    "  fputs(\"disable_auto_tools = yes\\n\", f);\n",
    "#endif\n",
    "#endif\n",
    "  fputc('\\n', f);\n",
    "\n",
    "  /* targets */\n",
    "#ifdef TACKFILE_TARGETS\n",
    "  {\n",
    "    const TargetDef *td = (const TargetDef*)TACKFILE_TARGETS;\n",
    "    while (td && td->name) {\n",
    "      fprintf(f, \"[target \\\"%s\\\"]\\n\", td->name);\n",
    "      if (td->src_dir)  fprintf(f, \"src = %s\\n\", td->src_dir);\n",
    "      if (td->bin_base) fprintf(f, \"bin = %s\\n\", td->bin_base);\n",
    "      if (td->id)       fprintf(f, \"id = %s\\n\", td->id);\n",
    "      if (td->remove) {\n",
    "        fputs(\"remove = yes\\n\", f);\n",
    "      } else if (!td->src_dir && !td->bin_base && !td->id) {\n",
    "        fputs(td->enabled ? \"enabled = yes\\n\" : \"enabled = no\\n\", f);\n",
    "      }\n",
    "      fputc('\\n', f);\n",
    "      td++;\n",
    "    }\n",
    "  }\n",
    "#endif\n",
    "\n",
    "  /* overrides (may augment existing [target] sections) */\n",
    "#ifdef TACKFILE_OVERRIDES\n",
    "  {\n",
    "    const TargetOverride *ov = (const TargetOverride*)TACKFILE_OVERRIDES;\n",
    "    while (ov && ov->name) {\n",
    "      fprintf(f, \"[target \\\"%s\\\"]\\n\", ov->name);\n",
    "      fputs(ov->use_core ? \"core = yes\\n\" : \"core = no\\n\", f);\n",
    "      emit_list(f, \"includes\", ov->includes);\n",
    "      emit_list(f, \"defines\",  ov->defines);\n",
    "      emit_list(f, \"cflags\",   ov->cflags);\n",
    "      emit_list(f, \"ldflags\",  ov->ldflags);\n",
    "      emit_list(f, \"libs\",     ov->libs);\n",
    "      fputc('\\n', f);\n",
    "      ov++;\n",
    "    }\n",
    "  }\n",
    "#endif\n",
    "\n",
    "  fclose(f);\n",
    "  return 0;\n",
    "}\n",
    0
  };

  FILE *f = fopen(gen_c_path, "wb");
  int i;

  if (!f) return 1;
  for (i = 0; lines[i]; i++) fputs(lines[i], f);
  fclose(f);
  return 0;
}

static int tackfile_prepare_generated_ini(void) {
  const char *cc;
  char dir[512], gen_c[512], gen_exe[512], gen_ini[512];
  long tf_t;

  if (!file_exists("tackfile.c")) return 0;

  cc = get_cc();
  tf_t = file_mtime("tackfile.c");

  ensure_dir(g_build_dir);
  path_join(dir, sizeof(dir), g_build_dir, "_tackfile");
  ensure_dir(dir);

  tackfile_gen_paths(dir, sizeof(dir), gen_c, sizeof(gen_c), gen_exe, sizeof(gen_exe), gen_ini, sizeof(gen_ini));

  strncpy(g_tackfile_generated_ini, gen_ini, sizeof(g_tackfile_generated_ini) - 1);
  g_tackfile_generated_ini[sizeof(g_tackfile_generated_ini) - 1] = '\0';

  /* cache: if generated ini is newer than tackfile.c, reuse */
  if (file_exists(gen_ini) && file_mtime(gen_ini) >= tf_t) return 0;

  if (tackfile_write_generator_source(gen_c) != 0) {
    fprintf(stderr, "tack: tackfile.c: cannot write generator source\n");
    return 1;
  }

  /* compile generator */
  {
    Argv av;
    int rc;

    av_init(&av);
    av_push(&av, cc);
    av_push(&av, "-I"); av_push(&av, ".");         /* to find tackfile.c */
    av_push(&av, "-I"); av_push(&av, g_inc_dir);   /* allow includes */
    av_push(&av, "-o"); av_push(&av, gen_exe);
    av_push(&av, gen_c);
    av_terminate(&av);

    rc = run_argv_wait(av.a, 0);
    if (rc != 0) {
      fprintf(stderr, "tack: tackfile.c: compile failed\n");
      print_argv(av.a);
      av_free(&av);
      return 1;
    }
    av_free(&av);
  }

  /* run generator */
  {
    char *runv[3];
    int rc;

    runv[0] = gen_exe;
    runv[1] = gen_ini;
    runv[2] = 0;

    rc = run_argv_wait(runv, 0);
    if (rc != 0) {
      fprintf(stderr, "tack: tackfile.c: generator failed\n");
      return 1;
    }
  }

  return 0;
}
#endif /* !TACK_USE_TACKFILE */

static void config_reset(void) {
  /* reset INI state and project globals for layered loads */
  free(g_config_default_target);
  g_config_default_target = 0;

  g_config_disable_auto_tools = 0;
  free(g_config_doc_template); g_config_doc_template = 0;
  free(g_config_doc_css); g_config_doc_css = 0;
  free(g_config_bom_template); g_config_bom_template = 0;
  free(g_config_bom_css); g_config_bom_css = 0;
  free(g_config_sbom_format); g_config_sbom_format = 0;
  free(g_config_sbom_spec_version); g_config_sbom_spec_version = 0;
  free(g_config_sbom_output); g_config_sbom_output = 0;

  free(g_config_doc_template); g_config_doc_template = 0;
  free(g_config_doc_css);      g_config_doc_css = 0;
  free(g_config_bom_template); g_config_bom_template = 0;
  free(g_config_bom_css);      g_config_bom_css = 0;

  free(g_config_sbom_format);
  g_config_sbom_format = 0;
  free(g_config_sbom_spec_version);
  g_config_sbom_spec_version = 0;
  free(g_config_sbom_output);
  g_config_sbom_output = 0;

  ini_targets_free();
  ini_overrides_free();
  ini_targets_init();
  ini_overrides_init();

  g_config_loaded = 0;
  g_config_path[0] = '\0';
}

static int config_add_ini_layer(const char *path) {
  if (!path || !path[0]) return 0;
  if (ini_load_file(path) != 0) return 1;

  /* record last loaded path (highest priority) */
  tack_check_len("config path", path, TACK_MAX_CONFIG_PATH);
  tack_copy(g_config_path, sizeof(g_config_path), path);
  g_config_loaded = 1;
  return 0;
}

static int config_auto_load(void) {
  if (g_no_config) return 0;

  config_reset();

#ifndef TACK_USE_TACKFILE
  /* low-priority layer: tackfile.c (compiled on the fly) */
  if (!g_no_code_config && file_exists("tackfile.c")) {
    if (tackfile_prepare_generated_ini() != 0) return 1;
    if (g_tackfile_generated_ini[0]) {
      if (config_add_ini_layer(g_tackfile_generated_ini) != 0) return 1;
    }
  }
#endif

  /* high-priority layer: explicit --config, otherwise tack.ini */
  if (g_config_path_cli && g_config_path_cli[0]) {
    if (config_add_ini_layer(g_config_path_cli) != 0) return 1;
  } else if (file_exists("tack.ini")) {
    if (config_add_ini_layer("tack.ini") != 0) return 1;
  }

  /* finally build override list */
  ini_materialize_overrides();
  return 0;
}

static void config_free(void) {
  ini_targets_free();
  ini_overrides_free();
  
  free(g_config_default_target);
  g_config_default_target = 0;
  
  free(g_config_doc_template); g_config_doc_template = 0;
  free(g_config_doc_css); g_config_doc_css = 0;
  free(g_config_bom_template); g_config_bom_template = 0;
  free(g_config_bom_css); g_config_bom_css = 0;
  free(g_config_sbom_format); g_config_sbom_format = 0;
  free(g_config_sbom_spec_version); g_config_sbom_spec_version = 0;
  free(g_config_sbom_output); g_config_sbom_output = 0;

  g_config_loaded = 0;
  g_config_path[0] = '\0';
}


static const Target *find_target(TargetVec *v, const char *name_or_id) {
  int i;
  for (i = 0; i < v->count; i++) {
    if (!v->items[i].enabled) continue;
    if (streq(v->items[i].name, name_or_id)) return &v->items[i];
    if (streq(v->items[i].id, name_or_id)) return &v->items[i];
  }
  return 0;
}

static void discover_targets(TargetVec *out, int disable_auto_tools) {
  /* app: prefer src/app if it exists, otherwise src */
  if (file_exists(g_app_dir) && is_dir_path(g_app_dir)) {
    tv_push(out, "app", g_app_dir, "app");
  } else {
    tv_push(out, "app", g_src_dir, "app");
  }

#ifdef TACKFILE_DISABLE_AUTO_TOOLS
  disable_auto_tools = 1;
#endif

  /* tools/<name> */
  if (!disable_auto_tools) {
  if (file_exists(g_tools_dir) && is_dir_path(g_tools_dir)) {

#ifdef _WIN32
  {
    char pattern[1024];
    WIN32_FIND_DATAA fd;
    HANDLE h;

    tack_copy(pattern, sizeof(pattern), g_tools_dir);
    {
      size_t ld = strlen(pattern);
      if (ld > 0 && pattern[ld - 1] != '\\' && pattern[ld - 1] != '/') {
        pattern[ld] = '\\';
        pattern[ld + 1] = '\0';
      }
    }
    tack_cat(pattern, sizeof(pattern), "*");

    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return;

    do {
      if (streq(fd.cFileName, ".") || streq(fd.cFileName, "..")) continue;
      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        char name[256];
        char *src;
        char *tname;
        size_t n;

        tack_copy(name, sizeof(name), fd.cFileName);

        src = path_join_alloc(g_tools_dir, name);

        n = strlen("tool:") + strlen(name) + 1;
        tname = (char*)xmalloc(n);
        tack_copy(tname, n, "tool:");
        tack_cat(tname, n, name);

        tv_push(out, tname, src, name);

        free(tname);
        free(src);
      }
    } while (FindNextFileA(h, &fd));

    FindClose(h);
  }
#else
  {
    DIR *d;
    struct dirent *e;

    d = opendir(g_tools_dir);
    if (!d) return;

    while ((e = readdir(d)) != 0) {
      char *full;
      if (streq(e->d_name, ".") || streq(e->d_name, "..")) continue;
      full = path_join_alloc(g_tools_dir, e->d_name);
      if (is_dir_path(full)) {
        char *tname;
        size_t n;
        n = strlen("tool:") + strlen(e->d_name) + 1;
        tname = (char*)xmalloc(n);
        tack_copy(tname, n, "tool:");
        tack_cat(tname, n, e->d_name);
        tv_push(out, tname, full, e->d_name);
        free(tname);
    }
      free(full);
  }
    closedir(d);
  }
#endif
}
  }
}

/* --------------------------- build paths --------------------------- */

/* build/<target>/<profile>/{obj,dep,bin} */
static void build_paths(char *root, size_t root_cap,
                        char *objd, size_t objd_cap,
                        char *depd, size_t depd_cap,
                        char *bind, size_t bind_cap,
                        const char *target_id, Profile p) {
  char tdir[512];
  char pdir[512];

  path_join(tdir, sizeof(tdir), g_build_dir, target_id);
  path_join(pdir, sizeof(pdir), tdir, profile_name(p));

  tack_copy(root, root_cap, pdir);
  path_join(objd, objd_cap, root, "obj");
  path_join(depd, depd_cap, root, "dep");
  path_join(bind, bind_cap, root, "bin");
}

static void exe_path(char *out, size_t out_cap, const char *target_id, Profile p, const char *bin_base) {
  char root[512], objd[512], depd[512], bind[512];
  char fn[256];

  build_paths(root, sizeof(root),
              objd, sizeof(objd),
              depd, sizeof(depd),
              bind, sizeof(bind),
              target_id, p);

#ifdef _WIN32
  tack_copy(fn, sizeof(fn), bin_base);
  tack_cat(fn, sizeof(fn), ".exe");
#else
  tack_copy(fn, sizeof(fn), bin_base);
#endif
  path_join(out, out_cap, bind, fn);
}

/* --------------------------- compilation helpers --------------------------- */

static void push_profile_flags(Argv *av, Profile p) {
  if (p == PROF_DEBUG) {
    av_push(av, "-g");
    av_push(av, "-bt20");
    av_push(av, "-DDEBUG=1");
  } else {
    av_push(av, "-O2");
    av_push(av, "-DNDEBUG=1");
  }
}

static void push_common_warnings(Argv *av, int strict) {
  av_push_list(av, g_warn_flags_base);
  if (strict) av_push_list(av, g_warn_flags_strict_add);
}

/* spawn compile jobs with -j pool */
typedef struct {
  Proc proc;
  char obj_path[1024];
  char dep_path[1024];
  char cache_key[64];
  int cache_enabled;
} CompileJob;

static int compile_sources(const char *cc, StrVec *srcs, const char *objd, const char *depd,
                           const char * const *inc_common,
                           const char * const *inc_extra,
                           const char * const *def_extra,
                           const char * const *cflags_extra,
                           Profile p, int verbose, int why, int force, int jobs, int strict,
                           StrVec *out_objs) {
  CompileJob *running;
  int running_n;
  int i;

  if (jobs < 1) jobs = 1;

  running = (CompileJob*)xmalloc((size_t)jobs * sizeof(CompileJob));
  running_n = 0;

  for (i = 0; i < srcs->count; i++) {
    const char *src = srcs->items[i];
    char sid[512], obj_name[768], dep_name[768];
    char obj_path[1024], dep_path[1024];
    char why_msg[512];
    int need;

    sanitize_path_to_id(sid, sizeof(sid), src);
    tack_copy(obj_name, sizeof(obj_name), sid); tack_cat(obj_name, sizeof(obj_name), ".o");
    tack_copy(dep_name, sizeof(dep_name), sid); tack_cat(dep_name, sizeof(dep_name), ".d");

    path_join(obj_path, sizeof(obj_path), objd, obj_name);
    path_join(dep_path, sizeof(dep_path), depd, dep_name);

    sv_push(out_objs, obj_path);

    need = obj_needs_rebuild_explain(obj_path, src, dep_path, force,
                                     why ? why_msg : 0, why ? sizeof(why_msg) : 0);
    if (!need) continue;

    /* build argv */
    {
      Argv av;
      StrVec tmp_defs;
      char cache_key[64] = {0};
      int cache_enabled = (!g_no_cache && !force);
      av_init(&av);
      sv_init(&tmp_defs);

      av_push(&av, cc);
      av_push(&av, "-c");

      push_common_warnings(&av, strict);
      push_profile_flags(&av, p);

      /* includes */
      {
        int k;
        for (k = 0; inc_common && inc_common[k]; k++) {
          av_push(&av, "-I");
          av_push(&av, inc_common[k]);
        }
        for (k = 0; inc_extra && inc_extra[k]; k++) {
          av_push(&av, "-I");
          av_push(&av, inc_extra[k]);
        }
      }

      /* extra defines */
      {
        int k;
        for (k = 0; def_extra && def_extra[k]; k++) {
          char *d;
          size_t n;
          n = strlen(def_extra[k]) + 3;
          d = (char*)xmalloc(n);
          tack_copy(d, n, "-D");
          tack_cat(d, n, def_extra[k]);
          sv_push_own(&tmp_defs, d);
          av_push(&av, d);
        }
      }


      /* extra cflags */
      av_push_list(&av, cflags_extra);

#if USE_DEPFILES
      av_push(&av, "-MD");
      av_push(&av, "-MF");
      av_push(&av, dep_path);
#endif

      av_push(&av, "-o");
      av_push(&av, obj_path);
      av_push(&av, src);

      av_terminate(&av);

      if (verbose) print_argv(av.a);

      if (cache_enabled) {
        cache_key_from_argv(cache_key, sizeof(cache_key), av.a);
        if (cache_restore(cache_key, obj_path, dep_path)) {
          if (verbose || why) printf("cache hit: %s <- %s\n", obj_path, src);
          sv_free(&tmp_defs);
          av_free(&av);
          continue;
        }
      }

      if (why) printf("why rebuild: %s <- %s (%s)\n", obj_path, src, why_msg);

      if (jobs == 1) {
        int rc = run_argv_wait(av.a, 0);
        sv_free(&tmp_defs);
        av_free(&av);
        if (rc != 0) { free(running); return 1; }
        if (cache_enabled) cache_store(cache_key, obj_path, dep_path);
      } else {
        if (running_n >= jobs) {
          int rcw = proc_wait(&running[0].proc);
          int m;
          if (rcw == 0 && running[0].cache_enabled) {
            cache_store(running[0].cache_key, running[0].obj_path, running[0].dep_path);
          }
          for (m = 1; m < running_n; m++) running[m - 1] = running[m];
          running_n--;
          if (rcw != 0) { sv_free(&tmp_defs); av_free(&av); free(running); return 1; }
        }
        if (proc_spawn_nowait(av.a, &running[running_n].proc) != 0) { sv_free(&tmp_defs); av_free(&av); free(running); return 1; }
        running[running_n].cache_enabled = cache_enabled;
        if (cache_enabled) {
          tack_copy(running[running_n].cache_key, sizeof(running[running_n].cache_key), cache_key);
          tack_copy(running[running_n].obj_path, sizeof(running[running_n].obj_path), obj_path);
          tack_copy(running[running_n].dep_path, sizeof(running[running_n].dep_path), dep_path);
        }
        running_n++;
        sv_free(&tmp_defs);
        av_free(&av);
      }
    }
  }

  if (jobs > 1) {
    int k;
    for (k = 0; k < running_n; k++) {
      int rc = proc_wait(&running[k].proc);
      if (rc == 0 && running[k].cache_enabled) {
        cache_store(running[k].cache_key, running[k].obj_path, running[k].dep_path);
      }
      if (rc != 0) { free(running); return 1; }
    }
  }

  free(running);
  return 0;
}

static int link_executable(const char *cc, const char *out_exe,
                           StrVec *objs,
                           const char * const *inc_common,
                           const char * const *inc_extra,
                           const char * const *def_extra,
                           const char * const *ldflags_extra,
                           const char * const *libs_extra,
                           Profile p, int verbose, int strict) {
  Argv av;
  int i;
  StrVec tmp_defs;

  av_init(&av);
  sv_init(&tmp_defs);

  av_push(&av, cc);

  push_common_warnings(&av, strict);
  push_profile_flags(&av, p);

  /* includes (mostly irrelevant for link, but harmless with tcc) */
  {
    int k;
    for (k = 0; inc_common && inc_common[k]; k++) {
      av_push(&av, "-I");
      av_push(&av, inc_common[k]);
    }
    for (k = 0; inc_extra && inc_extra[k]; k++) {
      av_push(&av, "-I");
      av_push(&av, inc_extra[k]);
    }
  }

  /* extra defines */
  {
    int k;
    for (k = 0; def_extra && def_extra[k]; k++) {
      char *d;
      size_t n;
      n = strlen(def_extra[k]) + 3;
      d = (char*)xmalloc(n);
      tack_copy(d, n, "-D");
      tack_cat(d, n, def_extra[k]);
      sv_push_own(&tmp_defs, d);
      av_push(&av, d);
    }
  }


  av_push_list(&av, ldflags_extra);

  av_push(&av, "-o");
  av_push(&av, out_exe);

  for (i = 0; i < objs->count; i++) av_push(&av, objs->items[i]);

  av_push_list(&av, libs_extra);

  av_terminate(&av);

  i = run_argv_wait(av.a, verbose);
  sv_free(&tmp_defs);
  av_free(&av);

  return i;
}

/* --------------------------- core + target build --------------------------- */

static int build_core(Profile p, int verbose, int why, int force, int jobs, int strict, StrVec *out_core_objs) {
  const char *cc;
  StrVec core_srcs;
  char root[512], objd[512], depd[512], bind[512];
  const char *inc_common[4];

  cc = get_cc();

  sv_init(&core_srcs);

  if (!file_exists(g_core_dir) || !is_dir_path(g_core_dir)) {
    /* no core */
    return 0;
  }

  scan_dir_recursive_suffix(&core_srcs, g_core_dir, ".c");
  if (core_srcs.count == 0) {
    sv_free(&core_srcs);
    return 0;
  }

  /* build dirs: build/_core/<profile>/{obj,dep,bin} (bin unused) */
  ensure_dir(g_build_dir);
  {
    char cdir[512];
    path_join(cdir, sizeof(cdir), g_build_dir, "_core");
    ensure_dir(cdir);
  }
  {
    char cdir[512], pdir[512];
    path_join(cdir, sizeof(cdir), g_build_dir, "_core");
    path_join(pdir, sizeof(pdir), cdir, profile_name(p));
    ensure_dir(pdir);
  }
  build_paths(root, sizeof(root), objd, sizeof(objd), depd, sizeof(depd), bind, sizeof(bind), "_core", p);
  ensure_dir(objd);
  ensure_dir(depd);
  ensure_dir(bind);

  /* common includes: include + src + src/core */
  inc_common[0] = g_inc_dir;
  inc_common[1] = g_src_dir;
  inc_common[2] = g_core_dir;
  inc_common[3] = 0;

  if (compile_sources(cc, &core_srcs, objd, depd,
                      inc_common, 0, 0, 0,
                      p, verbose, why, force, jobs, strict,
                      out_core_objs) != 0) {
    sv_free(&core_srcs);
    return 1;
  }

  sv_free(&core_srcs);
  return 0;
}

static int build_one_target(const Target *t, Profile p, int verbose, int why, int force, int jobs, int strict, int no_core) {
  const char *cc;
  const TargetOverride *ov;
  int use_core;

  StrVec srcs;
  StrVec objs;
  StrVec core_objs;

  char root[512], objd[512], depd[512], bind[512];
  char out_exe[512];

  const char *inc_common[5];

  cc = get_cc();

  ov = find_override(t->name);

  use_core = 0;
  if (ov) use_core = ov->use_core;
  if (no_core) use_core = 0;

  /* prepare dirs */
  ensure_dir(g_build_dir);
  {
    char tdir[512];
    path_join(tdir, sizeof(tdir), g_build_dir, t->id);
    ensure_dir(tdir);
  }
  {
    char tdir[512], pdir[512];
    path_join(tdir, sizeof(tdir), g_build_dir, t->id);
    path_join(pdir, sizeof(pdir), tdir, profile_name(p));
    ensure_dir(pdir);
  }
  build_paths(root, sizeof(root), objd, sizeof(objd), depd, sizeof(depd), bind, sizeof(bind), t->id, p);
  ensure_dir(objd);
  ensure_dir(depd);
  ensure_dir(bind);

  exe_path(out_exe, sizeof(out_exe), t->id, p, t->bin_base);

  /* scan sources:
   * - if app is using src/ (not src/app), skip "core" dir so we don't compile shared code twice
   */
  sv_init(&srcs);
  if (streq(t->name, "app") && streq(t->src_dir, g_src_dir) && file_exists(g_core_dir) && is_dir_path(g_core_dir)) {
    scan_dir_recursive_suffix_skip(&srcs, t->src_dir, ".c", "core");
  } else {
    scan_dir_recursive_suffix(&srcs, t->src_dir, ".c");
  }

  /* allow legacy src/main.c when using src/app */
  if (streq(t->name, "app") && streq(t->src_dir, g_app_dir)) {
    if (file_exists("src/main.c")) sv_push(&srcs, "src/main.c");
  }

  if (srcs.count == 0) {
    fprintf(stderr, "tack: build: no sources in %s for target %s\n", t->src_dir, t->name);
    sv_free(&srcs);
    return 1;
  }

  sv_init(&objs);
  sv_init(&core_objs);

  /* common includes: include + target src dir + src (for shared headers) */
  inc_common[0] = g_inc_dir;
  inc_common[1] = t->src_dir;
  inc_common[2] = g_src_dir;
  if (file_exists(g_core_dir) && is_dir_path(g_core_dir)) inc_common[3] = g_core_dir;
  else inc_common[3] = 0;
  inc_common[4] = 0;

  /* build core (once per target build invocation) */
  if (use_core) {
    if (build_core(p, verbose, why, force, jobs, strict, &core_objs) != 0) {
      sv_free(&srcs); sv_free(&objs); sv_free(&core_objs);
      return 1;
    }
  }

  /* compile target sources */
  if (compile_sources(cc, &srcs, objd, depd,
                      inc_common,
                      ov ? ov->includes : 0,
                      ov ? ov->defines : 0,
                      ov ? ov->cflags : 0,
                      p, verbose, why, force, jobs, strict,
                      &objs) != 0) {
    sv_free(&srcs); sv_free(&objs); sv_free(&core_objs);
    return 1;
  }

  /* link: objs + (core objs if any) */
  {
    StrVec all;
    int i;
    int need_link;

    sv_init(&all);

    for (i = 0; i < objs.count; i++) sv_push(&all, objs.items[i]);
    for (i = 0; i < core_objs.count; i++) sv_push(&all, core_objs.items[i]);

    {
      char why_msg[512];
      need_link = exe_needs_relink_explain(out_exe, &all, force,
                                          why ? why_msg : 0, why ? sizeof(why_msg) : 0);
      if (need_link && why) {
        printf("why link: %s (%s)\n", out_exe, why_msg);
      }
    }

    if (need_link) {
      int rc = link_executable(cc, out_exe, &all,
                               inc_common,
                               ov ? ov->includes : 0,
                               ov ? ov->defines : 0,
                               ov ? ov->ldflags : 0,
                               ov ? ov->libs : 0,
                               p, verbose, strict);
      sv_free(&all);
      if (rc != 0) { sv_free(&srcs); sv_free(&objs); sv_free(&core_objs); return 1; }
    } else if (verbose) {
      printf("up to date: %s\n", out_exe);
    }

    sv_free(&all);
  }

  sv_free(&srcs);
  sv_free(&objs);
  sv_free(&core_objs);

  return 0;
}

/* --------------------------- tests --------------------------- */

static int build_and_run_tests(Profile p, int verbose, int force, int strict) {
  StrVec tests;
  int i;
  const char *cc;

  char tests_root[512];
  char tests_bin[512];

  const char *inc_common[4];

  cc = get_cc();

  sv_init(&tests);
  scan_dir_recursive_suffix(&tests, g_tests_dir, "_test.c");
  if (tests.count == 0) {
    printf("tack: test: no tests found under %s\n", g_tests_dir);
    sv_free(&tests);
    return 0;
  }

  ensure_dir(g_build_dir);
  path_join(tests_root, sizeof(tests_root), g_build_dir, "tests");
  ensure_dir(tests_root);
  path_join(tests_root, sizeof(tests_root), tests_root, profile_name(p));
  ensure_dir(tests_root);
  path_join(tests_bin, sizeof(tests_bin), tests_root, "bin");
  ensure_dir(tests_bin);

  inc_common[0] = g_inc_dir;
  inc_common[1] = g_tests_dir;
  inc_common[2] = g_src_dir;
  inc_common[3] = 0;

  for (i = 0; i < tests.count; i++) {
    const char *src = tests.items[i];
    const char *base = path_base(src);
    char out_exe[1024];

#ifdef _WIN32
    {
      char tmp[512];
      char *dot;
      tack_copy(tmp, sizeof(tmp), base);
      dot = strrchr(tmp, '.');
      if (dot) *dot = '\0';
      tack_cat(tmp, sizeof(tmp), ".exe");
      path_join(out_exe, sizeof(out_exe), tests_bin, tmp);
    }
#else
    {
      char tmp[512];
      char *dot;
      tack_copy(tmp, sizeof(tmp), base);
      dot = strrchr(tmp, '.');
      if (dot) *dot = '\0';
      path_join(out_exe, sizeof(out_exe), tests_bin, tmp);
    }
#endif

    if (force || !file_exists(out_exe) || file_mtime(src) > file_mtime(out_exe)) {
      Argv av;
      int rc;

      av_init(&av);

      av_push(&av, cc);

      push_common_warnings(&av, strict);
      push_profile_flags(&av, p);

      /* includes */
      {
        int k;
        for (k = 0; inc_common[k]; k++) {
          av_push(&av, "-I");
          av_push(&av, inc_common[k]);
        }
      }

      av_push(&av, "-o");
      av_push(&av, out_exe);
      av_push(&av, src);

      av_terminate(&av);

      rc = run_argv_wait(av.a, verbose);
      av_free(&av);

      if (rc != 0) { sv_free(&tests); return 1; }
    }

    /* run test */
    {
      char *runv[2];
      runv[0] = out_exe;
      runv[1] = 0;
      if (run_argv_wait(runv, verbose) != 0) { sv_free(&tests); return 1; }
    }
  }

  sv_free(&tests);
  return 0;
}

/* --------------------------- commands --------------------------- */

static void print_help(void) {
  printf("tack %s - Tiny ANSI-C Kit\n\n", TACK_VERSION);

  puts("Usage:");
  puts("  tack help");
  puts("  tack version");
  puts("  tack doctor");
  puts("  tack init");
  puts("  tack list");
  puts("  tack build [debug|release] [--target NAME] [-v] [--why] [--rebuild] [-j N] [--strict] [--no-core]");
  puts("  tack run  [debug|release] [--target NAME] [-v] [--why] [--rebuild] [-j N] [--strict] [--no-core] [-- <args...>]");
  puts("  tack test [debug|release] [--target NAME] [-v] [--why] [--rebuild] [-j N] [--strict] [--no-core]");
  puts("  tack clean [-v]");
  puts("  tack clobber [-v]");
  puts("  tack bom");
  puts("  tack sbom");
  puts("  tack doc");
  puts("");

  puts("Global options (must come before the command):");
  puts("  --config <path>     use explicit INI file (highest priority)");
  puts("  --no-config         ignore tack.ini and tackfile.c");
  puts("  --no-code-config    ignore tackfile.c (still use tack.ini / --config)");
  puts("  --no-auto-tools     disable tool discovery at runtime");
  puts("  --no-cache          disable compile cache");
  puts("");

  puts("Notes:");
  puts("  - clean    = remove contents under build/ (keep the build directory)");
  puts("  - clobber  = remove build/ itself");
  puts("  - clean/clobber -v prints remaining locked paths (if any)");
  puts("  - init    = also provisions .gitignore and .fossil-settings/ignore-glob (non-destructive)");
  puts("  - bom     = writes build/bom.md and build/bom.html");
  puts("  - sbom    = writes SBOM (default: build/sbom.json; format/output via [sbom])");
  puts("  - sbom    = writes build/sbom.json (tack-sbom-1), build/sbom.cdx.json (cyclonedx),");
  puts("             build/sbom.spdx.json (spdx)");
  puts("  - doc     = writes HTML into build/doc/ (README/FAQ/ROADMAP/RELEASENOTES + BOM)");
  puts("  - sbom format is set via [sbom] format = tack-sbom-1 | cyclonedx | spdx");
  puts("  - --strict enables -Wunsupported");
  puts("  - --why prints short \"why rebuild\" diagnostics for compile/link decisions");
  puts("  - cache   = stored under .tack-cache/ (validated via mtime + size + hash; use --no-cache to disable)");
}

static void cmd_version(void) { printf("tack %s\n", TACK_VERSION); }

static void cmd_doctor(void) {
  printf("Compiler default: %s\n", g_cc_default);
  printf("Compiler override: set env TACK_CC\n");
  printf("Compiler in use: %s\n", get_cc());
#ifdef _WIN32
  printf("OS: Windows\n");
#else
  printf("OS: POSIX\n");
#endif
  printf("Build dir : %s\n", g_build_dir);
  printf("Cache dir : %s (%s)\n", g_cache_dir, g_no_cache ? "disabled" : "enabled");
  printf("Dirs      : src=%s include=%s tests=%s tools=%s core=%s\n",
         g_src_dir, g_inc_dir, g_tests_dir, g_tools_dir, g_core_dir);

  if (g_no_config) {
    printf("Config    : disabled (legacy mode)\n");
    printf("Code cfg  : disabled (legacy mode)\n");
  } else if (g_config_loaded) {
    printf("Config    : %s\n", g_config_path);
    printf("Code cfg  : %s\n", g_no_code_config ? "disabled" : (file_exists("tackfile.c") ? "tackfile.c present" : "none"));
  } else {
    printf("Config    : none\n");
    printf("Code cfg  : %s\n", g_no_code_config ? "disabled" : (file_exists("tackfile.c") ? "tackfile.c present" : "none"));
  }

  printf("Default target: %s\n", default_target_name());

#ifdef TACKFILE_DISABLE_AUTO_TOOLS
  printf("Auto tool discovery: disabled (tackfile compile-time)\n");
#else
  if (g_no_auto_tools_cli) printf("Auto tool discovery: disabled (CLI)\n");
  else if (g_config_loaded && g_config_disable_auto_tools) printf("Auto tool discovery: disabled (config)\n");
  else printf("Auto tool discovery: enabled\n");
#endif

  printf("Overrides : built-ins + optional tackfile.c + optional tack.ini\n");
}

/* --------------------------- init: ignore files --------------------------- */

static const char *TACK_GITIGNORE_BLOCK =
  "###############################################################################\n"
  "# tack (Tiny ANSI-C Kit)\n"
  "###############################################################################\n"
  "\n"
  "# tack build output\n"
  "/build/\n"
  "/.tack-cache/\n"
  "\n"
  "# tackfile.c generator artifacts (optional)\n"
  "/build/_tackfile/\n"
  "tackfile.generated.ini\n"
  "/build/_tackfile/tackfile.generated.ini\n"
  "\n"
  "# generated docs/BOM (optional)\n"
  "/build/bom.md\n"
  "/build/bom.html\n"
  "/build/sbom.json\n"
  "/build/sbom.cdx.json\n"
  "/build/sbom.spdx.json\n"
  "/build/doc/\n";

static const char *GITIGNORE_FULL_LINES[] = {
  "###############################################################################\n",
  "# tack (Tiny ANSI-C Kit)\n",
  "###############################################################################\n",
  "# tack build output\n",
  "/build/\n",
  "/.tack-cache/\n",
  "# tackfile.c generator artifacts (optional)\n",
  "/build/_tackfile/\n",
  "tackfile.generated.ini\n",
  "/build/_tackfile/tackfile.generated.ini\n",
  "# generated docs/BOM (optional)\n",
  "/build/bom.md\n",
  "/build/bom.html\n",
  "/build/sbom.json\n",
  "/build/sbom.cdx.json\n",
  "/build/sbom.spdx.json\n",
  "/build/doc/\n",
  "###############################################################################\n",
  "# Generic C / toolchain ignores\n",
  "###############################################################################\n",
  "# Prerequisites / depfiles\n",
  "*.d\n",
  "# Object files\n",
  "*.o\n",
  "*.ko\n",
  "*.obj\n",
  "*.elf\n",
  "# Linker output\n",
  "*.ilk\n",
  "*.map\n",
  "*.exp\n",
  "# Precompiled Headers\n",
  "*.gch\n",
  "*.pch\n",
  "# Libraries\n",
  "*.lib\n",
  "*.a\n",
  "*.la\n",
  "*.lo\n",
  "# Shared objects (inc. Windows DLLs)\n",
  "*.dll\n",
  "*.so\n",
  "*.so.*\n",
  "*.dylib\n",
  "# Executables\n",
  "*.exe\n",
  "*.out\n",
  "*.app\n",
  "*.i*86\n",
  "*.x86_64\n",
  "*.hex\n",
  "# Debug files\n",
  "*.dSYM/\n",
  "*.su\n",
  "*.idb\n",
  "*.pdb\n",
  "# Kernel Module Compile Results\n",
  "*.mod*\n",
  "*.cmd\n",
  ".tmp_versions/\n",
  "modules.order\n",
  "Module.symvers\n",
  "Mkfile.old\n",
  "dkms.conf\n",
  "# debug information files\n",
  "*.dwo\n",
  "###############################################################################\n",
  "# OS / editor noise\n",
  "###############################################################################\n",
  ".DS_Store\n",
  "Thumbs.db\n",
  0
};

static const char *FOSSIL_IGNORE_BLOCK_LINES[] = {
  "# tack\n",
  "build\n",
  ".tack-cache\n",
  "tackfile.generated.ini\n",
  0
};


static const char *FOSSIL_IGNORE_FULL_LINES[] = {
  "# tack\n",
  "build\n",
  ".tack-cache\n",
  "tackfile.generated.ini\n",
  "# objects / depfiles\n",
  "*.d\n",
  "*.o\n",
  "*.ko\n",
  "*.obj\n",
  "*.elf\n",
  "# linker output\n",
  "*.ilk\n",
  "*.map\n",
  "*.exp\n",
  "# precompiled headers\n",
  "*.gch\n",
  "*.pch\n",
  "# libraries\n",
  "*.lib\n",
  "*.a\n",
  "*.la\n",
  "*.lo\n",
  "# shared objects\n",
  "*.dll\n",
  "*.so\n",
  "*.so.*\n",
  "*.dylib\n",
  "# executables\n",
  "*.exe\n",
  "*.out\n",
  "*.app\n",
  "*.i*86\n",
  "*.x86_64\n",
  "*.hex\n",
  "# debug files\n",
  "*.dSYM\n",
  "*.su\n",
  "*.idb\n",
  "*.pdb\n",
  "# kernel/module stuff\n",
  "*.mod*\n",
  "*.cmd\n",
  ".tmp_versions\n",
  "modules.order\n",
  "Module.symvers\n",
  "Mkfile.old\n",
  "dkms.conf\n",
  "# debug info\n",
  "*.dwo\n",
  "# OS noise\n",
  ".DS_Store\n",
  "Thumbs.db\n",
  0
};

static int cmd_init(void) {
  FILE *f;

  ensure_dir(g_src_dir);
  ensure_dir(g_inc_dir);
  ensure_dir(g_tests_dir);
  ensure_dir(g_tools_dir);
  ensure_dir(g_build_dir);

  /* optional: create src/core and src/app */
  ensure_dir("src/core");
  ensure_dir("src/app");

  if (!file_exists("src/main.c") && !file_exists("src/app/main.c")) {
    /* default to src/main.c for backwards */
    f = fopen("src/main.c", "wb");
    if (!f) { fprintf(stderr, "tack: init: cannot create src/main.c\n"); return 1; }
    fprintf(f,
      "#include <stdio.h>\n\n"
      "int main(int argc, char **argv) {\n"
      "  (void)argc; (void)argv;\n"
      "  puts(\"Hello from tack v0.4.0!\");\n"
      "  return 0;\n"
      "}\n"
    );
    fclose(f);
  }

  if (!file_exists("tests/smoke_test.c")) {
    f = fopen("tests/smoke_test.c", "wb");
    if (!f) { fprintf(stderr, "tack: init: cannot create tests/smoke_test.c\n"); return 1; }
    fprintf(f,
      "#include <stdio.h>\n\n"
      "int main(void) {\n"
      "  puts(\"smoke_test: ok\");\n"
      "  return 0;\n"
      "}\n"
    );
    fclose(f);
  }

  /* -------------------- ignore files: git + fossil --------------------- */

  /* ignore files: git + fossil (non-destructive) */
  {
    /* .gitignore: if missing -> write full; else append tack block if not present */
  if (!file_exists(".gitignore")) {
      if (write_file_if_missing_lines(".gitignore", GITIGNORE_FULL_LINES) != 0) {
      fprintf(stderr, "tack: init: cannot create .gitignore\n");
      return 1;
    }
  } else {
      if (append_block_if_missing(".gitignore", "tack (Tiny ANSI-C Kit)", TACK_GITIGNORE_BLOCK) != 0) {
      fprintf(stderr, "tack: init: cannot update .gitignore\n");
      return 1;
    }
  }

  /* Fossil: .fossil-settings/ignore-glob */
  ensure_dir(".fossil-settings");
  {
      char fp[512];
      path_join(fp, sizeof(fp), ".fossil-settings", "ignore-glob");

    if (!file_exists(fp)) {
        FILE *ff = fopen(fp, "wb");
        if (!ff) { fprintf(stderr, "tack: init: cannot create %s\n", fp); return 1; }
        fputs_lines(ff, FOSSIL_IGNORE_FULL_LINES);
        fclose(ff);
    } else {
        if (append_block_if_missing_lines(fp, "# tack", FOSSIL_IGNORE_BLOCK_LINES) != 0) {
        fprintf(stderr, "tack: init: cannot update %s\n", fp);
        return 1;
      }
    }
  }
  }

  printf("tack: init: ensured src/include/tests/tools/build (+ ignore files)\n");
  return 0;
}

static int cmd_clean(int verbose) {
  int rc;
  /* clean = remove contents under build/, keep build directory */
  if (!file_exists(g_build_dir)) return 0;

  rm_collect_begin(verbose);
  rc = rm_rf_contents(g_build_dir);
  rm_collect_end_report("clean");

  if (rc != 0) {
    fprintf(stderr, "tack: clean: failed\n");
    return 1;
  }
  printf("tack: clean: done\n");
  return 0;
}

static int cmd_clobber(int verbose) {
  int rc;
  /* clobber = remove build/ itself */
  if (!file_exists(g_build_dir)) return 0;

  rm_collect_begin(verbose);
  rc = rm_rf(g_build_dir);
  rm_collect_end_report("clobber");

  if (rc != 0) {
    fprintf(stderr, "tack: clobber: failed\n");
    return 1;
  }
  printf("tack: clobber: done\n");
  return 0;
}

static int cmd_list_targets(TargetVec *tv) {
  int i;
  printf("Targets:\n");
  for (i = 0; i < tv->count; i++) {
    const TargetOverride *ov = find_override(tv->items[i].name);
    int use_core = ov ? ov->use_core : 0;
    printf("  %-16s  id=%-12s  src=%s  core=%s  enabled=%s\n",
           tv->items[i].name, tv->items[i].id, tv->items[i].src_dir,
           use_core ? "yes" : "no",
           tv->items[i].enabled ? "yes" : "no");
  }
  return 0;
}


/* --------------------------- BOM / DOC --------------------------- */

static void write_html_escaped(FILE *f, const char *s) {
  const unsigned char *p = (const unsigned char*)s;
  while (*p) {
    if (*p == '&') fputs("&amp;", f);
    else if (*p == '<') fputs("&lt;", f);
    else if (*p == '>') fputs("&gt;", f);
    else if (*p == '"') fputs("&quot;", f);
    else fputc((int)*p, f);
    p++;
  }
}

static void json_write_indent(FILE *f, int n) {
  int i;
  for (i = 0; i < n; i++) fputc(' ', f);
}

static void json_write_string(FILE *f, const char *s) {
  const unsigned char *p = (const unsigned char*)(s ? s : "");
  static const char hex[] = "0123456789abcdef";

  fputc('"', f);
  while (*p) {
    unsigned char c = *p++;
    if (c == '"') fputs("\\\"", f);
    else if (c == '\\') fputs("\\\\", f);
    else if (c == '\b') fputs("\\b", f);
    else if (c == '\f') fputs("\\f", f);
    else if (c == '\n') fputs("\\n", f);
    else if (c == '\r') fputs("\\r", f);
    else if (c == '\t') fputs("\\t", f);
    else if (c < 0x20) {
      fputs("\\u00", f);
      fputc(hex[(c >> 4) & 0x0f], f);
      fputc(hex[c & 0x0f], f);
    } else {
      fputc((int)c, f);
    }
  }
  fputc('"', f);
}

static void json_write_bool(FILE *f, int v) {
  fputs(v ? "true" : "false", f);
}

static void json_write_null(FILE *f) {
  fputs("null", f);
}

static int cmp_cstr_ptr(const void *a, const void *b) {
  const char * const *sa = (const char * const *)a;
  const char * const *sb = (const char * const *)b;
  return strcmp(*sa, *sb);
}

static void sv_sort_unique(StrVec *v) {
  int i;
  int w = 0;
  if (!v || v->count <= 1) return;
  qsort(v->items, (size_t)v->count, sizeof(char*), cmp_cstr_ptr);
  for (i = 0; i < v->count; i++) {
    if (w == 0 || strcmp(v->items[i], v->items[w - 1]) != 0) {
      v->items[w++] = v->items[i];
    } else {
      free(v->items[i]);
    }
  }
  v->count = w;
}

static void sv_append_list(StrVec *v, const char * const *lst) {
  int i;
  if (!v || !lst) return;
  for (i = 0; lst[i]; i++) sv_push(v, lst[i]);
}

static void json_write_string_array(FILE *f, int indent, const StrVec *v) {
  int i;
  fputs("[", f);
  if (!v || v->count == 0) {
    fputs("]", f);
    return;
  }
  fputc('\n', f);
  for (i = 0; i < v->count; i++) {
    json_write_indent(f, indent + 2);
    json_write_string(f, v->items[i]);
    if (i + 1 < v->count) fputc(',', f);
    fputc('\n', f);
  }
  json_write_indent(f, indent);
  fputc(']', f);
}

static void fprint_time_local(FILE *f) {
  time_t t = time(0);
  struct tm *tmv = localtime(&t);
  if (!tmv) {
    fprintf(f, "%ld", (long)t);
    return;
  }
  fprintf(f, "%04d-%02d-%02d %02d:%02d:%02d",
          tmv->tm_year + 1900, tmv->tm_mon + 1, tmv->tm_mday,
          tmv->tm_hour, tmv->tm_min, tmv->tm_sec);
}

/* --------------------------- HTML templating (optional) --------------------------- */

typedef void (*EmitFn)(FILE *out, void *ctx);

typedef struct {
  const char *page_title;
  const char *project_title;
  const char *head_assets_html;
  const char *nav_html;
  const char *toc_html;
  EmitFn emit_content;
  void *content_ctx;
  EmitFn emit_footer;
  void *footer_ctx;
} HtmlPage;

static char *read_entire_file_capped(const char *path, long *out_len, long cap) {
  FILE *f;
  long n;
  char *buf;

  if (cap <= 0) return 0;
  f = fopen(path, "rb");
  if (!f) return 0;
  if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
  n = ftell(f);
  if (n < 0 || n > cap) { fclose(f); return 0; }
  if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 0; }

  buf = (char*)xmalloc((size_t)n + 1);
  if (n > 0) {
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(buf); return 0; }
  }
  buf[n] = '\0';
  fclose(f);
  if (out_len) *out_len = n;
  return buf;
}

static const char *choose_template(const char *kind) {
  /* kind: "doc" or "bom" */
  if (streq(kind, "doc")) return g_config_doc_template;
  if (streq(kind, "bom")) {
    if (g_config_bom_template) return g_config_bom_template;
    /* fallback: reuse doc template if only one template is desired */
    return g_config_doc_template;
  }
  return 0;
}

static const char *choose_css(const char *kind) {
  if (streq(kind, "doc")) return g_config_doc_css;
  if (streq(kind, "bom")) {
    if (g_config_bom_css) return g_config_bom_css;
    return g_config_doc_css;
  }
  return 0;
}

static char *make_head_assets(const char *css_href) {
  /* css_href is a relative href for the generated page (may be NULL). */
  const char *pre = "";
  const char *post = "";
  const char *link_pre = "<link rel=\"stylesheet\" href=\"";
  const char *link_post = "\">\n";
  size_t n = strlen(pre) + strlen(post);
  char *out;

  if (css_href && css_href[0]) {
    n += strlen(link_pre) + strlen(css_href) + strlen(link_post);
  }
  out = (char*)xmalloc(n + 1);
  out[0] = '\0';
  strcat(out, pre);
  if (css_href && css_href[0]) {
    strcat(out, link_pre);
    strcat(out, css_href);
    strcat(out, link_post);
  }
  strcat(out, post);
  return out;
}

static char *make_nav_block(const char *inner_links_html) {
  const char *pre = "<nav id=\"tack-nav\">";
  const char *post = "</nav>\n";
  size_t n = strlen(pre) + strlen(post);
  char *out;

  if (inner_links_html) n += strlen(inner_links_html);
  out = (char*)xmalloc(n + 1);
  out[0] = '\0';
  strcat(out, pre);
  if (inner_links_html) strcat(out, inner_links_html);
  strcat(out, post);
  return out;
}

static char *make_empty_toc_block(void) {
  return xstrdup("");
}

static void emit_footer_default(FILE *out, void *ctx) {
  (void)ctx;
  fputs("<footer id=\"tack-footer\">", out);
  fputs("<p>Generated by tack at ", out);
  fprint_time_local(out);
  fputs(".</p></footer>\n", out);
}

static int tpl_render(FILE *out, const char *tpl_text, const HtmlPage *pg, int *saw_content) {
  const char *p = tpl_text;

  if (saw_content) *saw_content = 0;

  while (*p) {
    if (strncmp(p, "{{TACK_PAGE_TITLE}}", (int)(sizeof("{{TACK_PAGE_TITLE}}") - 1)) == 0) {
      if (pg->page_title) write_html_escaped(out, pg->page_title);
      p += (int)(sizeof("{{TACK_PAGE_TITLE}}") - 1);
      continue;
    }
    if (strncmp(p, "{{TACK_PROJECT_TITLE}}", (int)(sizeof("{{TACK_PROJECT_TITLE}}") - 1)) == 0) {
      if (pg->project_title) write_html_escaped(out, pg->project_title);
      p += (int)(sizeof("{{TACK_PROJECT_TITLE}}") - 1);
      continue;
    }
    if (strncmp(p, "{{TACK_HEAD_ASSETS}}", (int)(sizeof("{{TACK_HEAD_ASSETS}}") - 1)) == 0) {
      if (pg->head_assets_html) fputs(pg->head_assets_html, out);
      p += (int)(sizeof("{{TACK_HEAD_ASSETS}}") - 1);
      continue;
    }
    if (strncmp(p, "{{TACK_NAV_HTML}}", (int)(sizeof("{{TACK_NAV_HTML}}") - 1)) == 0) {
      if (pg->nav_html) fputs(pg->nav_html, out);
      p += (int)(sizeof("{{TACK_NAV_HTML}}") - 1);
      continue;
    }
    if (strncmp(p, "{{TACK_TOC_HTML}}", (int)(sizeof("{{TACK_TOC_HTML}}") - 1)) == 0) {
      if (pg->toc_html) fputs(pg->toc_html, out);
      p += (int)(sizeof("{{TACK_TOC_HTML}}") - 1);
      continue;
    }
    if (strncmp(p, "{{TACK_CONTENT_HTML}}", (int)(sizeof("{{TACK_CONTENT_HTML}}") - 1)) == 0) {
      if (saw_content) *saw_content = 1;
      if (pg->emit_content) pg->emit_content(out, pg->content_ctx);
      p += (int)(sizeof("{{TACK_CONTENT_HTML}}") - 1);
      continue;
    }
    if (strncmp(p, "{{TACK_FOOTER_HTML}}", (int)(sizeof("{{TACK_FOOTER_HTML}}") - 1)) == 0) {
      if (pg->emit_footer) pg->emit_footer(out, pg->footer_ctx);
      p += (int)(sizeof("{{TACK_FOOTER_HTML}}") - 1);
      continue;
    }

    fputc((unsigned char)*p, out);
    p++;
  }

  return 0;
}

static int write_html_page(const char *out_path, const char *kind, const HtmlPage *pg) {
  FILE *f;
  const char *tpl_path;
  int saw_content = 0;

  tpl_path = choose_template(kind);

  f = fopen(out_path, "wb");
  if (!f) return 1;

  /* template path configured -> fail-fast on missing/bad template */
  if (tpl_path && tpl_path[0]) {
    long n = 0;
    char *tpl;

    if (!file_exists(tpl_path)) {
      fclose(f);
      fprintf(stderr, "tack: %s: template not found: %s\n", kind, tpl_path);
      return 2;
    }

    tpl = read_entire_file_capped(tpl_path, &n, 1024L * 1024L);
    (void)n;
    if (!tpl) {
      fclose(f);
      fprintf(stderr, "tack: %s: cannot read template %s\n", kind, tpl_path);
      return 2;
    }

    tpl_render(f, tpl, pg, &saw_content);
    free(tpl);

    if (!saw_content) {
      fclose(f);
      fprintf(stderr, "tack: %s: template missing {{TACK_CONTENT_HTML}}: %s\n", kind, tpl_path);
      return 2;
    }

    fclose(f);
    return 0;
  }

  /* built-in default layout (markers are the output contract) */
  fputs("<!doctype html>\n<html><head><meta charset=\"utf-8\">", f);
  fputs("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">", f);
  fputs("<title>", f); if (pg->page_title) write_html_escaped(f, pg->page_title); fputs("</title>\n", f);

  fputs("<!-- TACK:BEGIN HEAD_ASSETS -->\n", f);
  if (pg->head_assets_html) fputs(pg->head_assets_html, f);
  fputs("<!-- TACK:END HEAD_ASSETS -->\n", f);

  fputs("</head><body>\n", f);

  fputs("<!-- TACK:BEGIN NAV -->\n", f);
  if (pg->nav_html) fputs(pg->nav_html, f);
  fputs("<!-- TACK:END NAV -->\n", f);

  fputs("<!-- TACK:BEGIN TOC -->\n", f);
  if (pg->toc_html) fputs(pg->toc_html, f);
  fputs("<!-- TACK:END TOC -->\n", f);

  fputs("<!-- TACK:BEGIN CONTENT -->\n", f);
  if (pg->emit_content) pg->emit_content(f, pg->content_ctx);
  fputs("<!-- TACK:END CONTENT -->\n", f);

  fputs("<!-- TACK:BEGIN FOOTER -->\n", f);
  if (pg->emit_footer) pg->emit_footer(f, pg->footer_ctx);
  fputs("<!-- TACK:END FOOTER -->\n", f);

  fputs("</body></html>\n", f);
  fclose(f);
  return 0;
}

typedef struct {
  const char *kind;          /* "doc" or "bom" */
  const char *project_title; /* optional; escaped in template */
  const char *css_href;      /* relative href for generated page (copied to outdir) */
  const char *nav_inner;     /* inner links (a-tags) */
} HtmlCfg;

typedef struct {
  char *md; /* owned; freed by caller */
} MdCtx;

static void emit_md_pre(FILE *out, void *ctx) {
  MdCtx *m = (MdCtx*)ctx;
  fputs("<main id=\"tack-content\"><pre>", out);
  if (m && m->md) write_html_escaped(out, m->md);
  fputs("</pre></main>\n", out);
}

static int write_doc_page(const char *out_path, const char *title, const char *nav_inner, const HtmlCfg *hc, const char *md_path) {
  HtmlPage pg;
  MdCtx mctx;
  long n = 0;
  char *md = 0;
  char *head_assets = 0;
  char *nav = 0;
  char *toc = 0;
  int rc;

  md = read_entire_file(md_path, &n);
  (void)n;

  memset(&pg, 0, sizeof(pg));
  memset(&mctx, 0, sizeof(mctx));
  mctx.md = md;

  head_assets = make_head_assets(hc ? hc->css_href : 0);
  nav = make_nav_block(nav_inner);
  toc = make_empty_toc_block();

  pg.page_title = title;
  pg.project_title = (hc && hc->project_title) ? hc->project_title : "tack";
  pg.head_assets_html = head_assets;
  pg.nav_html = nav;
  pg.toc_html = toc;
  pg.emit_content = emit_md_pre;
  pg.content_ctx = &mctx;
  pg.emit_footer = emit_footer_default;
  pg.footer_ctx = 0;

  rc = write_html_page(out_path, (hc && hc->kind) ? hc->kind : "doc", &pg);

  free(head_assets);
  free(nav);
  free(toc);
  if (md) free(md);

  return rc;
}

typedef struct {
  int has_readme;
  int has_faq;
  int has_roadmap;
  int has_releasenotes;
} DocIndexCtx;

static void emit_doc_index(FILE *out, void *ctx) {
  DocIndexCtx *d = (DocIndexCtx*)ctx;

  fputs("<main id=\"tack-content\">", out);
  fputs("<h1>tack docs</h1>", out);
  fputs("<ul>", out);
  if (!d || d->has_readme) fputs("<li><a href=\"readme.html\">README</a></li>", out);
  if (!d || d->has_faq) fputs("<li><a href=\"faq.html\">FAQ</a></li>", out);
  if (!d || d->has_roadmap) fputs("<li><a href=\"roadmap.html\">Roadmap</a></li>", out);
  if (!d || d->has_releasenotes) fputs("<li><a href=\"releasenotes.html\">Release Notes</a></li>", out);
  fputs("<li><a href=\"../bom.html\">BOM</a></li>", out);
  fputs("</ul>", out);
  fputs("<p>Generated by tack. Pages are simple offline HTML wrappers around Markdown (no JS).</p>", out);
  fputs("</main>\n", out);
}

static void md_list_strings(FILE *f, const char *title, const char * const *lst) {
  int i;
  if (!lst) return;
  fprintf(f, "### %s\n\n", title);
  for (i = 0; lst[i]; i++) fprintf(f, "- `%s`\n", lst[i]);
  fputc('\n', f);
}

static void md_list_sources(FILE *f, const char *title, StrVec *srcs) {
  int i;
  fprintf(f, "### %s\n\n", title);
  for (i = 0; i < srcs->count; i++) fprintf(f, "- `%s`\n", srcs->items[i]);
  fputc('\n', f);
}

static int gather_target_sources(StrVec *out, const Target *t) {
  /* mirror build_one_target scanning rules */
  if (!t || !t->enabled) return 1;

  if (streq(t->name, "app") && streq(t->src_dir, g_src_dir) &&
      file_exists(g_core_dir) && is_dir_path(g_core_dir)) {
    scan_dir_recursive_suffix_skip(out, t->src_dir, ".c", "core");
  } else {
    scan_dir_recursive_suffix(out, t->src_dir, ".c");
  }

  /* allow legacy src/main.c when using src/app */
  if (streq(t->name, "app") && streq(t->src_dir, g_app_dir)) {
    if (file_exists("src/main.c")) sv_push(out, "src/main.c");
  }

  return 0;
}

static int cmd_bom(Profile p, TargetVec *tv, const Target *t, int verbose, int strict, int no_core,
                   const char *outdir) {
  FILE *f;
  char bom_md[512];
  char bom_html[512];
  const TargetOverride *ov;
  int use_core_effective;

  StrVec srcs;
  StrVec core_srcs;

  const char *inc_common[5];

  (void)tv;

  if (!outdir) outdir = g_build_dir;

  ensure_dir(outdir);
  path_join(bom_md, sizeof(bom_md), outdir, "bom.md");
  path_join(bom_html, sizeof(bom_html), outdir, "bom.html");

  if (verbose) printf("tack: bom: writing %s and %s\n", bom_md, bom_html);

  f = fopen(bom_md, "wb");
  if (!f) {
    fprintf(stderr, "tack: bom: cannot write %s\n", bom_md);
    return 1;
  }

  fprintf(f, "# tack BOM (Build Manifest)\n\n");
  fprintf(f, "- Generated: **");
  fprint_time_local(f);
  fprintf(f, "**\n");
  fprintf(f, "- tack: **v%s**\n", TACK_VERSION);
#ifdef _WIN32
  fprintf(f, "- OS: **Windows**\n");
#else
  fprintf(f, "- OS: **POSIX**\n");
#endif
  fprintf(f, "- Compiler: **%s**\n", get_cc());
  fprintf(f, "- Profile: **%s**\n", profile_name(p));
  fprintf(f, "- Strict warnings: **%s**\n", strict ? "yes" : "no");
  fprintf(f, "- Core forced off (--no-core): **%s**\n\n", no_core ? "yes" : "no");

  fprintf(f, "## Configuration sources\n\n");
  if (g_no_config) fprintf(f, "- tack.ini: **disabled** (`--no-config`)\n");
  else if (g_config_loaded) fprintf(f, "- tack.ini: `%s`\n", g_config_path);
  else fprintf(f, "- tack.ini: *(none)*\n");

#ifdef TACK_USE_TACKFILE
  fprintf(f, "- tackfile.c: **compile-time include** (`-DTACK_USE_TACKFILE`)\n");
#else
  if (file_exists("tackfile.c")) {
    fprintf(f, "- tackfile.c: **runtime generator** (compiled & executed, fail-fast)\n");
  } else {
    fprintf(f, "- tackfile.c: *(none)*\n");
  }
#endif

  fprintf(f, "\n## Target\n\n");
  fprintf(f, "- name: `%s`\n", t->name);
  fprintf(f, "- id: `%s`\n", t->id);
  fprintf(f, "- src: `%s`\n", t->src_dir);
  fprintf(f, "- bin: `%s`\n", t->bin_base);
  fprintf(f, "- enabled: **%s**\n", t->enabled ? "yes" : "no");

  ov = find_override(t->name);
  use_core_effective = (ov && ov->use_core) ? 1 : 0;
  if (no_core) use_core_effective = 0;
  fprintf(f, "- core: **%s**\n\n", use_core_effective ? "yes" : "no");

  fprintf(f, "## Effective flags\n\n");

  fprintf(f, "### Common warnings\n\n");
  {
    int i;
    for (i = 0; g_warn_flags_base[i]; i++) fprintf(f, "- `%s`\n", g_warn_flags_base[i]);
    if (strict) {
      for (i = 0; g_warn_flags_strict_add[i]; i++) fprintf(f, "- `%s`\n", g_warn_flags_strict_add[i]);
    }
    fputc('\n', f);
  }

  fprintf(f, "### Profile flags\n\n");
  if (p == PROF_DEBUG) {
    fprintf(f, "- `-g`\n- `-bt20`\n- `-DDEBUG=1`\n\n");
  } else {
    fprintf(f, "- `-O2`\n- `-DNDEBUG=1`\n\n");
  }

  /* common includes: include + target src dir + src + (optional core) */
  inc_common[0] = g_inc_dir;
  inc_common[1] = t->src_dir;
  inc_common[2] = g_src_dir;
  if (file_exists(g_core_dir) && is_dir_path(g_core_dir)) inc_common[3] = g_core_dir;
  else inc_common[3] = 0;
  inc_common[4] = 0;

  md_list_strings(f, "Include search path (common)", inc_common);
  if (ov && ov->includes) md_list_strings(f, "Target includes (extra)", ov->includes);
  if (ov && ov->defines) md_list_strings(f, "Target defines (extra)", ov->defines);
  if (ov && ov->cflags) md_list_strings(f, "Target cflags (extra)", ov->cflags);
  if (ov && ov->ldflags) md_list_strings(f, "Target ldflags (extra)", ov->ldflags);
  if (ov && ov->libs) md_list_strings(f, "Target libs (extra)", ov->libs);

  sv_init(&srcs);
  if (gather_target_sources(&srcs, t) != 0 || srcs.count == 0) {
    fprintf(stderr, "tack: bom: cannot gather sources for %s\n", t->name);
    sv_free(&srcs);
    fclose(f);
    return 1;
  }

  if (use_core_effective) {
    sv_init(&core_srcs);
    if (file_exists(g_core_dir) && is_dir_path(g_core_dir)) {
      scan_dir_recursive_suffix(&core_srcs, g_core_dir, ".c");
    }
    md_list_sources(f, "Sources (core)", &core_srcs);
    sv_free(&core_srcs);
  }

  md_list_sources(f, "Sources (target)", &srcs);
  sv_free(&srcs);

  fprintf(f, "## Notes\n\n");
  fprintf(f, "- This BOM is a **build manifest**, not a full SBOM.\n");
  fprintf(f, "- It reflects **effective tack configuration** (tack.ini/tackfile.c + defaults) and discovered sources.\n");
  fprintf(f, "- For reproducible pipelines, commit `tack.ini` and (optionally) `tackfile.c`.\n");
  fclose(f);

  /* HTML: optional template wrapping */
  {
    HtmlCfg hc;
    char css_href[256];
    char css_dst[512];
    const char *css_path;
    const char *nav_inner = "<a href=\"doc/index.html\">Docs</a>";

    memset(&hc, 0, sizeof(hc));
    hc.kind = "bom";
    hc.project_title = "tack";

    css_href[0] = '\0';
    css_path = choose_css("bom");
    if (css_path && css_path[0] && !file_exists(css_path)) {
      fprintf(stderr, "tack: bom: css not found: %s\n", css_path);
      return 2;
    }
    if (css_path && css_path[0] && file_exists(css_path)) {
      const char *base = path_base(css_path);
      if (strlen(base) >= sizeof(css_href)) tack_die("css filename too long");
      tack_copy(css_href, sizeof(css_href), base);
      path_join(css_dst, sizeof(css_dst), outdir, css_href);
      if (copy_file(css_path, css_dst) != 0) {
        fprintf(stderr, "tack: bom: cannot copy css %s -> %s\n", css_path, css_dst);
        return 1;
      }
      hc.css_href = css_href;
    }

  {
    int rc2 = write_doc_page(bom_html, "tack BOM", nav_inner, &hc, bom_md);
    if (rc2 != 0) return rc2;
  }
}

  /* Always give a minimal CLI confirmation (doc already prints its own summary). */
  printf("tack: bom: wrote %s and %s\n", bom_md, bom_html);

  return 0;
}

static int cmd_sbom(Profile p, TargetVec *tv, const Target *t, int verbose, int strict, int no_core,
                    const char *outdir) {
  FILE *f;
  char sbom_path[TACK_MAX_CONFIG_PATH + 1];
  char format_buf[128];
  char dirbuf[TACK_MAX_CONFIG_PATH + 1];
  const TargetOverride *ov;
  int use_core_effective;
  const char *format;
  int format_tack = 0;
  const char *inc_common[5];
  const char *tackfile_mode = "none";
  const char *sbom_format;
  const char *sbom_spec_version;
  const char *sbom_output;
  const char *format_string = "tack-sbom-1";

  StrVec includes;
  StrVec defines;
  StrVec cflags;
  StrVec ldflags;
  StrVec libs;
  StrVec srcs;
  StrVec core_srcs;

  (void)tv;

  if (!outdir) outdir = g_build_dir;
  sbom_format = g_config_sbom_format ? g_config_sbom_format : "tack";
  sbom_spec_version = g_config_sbom_spec_version;
  sbom_output = g_config_sbom_output;

  if (strieq(sbom_format, "tack")) {
    if (sbom_spec_version && sbom_spec_version[0]) {
      tack_copy(format_buf, sizeof(format_buf), "tack-sbom-");
      tack_cat(format_buf, sizeof(format_buf), sbom_spec_version);
      format_string = format_buf;
    }
  } else {
    fprintf(stderr, "tack: sbom: format %s not supported (only tack)\n", sbom_format);
    return 2;
  }

  format = sbom_format_effective();
  if (strieq(format, "tack") || strieq(format, "tack-sbom-1")) {
    format_tack = 1;
  } else if (strieq(format, "cyclonedx") || strieq(format, "spdx")) {
    tack_die("sbom format not implemented (use format=tack)");
  } else {
    tack_die("unknown sbom format");
  }

  if (g_config_sbom_spec_version && g_config_sbom_spec_version[0]) {
    /* reserved for future sbom formats (CycloneDX/SPDX) */
  }

  if (g_config_sbom_output && g_config_sbom_output[0]) {
    if (strlen(g_config_sbom_output) >= sizeof(sbom_path)) tack_die("sbom output path too long");
    tack_copy(sbom_path, sizeof(sbom_path), g_config_sbom_output);
    ensure_parent_dir_recursive(sbom_path);
  if (sbom_output && sbom_output[0]) {
    const char *p;
    const char *last_sep = 0;

    if (outdir && outdir[0] && !is_abs_path(sbom_output)) {
      ensure_dir(outdir);
      path_join(sbom_path, sizeof(sbom_path), outdir, sbom_output);
    } else {
      tack_copy(sbom_path, sizeof(sbom_path), sbom_output);
    }

    for (p = sbom_path; *p; p++) {
      if (*p == '/' || *p == '\\') last_sep = p;
    }
    if (last_sep && last_sep > sbom_path) {
      size_t dlen = (size_t)(last_sep - sbom_path);
      if (dlen >= sizeof(dirbuf)) tack_die("sbom output path too long");
      memcpy(dirbuf, sbom_path, dlen);
      dirbuf[dlen] = '\0';
      ensure_dir(dirbuf);
    }
  } else {
    ensure_dir(outdir);
    path_join(sbom_path, sizeof(sbom_path), outdir, "sbom.json");
  }

  if (verbose) printf("tack: sbom: writing %s\n", sbom_path);

  f = fopen(sbom_path, "wb");
  if (!f) {
    fprintf(stderr, "tack: sbom: cannot write %s\n", sbom_path);
typedef enum {
  SBOM_FORMAT_TACK = 0,
  SBOM_FORMAT_CYCLONEDX = 1,
  SBOM_FORMAT_SPDX = 2
} SbomFormat;

typedef struct {
  const Target *t;
  const char *profile;
  const char *compiler;
  const char *tackfile_mode;
  int strict;
  int no_core;
  int config_enabled;
  const char *config_path;
  int use_core_effective;
  const StrVec *includes;
  const StrVec *defines;
  const StrVec *cflags;
  const StrVec *ldflags;
  const StrVec *libs;
  const StrVec *srcs;
  const StrVec *core_srcs;
} SbomData;

static const char *sbom_format_name(SbomFormat fmt) {
  if (fmt == SBOM_FORMAT_CYCLONEDX) return "cyclonedx";
  if (fmt == SBOM_FORMAT_SPDX) return "spdx";
  return "tack-sbom-1";
}

static const char *sbom_format_filename(SbomFormat fmt) {
  if (fmt == SBOM_FORMAT_CYCLONEDX) return "sbom.cdx.json";
  if (fmt == SBOM_FORMAT_SPDX) return "sbom.spdx.json";
  return "sbom.json";
}

static int sbom_format_from_string(const char *s, SbomFormat *out) {
  const char *v = s;
  if (!v || !v[0]) v = "tack-sbom-1";
  if (strieq(v, "tack") || strieq(v, "tack-sbom-1")) {
    if (out) *out = SBOM_FORMAT_TACK;
    return 1;
  }
  if (strieq(v, "cyclonedx") || strieq(v, "cyclonedx-1.4")) {
    if (out) *out = SBOM_FORMAT_CYCLONEDX;
    return 1;
  }
  if (strieq(v, "spdx") || strieq(v, "spdx-2.3")) {
    if (out) *out = SBOM_FORMAT_SPDX;
    return 1;
  }
  return 0;
}

static void write_sbom_tack(FILE *f, const SbomData *d) {
  fputs("{\n", f);
  json_write_indent(f, 2);
  fputs("\"format\": ", f);
  if (format_tack) {
    json_write_string(f, "tack-sbom-1");
  } else {
    json_write_string(f, format);
  }
  json_write_string(f, format_string);
  fputs(",\n", f);

  json_write_indent(f, 2);
  fputs("\"tool\": {\n", f);
  json_write_indent(f, 4);
  fputs("\"name\": ", f);
  json_write_string(f, "tack");
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"version\": ", f);
  json_write_string(f, TACK_VERSION);
  fputc('\n', f);
  json_write_indent(f, 2);
  fputs("},\n", f);

  json_write_indent(f, 2);
  fputs("\"compiler\": ", f);
  json_write_string(f, d->compiler);
  fputs(",\n", f);

  json_write_indent(f, 2);
  fputs("\"profile\": ", f);
  json_write_string(f, d->profile);
  fputs(",\n", f);

  json_write_indent(f, 2);
  fputs("\"strict\": ", f);
  json_write_bool(f, d->strict);
  fputs(",\n", f);

  json_write_indent(f, 2);
  fputs("\"no_core\": ", f);
  json_write_bool(f, d->no_core);
  fputs(",\n", f);

  json_write_indent(f, 2);
  fputs("\"config\": {\n", f);
  json_write_indent(f, 4);
  fputs("\"enabled\": ", f);
  json_write_bool(f, d->config_enabled);
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"tack_ini\": ", f);
  if (!d->config_enabled || !d->config_path || !d->config_path[0]) json_write_null(f);
  else json_write_string(f, d->config_path);
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"tackfile\": ", f);
  json_write_string(f, d->tackfile_mode);
  fputc('\n', f);
  json_write_indent(f, 2);
  fputs("},\n", f);

  json_write_indent(f, 2);
  fputs("\"target\": {\n", f);
  json_write_indent(f, 4);
  fputs("\"name\": ", f);
  json_write_string(f, d->t->name);
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"id\": ", f);
  json_write_string(f, d->t->id);
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"src\": ", f);
  json_write_string(f, d->t->src_dir);
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"bin\": ", f);
  json_write_string(f, d->t->bin_base);
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"enabled\": ", f);
  json_write_bool(f, d->t->enabled ? 1 : 0);
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"core\": ", f);
  json_write_bool(f, d->use_core_effective ? 1 : 0);
  fputc('\n', f);
  json_write_indent(f, 2);
  fputs("},\n", f);

  json_write_indent(f, 2);
  fputs("\"flags\": {\n", f);
  json_write_indent(f, 4);
  fputs("\"includes\": ", f);
  json_write_string_array(f, 4, d->includes);
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"defines\": ", f);
  json_write_string_array(f, 4, d->defines);
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"cflags\": ", f);
  json_write_string_array(f, 4, d->cflags);
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"ldflags\": ", f);
  json_write_string_array(f, 4, d->ldflags);
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"libs\": ", f);
  json_write_string_array(f, 4, d->libs);
  fputc('\n', f);
  json_write_indent(f, 2);
  fputs("},\n", f);

  json_write_indent(f, 2);
  fputs("\"sources\": {\n", f);
  json_write_indent(f, 4);
  fputs("\"core\": ", f);
  json_write_string_array(f, 4, d->core_srcs);
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"target\": ", f);
  json_write_string_array(f, 4, d->srcs);
  fputc('\n', f);
  json_write_indent(f, 2);
  fputs("}\n", f);

  fputs("}\n", f);
}

static void write_cdx_components(FILE *f, int indent, const StrVec *core_srcs, const StrVec *srcs) {
  int i;
  int count = 0;
  if (core_srcs) count += core_srcs->count;
  if (srcs) count += srcs->count;

  fputs("[", f);
  if (count == 0) {
    fputs("]", f);
    return;
  }
  fputc('\n', f);

  if (core_srcs) {
    for (i = 0; i < core_srcs->count; i++) {
      json_write_indent(f, indent + 2);
      fputs("{\n", f);
      json_write_indent(f, indent + 4);
      fputs("\"type\": ", f);
      json_write_string(f, "file");
      fputs(",\n", f);
      json_write_indent(f, indent + 4);
      fputs("\"name\": ", f);
      json_write_string(f, core_srcs->items[i]);
      fputc('\n', f);
      json_write_indent(f, indent + 2);
      fputs("}", f);
      if (i + 1 < core_srcs->count || (srcs && srcs->count > 0)) fputc(',', f);
      fputc('\n', f);
    }
  }

  if (srcs) {
    for (i = 0; i < srcs->count; i++) {
      json_write_indent(f, indent + 2);
      fputs("{\n", f);
      json_write_indent(f, indent + 4);
      fputs("\"type\": ", f);
      json_write_string(f, "file");
      fputs(",\n", f);
      json_write_indent(f, indent + 4);
      fputs("\"name\": ", f);
      json_write_string(f, srcs->items[i]);
      fputc('\n', f);
      json_write_indent(f, indent + 2);
      fputs("}", f);
      if (i + 1 < srcs->count) fputc(',', f);
      fputc('\n', f);
    }
  }

  json_write_indent(f, indent);
  fputc(']', f);
}

static void write_sbom_cyclonedx(FILE *f, const SbomData *d) {
  fputs("{\n", f);
  json_write_indent(f, 2);
  fputs("\"bomFormat\": ", f);
  json_write_string(f, "CycloneDX");
  fputs(",\n", f);
  json_write_indent(f, 2);
  fputs("\"specVersion\": ", f);
  json_write_string(f, "1.4");
  fputs(",\n", f);
  json_write_indent(f, 2);
  fputs("\"version\": 1,\n", f);

  json_write_indent(f, 2);
  fputs("\"metadata\": {\n", f);
  json_write_indent(f, 4);
  fputs("\"tools\": [\n", f);
  json_write_indent(f, 6);
  fputs("{\n", f);
  json_write_indent(f, 8);
  fputs("\"vendor\": ", f);
  json_write_string(f, "tack");
  fputs(",\n", f);
  json_write_indent(f, 8);
  fputs("\"name\": ", f);
  json_write_string(f, "tack");
  fputs(",\n", f);
  json_write_indent(f, 8);
  fputs("\"version\": ", f);
  json_write_string(f, TACK_VERSION);
  fputc('\n', f);
  json_write_indent(f, 6);
  fputs("}\n", f);
  json_write_indent(f, 4);
  fputs("],\n", f);
  json_write_indent(f, 4);
  fputs("\"component\": {\n", f);
  json_write_indent(f, 6);
  fputs("\"type\": ", f);
  json_write_string(f, "application");
  fputs(",\n", f);
  json_write_indent(f, 6);
  fputs("\"name\": ", f);
  json_write_string(f, d->t->name);
  fputc('\n', f);
  json_write_indent(f, 4);
  fputs("}\n", f);
  json_write_indent(f, 2);
  fputs("},\n", f);

  json_write_indent(f, 2);
  fputs("\"components\": ", f);
  write_cdx_components(f, 2, d->core_srcs, d->srcs);
  fputc('\n', f);

  fputs("}\n", f);
}

static void write_sbom_spdx(FILE *f, const SbomData *d) {
  char namespace_buf[512];
  char creator_buf[128];

  tack_copy(namespace_buf, sizeof(namespace_buf), "https://tack.invalid/spdx/");
  tack_cat(namespace_buf, sizeof(namespace_buf), d->t->name);
  tack_copy(creator_buf, sizeof(creator_buf), "Tool: tack ");
  tack_cat(creator_buf, sizeof(creator_buf), TACK_VERSION);

  fputs("{\n", f);
  json_write_indent(f, 2);
  fputs("\"spdxVersion\": ", f);
  json_write_string(f, "SPDX-2.3");
  fputs(",\n", f);
  json_write_indent(f, 2);
  fputs("\"dataLicense\": ", f);
  json_write_string(f, "CC0-1.0");
  fputs(",\n", f);
  json_write_indent(f, 2);
  fputs("\"SPDXID\": ", f);
  json_write_string(f, "SPDXRef-DOCUMENT");
  fputs(",\n", f);
  json_write_indent(f, 2);
  fputs("\"name\": ", f);
  json_write_string(f, "tack sbom");
  fputs(",\n", f);
  json_write_indent(f, 2);
  fputs("\"documentNamespace\": ", f);
  json_write_string(f, namespace_buf);
  fputs(",\n", f);

  json_write_indent(f, 2);
  fputs("\"creationInfo\": {\n", f);
  json_write_indent(f, 4);
  fputs("\"created\": ", f);
  json_write_string(f, "1970-01-01T00:00:00Z");
  fputs(",\n", f);
  json_write_indent(f, 4);
  fputs("\"creators\": [\n", f);
  json_write_indent(f, 6);
  json_write_string(f, creator_buf);
  fputc('\n', f);
  json_write_indent(f, 4);
  fputs("]\n", f);
  json_write_indent(f, 2);
  fputs("},\n", f);

  json_write_indent(f, 2);
  fputs("\"packages\": [\n", f);
  json_write_indent(f, 4);
  fputs("{\n", f);
  json_write_indent(f, 6);
  fputs("\"name\": ", f);
  json_write_string(f, d->t->name);
  fputs(",\n", f);
  json_write_indent(f, 6);
  fputs("\"SPDXID\": ", f);
  json_write_string(f, "SPDXRef-Package");
  fputs(",\n", f);
  json_write_indent(f, 6);
  fputs("\"downloadLocation\": ", f);
  json_write_string(f, "NOASSERTION");
  fputs(",\n", f);
  json_write_indent(f, 6);
  fputs("\"filesAnalyzed\": ", f);
  json_write_bool(f, 0);
  fputc('\n', f);
  json_write_indent(f, 4);
  fputs("}\n", f);
  json_write_indent(f, 2);
  fputs("],\n", f);

  json_write_indent(f, 2);
  fputs("\"relationships\": [\n", f);
  json_write_indent(f, 4);
  fputs("{\n", f);
  json_write_indent(f, 6);
  fputs("\"spdxElementId\": ", f);
  json_write_string(f, "SPDXRef-DOCUMENT");
  fputs(",\n", f);
  json_write_indent(f, 6);
  fputs("\"relationshipType\": ", f);
  json_write_string(f, "DESCRIBES");
  fputs(",\n", f);
  json_write_indent(f, 6);
  fputs("\"relatedSpdxElement\": ", f);
  json_write_string(f, "SPDXRef-Package");
  fputc('\n', f);
  json_write_indent(f, 4);
  fputs("}\n", f);
  json_write_indent(f, 2);
  fputs("]\n", f);

  fputs("}\n", f);
}

static int cmd_sbom(Profile p, TargetVec *tv, const Target *t, int verbose, int strict, int no_core,
                    const char *outdir) {
  FILE *f;
  char sbom_path[512];
  const TargetOverride *ov;
  int use_core_effective;
  const char *inc_common[5];
  const char *tackfile_mode = "none";
  SbomFormat format;
  const char *format_cfg;
  SbomData data;

  StrVec includes;
  StrVec defines;
  StrVec cflags;
  StrVec ldflags;
  StrVec libs;
  StrVec srcs;
  StrVec core_srcs;

  (void)tv;

  if (!outdir) outdir = g_build_dir;

  ensure_dir(outdir);
  format_cfg = g_config_sbom_format ? g_config_sbom_format : "tack-sbom-1";
  if (!sbom_format_from_string(format_cfg, &format)) {
    fprintf(stderr, "tack: sbom: unknown format '%s'\n", format_cfg);
    return 1;
  }
  path_join(sbom_path, sizeof(sbom_path), outdir, sbom_format_filename(format));

  if (verbose) printf("tack: sbom: writing %s (%s)\n", sbom_path, sbom_format_name(format));

  f = fopen(sbom_path, "wb");
  if (!f) {
    fprintf(stderr, "tack: sbom: cannot write %s\n", sbom_path);
    return 1;
  }

  ov = find_override(t->name);
  use_core_effective = (ov && ov->use_core) ? 1 : 0;
  if (no_core) use_core_effective = 0;

  inc_common[0] = g_inc_dir;
  inc_common[1] = t->src_dir;
  inc_common[2] = g_src_dir;
  if (file_exists(g_core_dir) && is_dir_path(g_core_dir)) inc_common[3] = g_core_dir;
  else inc_common[3] = 0;
  inc_common[4] = 0;

  sv_init(&includes);
  sv_init(&defines);
  sv_init(&cflags);
  sv_init(&ldflags);
  sv_init(&libs);
  sv_init(&srcs);
  sv_init(&core_srcs);

  sv_append_list(&includes, inc_common);
  if (ov && ov->includes) sv_append_list(&includes, (const char * const *)ov->includes);
  if (ov && ov->defines) sv_append_list(&defines, (const char * const *)ov->defines);
  if (ov && ov->cflags) sv_append_list(&cflags, (const char * const *)ov->cflags);
  if (ov && ov->ldflags) sv_append_list(&ldflags, (const char * const *)ov->ldflags);
  if (ov && ov->libs) sv_append_list(&libs, (const char * const *)ov->libs);

  if (gather_target_sources(&srcs, t) != 0 || srcs.count == 0) {
    fprintf(stderr, "tack: sbom: cannot gather sources for %s\n", t->name);
    sv_free(&includes);
    sv_free(&defines);
    sv_free(&cflags);
    sv_free(&ldflags);
    sv_free(&libs);
    sv_free(&srcs);
    sv_free(&core_srcs);
    fclose(f);
    return 1;
  }

  if (use_core_effective) {
    if (file_exists(g_core_dir) && is_dir_path(g_core_dir)) {
      scan_dir_recursive_suffix(&core_srcs, g_core_dir, ".c");
    }
  }

  sv_sort_unique(&includes);
  sv_sort_unique(&defines);
  sv_sort_unique(&cflags);
  sv_sort_unique(&ldflags);
  sv_sort_unique(&libs);
  sv_sort_unique(&srcs);
  sv_sort_unique(&core_srcs);

  if (g_no_config) {
    tackfile_mode = "disabled";
  } else {
#ifdef TACK_USE_TACKFILE
    tackfile_mode = "compile-time";
#else
    if (file_exists("tackfile.c")) tackfile_mode = "runtime";
    else tackfile_mode = "none";
#endif
  }

  memset(&data, 0, sizeof(data));
  data.t = t;
  data.profile = profile_name(p);
  data.compiler = get_cc();
  data.tackfile_mode = tackfile_mode;
  data.strict = strict;
  data.no_core = no_core;
  data.config_enabled = g_no_config ? 0 : 1;
  data.config_path = g_config_loaded ? g_config_path : 0;
  data.use_core_effective = use_core_effective;
  data.includes = &includes;
  data.defines = &defines;
  data.cflags = &cflags;
  data.ldflags = &ldflags;
  data.libs = &libs;
  data.srcs = &srcs;
  data.core_srcs = &core_srcs;

  if (format == SBOM_FORMAT_TACK) write_sbom_tack(f, &data);
  else if (format == SBOM_FORMAT_CYCLONEDX) write_sbom_cyclonedx(f, &data);
  else write_sbom_spdx(f, &data);
  fclose(f);

  sv_free(&includes);
  sv_free(&defines);
  sv_free(&cflags);
  sv_free(&ldflags);
  sv_free(&libs);
  sv_free(&srcs);
  sv_free(&core_srcs);

  printf("tack: sbom: wrote %s (%s)\n", sbom_path, sbom_format_name(format));
  return 0;
}

static int cmd_doc(TargetVec *tv, const Target *t, int verbose, int strict, int no_core,
                   const char *outdir, Profile p) {
  char docdir[512];
  char idx[512];
  int rc;

  if (!outdir) outdir = "build/doc";

  /* ensure build/ and doc/ */
  ensure_dir("build");
  ensure_dir("build/doc");
  if (outdir && !streq(outdir, "build/doc")) ensure_dir(outdir);

  tack_copy(docdir, sizeof(docdir), outdir);

  /* generate BOM first (default: alongside doc output) */
  {
    char bomdir[512];
    const char *d = docdir;
    size_t dl = strlen(d);

    /* fallback */
    tack_copy(bomdir, sizeof(bomdir), "build");

    /* trim trailing slashes */
    while (dl > 0 && (d[dl - 1] == '/' || d[dl - 1] == '\\')) dl--;

    /* if docdir ends with ".../doc", place BOM in parent directory; else use docdir */
    if (dl >= 3 &&
        (tolower((unsigned char)d[dl - 3]) == 'd') &&
        (tolower((unsigned char)d[dl - 2]) == 'o') &&
        (tolower((unsigned char)d[dl - 1]) == 'c')) {
      size_t cut = dl - 3;
      while (cut > 0 && (d[cut - 1] == '/' || d[cut - 1] == '\\')) cut--;
      if (cut == 0) {
        tack_copy(bomdir, sizeof(bomdir), ".");
      } else {
        if (cut >= sizeof(bomdir)) tack_die("path too long");
        memcpy(bomdir, d, cut);
        bomdir[cut] = '\0';
      }
    } else {
      if (dl >= sizeof(bomdir)) tack_die("path too long");
      memcpy(bomdir, d, dl);
      bomdir[dl] = '\0';
    }

    rc = cmd_bom(p, tv, t, verbose, strict, no_core, bomdir);
  }
  if (rc != 0) return rc;

  /* write pages */
  {
    const char *nav_inner =
      "<a href=\"index.html\">Index</a> | "
      "<a href=\"readme.html\">README</a> | "
      "<a href=\"faq.html\">FAQ</a> | "
      "<a href=\"roadmap.html\">Roadmap</a> | "
      "<a href=\"releasenotes.html\">Release Notes</a> | "
      "<a href=\"../bom.html\">BOM</a>";
    HtmlCfg hc;
    char css_href[256];
    char css_dst[512];
    const char *css_path;
    char out[512];

    memset(&hc, 0, sizeof(hc));
    hc.kind = "doc";
    hc.project_title = "tack";

    css_href[0] = '\0';
    css_path = choose_css("doc");
    if (css_path && css_path[0] && !file_exists(css_path)) {
      fprintf(stderr, "tack: doc: css not found: %s\n", css_path);
      return 2;
    }
    if (css_path && css_path[0] && file_exists(css_path)) {
      const char *base = path_base(css_path);

      if (strlen(base) >= sizeof(css_href)) tack_die("css filename too long");
      tack_copy(css_href, sizeof(css_href), base);
      path_join(css_dst, sizeof(css_dst), docdir, css_href);

      if (copy_file(css_path, css_dst) != 0) {
        fprintf(stderr, "tack: doc: cannot copy css %s -> %s\n", css_path, css_dst);
        return 1;
      }

      hc.css_href = css_href;
    }

    if (file_exists("README.md")) {
      path_join(out, sizeof(out), docdir, "readme.html");
      if (verbose) printf("tack: doc: %s\n", out);
      { int rc2 = write_doc_page(out, "tack README", nav_inner, &hc, "README.md"); if (rc2 != 0) return rc2; }
    }
    if (file_exists("FAQ.md")) {
      path_join(out, sizeof(out), docdir, "faq.html");
      if (verbose) printf("tack: doc: %s\n", out);
      { int rc2 = write_doc_page(out, "tack FAQ", nav_inner, &hc, "FAQ.md"); if (rc2 != 0) return rc2; }
    }
    if (file_exists("ROADMAP.md")) {
      path_join(out, sizeof(out), docdir, "roadmap.html");
      if (verbose) printf("tack: doc: %s\n", out);
      { int rc2 = write_doc_page(out, "tack Roadmap", nav_inner, &hc, "ROADMAP.md"); if (rc2 != 0) return rc2; }
    }
    if (file_exists("RELEASENOTES.md")) {
      path_join(out, sizeof(out), docdir, "releasenotes.html");
      if (verbose) printf("tack: doc: %s\n", out);
      { int rc2 = write_doc_page(out, "tack Release Notes", nav_inner, &hc, "RELEASENOTES.md"); if (rc2 != 0) return rc2; }
    }

    /* index */
    path_join(idx, sizeof(idx), docdir, "index.html");
    if (verbose) printf("tack: doc: %s\n", idx);
    {
      HtmlPage pg;
      DocIndexCtx dctx;
      char *head_assets = 0;
      char *nav = 0;
      char *toc = 0;

      dctx.has_readme = file_exists("README.md");
      dctx.has_faq = file_exists("FAQ.md");
      dctx.has_roadmap = file_exists("ROADMAP.md");
      dctx.has_releasenotes = file_exists("RELEASENOTES.md");

      head_assets = make_head_assets(hc.css_href);
      nav = make_nav_block(nav_inner);
      toc = make_empty_toc_block();

      memset(&pg, 0, sizeof(pg));
      pg.page_title = "tack docs";
      pg.project_title = "tack";
      pg.head_assets_html = head_assets;
      pg.nav_html = nav;
      pg.toc_html = toc;
      pg.emit_content = emit_doc_index;
      pg.content_ctx = &dctx;
      pg.emit_footer = emit_footer_default;
      pg.footer_ctx = 0;

      {
        int rc2 = write_html_page(idx, "doc", &pg);
        if (rc2 != 0) { free(head_assets); free(nav); free(toc); return rc2; }
      }

      free(head_assets);
      free(nav);
      free(toc);
    }
  }

  printf("tack: doc: wrote %s\n", docdir);
  return 0;
}

/* --------------------------- args --------------------------- */

static Profile parse_profile(int *argi, int argc, char **argv) {
  if (*argi < argc) {
    if (streq(argv[*argi], "release")) { (*argi)++; return PROF_RELEASE; }
    if (streq(argv[*argi], "debug"))   { (*argi)++; return PROF_DEBUG; }
  }
  return PROF_DEBUG;
}

static int parse_int(const char *s) {
  int v = 0;
  if (!s || !*s) return -1;
  while (*s) {
    int digit;
    if (!isdigit((unsigned char)*s)) return -1;
    digit = (*s - '0');
    if (v > (INT_MAX - digit) / 10) return -1;
    v = v * 10 + digit;
    s++;
  }
  return v;
}

int main(int argc, char **argv) {
  TargetVec tv;
  const char *cmd;
  int argi;
  int disable_auto_tools;

  /* parse global options (must precede command) */
  argi = 1;
  while (argi < argc) {
    if (streq(argv[argi], "-h") || streq(argv[argi], "--help")) { print_help(); return 0; }
    if (streq(argv[argi], "--no-config")) { g_no_config = 1; argi++; continue; }
    if (streq(argv[argi], "--no-code-config")) { g_no_code_config = 1; argi++; continue; }
    if (streq(argv[argi], "--config")) {
      if (argi + 1 >= argc) { fprintf(stderr, "tack: --config needs PATH\n"); return 2; }
      g_config_path_cli = argv[argi + 1];
      tack_check_len("--config path", g_config_path_cli, TACK_MAX_CONFIG_PATH);
      argi += 2;
      continue;
    }
    if (streq(argv[argi], "--no-auto-tools")) { g_no_auto_tools_cli = 1; argi++; continue; }
    if (streq(argv[argi], "--no-cache")) { g_no_cache = 1; argi++; continue; }
    break;
  }

  /* load config (tack.ini) unless disabled */
  if (config_auto_load() != 0) {
    fprintf(stderr, "tack: config: failed to load\n");
    config_free();
    return 2;
  }

  disable_auto_tools = 0;
#ifdef TACKFILE_DISABLE_AUTO_TOOLS
  disable_auto_tools = 1;
#else
  if (g_no_auto_tools_cli) disable_auto_tools = 1;
  else if (g_config_loaded && g_config_disable_auto_tools) disable_auto_tools = 1;
#endif

  tv_init(&tv);
  discover_targets(&tv, disable_auto_tools);

  /* tackfile.c may add/modify/remove/disable targets (compile-time) */
  apply_tackfile_targets(&tv);

  /* tack.ini may add/modify/remove/disable targets (runtime) */
  apply_ini_targets(&tv);

  /* no command -> default build debug default target */
  if (argi >= argc) {
    const Target *t = find_target(&tv, default_target_name());
    int rc;
    if (!t) { fprintf(stderr, "tack: default target missing\n"); tv_free(&tv); config_free(); return 2; }
    rc = build_one_target(t, PROF_DEBUG, 0, 0, 0, 1, 0, 0);
    tv_free(&tv);
    config_free();
    return rc;
  }

  cmd = argv[argi++];

  if (streq(cmd, "help"))    { print_help(); tv_free(&tv); config_free(); return 0; }
  if (streq(cmd, "version")) { cmd_version(); tv_free(&tv); config_free(); return 0; }
  if (streq(cmd, "doctor"))  { cmd_doctor(); tv_free(&tv); config_free(); return 0; }
  if (streq(cmd, "init"))    { int rc = cmd_init(); tv_free(&tv); config_free(); return rc; }
  if (streq(cmd, "clean") || streq(cmd, "clobber")) {
    int verbose = 0;
    for (; argi < argc; argi++) {
      if (streq(argv[argi], "-h") || streq(argv[argi], "--help")) { print_help(); tv_free(&tv); config_free(); return 0; }
      if (streq(argv[argi], "-v") || streq(argv[argi], "--verbose")) verbose = 1;
      else {
        fprintf(stderr, "tack: %s: unknown arg: %s\n", cmd, argv[argi]);
        tv_free(&tv);
        config_free();
        return 2;
      }
    }
    if (streq(cmd, "clean"))   { int rc = cmd_clean(verbose); tv_free(&tv); config_free(); return rc; }
    else                       { int rc = cmd_clobber(verbose); tv_free(&tv); config_free(); return rc; }
  }
  if (streq(cmd, "list"))    {
    if (g_no_config) printf("config: disabled (legacy mode)\n");
    else if (g_config_loaded) printf("config: %s\n", g_config_path);
    else printf("config: none\n");
    { int rc = cmd_list_targets(&tv); tv_free(&tv); config_free(); return rc; }
  }


  if (streq(cmd, "bom") || streq(cmd, "sbom") || streq(cmd, "doc")) {
    int verbose = 0;
    int strict = 0;
    int no_core = 0;
    const char *outdir = 0;

    int i = argi;
    Profile p = parse_profile(&i, argc, argv);

    const char *target_name = default_target_name();
    const Target *t = 0;

    for (; i < argc; i++) {
      if (streq(argv[i], "-h") || streq(argv[i], "--help")) { print_help(); tv_free(&tv); config_free(); return 0; }
      if (streq(argv[i], "-v") || streq(argv[i], "--verbose")) verbose = 1;
      else if (streq(argv[i], "--strict")) strict = 1;
      else if (streq(argv[i], "--no-core")) no_core = 1;
      else if (streq(argv[i], "--target")) {
        if (i + 1 >= argc) { fprintf(stderr, "tack: --target needs NAME\n"); tv_free(&tv); config_free(); return 2; }
        target_name = argv[++i];
      } else if (streq(argv[i], "--outdir")) {
        if (i + 1 >= argc) { fprintf(stderr, "tack: --outdir needs DIR\n"); tv_free(&tv); config_free(); return 2; }
        outdir = argv[++i];
      } else {
        fprintf(stderr, "tack: %s: unknown arg: %s\n", cmd, argv[i]);
        tv_free(&tv);
        config_free();
        return 2;
      }
    }

    t = find_target(&tv, target_name);
    if (!t) {
      fprintf(stderr, "tack: unknown target: %s\n", target_name);
      fprintf(stderr, "tack: hint: use 'tack list'\n");
      tv_free(&tv);
      config_free();
      return 2;
    }

    if (streq(cmd, "bom")) {
      int rc = cmd_bom(p, &tv, t, verbose, strict, no_core, outdir);
      tv_free(&tv);
      config_free();
      return rc;
    } else if (streq(cmd, "sbom")) {
      int rc = cmd_sbom(p, &tv, t, verbose, strict, no_core, outdir);
      tv_free(&tv);
      config_free();
      return rc;
    } else {
      int rc = cmd_doc(&tv, t, verbose, strict, no_core, outdir, p);
      tv_free(&tv);
      config_free();
      return rc;
    }
  }

  if (streq(cmd, "build") || streq(cmd, "run") || streq(cmd, "test")) {
    int verbose = 0;
    int why = 0;
    int force = 0;
    int jobs = 1;
    int strict = 0;
    int no_core = 0;

    Profile p = parse_profile(&argi, argc, argv);

    const char *target_name = default_target_name();
    const Target *t = 0;

    /* parse options; for run, args may follow '--' */
    for (; argi < argc; argi++) {
      if (streq(argv[argi], "--")) break;
      if (streq(argv[argi], "-h") || streq(argv[argi], "--help")) { print_help(); tv_free(&tv); config_free(); return 0; }
      if (streq(argv[argi], "-v") || streq(argv[argi], "--verbose")) verbose = 1;
      else if (streq(argv[argi], "--why") || streq(argv[argi], "--explain")) why = 1;
      else if (streq(argv[argi], "--rebuild")) force = 1;
      else if (streq(argv[argi], "--strict")) strict = 1;
      else if (streq(argv[argi], "--no-core")) no_core = 1;
      else if (streq(argv[argi], "--target")) {
        if (argi + 1 >= argc) { fprintf(stderr, "tack: --target needs NAME\n"); tv_free(&tv); config_free(); return 2; }
        target_name = argv[++argi];
      } else if (streq(argv[argi], "-j") || streq(argv[argi], "--jobs")) {
        int v;
        if (argi + 1 >= argc) { fprintf(stderr, "tack: -j needs N\n"); tv_free(&tv); config_free(); return 2; }
        v = parse_int(argv[++argi]);
        if (v < 1) { fprintf(stderr, "tack: invalid -j %s\n", argv[argi]); tv_free(&tv); config_free(); return 2; }
        jobs = v;
      } else {
        /* run: allow args without -- (best effort) */
        if (streq(cmd, "run")) break;
        fprintf(stderr, "tack: %s: unknown arg: %s\n", cmd, argv[argi]);
        tv_free(&tv);
        config_free();
        return 2;
      }
    }

    if (streq(cmd, "test")) {
      int rc = build_and_run_tests(p, verbose, force, strict);
      tv_free(&tv);
      config_free();
      return rc;
    }

    t = find_target(&tv, target_name);
    if (!t) {
      fprintf(stderr, "tack: unknown or disabled target: %s\n", target_name);
      fprintf(stderr, "tack: hint: use 'tack list'\n");
      tv_free(&tv);
      config_free();
      return 2;
    }

    if (streq(cmd, "build")) {
      int rc = build_one_target(t, p, verbose, why, force, jobs, strict, no_core);
      tv_free(&tv);
      config_free();
      return rc;
    }

    /* run */
    {
      int run_argi = argi;
      char exe[512];

      if (run_argi < argc && streq(argv[run_argi], "--")) run_argi++;

      if (build_one_target(t, p, verbose, why, force, jobs, strict, no_core) != 0) { tv_free(&tv); config_free(); return 1; }
      exe_path(exe, sizeof(exe), t->id, p, t->bin_base);

      /* build argv: exe + rest args */
      {
        Argv av;
        int k;
        int rc;

        av_init(&av);
        av_push(&av, exe);
        for (k = run_argi; k < argc; k++) av_push(&av, argv[k]);
        av_terminate(&av);

        rc = run_argv_wait(av.a, verbose);
        av_free(&av);
        tv_free(&tv);
        config_free();
        return rc;
      }
    }
  }

  fprintf(stderr, "tack: unknown command: %s\n\n", cmd);
  print_help();
  tv_free(&tv);
  config_free();
  return 2;
}
