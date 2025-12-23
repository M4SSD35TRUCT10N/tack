/* tack.c - Tiny ANSI-C Kit
 * v0.2.0
 *
 * Goals:
 * - single file build tool (C89)
 * - no make/cmake/ninja
 * - auto source scanning
 * - targets via --target
 * - decent quoting on Windows
 *
 * Typical:
 *   tcc -run tack.c init
 *   tcc -run tack.c build debug -v
 *   tcc -run tack.c run  debug -- --arg1 "hello world"
 *
 * Materialize:
 *   tcc tack.c -o tack.exe
 *   tack.exe build release --target app
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef _WIN32
  #include <windows.h>
  #include <direct.h>
  #define PATH_SEP '\\'
  #define STAT_FN _stat
  #define STAT_ST struct _stat
#else
  #include <dirent.h>
  #include <unistd.h>
  #define PATH_SEP '/'
  #define STAT_FN stat
  #define STAT_ST struct stat
#endif

/* ----------------------------- CONFIG ----------------------------- */

#define TACK_VERSION "0.2.0"

static const char *g_cc = "tcc";
static const char *g_build_dir = "build";

/* conventions */
static const char *g_src_dir   = "src";
static const char *g_inc_dir_1 = "include";
static const char *g_tests_dir = "tests";

/* tool defaults */
static const char *g_default_target = "app";

/* flags */
static const char *g_warn_flags =
  " -Wall -Werror -Wwrite-strings -Wno-unsupported -Wimplicit-function-declaration";

static const char *g_dbg_flags =
  " -g -bt20";

static const char *g_rel_flags =
  " -O2";

/* depfiles */
#define USE_DEPFILES 1

/* --------------------------- small helpers --------------------------- */

static int streq(const char *a, const char *b) { return strcmp(a, b) == 0; }

