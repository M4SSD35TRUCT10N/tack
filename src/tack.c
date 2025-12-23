/* tack.c - Tiny ANSI-C Kit
 * v0.3.0
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

/* ----------------------------- CONFIG ----------------------------- */

#define TACK_VERSION "0.3.0"

static const char *g_cc_default = "tcc";
static const char *g_build_dir  = "build";

static const char *g_src_dir    = "src";
static const char *g_inc_dir    = "include";
static const char *g_tests_dir  = "tests";
static const char *g_tools_dir  = "tools";

static const char *g_default_target = "app";

/* keep warnings strict, but DO NOT die on tcc ignoring GCC attributes in system headers */
static const char *g_warn_flags_base = " -Wall -Werror -Wwrite-strings -Wimplicit-function-declaration -Wno-unsupported";

/* optional strict: add -Wunsupported */
static const char *g_warn_flags_strict_add = " -Wunsupported";

static const char *g_dbg_flags = " -g -bt20";
static const char *g_rel_flags = " -O2";

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

static int file_exists(const char *path) {
  STAT_ST st;
  return STAT_FN(path, &st) == 0;
}

static long file_mtime(const char *path) {
  STAT_ST st;
  if (STAT_FN(path, &st) != 0) return -1;
  return (long)st.st_mtime;
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

/* --------------------------- string vector --------------------------- */

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

static void sv_free(StrVec *v) {
  int i;
  for (i = 0; i < v->count; i++) free(v->items[i]);
  free(v->items);
  v->items = 0; v->count = 0; v->cap = 0;
}

/* --------------------------- recursive delete --------------------------- */

static int is_dir_path(const char *path) {
  STAT_ST st;
  if (STAT_FN(path, &st) != 0) return 0;
#ifdef _WIN32
  return (st.st_mode & _S_IFDIR) != 0;
#else
  return S_ISDIR(st.st_mode);
#endif
}

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
    DIR *d = opendir(path);
    struct dirent *e;
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
    DIR *d = opendir(dir);
    struct dirent *e;
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
  int i = 0;
  while (argv[i]) {
    const char *a = argv[i];
    int needq = 0;
    const char *p;
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

/* --------------------------- scanning --------------------------- */

static int is_dir_entry(const char *fullpath) {
  return is_dir_path(fullpath);
}

static void scan_dir_recursive_suffix(StrVec *out, const char *dir, const char *suffix) {
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

    path_join(full, dir, fd.cFileName);

    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      /* skip build output if someone scans too broadly */
      if (streq(fd.cFileName, "build")) continue;
      scan_dir_recursive_suffix(out, full, suffix);
    } else {
      if (ends_with(fd.cFileName, suffix)) sv_push(out, full);
    }
  } while (FindNextFileA(h, &fd));

  FindClose(h);
#else
  DIR *d = opendir(dir);
  struct dirent *e;
  if (!d) return;

  while ((e = readdir(d)) != 0) {
    char full[1024];
    if (streq(e->d_name, ".") || streq(e->d_name, "..")) continue;
    path_join(full, dir, e->d_name);

    if (is_dir_entry(full)) {
      if (streq(e->d_name, "build")) continue;
      scan_dir_recursive_suffix(out, full, suffix);
    } else {
      if (ends_with(e->d_name, suffix)) sv_push(out, full);
    }
  }
  closedir(d);
#endif
}

/* --------------------------- dep parsing (handles escaped spaces) --------------------------- */

static int depfile_needs_rebuild(const char *obj_path, const char *dep_path) {
#if USE_DEPFILES
  FILE *f;
  long obj_t = file_mtime(obj_path);
  int c;
  char tok[2048];
  int ti = 0;
  int seen_colon = 0;

  if (obj_t < 0) return 1;
  f = fopen(dep_path, "rb");
  if (!f) return 1;

  while ((c = fgetc(f)) != EOF) {
    if (c == '\\') {
      int n = fgetc(f);
      if (n == '\n' || n == '\r') {
        /* line continuation */
        continue;
      }
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
        long dt;
        tok[ti] = '\0';
        ti = 0;
        if (seen_colon) {
          dt = file_mtime(tok);
          if (dt < 0 || dt > obj_t) { fclose(f); return 1; }
        }
      }
      continue;
    }

    if (ti < (int)sizeof(tok) - 1) tok[ti++] = (char)c;
  }

  if (ti > 0 && seen_colon) {
    long dt;
    tok[ti] = '\0';
    dt = file_mtime(tok);
    if (dt < 0 || dt > obj_t) { fclose(f); return 1; }
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

/* --------------------------- targets & discovery --------------------------- */

typedef enum { PROF_DEBUG = 0, PROF_RELEASE = 1 } Profile;
static const char *profile_name(Profile p) { return (p == PROF_RELEASE) ? "release" : "debug"; }

typedef struct {
  char *name;     /* e.g. "app" or "tool:foo" */
  char *src_dir;  /* e.g. "src" or "tools/foo" */
  char *bin_base; /* e.g. "app" or "foo" */
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
    free(v->items[i].src_dir);
    free(v->items[i].bin_base);
  }
  free(v->items);
  v->items = 0; v->count = 0; v->cap = 0;
}

