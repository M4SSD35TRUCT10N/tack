
/* tack.c - Tiny ANSI-C Kit
 * v0.6.0
 *
 * Adds:
 * - Real target configuration overrides (includes/defines/libs per target)
 * - Shared "core" code (src/core/ (recursive .c files)) built once per profile and linked into targets
 * - Keeps: recursive scanning, tool discovery, -j parallel compile, robust process execution
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
 *   tcc -run tack.c init
 *   tcc -run tack.c list
 *   tcc -run tack.c build debug -v -j 8
 *   tcc -run tack.c run debug -- --hello "world"
 *   tcc -run tack.c build release --target tool:foo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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

#define TACK_VERSION "0.6.0"


/* --------------------------- runtime config (globals) --------------------------- */
/* Optional project configuration (data-only, no code execution):
 * - auto-loads tack.ini if present
 * - CLI may override with --config / disable with --no-config
 *
 * Global options (must appear before the command):
 *   --no-config        ignore all config files (legacy mode)
 *   --config <path>    use explicit INI file
 *   --no-auto-tools    disable tool discovery at runtime
 */
static int g_no_config = 0;
static const char *g_config_path_cli = 0;
static int g_no_auto_tools_cli = 0;

static int g_config_loaded = 0;
static char g_config_path[512] = {0};
static char *g_config_default_target = 0; /* owned; freed at exit */
static int g_config_disable_auto_tools = 0;


static const char *g_cc_default = "tcc";
static const char *g_build_dir  = "build";

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

static const char *env_or_default(const char *key, const char *defv) {
  const char *v = getenv(key);
  if (v && v[0]) return v;
  return defv;
}

static int file_exists(const char *path) {
  STAT_ST st;
  return STAT_FN(path, &st) == 0;
}