static void *xmalloc(size_t n) {
  void *p = malloc(n);
  if (!p) { fprintf(stderr, "out of memory\n"); exit(1); }
  return p;
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

/* replace \ / . : with '_' to make a unique-ish object filename */
static void sanitize_path_to_id(char *out, size_t cap, const char *path) {
  size_t i = 0;
  while (*path && i + 1 < cap) {
    char c = *path++;
    if (c == '/' || c == '\\' || c == '.' || c == ':' ) c = '_';
    out[i++] = c;
  }
  out[i] = '\0';
}

/* quote args robust-ish for cmd.exe and system() */
static void append_quoted(char *dst, size_t cap, const char *arg) {
  int needs = 0;
  const char *p;

  for (p = arg; *p; p++) {
    if (isspace((unsigned char)*p) || *p == '"' ) { needs = 1; break; }
  }

  if (!needs) {
    strncat(dst, arg, cap - strlen(dst) - 1);
    return;
  }

  strncat(dst, "\"", cap - strlen(dst) - 1);
  for (p = arg; *p; p++) {
    if (*p == '"') {
      /* escape quotes: \" */
      strncat(dst, "\\\"", cap - strlen(dst) - 1);
    } else {
      char tmp[2]; tmp[0] = *p; tmp[1] = '\0';
      strncat(dst, tmp, cap - strlen(dst) - 1);
    }
  }
  strncat(dst, "\"", cap - strlen(dst) - 1);
}

static int run_cmd(const char *cmd, int verbose) {
  if (verbose) printf("%s\n", cmd);
  return system(cmd);
}

/* --------------------------- tiny vector --------------------------- */

typedef struct {
  char **items;
  int count;
  int cap;
} StrVec;

static void sv_init(StrVec *v) { v->items = 0; v->count = 0; v->cap = 0; }

static void sv_push(StrVec *v, const char *s) {
  if (v->count + 1 > v->cap) {
    int ncap = v->cap ? v->cap * 2 : 16;
    char **nitems = (char**)realloc(v->items, (size_t)ncap * sizeof(char*));
    if (!nitems) { fprintf(stderr, "out of memory\n"); exit(1); }
    v->items = nitems;
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

/* --------------------------- scanning --------------------------- */

static void scan_dir_for_suffix(StrVec *out, const char *dir, const char *suffix) {
#ifdef _WIN32
  char pattern[512];
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
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
    if (ends_with(fd.cFileName, suffix)) {
      char full[512];
      path_join(full, dir, fd.cFileName);
      sv_push(out, full);
    }
  } while (FindNextFileA(h, &fd));

  FindClose(h);
#else
  DIR *d = opendir(dir);
  struct dirent *e;
  if (!d) return;

  while ((e = readdir(d)) != 0) {
    if (streq(e->d_name, ".") || streq(e->d_name, "..")) continue;
    /* non-recursive in v0.2.0 */
    if (ends_with(e->d_name, suffix)) {
      char full[512];
      path_join(full, dir, e->d_name);
      sv_push(out, full);
    }
  }
  closedir(d);
#endif
}

/* --------------------------- dep parsing --------------------------- */

static int depfile_needs_rebuild(const char *obj_path, const char *dep_path) {
#if USE_DEPFILES
  FILE *f;
  long obj_t = file_mtime(obj_path);
  int c;
  char tok[1024];
  int ti = 0;
  int seen_colon = 0;

  if (obj_t < 0) return 1;
  f = fopen(dep_path, "rb");
  if (!f) return 1;

  while ((c = fgetc(f)) != EOF) {
    if (c == '\\') {
      int n = fgetc(f);
      if (n != '\n' && n != '\r' && n != EOF) ungetc(n, f);
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

/* --------------------------- targets --------------------------- */

typedef enum { PROF_DEBUG = 0, PROF_RELEASE = 1 } Profile;

static const char *profile_name(Profile p) { return (p == PROF_RELEASE) ? "release" : "debug"; }

typedef struct {
  const char *name;     /* e.g. "app" */
  const char *src_dir;  /* e.g. "src" */
  const char *bin_name; /* output file base name (without .exe) */
} Target;

static Target g_targets[] = {
  { "app", "src", "app" },
  /* add more later:
     { "tool_foo", "tools\\foo", "foo" },
  */
  { 0, 0, 0 }
};

static const Target *find_target(const char *name) {
  int i;
  for (i = 0; g_targets[i].name; i++) {
    if (streq(g_targets[i].name, name)) return &g_targets[i];
  }
  return 0;
}

/* build/<target>/<profile>/{obj,dep,bin} */
static void build_paths(char *out_root, char *out_obj, char *out_dep, char *out_bin,
                        size_t cap, const char *target, Profile p) {
  char tdir[256];
  char pdir[256];
  (void)cap;

  /* root = build/<target>/<profile> */
  path_join(tdir, g_build_dir, target);
  path_join(pdir, tdir, profile_name(p));
  strcpy(out_root, pdir);

  /* obj/dep/bin */
  path_join(out_obj, out_root, "obj");
  path_join(out_dep, out_root, "dep");
  path_join(out_bin, out_root, "bin");
}

/* output executable path */
static void exe_path(char *out, size_t cap, const char *target, Profile p, const char *bin_base) {
  char root[512], objd[512], depd[512], bind[512];
  char fn[256];
  (void)cap;

  build_paths(root, objd, depd, bind, sizeof(root), target, p);

#ifdef _WIN32
  strcpy(fn, bin_base);
  strcat(fn, ".exe");
#else
  strcpy(fn, bin_base);
#endif
  path_join(out, bind, fn);
}

/* --------------------------- build core --------------------------- */

static void append_includes(char *cmd, size_t cap) {
  strncat(cmd, " -I", cap - strlen(cmd) - 1);
  strncat(cmd, g_inc_dir_1, cap - strlen(cmd) - 1);
  strncat(cmd, " -I", cap - strlen(cmd) - 1);
  strncat(cmd, g_src_dir, cap - strlen(cmd) - 1);
}

static void append_defines(char *cmd, size_t cap, Profile p) {
  if (p == PROF_DEBUG) {
    strncat(cmd, " -DDEBUG=1", cap - strlen(cmd) - 1);
  } else {
    strncat(cmd, " -DNDEBUG=1", cap - strlen(cmd) - 1);
  }
}

static int build_target(const Target *t, Profile p, int verbose, int force_rebuild) {
  StrVec srcs, objs;
  int i;
  int any_obj_built = 0;

  char root[512], objd[512], depd[512], bind[512];
  char out_exe[512];

  sv_init(&srcs);
  sv_init(&objs);

  /* scan sources */
  scan_dir_for_suffix(&srcs, t->src_dir, ".c");
  if (srcs.count == 0) {
    fprintf(stderr, "build: no sources found in %s for target %s\n", t->src_dir, t->name);
    sv_free(&srcs);
    sv_free(&objs);
    return 1;
  }

  /* dirs */
  ensure_dir(g_build_dir);
  {
    char tdir[512]; path_join(tdir, g_build_dir, t->name); ensure_dir(tdir);
  }
  {
    char tdir[512], pdir[512];
    path_join(tdir, g_build_dir, t->name);
    path_join(pdir, tdir, profile_name(p));
    ensure_dir(pdir);
  }
  build_paths(root, objd, depd, bind, sizeof(root), t->name, p);
  ensure_dir(objd);
  ensure_dir(depd);
  ensure_dir(bind);

  /* compile objects */
  for (i = 0; i < srcs.count; i++) {
    const char *src = srcs.items[i];
    char sid[512];
    char obj_name[640];
    char dep_name[640];
    char obj_path[1024];
    char dep_path[1024];
    char cmd[8192];

    sanitize_path_to_id(sid, sizeof(sid), src);

    strcpy(obj_name, sid); strcat(obj_name, ".o");
    strcpy(dep_name, sid); strcat(dep_name, ".d");

    path_join(obj_path, objd, obj_name);
    path_join(dep_path, depd, dep_name);

    if (!obj_needs_rebuild(obj_path, src, dep_path, force_rebuild)) {
      sv_push(&objs, obj_path);
      continue;
    }

    any_obj_built = 1;

    strcpy(cmd, g_cc);
    strcat(cmd, " -c");
    strcat(cmd, g_warn_flags);
    if (p == PROF_DEBUG) strcat(cmd, g_dbg_flags);
    if (p == PROF_RELEASE) strcat(cmd, g_rel_flags);

    append_includes(cmd, sizeof(cmd));
    append_defines(cmd, sizeof(cmd), p);

#if USE_DEPFILES
    strcat(cmd, " -MD -MF ");
    append_quoted(cmd, sizeof(cmd), dep_path);
#endif

    strcat(cmd, " -o ");
    append_quoted(cmd, sizeof(cmd), obj_path);
    strcat(cmd, " ");
    append_quoted(cmd, sizeof(cmd), src);

    if (run_cmd(cmd, verbose) != 0) {
      sv_free(&srcs);
      sv_free(&objs);
      return 1;
    }

    sv_push(&objs, obj_path);
  }

  /* link */
  exe_path(out_exe, sizeof(out_exe), t->name, p, t->bin_name);

  if (force_rebuild || any_obj_built || !file_exists(out_exe)) {
    char cmd[16384];

    strcpy(cmd, g_cc);
    strcat(cmd, g_warn_flags);
    if (p == PROF_DEBUG) strcat(cmd, g_dbg_flags);
    if (p == PROF_RELEASE) strcat(cmd, g_rel_flags);

    append_includes(cmd, sizeof(cmd));
    append_defines(cmd, sizeof(cmd), p);

    strcat(cmd, " -o ");
    append_quoted(cmd, sizeof(cmd), out_exe);

    for (i = 0; i < objs.count; i++) {
      strcat(cmd, " ");
      append_quoted(cmd, sizeof(cmd), objs.items[i]);
    }

    if (run_cmd(cmd, verbose) != 0) {
      sv_free(&srcs);
      sv_free(&objs);
      return 1;
    }
  } else if (verbose) {
    printf("up to date: %s\n", out_exe);
  }

  sv_free(&srcs);
  sv_free(&objs);
  return 0;
}

/* --------------------------- tests --------------------------- */

static int build_and_run_tests(Profile p, int verbose, int force_rebuild) {
  StrVec tests;
  int i;

  char tests_root[512];
  char tests_bin[512];

  sv_init(&tests);
  scan_dir_for_suffix(&tests, g_tests_dir, "_test.c");

  if (tests.count == 0) {
    printf("test: no tests found in %s (pattern *_test.c)\n", g_tests_dir);
    sv_free(&tests);
    return 0;
  }

  /* build/tests/<profile>/bin */
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
    char cmd[8192];

#ifdef _WIN32
    {
      char tmp[512];
      strcpy(tmp, base);
      /* replace .c with .exe */
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

    if (force_rebuild || !file_exists(out_exe) || file_mtime(src) > file_mtime(out_exe)) {
      strcpy(cmd, g_cc);
      strcat(cmd, g_warn_flags);
      if (p == PROF_DEBUG) strcat(cmd, g_dbg_flags);
      if (p == PROF_RELEASE) strcat(cmd, g_rel_flags);

      append_includes(cmd, sizeof(cmd));
      append_defines(cmd, sizeof(cmd), p);

      strcat(cmd, " -o ");
      append_quoted(cmd, sizeof(cmd), out_exe);
      strcat(cmd, " ");
      append_quoted(cmd, sizeof(cmd), src);

      if (run_cmd(cmd, verbose) != 0) { sv_free(&tests); return 1; }
    }

    /* run */
    {
      char runline[2048];
      runline[0] = '\0';
      append_quoted(runline, sizeof(runline), out_exe);
      if (run_cmd(runline, verbose) != 0) { sv_free(&tests); return 1; }
    }
  }

  sv_free(&tests);
  return 0;
}

/* --------------------------- housekeeping --------------------------- */

static int cmd_clean(void) {
  /* clean = delete build outputs but keep the build directory */
#ifdef _WIN32
  char cmd[512];
  /* remove target/profile content but keep build dir */
  /* easiest: remove everything under build, then recreate build */
  strcpy(cmd, "if exist ");
  strcat(cmd, g_build_dir);
  strcat(cmd, " rmdir /s /q ");
  strcat(cmd, g_build_dir);
  system(cmd);
  ensure_dir(g_build_dir);
#else
  char cmd[512];
  strcpy(cmd, "rm -rf ");
  strcat(cmd, g_build_dir);
  strcat(cmd, "/*");
  system(cmd);
#endif
  printf("clean: done\n");
  return 0;
}

static int cmd_clobber(void) {
  /* clobber = remove build dir itself */
#ifdef _WIN32
  char cmd[512];
  strcpy(cmd, "if exist ");
  strcat(cmd, g_build_dir);
  strcat(cmd, " rmdir /s /q ");
  strcat(cmd, g_build_dir);
  system(cmd);
#else
  char cmd[512];
  strcpy(cmd, "rm -rf ");
  strcat(cmd, g_build_dir);
  system(cmd);
#endif
  printf("clobber: done\n");
  return 0;
}

/* --------------------------- UX --------------------------- */

static void print_help(void) {
  printf(
    "tack %s - Tiny ANSI-C Kit\n\n"
    "Usage:\n"
    "  tack help\n"
    "  tack version\n"
    "  tack doctor\n"
    "  tack init\n"
    "  tack build [debug|release] [--target NAME] [-v] [--rebuild]\n"
    "  tack run  [debug|release] [--target NAME] [-v] [--rebuild] [-- <args...>]\n"
    "  tack test [debug|release] [-v] [--rebuild]\n"
    "  tack clean\n"
    "  tack clobber\n\n"
    "Notes:\n"
    "  clean   = remove contents under build/ (keep build/)\n"
    "  clobber = remove build/ itself\n",
    TACK_VERSION
  );
}

static void cmd_version(void) { printf("tack %s\n", TACK_VERSION); }

static void cmd_doctor(void) {
  printf("Compiler: %s\n", g_cc);
#ifdef _WIN32
  printf("OS: Windows\n");
#else
  printf("OS: POSIX\n");
#endif
  printf("Build dir: %s\n", g_build_dir);
  printf("Depfiles: %s\n", USE_DEPFILES ? "on" : "off");
  printf("Conventions:\n");
  printf("  src dir   : %s\n", g_src_dir);
  printf("  include   : %s\n", g_inc_dir_1);
  printf("  tests dir : %s\n", g_tests_dir);
  printf("Try: %s -v\n", g_cc);
  system("tcc -v");
}

static int cmd_init(void) {
  FILE *f;

  ensure_dir(g_src_dir);
  ensure_dir(g_inc_dir_1);
  ensure_dir(g_tests_dir);
  ensure_dir(g_build_dir);

  if (!file_exists("src/main.c")) {
    f = fopen("src/main.c", "wb");
    if (!f) { fprintf(stderr, "init: cannot create src/main.c\n"); return 1; }
    fprintf(f,
      "#include <stdio.h>\n\n"
      "int main(int argc, char **argv) {\n"
      "  (void)argc; (void)argv;\n"
      "  puts(\"Hello from tack v0.2.0!\");\n"
      "  return 0;\n"
      "}\n"
    );
    fclose(f);
  }

  if (!file_exists("tests/smoke_test.c")) {
    f = fopen("tests/smoke_test.c", "wb");
    if (!f) { fprintf(stderr, "init: cannot create tests/smoke_test.c\n"); return 1; }
    fprintf(f,
      "#include <stdio.h>\n\n"
      "int main(void) {\n"
      "  puts(\"smoke_test: ok\");\n"
      "  return 0;\n"
      "}\n"
    );
    fclose(f);
  }

  printf("init: ensured src/include/tests/build\n");
  return 0;
}

/* --------------------------- arg parsing --------------------------- */

static Profile parse_profile(int *argi, int argc, char **argv) {
  if (*argi < argc) {
    if (streq(argv[*argi], "release")) { (*argi)++; return PROF_RELEASE; }
    if (streq(argv[*argi], "debug"))   { (*argi)++; return PROF_DEBUG; }
  }
  return PROF_DEBUG;
}

static int is_flag(const char *s, const char *f1, const char *f2) {
  if (!s) return 0;
  if (f1 && streq(s, f1)) return 1;
  if (f2 && streq(s, f2)) return 1;
  return 0;
}

int main(int argc, char **argv) {
  const char *cmd;
  int verbose = 0;
  int force_rebuild = 0;

  if (argc < 2) {
    /* default */
    const Target *t = find_target(g_default_target);
    if (!t) { fprintf(stderr, "default target missing\n"); return 2; }
    return build_target(t, PROF_DEBUG, 0, 0);
  }

  cmd = argv[1];

  if (streq(cmd, "help"))    { print_help(); return 0; }
  if (streq(cmd, "version")) { cmd_version(); return 0; }
  if (streq(cmd, "doctor"))  { cmd_doctor(); return 0; }
  if (streq(cmd, "init"))    { return cmd_init(); }
  if (streq(cmd, "clean"))   { return cmd_clean(); }
  if (streq(cmd, "clobber")) { return cmd_clobber(); }

  if (streq(cmd, "build") || streq(cmd, "run") || streq(cmd, "test")) {
    int i = 2;
    Profile p = parse_profile(&i, argc, argv);

    const char *target_name = g_default_target;
    const Target *t = 0;

    /* parse flags */
    for (; i < argc; i++) {
      if (streq(argv[i], "--")) { break; } /* for run */
      if (is_flag(argv[i], "-v", "--verbose")) verbose = 1;
      else if (is_flag(argv[i], "--rebuild", 0)) force_rebuild = 1;
      else if (streq(argv[i], "--target")) {
        if (i + 1 >= argc) { fprintf(stderr, "--target needs NAME\n"); return 2; }
        target_name = argv[++i];
      } else {
        /* run: args start after --; test/build should not have extras */
        if (streq(cmd, "run")) {
          /* allow args without -- as well (best effort) */
          break;
        }
        fprintf(stderr, "%s: unknown arg: %s\n", cmd, argv[i]);
        return 2;
      }
    }

    if (streq(cmd, "test")) {
      return build_and_run_tests(p, verbose, force_rebuild);
    }

    t = find_target(target_name);
    if (!t) {
      fprintf(stderr, "unknown target: %s\n", target_name);
      return 2;
    }

    if (streq(cmd, "build")) {
      return build_target(t, p, verbose, force_rebuild);
    }

    /* run */
    {
      char out_exe[512];
      char runline[8192];
      int argi = i;

      /* if we stopped at "--", skip it */
      if (argi < argc && streq(argv[argi], "--")) argi++;

      if (build_target(t, p, verbose, force_rebuild) != 0) return 1;
      exe_path(out_exe, sizeof(out_exe), t->name, p, t->bin_name);

      runline[0] = '\0';
      append_quoted(runline, sizeof(runline), out_exe);

      for (; argi < argc; argi++) {
        strcat(runline, " ");
        append_quoted(runline, sizeof(runline), argv[argi]);
      }

      return run_cmd(runline, verbose);
    }
  }

  fprintf(stderr, "unknown command: %s\n\n", cmd);
  print_help();
  return 2;
}

