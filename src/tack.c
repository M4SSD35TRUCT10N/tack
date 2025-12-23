/* tack.c - Tiny ANSI-C Kit
 *
 * Build-Driver in C89: no make/cmake/ninja needed.
 *
 * Typical usage (Windows):
 *   tcc -run tack.c help
 *   tcc -run tack.c build debug -v
 *   tcc tack.c -o tack.exe && tack.exe run debug -- --arg1 foo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/stat.h>

#ifdef _WIN32
  #include <direct.h> /* _mkdir */
  #define PATH_SEP '\\'
  #define STAT_FN _stat
  #define STAT_ST struct _stat
#else
  #include <unistd.h>
  #define PATH_SEP '/'
  #define STAT_FN stat
  #define STAT_ST struct stat
#endif

/* ----------------------------- CONFIG ----------------------------- */

#define TACK_VERSION "0.1.0"

static const char *g_cc = "tcc";          /* set to "tools\\tcc\\tcc.exe" if needed */
static const char *g_build_dir = "build"; /* root build dir */
static const char *g_app_name = "app";    /* output name without extension */

/* Project sources (NULL-terminated). Start small. */
static const char *g_srcs[] = {
  "src/main.c",
  /* "src/util.c", */
  0
};

static const char *g_test_srcs[] = {
  /* "tests/smoke_test.c", */
  0
};

/* Include dirs / defines (NULL-terminated). */
static const char *g_includes[] = {
  "include",
  "src",
  0
};

static const char *g_defines_debug[] = {
  "DEBUG=1",
  0
};

static const char *g_defines_release[] = {
  "NDEBUG=1",
  0
};

/* Warnings/checks (tcc-friendly). Keep as strings we append. */
static const char *g_warn_flags =
  " -Wall -Werror -Wwrite-strings -Wunsupported -Wimplicit-function-declaration";

static const char *g_dbg_flags =
  " -g -bt20"; /* debug symbols + backtrace depth */

static const char *g_rel_flags =
  " -O2"; /* tcc supports -O, -O2 in practice */

/* If you want depfiles: -MD -MF <file.d> */
#define USE_DEPFILES 1

/* --------------------------- small helpers --------------------------- */

static int streq(const char *a, const char *b) { return strcmp(a, b) == 0; }

static int file_exists(const char *path) {
  STAT_ST st;
  return STAT_FN(path, &st) == 0;
}

static long file_mtime(const char *path) {
  STAT_ST st;
  if (STAT_FN(path, &st) != 0) return -1;
  return (long)st.st_mtime;
}

/* crude mkdir (single level) */
static void ensure_dir(const char *path) {
#ifdef _WIN32
  _mkdir(path);
#else
  mkdir(path, 0777);
#endif
}

/* join: out = a + SEP + b  (out must be large enough) */
static void path_join(char *out, const char *a, const char *b) {
  size_t la = strlen(a);
  strcpy(out, a);
  if (la > 0 && out[la - 1] != PATH_SEP) {
    out[la] = PATH_SEP;
    out[la + 1] = '\0';
  }
  strcat(out, b);
}

/* basename pointer (does not allocate) */
static const char *path_base(const char *p) {
  const char *s1 = strrchr(p, '/');
  const char *s2 = strrchr(p, '\\');
  const char *s = s1 > s2 ? s1 : s2;
  return s ? (s + 1) : p;
}

/* change extension: foo.c -> foo.o (out must be large enough) */
static void change_ext(char *out, const char *filename, const char *newext) {
  const char *dot = strrchr(filename, '.');
  if (!dot) {
    strcpy(out, filename);
    strcat(out, newext);
    return;
  }
  /* copy up to dot */
  {
    size_t n = (size_t)(dot - filename);
    memcpy(out, filename, n);
    out[n] = '\0';
    strcat(out, newext);
  }
}

/* run a command (optionally print) */
static int run_cmd(const char *cmd, int verbose) {
  if (verbose) printf("%s\n", cmd);
  return system(cmd);
}

/* append " -I<dir>" for each include */
static void append_includes(char *cmd, size_t cap, const char * const *incs) {
  int i;
  for (i = 0; incs && incs[i]; i++) {
    strncat(cmd, " -I", cap - strlen(cmd) - 1);
    strncat(cmd, incs[i], cap - strlen(cmd) - 1);
  }
}

/* append " -DNAME=VAL" for each define */
static void append_defines(char *cmd, size_t cap, const char * const *defs) {
  int i;
  for (i = 0; defs && defs[i]; i++) {
    strncat(cmd, " -D", cap - strlen(cmd) - 1);
    strncat(cmd, defs[i], cap - strlen(cmd) - 1);
  }
}

/* simplistic depfile parser:
 * depfile looks like: obj: dep1 dep2 dep3 \
 *                     dep4 ...
 * We check if any dependency is newer than obj.
 */