static void tv_push(TargetVec *v, const char *name, const char *src_dir, const char *bin_base) {
  Target *t;
  if (v->count + 1 > v->cap) {
    int ncap = v->cap ? v->cap * 2 : 8;
    v->items = (Target*)xrealloc(v->items, (size_t)ncap * sizeof(Target));
    v->cap = ncap;
  }
  t = &v->items[v->count++];
  t->name = xstrdup(name);
  t->src_dir = xstrdup(src_dir);
  t->bin_base = xstrdup(bin_base);
}

static const Target *find_target(TargetVec *v, const char *name) {
  int i;
  for (i = 0; i < v->count; i++) {
    if (streq(v->items[i].name, name)) return &v->items[i];
  }
  return 0;
}

static void discover_targets(TargetVec *out) {
  /* always app */
  tv_push(out, "app", g_src_dir, "app");

  /* discover tools/<name> (immediate subdirs only) */
  if (!file_exists(g_tools_dir) || !is_dir_path(g_tools_dir)) return;

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
        char name[256], src[512], tname[512];
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
    DIR *d = opendir(g_tools_dir);
    struct dirent *e;
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

/* --------------------------- build paths --------------------------- */

/* build/<target>/<profile>/{obj,dep,bin} */
static void build_paths(char *root, char *objd, char *depd, char *bind,
                        const char *target, Profile p) {
  char tdir[512];
  char pdir[512];

  path_join(tdir, g_build_dir, target);
  path_join(pdir, tdir, profile_name(p));

  strcpy(root, pdir);
  path_join(objd, root, "obj");
  path_join(depd, root, "dep");
  path_join(bind, root, "bin");
}

static void exe_path(char *out, const char *target, Profile p, const char *bin_base) {
  char root[512], objd[512], depd[512], bind[512];
  char fn[256];

  build_paths(root, objd, depd, bind, target, p);

#ifdef _WIN32
  strcpy(fn, bin_base);
  strcat(fn, ".exe");
#else
  strcpy(fn, bin_base);
#endif
  path_join(out, bind, fn);
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
  v->a[v->n++] = (char*)s; /* pointers must remain valid (we use stable strings) */
}

static void av_terminate(Argv *v) {
  av_push(v, 0);
}

/* --------------------------- build core --------------------------- */

static const char *env_or_default(const char *key, const char *defv) {
  const char *v = getenv(key);
  if (v && v[0]) return v;
  return defv;
}

static int build_target(const Target *t, Profile p, int verbose, int force, int jobs, int strict) {
  StrVec srcs;
  int i;

  char cc_buf[512];
  const char *cc;

  char root[512], objd[512], depd[512], bind[512];
  char out_exe[512];

  ensure_dir(g_build_dir);

  /* ensure build/<target>/<profile>/... exists */
  {
    char tdir[512];
    path_join(tdir, g_build_dir, t->name);
    ensure_dir(tdir);
  }
  {
    char tdir[512], pdir[512];
    path_join(tdir, g_build_dir, t->name);
    path_join(pdir, tdir, profile_name(p));
    ensure_dir(pdir);
  }
  build_paths(root, objd, depd, bind, t->name, p);
  ensure_dir(objd);
  ensure_dir(depd);
  ensure_dir(bind);

  exe_path(out_exe, t->name, p, t->bin_base);

  /* scan sources recursively */
  sv_init(&srcs);
  scan_dir_recursive_suffix(&srcs, t->src_dir, ".c");
  if (srcs.count == 0) {
    fprintf(stderr, "tack: build: no sources in %s for target %s\n", t->src_dir, t->name);
    sv_free(&srcs);
    return 1;
  }

  /* compiler */
  cc = env_or_default("TACK_CC", g_cc_default);
  strcpy(cc_buf, cc);

  /* prepare object list */
  {
    StrVec objs;
    sv_init(&objs);

    /* compile job list */
    {
      /* We do a simple job pool only for compile steps. */
      Proc *running = 0;
      int running_n = 0;

      if (jobs < 1) jobs = 1;
      running = (Proc*)xmalloc((size_t)jobs * sizeof(Proc));
      running_n = 0;

      for (i = 0; i < srcs.count; i++) {
        const char *src = srcs.items[i];
        char sid[512], obj_name[768], dep_name[768];
        char obj_path[1024], dep_path[1024];

        sanitize_path_to_id(sid, sizeof(sid), src);
        strcpy(obj_name, sid); strcat(obj_name, ".o");
        strcpy(dep_name, sid); strcat(dep_name, ".d");

        path_join(obj_path, objd, obj_name);
        path_join(dep_path, depd, dep_name);

        sv_push(&objs, obj_path);

        if (!obj_needs_rebuild(obj_path, src, dep_path, force)) continue;

        /* build argv for tcc compile */
        {
          Argv av;
          av_init(&av);

          av_push(&av, cc_buf);
          av_push(&av, "-c");

          /* warnings */
          /* we pass flags as separate args */
          av_push(&av, "-Wall");
          av_push(&av, "-Werror");
          av_push(&av, "-Wwrite-strings");
          av_push(&av, "-Wimplicit-function-declaration");
          av_push(&av, "-Wno-unsupported");
          if (strict) av_push(&av, "-Wunsupported");

          if (p == PROF_DEBUG) { av_push(&av, "-g"); av_push(&av, "-bt20"); }
          if (p == PROF_RELEASE) { av_push(&av, "-O2"); }

          /* includes: -Iinclude -Itargetsrc */
          av_push(&av, "-I");
          av_push(&av, g_inc_dir);
          av_push(&av, "-I");
          av_push(&av, t->src_dir);

          /* defines */
          if (p == PROF_DEBUG) av_push(&av, "-DDEBUG=1");
          else av_push(&av, "-DNDEBUG=1");

#if USE_DEPFILES
          av_push(&av, "-MD");
          av_push(&av, "-MF");
          av_push(&av, dep_path);
#endif
          av_push(&av, "-o");
          av_push(&av, obj_path);
          av_push(&av, src);

          av_terminate(&av);

          /* run with job pool */
          if (verbose) print_argv(av.a);

          if (jobs == 1) {
            if (run_argv_wait(av.a, 0) != 0) { av_free(&av); free(running); sv_free(&srcs); sv_free(&objs); return 1; }
          } else {
            /* if pool full, wait one */
            if (running_n >= jobs) {
              int rc = proc_wait(&running[0]);
              int k;
              for (k = 1; k < running_n; k++) running[k - 1] = running[k];
              running_n--;
              if (rc != 0) { av_free(&av); free(running); sv_free(&srcs); sv_free(&objs); return 1; }
            }
            if (proc_spawn_nowait(av.a, &running[running_n]) != 0) {
              av_free(&av); free(running); sv_free(&srcs); sv_free(&objs); return 1;
            }
            running_n++;
          }

          av_free(&av);
        }
      }

      /* wait remaining */
      if (jobs > 1) {
        int k;
        for (k = 0; k < running_n; k++) {
          int rc = proc_wait(&running[k]);
          if (rc != 0) { free(running); sv_free(&srcs); sv_free(&objs); return 1; }
        }
      }
      free(running);
    }

    /* link */
    {
      int need_link = force || !file_exists(out_exe);
      if (!need_link) {
        long exe_t = file_mtime(out_exe);
        for (i = 0; i < objs.count; i++) {
          long ot = file_mtime(objs.items[i]);
          if (ot < 0 || ot > exe_t) { need_link = 1; break; }
        }
      }

      if (need_link) {
        Argv av;
        av_init(&av);

        av_push(&av, cc_buf);

        /* warnings */
        av_push(&av, "-Wall");
        av_push(&av, "-Werror");
        av_push(&av, "-Wwrite-strings");
        av_push(&av, "-Wimplicit-function-declaration");
        av_push(&av, "-Wno-unsupported");
        if (strict) av_push(&av, "-Wunsupported");

        if (p == PROF_DEBUG) { av_push(&av, "-g"); av_push(&av, "-bt20"); }
        if (p == PROF_RELEASE) { av_push(&av, "-O2"); }

        av_push(&av, "-I"); av_push(&av, g_inc_dir);
        av_push(&av, "-I"); av_push(&av, t->src_dir);

        if (p == PROF_DEBUG) av_push(&av, "-DDEBUG=1");
        else av_push(&av, "-DNDEBUG=1");

        av_push(&av, "-o");
        av_push(&av, out_exe);

        for (i = 0; i < objs.count; i++) av_push(&av, objs.items[i]);

        av_terminate(&av);

        if (run_argv_wait(av.a, verbose) != 0) { av_free(&av); sv_free(&srcs); sv_free(&objs); return 1; }
        av_free(&av);
      } else if (verbose) {
        printf("up to date: %s\n", out_exe);
      }
    }

    sv_free(&objs);
  }

  sv_free(&srcs);
  return 0;
}

/* --------------------------- tests --------------------------- */

static int build_and_run_tests(Profile p, int verbose, int force, int strict) {
  StrVec tests;
  int i;
  const char *cc = env_or_default("TACK_CC", g_cc_default);

  char tests_root[512];
  char tests_bin[512];

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

  for (i = 0; i < tests.count; i++) {
    const char *src = tests.items[i];
    const char *base = path_base(src);
    char out_exe[1024];

    /* build test exe name */
#ifdef _WIN32
    {
      char tmp[512];
      strcpy(tmp, base);
      {
        char *dot = strrchr(tmp, '.');
        if (dot) *dot = '\0';
      }
      strcat(tmp, ".exe");
      path_join(out_exe, tests_bin, tmp);
    }
#else
    {
      char tmp[512];
      strcpy(tmp, base);
      {
        char *dot = strrchr(tmp, '.');
        if (dot) *dot = '\0';
      }
      path_join(out_exe, tests_bin, tmp);
    }
#endif

    if (force || !file_exists(out_exe) || file_mtime(src) > file_mtime(out_exe)) {
      Argv av;
      av_init(&av);

      av_push(&av, (char*)cc);

      av_push(&av, "-Wall");
      av_push(&av, "-Werror");
      av_push(&av, "-Wwrite-strings");
      av_push(&av, "-Wimplicit-function-declaration");
      av_push(&av, "-Wno-unsupported");
      if (strict) av_push(&av, "-Wunsupported");

      if (p == PROF_DEBUG) { av_push(&av, "-g"); av_push(&av, "-bt20"); }
      if (p == PROF_RELEASE) { av_push(&av, "-O2"); }

      av_push(&av, "-I"); av_push(&av, g_inc_dir);
      av_push(&av, "-I"); av_push(&av, g_tests_dir);

      if (p == PROF_DEBUG) av_push(&av, "-DDEBUG=1");
      else av_push(&av, "-DNDEBUG=1");

      av_push(&av, "-o"); av_push(&av, out_exe);
      av_push(&av, src);

      av_terminate(&av);

      if (run_argv_wait(av.a, verbose) != 0) { av_free(&av); sv_free(&tests); return 1; }
      av_free(&av);
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
  printf(
    "tack %s - Tiny ANSI-C Kit\n\n"
    "Usage:\n"
    "  tack help\n"
    "  tack version\n"
    "  tack doctor\n"
    "  tack init\n"
    "  tack list\n"
    "  tack build [debug|release] [--target NAME] [-v] [--rebuild] [-j N] [--strict]\n"
    "  tack run  [debug|release] [--target NAME] [-v] [--rebuild] [-j N] [--strict] [-- <args...>]\n"
    "  tack test [debug|release] [-v] [--rebuild] [--strict]\n"
    "  tack clean\n"
    "  tack clobber\n\n"
    "Targets:\n"
    "  app                -> sources under src/\n"
    "  tool:<name>        -> sources under tools/<name>/ (auto-discovered)\n\n"
    "Notes:\n"
    "  clean   = remove contents under build/ (keep the build directory)\n"
    "  clobber = remove build/ itself\n"
    "  --strict enables -Wunsupported (default suppresses unsupported warnings)\n",
    TACK_VERSION
  );
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
  printf("Depfiles  : %s\n", USE_DEPFILES ? "on" : "off");
  printf("Dirs      : src=%s include=%s tests=%s tools=%s\n", g_src_dir, g_inc_dir, g_tests_dir, g_tools_dir);
}

static int cmd_init(void) {
  FILE *f;

  ensure_dir(g_src_dir);
  ensure_dir(g_inc_dir);
  ensure_dir(g_tests_dir);
  ensure_dir(g_tools_dir);
  ensure_dir(g_build_dir);

  if (!file_exists("src/main.c")) {
    f = fopen("src/main.c", "wb");
    if (!f) { fprintf(stderr, "tack: init: cannot create src/main.c\n"); return 1; }
    fprintf(f,
      "#include <stdio.h>\n\n"
      "int main(int argc, char **argv) {\n"
      "  (void)argc; (void)argv;\n"
      "  puts(\"Hello from tack v0.3.0!\");\n"
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
    printf("  %-16s  src=%s  bin=%s\n", tv->items[i].name, tv->items[i].src_dir, tv->items[i].bin_base);
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

  tv_init(&tv);
  discover_targets(&tv);

  if (argc < 2) {
    const Target *t = find_target(&tv, g_default_target);
    int rc;
    if (!t) { fprintf(stderr, "tack: default target missing\n"); tv_free(&tv); return 2; }
    rc = build_target(t, PROF_DEBUG, 0, 0, 1, 0);
    tv_free(&tv);
    return rc;
  }

  cmd = argv[1];

  if (streq(cmd, "help"))    { print_help(); tv_free(&tv); return 0; }
  if (streq(cmd, "version")) { cmd_version(); tv_free(&tv); return 0; }
  if (streq(cmd, "doctor"))  { cmd_doctor(); tv_free(&tv); return 0; }
  if (streq(cmd, "init"))    { int rc = cmd_init(); tv_free(&tv); return rc; }
  if (streq(cmd, "clean"))   { int rc = cmd_clean(); tv_free(&tv); return rc; }
  if (streq(cmd, "clobber")) { int rc = cmd_clobber(); tv_free(&tv); return rc; }
  if (streq(cmd, "list"))    { int rc = cmd_list_targets(&tv); tv_free(&tv); return rc; }

  if (streq(cmd, "build") || streq(cmd, "run") || streq(cmd, "test")) {
    int verbose = 0;
    int force = 0;
    int jobs = 1;
    int strict = 0;

    int i = 2;
    Profile p = parse_profile(&i, argc, argv);

    const char *target_name = g_default_target;
    const Target *t = 0;

    /* parse options; for run, args may follow '--' */
    for (; i < argc; i++) {
      if (streq(argv[i], "--")) { break; }
      if (streq(argv[i], "-v") || streq(argv[i], "--verbose")) verbose = 1;
      else if (streq(argv[i], "--rebuild")) force = 1;
      else if (streq(argv[i], "--strict")) strict = 1;
      else if (streq(argv[i], "--target")) {
        if (i + 1 >= argc) { fprintf(stderr, "tack: --target needs NAME\n"); tv_free(&tv); return 2; }
        target_name = argv[++i];
      } else if (streq(argv[i], "-j") || streq(argv[i], "--jobs")) {
        int v;
        if (i + 1 >= argc) { fprintf(stderr, "tack: -j needs N\n"); tv_free(&tv); return 2; }
        v = parse_int(argv[++i]);
        if (v < 1) { fprintf(stderr, "tack: invalid -j %s\n", argv[i]); tv_free(&tv); return 2; }
        jobs = v;
      } else {
        /* run: allow args without -- (best effort) */
        if (streq(cmd, "run")) break;
        fprintf(stderr, "tack: %s: unknown arg: %s\n", cmd, argv[i]);
        tv_free(&tv);
        return 2;
      }
    }

    if (streq(cmd, "test")) {
      int rc = build_and_run_tests(p, verbose, force, strict);
      tv_free(&tv);
      return rc;
    }

    t = find_target(&tv, target_name);
    if (!t) {
      fprintf(stderr, "tack: unknown target: %s\n", target_name);
      fprintf(stderr, "tack: hint: use 'tack list'\n");
      tv_free(&tv);
      return 2;
    }

    if (streq(cmd, "build")) {
      int rc = build_target(t, p, verbose, force, jobs, strict);
      tv_free(&tv);
      return rc;
    }

    /* run */
    {
      int argi = i;
      char exe[512];

      if (argi < argc && streq(argv[argi], "--")) argi++;

      if (build_target(t, p, verbose, force, jobs, strict) != 0) { tv_free(&tv); return 1; }
      exe_path(exe, t->name, p, t->bin_base);

      /* build argv: exe + rest args */
      {
        Argv av;
        int k;

        av_init(&av);
        av_push(&av, exe);

        for (k = argi; k < argc; k++) av_push(&av, argv[k]);
        av_terminate(&av);

        {
          int rc = run_argv_wait(av.a, verbose);
          av_free(&av);
          tv_free(&tv);
          return rc;
        }
      }
    }
  }

  fprintf(stderr, "tack: unknown command: %s\n\n", cmd);
  print_help();
  tv_free(&tv);
  return 2;
}