static long file_mtime(const char *path) {
  STAT_ST st;
  if (STAT_FN(path, &st) != 0) return -1;
  return (long)st.st_mtime;
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

static void ensure_dir(const char *path) {
#ifdef _WIN32
  _mkdir(path);
#else
  mkdir(path, 0777);
#endif
}

static void path_join(char *out, const char *a, const char *b) {
  size_t la = strlen(a);
  strcpy(out, a);
  if (la > 0 && out[la - 1] != PATH_SEP) {
    out[la] = PATH_SEP;
    out[la + 1] = '\0';
  }
  strcat(out, b);
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

static void scan_dir_recursive_suffix_skip(StrVec *out, const char *dir, const char *suffix,
                                           const char *skip_dirname) {
#ifdef _WIN32
  char pattern[1024];
  WIN32_FIND_DATAA fd;
  HANDLE h;

  strcpy(pattern, dir);
  {
    size_t ld = strlen(pattern);
    if (ld > 0 && pattern[ld - 1] != '\\' && pattern[ld - 1] != '/') {
      pattern[ld] = '\\';
      pattern[ld + 1] = '\0';
    }
  }
  strcat(pattern, "*");

  h = FindFirstFileA(pattern, &fd);
  if (h == INVALID_HANDLE_VALUE) return;

  do {
    char full[1024];
    if (streq(fd.cFileName, ".") || streq(fd.cFileName, "..")) continue;

    if (skip_dirname && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      if (streq(fd.cFileName, skip_dirname)) continue;
      if (streq(fd.cFileName, "build")) continue;
    }

    path_join(full, dir, fd.cFileName);

    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      scan_dir_recursive_suffix_skip(out, full, suffix, skip_dirname);
    } else {
      if (ends_with(fd.cFileName, suffix)) sv_push(out, full);
    }
  } while (FindNextFileA(h, &fd));

  FindClose(h);
#else
  DIR *d;
  struct dirent *e;

  d = opendir(dir);
  if (!d) return;

  while ((e = readdir(d)) != 0) {
    char full[1024];
    if (streq(e->d_name, ".") || streq(e->d_name, "..")) continue;

    if (skip_dirname && streq(e->d_name, skip_dirname)) continue;
    if (streq(e->d_name, "build")) continue;

    path_join(full, dir, e->d_name);

    if (is_dir_path(full)) {
      scan_dir_recursive_suffix_skip(out, full, suffix, skip_dirname);
    } else {
      if (ends_with(e->d_name, suffix)) sv_push(out, full);
    }
  }
  closedir(d);
#endif
}

static void scan_dir_recursive_suffix(StrVec *out, const char *dir, const char *suffix) {
  scan_dir_recursive_suffix_skip(out, dir, suffix, 0);
}

/* --------------------------- rm -rf --------------------------- */

static int rm_rf(const char *path);
static int rm_rf_contents(const char *dir);

static int rm_rf(const char *path) {
  if (!file_exists(path)) return 0;

  if (!is_dir_path(path)) {
#ifdef _WIN32
    return DeleteFileA(path) ? 0 : 1;
#else
    return unlink(path) == 0 ? 0 : 1;
#endif
  }

#ifdef _WIN32
  {
    char pattern[1024];
    WIN32_FIND_DATAA fd;
    HANDLE h;

    strcpy(pattern, path);
    {
      size_t lp = strlen(pattern);
      if (lp > 0 && pattern[lp - 1] != '\\' && pattern[lp - 1] != '/') {
        pattern[lp] = '\\';
        pattern[lp + 1] = '\0';
      }
    }
    strcat(pattern, "*");

    h = FindFirstFileA(pattern, &fd);
    if (h != INVALID_HANDLE_VALUE) {
      do {
        char child[1024];
        if (streq(fd.cFileName, ".") || streq(fd.cFileName, "..")) continue;
        path_join(child, path, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          if (rm_rf(child) != 0) { FindClose(h); return 1; }
        } else {
          if (!DeleteFileA(child)) { FindClose(h); return 1; }
        }
      } while (FindNextFileA(h, &fd));
      FindClose(h);
    }
    return RemoveDirectoryA(path) ? 0 : 1;
  }
#else
  {
    DIR *d;
    struct dirent *e;

    d = opendir(path);
    if (!d) return 1;

    while ((e = readdir(d)) != 0) {
      char child[1024];
      if (streq(e->d_name, ".") || streq(e->d_name, "..")) continue;
      path_join(child, path, e->d_name);
      if (rm_rf(child) != 0) { closedir(d); return 1; }
    }
    closedir(d);
    return rmdir(path) == 0 ? 0 : 1;
  }
#endif
}

static int rm_rf_contents(const char *dir) {
  if (!file_exists(dir)) return 0;
  if (!is_dir_path(dir)) return 1;

#ifdef _WIN32
  {
    char pattern[1024];
    WIN32_FIND_DATAA fd;
    HANDLE h;

    strcpy(pattern, dir);
    {
      size_t lp = strlen(pattern);
      if (lp > 0 && pattern[lp - 1] != '\\' && pattern[lp - 1] != '/') {
        pattern[lp] = '\\';
        pattern[lp + 1] = '\0';
      }
    }
    strcat(pattern, "*");

    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;

    do {
      char child[1024];
      if (streq(fd.cFileName, ".") || streq(fd.cFileName, "..")) continue;
      path_join(child, dir, fd.cFileName);

      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (rm_rf(child) != 0) { FindClose(h); return 1; }
      } else {
        if (!DeleteFileA(child)) { FindClose(h); return 1; }
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
    if (!d) return 1;

    while ((e = readdir(d)) != 0) {
      char child[1024];
      if (streq(e->d_name, ".") || streq(e->d_name, "..")) continue;
      path_join(child, dir, e->d_name);
      if (rm_rf(child) != 0) { closedir(d); return 1; }
    }
    closedir(d);
    return 0;
  }
#endif
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
  if (proc_spawn_nowait(argv, &p) != 0) return 1;
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

/* --------------------------- dep parsing --------------------------- */

static int depfile_needs_rebuild(const char *obj_path, const char *dep_path) {
#if USE_DEPFILES
  FILE *f;
  long obj_t;
  int c;
  char tok[2048];
  int ti;
  int seen_colon;

  obj_t = file_mtime(obj_path);
  if (obj_t < 0) return 1;

  f = fopen(dep_path, "rb");
  if (!f) return 1;

  ti = 0;
  seen_colon = 0;

  while ((c = fgetc(f)) != EOF) {
    if (c == '\\') {
      int n = fgetc(f);
      if (n == '\n' || n == '\r') continue;
      if (n == EOF) break;
      /* escaped char (including space) becomes part of token */
      if (ti < (int)sizeof(tok) - 1) tok[ti++] = (char)n;
      continue;
    }

    if (c == ':' && !seen_colon) {
      tok[ti] = '\0';
      ti = 0;
      seen_colon = 1;
      continue;
    }

    if (isspace((unsigned char)c)) {
      if (ti > 0) {
        tok[ti] = '\0';
        ti = 0;
        if (seen_colon) {
          long dt = file_mtime(tok);
          if (dt < 0 || dt > obj_t) { fclose(f); return 1; }
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
      if (dt < 0 || dt > obj_t) { fclose(f); return 1; }
    }
  }

  fclose(f);
  return 0;
#else
  (void)obj_path; (void)dep_path;
  return 1;
#endif
}

static int obj_needs_rebuild(const char *obj_path, const char *src_path, const char *dep_path, int force) {
  long obj_t, src_t;
  if (force) return 1;
  obj_t = file_mtime(obj_path);
  if (obj_t < 0) return 1;
  src_t = file_mtime(src_path);
  if (src_t < 0) return 1;
  if (src_t > obj_t) return 1;
#if USE_DEPFILES
  if (depfile_needs_rebuild(obj_path, dep_path)) return 1;
#else
  (void)dep_path;
#endif
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
 * If you want to keep project-specific settings out of tack.c, create a file
 * named "tackfile.c" next to this file and compile tack with:
 *   tcc -DTACK_USE_TACKFILE -run tack.c build debug
 * or (for a standalone tack.exe):
 *   tcc -DTACK_USE_TACKFILE tack.c -o tack.exe
 *
* In tackfile.c you may define:
 *
 *   1) Overrides (includes/defines/libs per target):
 *   #define TACKFILE_OVERRIDES my_overrides
 *   static const TargetOverride my_overrides[] = {
 *     { "app", 0, 0, 0, 0, 0, 1 },
 *     { "tool:foo", 0, (const char*[]){"TOOL_FOO=1",0}, 0, 0, 0, 1 },
 *     { 0,0,0,0,0,0,0 }
 *   };
 *
 *   2) Declarative target graph (add/modify/remove/disable):
 *      #define TACKFILE_TARGETS my_targets
 *      static const TargetDef my_targets[] = {
 *        { "app", "src/app", "app", 0, 1, 0 },            // upsert (enabled by default)
 *        { "demo:hello", "demos/hello", "hello", "demo_hello", 1, 0 },
 *        { "tool:gen", "extras/gen", "gen", 0, 1, 0 },
 *        { "tool:old", 0, 0, 0, 0, 0 },                   // action: disable existing
 *        { "tool:tmp", 0, 0, 0, 0, 1 },                   // action: remove existing
 *        { 0,0,0,0,0,0 }
 *      };
 *
 *   3) Default target:
 *      #define TACKFILE_DEFAULT_TARGET "app"
 *
 *   4) Disable auto tool discovery (for fully declarative builds):
 *      #define TACKFILE_DISABLE_AUTO_TOOLS 1
 *
 * tack will search TACKFILE_OVERRIDES first, then its built-in g_overrides[].
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

static void split_list_semicolon(StrVec *out, const char *v) {
  const char *p = v;
  while (p && *p) {
    const char *q;
    char tmp[1024];
    size_t n;

    while (*p && (*p == ';' || isspace((unsigned char)*p))) p++;
    if (!*p) break;

    q = p;
    while (*q && *q != ';') q++;

    n = (size_t)(q - p);
    if (n >= sizeof(tmp)) n = sizeof(tmp) - 1;
    memcpy(tmp, p, n);
    tmp[n] = '\0';
    {
      char *t = trim(tmp);
      if (t[0]) sv_push(out, t);
    }

    p = q;
    if (*p == ';') p++;
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
  char line[2048];
  int lineno = 0;

  enum { SEC_NONE = 0, SEC_PROJECT = 1, SEC_TARGET = 2 } sec = SEC_NONE;
  IniTargetCfg *cur_t = 0;

  f = fopen(path, "rb");
  if (!f) return 1;

  ini_targets_init();
  ini_overrides_init();

  while (fgets(line, sizeof(line), f)) {
    char *s, *eq;
    lineno++;

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

      if ((tolower((unsigned char)s[0])=='t') && (tolower((unsigned char)s[1])=='a') && (tolower((unsigned char)s[2])=='r') && (tolower((unsigned char)s[3])=='g') && (tolower((unsigned char)s[4])=='e') && (tolower((unsigned char)s[5])=='t')) {
        char *p = s + 6;
        char namebuf[512];
        p = trim(p);
        if (*p == '"') {
          char *q = strchr(p + 1, '"');
          if (!q) continue;
          *q = '\0';
          strncpy(namebuf, p + 1, sizeof(namebuf) - 1);
          namebuf[sizeof(namebuf) - 1] = '\0';
        } else {
          strncpy(namebuf, p, sizeof(namebuf) - 1);
          namebuf[sizeof(namebuf) - 1] = '\0';
          rtrim_inplace(namebuf);
        }
        if (namebuf[0]) {
          sec = SEC_TARGET;
          cur_t = ini_get_or_add_target(namebuf);
        }
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
          g_config_default_target = xstrdup(val);
        } else if (strieq(key, "disable_auto_tools")) {
          int b;
          if (parse_bool(val, &b)) g_config_disable_auto_tools = b;
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
          sv_free(&cur_t->includes); sv_init(&cur_t->includes); split_list_semicolon(&cur_t->includes, val);
        } else if (strieq(key, "defines")) {
          sv_free(&cur_t->defines); sv_init(&cur_t->defines); split_list_semicolon(&cur_t->defines, val);
        } else if (strieq(key, "cflags")) {
          sv_free(&cur_t->cflags); sv_init(&cur_t->cflags); split_list_semicolon(&cur_t->cflags, val);
        } else if (strieq(key, "ldflags")) {
          sv_free(&cur_t->ldflags); sv_init(&cur_t->ldflags); split_list_semicolon(&cur_t->ldflags, val);
        } else if (strieq(key, "libs")) {
          sv_free(&cur_t->libs); sv_init(&cur_t->libs); split_list_semicolon(&cur_t->libs, val);
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
static char g_tackfile_generated_ini[512] = {0};


static void tackfile_gen_paths(char *dir, char *gen_c, char *gen_exe, char *gen_ini) {
  char tmp[512];

  path_join(dir, g_build_dir, "_tackfile");
  path_join(gen_c, dir, "tackfile_gen.c");
#ifdef _WIN32
  path_join(tmp, dir, "tackfile_gen.exe");
#else
  path_join(tmp, dir, "tackfile_gen");
#endif
  strcpy(gen_exe, tmp);
  path_join(gen_ini, dir, "tackfile.generated.ini");
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

  cc = env_or_default("TACK_CC", g_cc_default);
  tf_t = file_mtime("tackfile.c");

  ensure_dir(g_build_dir);
  path_join(dir, g_build_dir, "_tackfile");
  ensure_dir(dir);

  tackfile_gen_paths(dir, gen_c, gen_exe, gen_ini);

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
  strncpy(g_config_path, path, sizeof(g_config_path) - 1);
  g_config_path[sizeof(g_config_path) - 1] = '\0';
  g_config_loaded = 1;
  return 0;
}

static int config_auto_load(void) {
  if (g_no_config) return 0;

  config_reset();

#ifndef TACK_USE_TACKFILE
  /* low-priority layer: tackfile.c (compiled on the fly) */
  if (file_exists("tackfile.c")) {
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

    strcpy(pattern, g_tools_dir);
    {
      size_t ld = strlen(pattern);
      if (ld > 0 && pattern[ld - 1] != '\\' && pattern[ld - 1] != '/') {
        pattern[ld] = '\\';
        pattern[ld + 1] = '\0';
      }
    }
    strcat(pattern, "*");

    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return;

    do {
      if (streq(fd.cFileName, ".") || streq(fd.cFileName, "..")) continue;
      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        char name[256];
        char src[512];
        char tname[512];

        strcpy(name, fd.cFileName);
        path_join(src, g_tools_dir, name);

        strcpy(tname, "tool:");
        strcat(tname, name);

        tv_push(out, tname, src, name);
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
      char full[1024];
      if (streq(e->d_name, ".") || streq(e->d_name, "..")) continue;
      path_join(full, g_tools_dir, e->d_name);
      if (is_dir_path(full)) {
        char tname[512];
        strcpy(tname, "tool:");
        strcat(tname, e->d_name);
        tv_push(out, tname, full, e->d_name);
      }
    }
    closedir(d);
  }
#endif
  }
  }
}


/* --------------------------- build paths --------------------------- */

static void build_paths(char *root, char *objd, char *depd, char *bind,
                        const char *target_id, Profile p) {
  char tdir[512];
  char pdir[512];

  path_join(tdir, g_build_dir, target_id);
  path_join(pdir, tdir, profile_name(p));

  strcpy(root, pdir);
  path_join(objd, root, "obj");
  path_join(depd, root, "dep");
  path_join(bind, root, "bin");
}

static void exe_path(char *out, const char *target_id, Profile p, const char *bin_base) {
  char root[512], objd[512], depd[512], bind[512];
  char fn[256];

  build_paths(root, objd, depd, bind, target_id, p);

#ifdef _WIN32
  strcpy(fn, bin_base);
  strcat(fn, ".exe");
#else
  strcpy(fn, bin_base);
#endif
  path_join(out, bind, fn);
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
static int compile_sources(const char *cc, StrVec *srcs, const char *objd, const char *depd,
                           const char * const *inc_common,
                           const char * const *inc_extra,
                           const char * const *def_extra,
                           const char * const *cflags_extra,
                           Profile p, int verbose, int force, int jobs, int strict,
                           StrVec *out_objs) {
  Proc *running;
  int running_n;
  int i;

  if (jobs < 1) jobs = 1;

  running = (Proc*)xmalloc((size_t)jobs * sizeof(Proc));
  running_n = 0;

  for (i = 0; i < srcs->count; i++) {
    const char *src = srcs->items[i];
    char sid[512], obj_name[768], dep_name[768];
    char obj_path[1024], dep_path[1024];

    sanitize_path_to_id(sid, sizeof(sid), src);
    strcpy(obj_name, sid); strcat(obj_name, ".o");
    strcpy(dep_name, sid); strcat(dep_name, ".d");

    path_join(obj_path, objd, obj_name);
    path_join(dep_path, depd, dep_name);

    sv_push(out_objs, obj_path);

    if (!obj_needs_rebuild(obj_path, src, dep_path, force)) continue;

    /* build argv */
    {
      Argv av;
      StrVec tmp_defs;
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
          strcpy(d, "-D");
          strcat(d, def_extra[k]);
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

      if (jobs == 1) {
        int rc = run_argv_wait(av.a, 0);
        sv_free(&tmp_defs);
        av_free(&av);
        if (rc != 0) { free(running); return 1; }
      } else {
        if (running_n >= jobs) {
          int rcw = proc_wait(&running[0]);
          int m;
          for (m = 1; m < running_n; m++) running[m - 1] = running[m];
          running_n--;
          if (rcw != 0) { sv_free(&tmp_defs); av_free(&av); free(running); return 1; }
        }
        if (proc_spawn_nowait(av.a, &running[running_n]) != 0) { sv_free(&tmp_defs); av_free(&av); free(running); return 1; }
        running_n++;
        sv_free(&tmp_defs);
        av_free(&av);
      }
    }
  }

  if (jobs > 1) {
    int k;
    for (k = 0; k < running_n; k++) {
      int rc = proc_wait(&running[k]);
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
      strcpy(d, "-D");
      strcat(d, def_extra[k]);
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

static int build_core(Profile p, int verbose, int force, int jobs, int strict, StrVec *out_core_objs) {
  const char *cc;
  StrVec core_srcs;
  char root[512], objd[512], depd[512], bind[512];
  const char *inc_common[4];

  cc = env_or_default("TACK_CC", g_cc_default);

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
    path_join(cdir, g_build_dir, "_core");
    ensure_dir(cdir);
  }
  {
    char cdir[512], pdir[512];
    path_join(cdir, g_build_dir, "_core");
    path_join(pdir, cdir, profile_name(p));
    ensure_dir(pdir);
  }
  build_paths(root, objd, depd, bind, "_core", p);
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
                      p, verbose, force, jobs, strict,
                      out_core_objs) != 0) {
    sv_free(&core_srcs);
    return 1;
  }

  sv_free(&core_srcs);
  return 0;
}

static int build_one_target(const Target *t, Profile p, int verbose, int force, int jobs, int strict, int no_core) {
  const char *cc;
  const TargetOverride *ov;
  int use_core;

  StrVec srcs;
  StrVec objs;
  StrVec core_objs;

  char root[512], objd[512], depd[512], bind[512];
  char out_exe[512];

  const char *inc_common[5];

  cc = env_or_default("TACK_CC", g_cc_default);

  ov = find_override(t->name);

  use_core = 0;
  if (ov) use_core = ov->use_core;
  if (no_core) use_core = 0;

  /* prepare dirs */
  ensure_dir(g_build_dir);
  {
    char tdir[512];
    path_join(tdir, g_build_dir, t->id);
    ensure_dir(tdir);
  }
  {
    char tdir[512], pdir[512];
    path_join(tdir, g_build_dir, t->id);
    path_join(pdir, tdir, profile_name(p));
    ensure_dir(pdir);
  }
  build_paths(root, objd, depd, bind, t->id, p);
  ensure_dir(objd);
  ensure_dir(depd);
  ensure_dir(bind);

  exe_path(out_exe, t->id, p, t->bin_base);

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
    if (build_core(p, verbose, force, jobs, strict, &core_objs) != 0) {
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
                      p, verbose, force, jobs, strict,
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

    need_link = force || !file_exists(out_exe);
    if (!need_link) {
      long exe_t = file_mtime(out_exe);
      for (i = 0; i < all.count; i++) {
        long ot = file_mtime(all.items[i]);
        if (ot < 0 || ot > exe_t) { need_link = 1; break; }
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

  cc = env_or_default("TACK_CC", g_cc_default);

  sv_init(&tests);
  scan_dir_recursive_suffix(&tests, g_tests_dir, "_test.c");
  if (tests.count == 0) {
    printf("tack: test: no tests found under %s\n", g_tests_dir);
    sv_free(&tests);
    return 0;
  }

  ensure_dir(g_build_dir);
  path_join(tests_root, g_build_dir, "tests");
  ensure_dir(tests_root);
  path_join(tests_root, tests_root, profile_name(p));
  ensure_dir(tests_root);
  path_join(tests_bin, tests_root, "bin");
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
      strcpy(tmp, base);
      dot = strrchr(tmp, '.');
      if (dot) *dot = '\0';
      strcat(tmp, ".exe");
      path_join(out_exe, tests_bin, tmp);
    }
#else
    {
      char tmp[512];
      char *dot;
      strcpy(tmp, base);
      dot = strrchr(tmp, '.');
      if (dot) *dot = '\0';
      path_join(out_exe, tests_bin, tmp);
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
  printf("Usage:\n"
         "  tack help\n"
         "  tack version\n"
         "  tack doctor\n"
         "  tack init\n"
         "  tack list\n"
         "  tack build [debug|release] [--target NAME] [-v] [--rebuild] [-j N] [--strict] [--no-core]\n"
         "  tack run  [debug|release] [--target NAME] [-v] [--rebuild] [-j N] [--strict] [--no-core] [-- <args...>]\n"
         "  tack test [debug|release] [-v] [--rebuild] [--strict]\n"
         "  tack clean\n"
         "  tack clobber\n");
  printf("\nConventions:\n"
         "  app         : src/ or src/app/\n"
         "  shared core : src/core/ (linked if enabled for target)\n"
         "  tools       : tools/<name>/  (target name: tool:<name>)\n"
         "  tests       : tests/ (recursive _test.c files)\n");
  printf("\nNotes:\n"
         "  clean   = remove contents under build/ (keep the build directory)\n"
         "  clobber = remove build/ itself\n"
         "  --strict enables -Wunsupported\n");
}

static void cmd_version(void) { printf("tack %s\n", TACK_VERSION); }

static void cmd_doctor(void) {
  printf("Compiler default: %s\n", g_cc_default);
  printf("Compiler override: set env TACK_CC\n");
#ifdef _WIN32
  printf("OS: Windows\n");
#else
  printf("OS: POSIX\n");
#endif
  printf("Build dir : %s\n", g_build_dir);
  printf("Dirs      : src=%s include=%s tests=%s tools=%s core=%s\n",
         g_src_dir, g_inc_dir, g_tests_dir, g_tools_dir, g_core_dir);

  if (g_no_config) {
    printf("Config    : disabled (legacy mode)\n");
  } else if (g_config_loaded) {
    printf("Config    : %s\n", g_config_path);
  } else {
    printf("Config    : none\n");
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

  printf("tack: init: ensured src/include/tests/tools/build\n");
  return 0;
}

static int cmd_clean(void) {
  /* clean = remove contents under build/, keep build directory */
  if (!file_exists(g_build_dir)) return 0;
  if (rm_rf_contents(g_build_dir) != 0) {
    fprintf(stderr, "tack: clean: failed\n");
    return 1;
  }
  printf("tack: clean: done\n");
  return 0;
}

static int cmd_clobber(void) {
  /* clobber = remove build/ itself */
  if (!file_exists(g_build_dir)) return 0;
  if (rm_rf(g_build_dir) != 0) {
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
    if (!isdigit((unsigned char)*s)) return -1;
    v = v * 10 + (*s - '0');
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
    if (streq(argv[argi], "--no-config")) { g_no_config = 1; argi++; continue; }
    if (streq(argv[argi], "--config")) {
      if (argi + 1 >= argc) { fprintf(stderr, "tack: --config needs PATH\n"); return 2; }
      g_config_path_cli = argv[argi + 1];
      argi += 2;
      continue;
    }
    if (streq(argv[argi], "--no-auto-tools")) { g_no_auto_tools_cli = 1; argi++; continue; }
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
    rc = build_one_target(t, PROF_DEBUG, 0, 0, 1, 0, 0);
    tv_free(&tv);
    config_free();
    return rc;
  }

  cmd = argv[argi++];

  if (streq(cmd, "help"))    { print_help(); tv_free(&tv); config_free(); return 0; }
  if (streq(cmd, "version")) { cmd_version(); tv_free(&tv); config_free(); return 0; }
  if (streq(cmd, "doctor"))  { cmd_doctor(); tv_free(&tv); config_free(); return 0; }
  if (streq(cmd, "init"))    { int rc = cmd_init(); tv_free(&tv); config_free(); return rc; }
  if (streq(cmd, "clean"))   { int rc = cmd_clean(); tv_free(&tv); config_free(); return rc; }
  if (streq(cmd, "clobber")) { int rc = cmd_clobber(); tv_free(&tv); config_free(); return rc; }
  if (streq(cmd, "list"))    {
    if (g_no_config) printf("config: disabled (legacy mode)\n");
    else if (g_config_loaded) printf("config: %s\n", g_config_path);
    else printf("config: none\n");
    { int rc = cmd_list_targets(&tv); tv_free(&tv); config_free(); return rc; }
  }

  if (streq(cmd, "build") || streq(cmd, "run") || streq(cmd, "test")) {
    int verbose = 0;
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
      if (streq(argv[argi], "-v") || streq(argv[argi], "--verbose")) verbose = 1;
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
      int rc = build_one_target(t, p, verbose, force, jobs, strict, no_core);
      tv_free(&tv);
      config_free();
      return rc;
    }

    /* run */
    {
      int run_argi = argi;
      char exe[512];

      if (run_argi < argc && streq(argv[run_argi], "--")) run_argi++;

      if (build_one_target(t, p, verbose, force, jobs, strict, no_core) != 0) { tv_free(&tv); config_free(); return 1; }
      exe_path(exe, t->id, p, t->bin_base);

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