static int depfile_needs_rebuild(const char *obj_path, const char *dep_path) {
#if USE_DEPFILES
  FILE *f;
  long obj_t = file_mtime(obj_path);
  int c;
  char tok[1024];
  int ti = 0;
  int seen_colon = 0; /* deps after ':' */
  int is_first_tok = 1;

  if (obj_t < 0) return 1;
  f = fopen(dep_path, "rb");
  if (!f) return 1; /* no depfile => conservative */

  while ((c = fgetc(f)) != EOF) {
    if (c == '\\') {
      /* line continuation: skip next newline if present */
      int n = fgetc(f);
      if (n != '\n' && n != '\r' && n != EOF) ungetc(n, f);
      continue;
    }
    if (c == ':' && !seen_colon) {
      /* finish token (target), then switch to deps */
      tok[ti] = '\0';
      ti = 0;
      seen_colon = 1;
      is_first_tok = 0;
      continue;
    }

    if (isspace((unsigned char)c)) {
      if (ti > 0) {
        tok[ti] = '\0';
        ti = 0;

        if (seen_colon) {
          long dt = file_mtime(tok);
          if (dt < 0 || dt > obj_t) { fclose(f); return 1; }
        } else if (is_first_tok) {
          /* ignore leading target token */
          is_first_tok = 0;
        }
      }
      continue;
    }

    if (ti < (int)sizeof(tok) - 1) tok[ti++] = (char)c;
  }

  if (ti > 0) {
    tok[ti] = '\0';
    if (seen_colon) {
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

/* Decide whether to rebuild an object:
 * - if obj missing -> rebuild
 * - if src newer than obj -> rebuild
 * - if depfile says dep newer -> rebuild
 */
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

/* --------------------------- commands --------------------------- */

static void print_help(void) {
  printf(
    "tack %s - Tiny ANIS-C Kit\n"
    "\n"
    "Usage:\n"
    "  tack help [cmd]\n"
    "  tack version\n"
    "  tack doctor\n"
    "  tack init [name]\n"
    "  tack build [debug|release] [-v] [--rebuild]\n"
    "  tack run  [debug|release] [-v] [--rebuild] [-- <args...>]\n"
    "  tack test [debug|release] [-v] [--rebuild]\n"
    "  tack clean\n"
    "  tack clobber\n"
    "\n"
    "Notes:\n"
    "  clean   = remove intermediates (.o/.d), keep build dir\n"
    "  clobber = remove whole build dir (nuke it)\n",
    TACK_VERSION
  );
}

static void cmd_version(void) {
  printf("tack %s\n", TACK_VERSION);
}

static void cmd_doctor(void) {
  printf("Compiler: %s\n", g_cc);
#ifdef _WIN32
  printf("OS: Windows\n");
#else
  printf("OS: POSIX\n");
#endif
  printf("Build dir: %s\n", g_build_dir);
  printf("Depfiles: %s\n", USE_DEPFILES ? "on" : "off");
  printf("Try compiler version:\n");
#ifdef _WIN32
  system("tcc -v");
#else
  system("tcc -v");
#endif
}

static int cmd_init(const char *name) {
  FILE *f;
  (void)name;

  ensure_dir("src");
  ensure_dir("include");
  ensure_dir("tests");
  ensure_dir(g_build_dir);

  if (!file_exists("src/main.c")) {
    f = fopen("src/main.c", "wb");
    if (!f) { fprintf(stderr, "init: cannot create src/main.c\n"); return 1; }
    fprintf(f,
      "#include <stdio.h>\n"
      "\n"
      "int main(int argc, char **argv) {\n"
      "  (void)argc; (void)argv;\n"
      "  puts(\"Hello from tack!\");\n"
      "  return 0;\n"
      "}\n"
    );
    fclose(f);
  }

  printf("init: created src/include/tests/%s (if missing)\n", g_build_dir);
  return 0;
}

/* build profile */
typedef enum {
  PROF_DEBUG = 0,
  PROF_RELEASE = 1
} Profile;

static const char *profile_name(Profile p) { return p == PROF_RELEASE ? "release" : "debug"; }

static void output_exe_path(char *out, size_t cap, Profile p) {
  char fn[256];
#ifdef _WIN32
  (void)cap;
  strcpy(fn, g_app_name);
  strcat(fn, (p == PROF_RELEASE) ? "_release.exe" : "_debug.exe");
  path_join(out, g_build_dir, fn);
#else
  (void)cap;
  strcpy(fn, g_app_name);
  strcat(fn, (p == PROF_RELEASE) ? "_release" : "_debug");
  path_join(out, g_build_dir, fn);
#endif
}

static int build_app(Profile p, int verbose, int force_rebuild) {
  int i;
  int any_obj_built = 0;
  char exe[512];

  /* ensure build dir exists */
  ensure_dir(g_build_dir);

  output_exe_path(exe, sizeof(exe), p);

  /* compile objects */
  for (i = 0; g_srcs[i]; i++) {
    const char *src = g_srcs[i];
    char base[256], obj_name[256], dep_name[256];
    char obj_path[512], dep_path[512];
    char cmd[4096];

    /* base name from src file */
    strcpy(base, path_base(src));         /* main.c */
    change_ext(obj_name, base, ".o");     /* main.o */
    change_ext(dep_name, base, ".d");     /* main.d */

    path_join(obj_path, g_build_dir, obj_name);
    path_join(dep_path, g_build_dir, dep_name);

    if (!obj_needs_rebuild(obj_path, src, dep_path, force_rebuild)) {
      continue;
    }

    any_obj_built = 1;

    /* tcc -c src -o obj [flags] [-MD -MF dep] */
    strcpy(cmd, g_cc);
    strcat(cmd, " -c");
    strcat(cmd, g_warn_flags);
    if (p == PROF_DEBUG) strcat(cmd, g_dbg_flags);
    if (p == PROF_RELEASE) strcat(cmd, g_rel_flags);

    append_includes(cmd, sizeof(cmd), g_includes);
    if (p == PROF_DEBUG) append_defines(cmd, sizeof(cmd), g_defines_debug);
    if (p == PROF_RELEASE) append_defines(cmd, sizeof(cmd), g_defines_release);

#if USE_DEPFILES
    strcat(cmd, " -MD -MF ");
    strcat(cmd, dep_path);
#endif

    strcat(cmd, " -o ");
    strcat(cmd, obj_path);
    strcat(cmd, " ");
    strcat(cmd, src);

    if (run_cmd(cmd, verbose) != 0) return 1;
  }

  /* link if needed */
  if (force_rebuild || any_obj_built || !file_exists(exe)) {
    char cmd[8192];
    strcpy(cmd, g_cc);
    strcat(cmd, g_warn_flags);
    if (p == PROF_DEBUG) strcat(cmd, g_dbg_flags);
    if (p == PROF_RELEASE) strcat(cmd, g_rel_flags);

    append_includes(cmd, sizeof(cmd), g_includes);
    if (p == PROF_DEBUG) append_defines(cmd, sizeof(cmd), g_defines_debug);
    if (p == PROF_RELEASE) append_defines(cmd, sizeof(cmd), g_defines_release);

    strcat(cmd, " -o ");
    strcat(cmd, exe);

    for (i = 0; g_srcs[i]; i++) {
      char base[256], obj_name[256], obj_path[512];
      strcpy(base, path_base(g_srcs[i]));
      change_ext(obj_name, base, ".o");
      path_join(obj_path, g_build_dir, obj_name);
      strcat(cmd, " ");
      strcat(cmd, obj_path);
    }

    if (run_cmd(cmd, verbose) != 0) return 1;
  } else if (verbose) {
    printf("up to date: %s (%s)\n", exe, profile_name(p));
  }

  return 0;
}

static int cmd_run(Profile p, int verbose, int force_rebuild, int argc, char **argv, int argi) {
  char exe[512];
  char cmd[8192];
  int i;

  if (build_app(p, verbose, force_rebuild) != 0) return 1;
  output_exe_path(exe, sizeof(exe), p);

  /* build command line: "exe" args... */
  strcpy(cmd, "\"");
  strcat(cmd, exe);
  strcat(cmd, "\"");

  for (i = argi; i < argc; i++) {
    strcat(cmd, " ");
    /* naive quoting */
    if (strchr(argv[i], ' ') || strchr(argv[i], '\t')) {
      strcat(cmd, "\"");
      strcat(cmd, argv[i]);
      strcat(cmd, "\"");
    } else {
      strcat(cmd, argv[i]);
    }
  }

  return run_cmd(cmd, verbose);
}

static int cmd_test(Profile p, int verbose, int force_rebuild) {
  int i;
  char tests_dir[512];

  /* ensure build/tests exists */
  path_join(tests_dir, g_build_dir, "tests");
  ensure_dir(g_build_dir);
  ensure_dir(tests_dir);

  if (!g_test_srcs[0]) {
    printf("test: no tests configured (edit g_test_srcs[])\n");
    return 0;
  }

  for (i = 0; g_test_srcs[i]; i++) {
    const char *src = g_test_srcs[i];
    char base[256], exe_name[256], exe_path[512];
    char cmd[4096];

    strcpy(base, path_base(src)); /* smoke_test.c */
#ifdef _WIN32
    change_ext(exe_name, base, ".exe");
#else
    change_ext(exe_name, base, "");
#endif
    path_join(exe_path, tests_dir, exe_name);

    /* compile test as single TU (simple) */
    if (force_rebuild || !file_exists(exe_path) || file_mtime(src) > file_mtime(exe_path)) {
      strcpy(cmd, g_cc);
      strcat(cmd, g_warn_flags);
      if (p == PROF_DEBUG) strcat(cmd, g_dbg_flags);
      if (p == PROF_RELEASE) strcat(cmd, g_rel_flags);

      append_includes(cmd, sizeof(cmd), g_includes);
      if (p == PROF_DEBUG) append_defines(cmd, sizeof(cmd), g_defines_debug);
      if (p == PROF_RELEASE) append_defines(cmd, sizeof(cmd), g_defines_release);

      strcat(cmd, " -o ");
      strcat(cmd, exe_path);
      strcat(cmd, " ");
      strcat(cmd, src);

      if (run_cmd(cmd, verbose) != 0) return 1;
    }

    /* run test */
    {
      char runline[1024];
      strcpy(runline, "\"");
      strcat(runline, exe_path);
      strcat(runline, "\"");
      if (run_cmd(runline, verbose) != 0) return 1;
    }
  }

  return 0;
}

static int cmd_clean(void) {
#ifdef _WIN32
  /* delete *.o and *.d in build, keep directory */
  system("if exist build\\*.o del /q build\\*.o");
  system("if exist build\\*.d del /q build\\*.d");
  system("if exist build\\tests\\*.exe del /q build\\tests\\*.exe");
#else
  system("rm -f build/*.o build/*.d build/tests/*");
#endif
  printf("clean: done\n");
  return 0;
}

static int cmd_clobber(void) {
#ifdef _WIN32
  system("if exist build rmdir /s /q build");
#else
  system("rm -rf build");
#endif
  printf("clobber: done\n");
  return 0;
}

/* --------------------------- arg parsing --------------------------- */

static int is_flag(const char *s, const char *f1, const char *f2) {
  if (!s) return 0;
  if (f1 && streq(s, f1)) return 1;
  if (f2 && streq(s, f2)) return 1;
  return 0;
}

static Profile parse_profile(int *argi, int argc, char **argv) {
  if (*argi < argc) {
    if (streq(argv[*argi], "release")) { (*argi)++; return PROF_RELEASE; }
    if (streq(argv[*argi], "debug"))   { (*argi)++; return PROF_DEBUG; }
  }
  return PROF_DEBUG;
}

int main(int argc, char **argv) {
  const char *cmd;
  int verbose = 0;
  int force_rebuild = 0;

  if (argc < 2) {
    /* default: build debug */
    return build_app(PROF_DEBUG, 0, 0);
  }

  cmd = argv[1];

  if (streq(cmd, "help"))    { print_help(); return 0; }
  if (streq(cmd, "version")) { cmd_version(); return 0; }
  if (streq(cmd, "doctor"))  { cmd_doctor(); return 0; }

  if (streq(cmd, "init")) {
    const char *name = (argc >= 3) ? argv[2] : "app";
    return cmd_init(name);
  }

  if (streq(cmd, "clean"))   return cmd_clean();
  if (streq(cmd, "clobber")) return cmd_clobber();

  if (streq(cmd, "build")) {
    int i = 2;
    Profile p = parse_profile(&i, argc, argv);

    for (; i < argc; i++) {
      if (is_flag(argv[i], "-v", "--verbose")) verbose = 1;
      else if (is_flag(argv[i], "--rebuild", 0)) force_rebuild = 1;
      else { fprintf(stderr, "build: unknown arg: %s\n", argv[i]); return 2; }
    }

    return build_app(p, verbose, force_rebuild);
  }

  if (streq(cmd, "run")) {
    int i = 2;
    Profile p = parse_profile(&i, argc, argv);

    /* parse flags until "--" */
    for (; i < argc; i++) {
      if (streq(argv[i], "--")) { i++; break; }
      if (is_flag(argv[i], "-v", "--verbose")) verbose = 1;
      else if (is_flag(argv[i], "--rebuild", 0)) force_rebuild = 1;
      else { fprintf(stderr, "run: unknown arg: %s\n", argv[i]); return 2; }
    }

    return cmd_run(p, verbose, force_rebuild, argc, argv, i);
  }

  if (streq(cmd, "test")) {
    int i = 2;
    Profile p = parse_profile(&i, argc, argv);

    for (; i < argc; i++) {
      if (is_flag(argv[i], "-v", "--verbose")) verbose = 1;
      else if (is_flag(argv[i], "--rebuild", 0)) force_rebuild = 1;
      else { fprintf(stderr, "test: unknown arg: %s\n", argv[i]); return 2; }
    }

    return cmd_test(p, verbose, force_rebuild);
  }

  fprintf(stderr, "unknown command: %s\n", cmd);
  print_help();
  return 2;
}
