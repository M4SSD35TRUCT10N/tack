/* M2 process execution: real child-received argv, errors and -j cleanup.
 * ISO C89; fault wrappers are compiled only into this test. */
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <errno.h>
#else
#include <unistd.h>
#endif
/* Parent-only process-accounting probe. A successful platform spawn increments
 * the live count; its completed platform wait decrements it. This bounds even
 * children that have finished but have not yet been reaped. The second-attempt
 * failure is injected while the first compiler child is still running. */
static int m2_pool_probe_on, m2_pool_probe_limit, m2_pool_probe_fail_at;
static int m2_pool_attempts, m2_pool_starts, m2_pool_reaps;
static int m2_pool_live, m2_pool_peak, m2_pool_violation;
static int m2_pool_hold_observed;
static void m2_pool_probe_begin(int limit, int fail_at) {
  m2_pool_probe_limit = limit;
  m2_pool_probe_fail_at = fail_at;
  m2_pool_attempts = 0;
  m2_pool_starts = 0;
  m2_pool_reaps = 0;
  m2_pool_live = 0;
  m2_pool_peak = 0;
  m2_pool_violation = 0;
  m2_pool_hold_observed = 0;
  m2_pool_probe_on = 1;
}
static int m2_pool_probe_should_fail(void) {
  int tries, max_tries;
  FILE *f;
  if (!m2_pool_probe_on) return 0;
  m2_pool_attempts++;
  if (m2_pool_probe_fail_at <= 0 ||
      m2_pool_attempts != m2_pool_probe_fail_at) return 0;
  /* P20's first child must really be running, not merely unreaped.
   * Wait for its ready marker before failing spawn #2, then release it.
   * Both waits are bounded and the child checks the release marker. */
#ifdef _WIN32
  max_tries = 150;
#else
  max_tries = 8;
#endif
  for (tries = 0; tries < max_tries; tries++) {
    f = fopen("build/m2-process/hold-ready", "rb");
    if (f) {
      fclose(f);
      m2_pool_hold_observed = 1;
      break;
    }
#ifdef _WIN32
    Sleep(20);
#else
    sleep(1);
#endif
  }
  f = fopen("build/m2-process/hold-release", "wb");
  if (!f) m2_pool_violation = 1;
  else {
    int wrote = fputs("release", f) != EOF;
    int closed = fclose(f) == 0;
    if (!wrote || !closed) m2_pool_violation = 1;
  }
  return 1;
}
static void m2_pool_probe_started(void) {
  if (!m2_pool_probe_on) return;
  m2_pool_starts++;
  m2_pool_live++;
  if (m2_pool_live > m2_pool_peak) m2_pool_peak = m2_pool_live;
  if (m2_pool_live > m2_pool_probe_limit) m2_pool_violation = 1;
}
static void m2_pool_probe_waited(void) {
  if (!m2_pool_probe_on) return;
  m2_pool_reaps++;
  if (m2_pool_live <= 0) m2_pool_violation = 1;
  else m2_pool_live--;
}
static void m2_pool_probe_end(void) { m2_pool_probe_on = 0; }
#ifdef _WIN32
static int m2_win_fail_create, m2_win_fail_wait, m2_win_fail_exit, m2_win_fail_close;
static int m2_win_closes;
static BOOL WINAPI m2_wrap_create(LPCSTR app, LPSTR cmd,
    LPSECURITY_ATTRIBUTES proc_attr, LPSECURITY_ATTRIBUTES thread_attr,
    BOOL inherit, DWORD flags, LPVOID env, LPCSTR cwd,
    LPSTARTUPINFOA startup, LPPROCESS_INFORMATION info) {
  BOOL ok;
  if (m2_pool_probe_should_fail() || m2_win_fail_create) {
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
  }
  ok = CreateProcessA(app, cmd, proc_attr, thread_attr, inherit, flags,
                      env, cwd, startup, info);
  if (ok) m2_pool_probe_started();
  return ok;
}
static DWORD WINAPI m2_wrap_wait(HANDLE process, DWORD timeout) {
  DWORD result;
  if (m2_win_fail_wait) { SetLastError(ERROR_INVALID_HANDLE); return WAIT_FAILED; }
  result = WaitForSingleObject(process, timeout);
  if (result == WAIT_OBJECT_0) m2_pool_probe_waited();
  return result;
}
static BOOL WINAPI m2_wrap_exit(HANDLE process, LPDWORD exit_code) {
  if (m2_win_fail_exit) { SetLastError(ERROR_INVALID_HANDLE); return FALSE; }
  return GetExitCodeProcess(process, exit_code);
}
static BOOL WINAPI m2_wrap_close(HANDLE handle) {
  BOOL real_result;
  m2_win_closes++;
  real_result = CloseHandle(handle);
  if (m2_win_fail_close) {
    /* Release the real handle even in the injected failure path. */
    m2_win_fail_close = 0;
    SetLastError(ERROR_INVALID_HANDLE);
    return FALSE;
  }
  return real_result;
}
#define CreateProcessA m2_wrap_create
#define WaitForSingleObject m2_wrap_wait
#define GetExitCodeProcess m2_wrap_exit
#define CloseHandle m2_wrap_close
#endif
#ifndef _WIN32
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
static int m2_fail_fork, m2_eintr_once, m2_fail_wait;
static pid_t m2_fork(void) {
  pid_t child;
  if (m2_pool_probe_should_fail() || m2_fail_fork) {
    errno = EAGAIN;
    return (pid_t)-1;
  }
  child = fork();
  if (child > 0) m2_pool_probe_started();
  return child;
}
static pid_t m2_waitpid(pid_t pid, int *status, int flags) {
  if (m2_eintr_once) { m2_eintr_once = 0; errno = EINTR; return (pid_t)-1; }
  if (m2_fail_wait) {
    pid_t r;
    m2_fail_wait = 0;
    do { r = waitpid(pid, status, flags); } while (r < 0 && errno == EINTR);
    errno = ECHILD;
    return (pid_t)-1;
  }
  {
    pid_t r = waitpid(pid, status, flags);
    if (r > 0) m2_pool_probe_waited();
    return r;
  }
}
#define fork m2_fork
#define waitpid m2_waitpid
#endif
#include "tests/tack_test_embed.h"
#ifdef _WIN32
#undef CreateProcessA
#undef WaitForSingleObject
#undef GetExitCodeProcess
#undef CloseHandle
#endif
#ifndef _WIN32
#undef fork
#undef waitpid
#endif

/* Keep test arguments const. Copy only the pointers for the existing
 * char ** API, never mutate the pointed-to string literals. */
static char **m2_argv_copy(const char * const *argv) {
  char **copy;
  int n, i;
  if (!argv) return 0;
  for (n = 0; argv[n]; n++) if (n >= 8191) tack_die("m2: too many test args");
  copy = (char**)xmalloc((size_t)(n + 1) * sizeof(char*));
  for (i = 0; i < n; i++) copy[i] = (char*)argv[i];
  copy[n] = 0;
  return copy;
}
static int m2_run(const char * const *argv, int verbose) {
  char **copy = m2_argv_copy(argv);
  int rc, saved;
  rc = run_argv_wait(copy, verbose);
  saved = errno;
  free(copy);
  errno = saved;
  return rc;
}
static int m2_spawn(const char * const *argv, Proc *out) {
  char **copy = m2_argv_copy(argv);
  int rc, saved;
  rc = proc_spawn_nowait(copy, out);
  saved = errno;
  free(copy);
  errno = saved;
  return rc;
}
#ifdef _WIN32
static int m2_fmt(const char * const *argv, int verbose) {
  char **copy = m2_argv_copy(argv);
  int rc, saved;
  rc = fmt_run_win32(copy, verbose);
  saved = errno;
  free(copy);
  errno = saved;
  return rc;
}
#endif

static int m2_failed;
static void m2_assert(int ok, const char *name) {
  if (!ok) { fprintf(stderr, "process_execution_test: FAIL %s\n", name); m2_failed++; }
}
static int m2_write(const char *path, const char *body) {
  FILE *f = fopen(path, "wb");
  if (!f) return 1;
  if (fputs(body, f) == EOF) { fclose(f); return 1; }
  return fclose(f) != 0;
}
/* Bounded interprocess fixture handshake: no sleeps as proof of ordering.
 * The child exits with failure if its partner's marker never appears. */
static void m2_fixture_pause(void) {
#ifdef _WIN32
  Sleep(50);
#else
  sleep(1);
#endif
}
static int m2_fixture_wait(const char *path) {
  int attempts;
#ifdef _WIN32
  const int max_attempts = 100;
#else
  const int max_attempts = 10;
#endif
  for (attempts = 0; attempts < max_attempts; attempts++) {
    if (file_exists(path)) return 0;
    m2_fixture_pause();
  }
  return 1;
}
static int m2_child(int argc, char **argv) {
  static const char *expected[] = {
    "space here", "tab\there", "", "\"quoted\"", "C:\\spaced path\\tail\\",
    "backslash\\\"quote", "&|;$\x60<>", "one", "two"
  };
  int i;
  if (argc < 2) return -1;
  if (!strcmp(argv[1], "--m2-args")) {
    if (argc != 2 + (int)(sizeof(expected)/sizeof(expected[0]))) {
      fprintf(stderr, "m2 child: argc=%d expected=%lu\n",
              argc, (unsigned long)(2u + sizeof(expected)/sizeof(expected[0])));
      return 61;
    }
    for (i = 0; i < (int)(sizeof(expected)/sizeof(expected[0])); i++) {
      if (strcmp(argv[i + 2], expected[i])) {
        fprintf(stderr, "m2 child: argv[%d] mismatch: actual=[%s] expected=[%s]\n",
                i + 2, argv[i + 2], expected[i]);
        return 62 + i;
      }
    }
    return 0;
  }
#ifdef _WIN32
  if (!strcmp(argv[1], "--m2-win-limit-ok-child"))
    return (argc == 3 && strlen(argv[2]) == 16000u) ? 0 : 69;
  if (!strcmp(argv[1], "--m2-win-limit") ||
      !strcmp(argv[1], "--m2-win-total") ||
      !strcmp(argv[1], "--m2-win-limit-ok")) {
    char *big = (char*)xmalloc(16002u);
    const char *cmd[6];
    int i, rc;
    for (i = 0; i < 16001; i++) big[i] = 'x';
    big[16001] = '\0';
    cmd[0] = argv[0];
    cmd[1] = "--m2-win-limit-ok-child";
    cmd[2] = big;
    cmd[3] = 0;
    if (!strcmp(argv[1], "--m2-win-limit-ok")) big[16000] = '\0';
    if (!strcmp(argv[1], "--m2-win-total")) {
      big[15000] = '\0';
      cmd[3] = big;
      cmd[4] = big;
      cmd[5] = 0;
    }
    rc = m2_fmt(cmd, 0);
    free(big);
    return rc;
  }
#endif
  if (!strcmp(argv[1], "--m2-exit0")) return 0;
  if (!strcmp(argv[1], "--m2-exit13")) return 13;
  if (!strcmp(argv[1], "--m2-exit127")) return 127;
#ifndef _WIN32
  if (!strcmp(argv[1], "--m2-signal")) { raise(SIGTERM); return 74; }
#endif
  if (!strcmp(argv[1], "--m2-overflow")) {
    Argv av;
    av_init(&av);
    for (i = 0; i < 8193; i++) av_push(&av, "x");
    av_free(&av);
    return 75;
  }
  if (!strcmp(argv[1], "-c")) {
    const char *out = 0;
    const char *src = argv[argc - 1];
    for (i = 1; i + 1 < argc; i++)
      if (!strcmp(argv[i], "-o")) out = argv[i + 1];
    if (!out) return 76;
    if (strstr(src, "m2_fail.c")) return 23;
    if (strstr(src, "m2_job_")) {
      const char *tag = strstr(src, "m2_job_") + 7;
      char mine[96], peer[96], id[2], other[2];
      if (tag[0] < '0' || tag[0] > '3' || strcmp(tag + 1, ".c")) return 81;
      id[0] = tag[0]; id[1] = '\0';
      other[0] = (char)('0' + ((tag[0] - '0') ^ 1)); other[1] = '\0';
      tack_copy(mine, sizeof(mine), "build/m2-process/job-");
      tack_cat(mine, sizeof(mine), id);
      tack_cat(mine, sizeof(mine), ".ready");
      tack_copy(peer, sizeof(peer), "build/m2-process/job-");
      tack_cat(peer, sizeof(peer), other);
      tack_cat(peer, sizeof(peer), ".ready");
      if (m2_write(mine, "started") != 0) return 82;
      if (m2_fixture_wait(peer) != 0) return 83;
    }
    if (strstr(src, "m2_hold.c")) {
      if (m2_write("build/m2-process/hold-ready", "started") != 0) return 87;
      if (m2_fixture_wait("build/m2-process/hold-release") != 0) return 88;
      if (m2_write("build/m2-process/hold-complete", "done") != 0) return 89;
    }
    if (strstr(src, "m2_order_fast.c")) {
      if (m2_write(out, "object") != 0) return 84;
      return m2_write("build/m2-process/order-fast.done", "done");
    }
    if (strstr(src, "m2_order_slow.c")) {
      if (m2_fixture_wait("build/m2-process/order-fast.done") != 0) return 85;
      /* Fast writes its output before publishing this marker. Slow cannot
       * complete first, independent of startup scheduling or wall-clock time. */
      if (m2_write("build/m2-process/order-slow.saw-fast", "done") != 0) return 86;
      return m2_write(out, "object");
    }
    if (strstr(src, "m2_slow.c")) {
#ifdef _WIN32
      Sleep(900);
#else
      sleep(1);
#endif
      if (m2_write("build/m2-process/slow-complete", "complete") != 0) return 77;
    }
    return m2_write(out, "object");
  }
  return -1;
}
static void m2_process_cases(const char *self) {
  const char *args[] = {0, "--m2-args", "space here", "tab\there", "",
                  "\"quoted\"", "C:\\spaced path\\tail\\",
                  "backslash\\\"quote", "&|;$\x60<>", "one", "two", 0};
  const char *ok[] = {0, "--m2-exit0", 0};
  const char *fail[] = {0, "--m2-exit13", 0};
  const char *exit127[] = {0, "--m2-exit127", 0};
  const char *over[] = {0, "--m2-overflow", 0};
  const char *missing[] = {"build/m2-process/never_exists", 0};
  const char *none[] = {0};
  const char *empty[] = {"", 0};
  Argv av;
  Proc p;
  int i;
  args[0] = self;
  ok[0] = self;
  fail[0] = self;
  exit127[0] = self;
  over[0] = self;
  m2_assert(m2_run(args, 0) == 0, "child sees exact argv and literal metacharacters");
  m2_assert(m2_run(ok, 0) == 0, "exit zero");
  m2_assert(m2_run(fail, 0) == 13, "nonzero child exit");
  m2_assert(m2_run(exit127, 0) == 127, "real child exit 127");
  m2_assert(m2_spawn(0, &p) != 0 && errno == EINVAL, "NULL argv");
  m2_assert(m2_spawn(none, &p) != 0 && errno == EINVAL, "NULL program");
  m2_assert(m2_spawn(empty, &p) != 0 && errno == EINVAL, "empty program");
  m2_assert(m2_spawn(ok, 0) != 0 && errno == EINVAL, "NULL process result");
  m2_assert(m2_run(0, 1) != 0, "verbose NULL argv handled");
  m2_assert(m2_run(missing, 0) != 0, "missing executable fails");
  av_init(&av); av_push(&av, "executable"); av_push(&av, ""); av_terminate(&av);
  m2_assert(av.n == 3 && av.a[2] == 0 && !strcmp(av.a[1], ""),
            "empty argument separated from NULL sentinel");
  av_free(&av);
  av_init(&av);
  for (i = 0; i < 8191; i++) av_push(&av, "x");
  av_terminate(&av);
  m2_assert(av.n == 8192 && av.a[8191] == 0, "bounded NULL terminated argv");
  av_free(&av);
  m2_assert(m2_run(over, 0) == 2, "oversize argv rejected");
#ifdef _WIN32
  m2_assert(m2_fmt(args, 0) == 0, "CreateProcessA exact received argv");
  m2_assert(m2_fmt(fail, 0) != 0, "formatter child error propagated");
  {
    const char *long_bad[] = {0, "--m2-win-limit", 0};
    const char *total_bad[] = {0, "--m2-win-total", 0};
    const char *long_ok[] = {0, "--m2-win-limit-ok", 0};
    long_bad[0] = self;
    total_bad[0] = self;
    long_ok[0] = self;
    m2_assert(m2_run(long_bad, 0) == 2,
              "Windows argument longer than 16000 rejected");
    m2_assert(m2_run(total_bad, 0) == 2,
              "Windows aggregate command length rejected");
    m2_assert(m2_run(long_ok, 0) == 0,
              "Windows 16000-byte argument reaches child intact");
  }
  {
    char self_path[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, self_path, MAX_PATH);
    const char *spaced = "build/m2-process/space dir/m2 child.exe";
    if (n > 0u && n < MAX_PATH) {
      const char *spaced_argv[] = {0, "--m2-args", "space here",
                            "tab\there", "", "\"quoted\"", "C:\\spaced path\\tail\\",
                            "backslash\\\"quote", "&|;$\x60<>", "one", "two", 0};
      spaced_argv[0] = spaced;
      ensure_dir_recursive("build/m2-process/space dir");
      m2_assert(CopyFileA(self_path, spaced, FALSE) != 0,
                "prepare executable with spaced path");
      m2_assert(m2_fmt(spaced_argv, 0) == 0,
                "formatter starts executable whose path contains spaces");
      m2_assert(m2_run(spaced_argv, 0) == 0,
                "general subprocess starts spaced executable with exact argv");
      m2_assert(DeleteFileA(spaced) != 0, "remove spaced child executable fixture");
    } else m2_assert(0, "query own Windows executable path");
  }
  m2_win_fail_create = 1;
  m2_win_closes = 0;
  m2_assert(m2_fmt(ok, 0) != 0 && m2_win_closes == 0,
            "CreateProcessA forced failure without leaked handles");
  m2_win_fail_create = 0;
  m2_win_fail_wait = 1;
  m2_win_closes = 0;
  m2_assert(m2_fmt(ok, 0) != 0 && m2_win_closes == 2,
            "WaitForSingleObject forced error closes both handles");
  m2_win_fail_wait = 0;
  m2_win_fail_exit = 1;
  m2_win_closes = 0;
  m2_assert(m2_fmt(ok, 0) != 0 && m2_win_closes == 2,
            "GetExitCodeProcess forced error closes both handles");
  m2_win_fail_exit = 0;
  m2_win_closes = 0;
  m2_assert(m2_fmt(ok, 0) == 0 && m2_win_closes == 2,
            "successful formatter closes both handles");
  {
    Proc p2;
    int started;
    p2.pid = -1;
    m2_assert(proc_wait(&p2) != 0, "invalid Win32 child handle rejected");
    m2_win_closes = 0;
    m2_win_fail_create = 1;
    m2_assert(m2_spawn(ok, &p2) != 0 && m2_win_closes == 0,
              "generic CreateProcessA failure propagates, no handles leaked");
    m2_win_fail_create = 0;
    m2_win_closes = 0;
    started = m2_spawn(ok, &p2) == 0;
    m2_assert(started && m2_win_closes == 1,
              "generic spawn closes thread handle, retains process handle");
    if (started) {
      m2_win_fail_wait = 1;
      m2_assert(proc_wait(&p2) != 0 && m2_win_closes == 2 && p2.pid == 0,
                "generic wait error closes process handle and resets state");
      m2_win_fail_wait = 0;
    }
    m2_win_closes = 0;
    started = m2_spawn(ok, &p2) == 0;
    m2_assert(started && m2_win_closes == 1,
              "generic spawn for exit status fault");
    if (started) {
      m2_win_fail_exit = 1;
      m2_assert(proc_wait(&p2) != 0 && m2_win_closes == 2 && p2.pid == 0,
                "generic GetExitCodeProcess fault closes process handle");
      m2_win_fail_exit = 0;
    }
    m2_win_closes = 0;
    started = m2_spawn(ok, &p2) == 0;
    m2_assert(started && m2_win_closes == 1,
              "generic spawn for handle-close fault");
    if (started) {
      m2_win_fail_close = 1;
      m2_assert(proc_wait(&p2) != 0 && m2_win_closes == 2 && p2.pid == 0,
                "generic CloseHandle failure detected and state reset");
      m2_win_fail_close = 0;
    }
    m2_win_closes = 0;
    started = m2_spawn(ok, &p2) == 0;
    m2_assert(started && m2_win_closes == 1,
              "generic spawn for success lifecycle");
    if (started) {
      m2_assert(proc_wait(&p2) == 0 && m2_win_closes == 2 && p2.pid == 0,
                "generic subprocess success closes both thread and process handles");
    }
  }
#else
  {
    const char *sig[] = {0, "--m2-signal", 0};
    char *noexec[] = {"build/m2-process/noexec", 0};
    sig[0] = self;
    m2_assert(m2_spawn(missing, &p) != 0 && errno == ENOENT,
              "failed execvp ENOENT differs from real exit 127");
    m2_assert(m2_write(noexec[0], "not executable") == 0, "permission fixture write");
    m2_assert(chmod(noexec[0], 0600) == 0, "permission fixture mode");
    m2_assert(m2_spawn(noexec, &p) != 0 && errno == EACCES,
              "failed execvp EACCES");
    m2_assert(remove(noexec[0]) == 0, "remove permission fixture");
    m2_fail_fork = 1;
    m2_assert(m2_spawn(ok, &p) != 0 && errno == EAGAIN,
              "injected fork failure propagates");
    m2_fail_fork = 0;
    if (m2_spawn(ok, &p) == 0) {
      m2_eintr_once = 1;
      m2_assert(proc_wait(&p) == 0 && m2_eintr_once == 0,
                "waitpid EINTR retry");
    } else m2_assert(0, "spawn EINTR fixture");
    if (m2_spawn(ok, &p) == 0) {
      m2_fail_wait = 1;
      m2_assert(proc_wait(&p) != 0 && m2_fail_wait == 0,
                "injected wait failure propagates");
    } else m2_assert(0, "spawn wait error fixture");
    m2_assert(m2_run(sig, 0) == 128 + SIGTERM,
              "signal failure reported");
  }
#endif
}
static void m2_pool_and_cwd(const char *self) {
  StrVec srcs, objs;
  char cmd[4096], before[1024], after[1024];
  int touched = 0, rc;
  ensure_dir_recursive("build/m2-process/obj");
  ensure_dir_recursive("build/m2-process/dep");
  m2_assert(m2_write("build/m2-process/m2_fail.c", "int fail_source;\n") == 0, "fail source");
  m2_assert(m2_write("build/m2-process/m2_slow.c", "int slow_source;\n") == 0, "slow source");
  m2_assert(m2_write("build/m2-process/m2_third.c", "int third_source;\n") == 0, "third source");
  (void)remove("build/m2-process/slow-complete");
  sv_init(&srcs); sv_init(&objs);
  sv_push(&srcs, "build/m2-process/m2_fail.c");
  sv_push(&srcs, "build/m2-process/m2_slow.c");
  sv_push(&srcs, "build/m2-process/m2_third.c");
  rc = compile_sources(self, &srcs, "build/m2-process/obj", "build/m2-process/dep",
                       0, 0, 0, 0, PROF_DEBUG, 0, 0, 1, 2, 0, &objs, &touched);
  m2_assert(rc != 0 && file_exists("build/m2-process/slow-complete"),
            "failed -j pool reaps slow running sibling");
  sv_free(&srcs); sv_free(&objs);
  sv_init(&srcs); sv_init(&objs);
  sv_push(&srcs, "build/m2-process/m2_slow.c");
  sv_push(&srcs, "build/m2-process/m2_third.c");
  rc = compile_sources(self, &srcs, "build/m2-process/obj", "build/m2-process/dep",
                       0, 0, 0, 0, PROF_DEBUG, 0, 0, 1, 2, 0, &objs, &touched);
  m2_assert(rc == 0 && objs.count == 2 &&
            file_exists(objs.items[0]) && file_exists(objs.items[1]),
            "successful -j 2 compilation with slow first child");
  sv_free(&srcs); sv_free(&objs);
  sv_init(&srcs); sv_init(&objs);
  sv_push(&srcs, "build/m2-process/m2_third.c");
  sv_push(&srcs, "build/m2-process/m2_slow.c");
  rc = compile_sources(self, &srcs, "build/m2-process/obj", "build/m2-process/dep",
                       0, 0, 0, 0, PROF_DEBUG, 0, 0, 1, 2, 0, &objs, &touched);
  m2_assert(rc == 0 && objs.count == 2 &&
            file_exists(objs.items[0]) && file_exists(objs.items[1]),
            "successful -j 2 compilation with slow last child");
  sv_free(&srcs); sv_free(&objs);
  sv_init(&srcs); sv_init(&objs);
  m2_assert(compile_sources(self, &srcs, "build/m2-process/obj", "build/m2-process/dep",
                            0, 0, 0, 0, PROF_DEBUG, 0, 0, 1, 257, 0, &objs, &touched) != 0,
            "invalid unbounded -j rejected");
  sv_free(&srcs); sv_free(&objs);
#ifdef _WIN32
  m2_assert(_getcwd(before, (int)sizeof(before)) != 0, "get Windows cwd");
#else
  m2_assert(getcwd(before, sizeof(before)) != 0, "get POSIX cwd");
#endif
  tack_copy(cmd, sizeof(cmd), "\"");
  tack_cat(cmd, sizeof(cmd), self);
  tack_cat(cmd, sizeof(cmd), "\" --m2-exit0");
  m2_assert(fmt_exec_cmd(cmd, 0, 0, 0, 0, "build/m2-process/missing-dir", 0) != 0,
            "formatter rejects invalid workdir");
  m2_assert(fmt_exec_cmd(cmd, 0, 0, 0, 0, ".", 0) == 0,
            "formatter executes in valid workdir");
#ifdef _WIN32
  m2_assert(_getcwd(after, (int)sizeof(after)) != 0, "get restored Windows cwd");
#else
  m2_assert(getcwd(after, sizeof(after)) != 0, "get restored POSIX cwd");
#endif
  m2_assert(!strcmp(before, after), "formatter restores original cwd");
}

/* P17: mutually dependent child start markers prove two genuine concurrent
 * compiler processes; parent OS wrappers count all unreaped children.
 * P19: the slow child cannot publish completion before the fast child does,
 * even when the slow child was placed first in the FIFO job queue.
 * P20: fail the second spawn with a first child already live, then assert
 * it was drained and no success metadata was committed for that child. */
static void m2_parallel_probe_cases(const char *self) {
  StrVec srcs, objs;
  char path[160], id[2], obj_name[768], meta_name[768], meta_path[1024];
  int k, rc, touched = 0;
  const char *hold_src = "build/m2-process/m2_hold.c";
  ensure_dir_recursive("build/m2-process/probe_obj");
  ensure_dir_recursive("build/m2-process/probe_dep");
  ensure_dir_recursive("build/m2-process/order_obj");
  ensure_dir_recursive("build/m2-process/order_dep");
  ensure_dir_recursive("build/m2-process/fault_obj");
  ensure_dir_recursive("build/m2-process/fault_dep");
  for (k = 0; k < 4; k++) {
    id[0] = (char)('0' + k); id[1] = '\0';
    tack_copy(path, sizeof(path), "build/m2-process/m2_job_");
    tack_cat(path, sizeof(path), id);
    tack_cat(path, sizeof(path), ".c");
    m2_assert(m2_write(path, "int m2_job;\n") == 0, "P17 job source fixture");
    tack_copy(path, sizeof(path), "build/m2-process/job-");
    tack_cat(path, sizeof(path), id);
    tack_cat(path, sizeof(path), ".ready");
    (void)remove(path);
  }
  (void)rm_rf_contents("build/m2-process/probe_obj");
  (void)rm_rf_contents("build/m2-process/probe_dep");
  sv_init(&srcs); sv_init(&objs);
  for (k = 0; k < 4; k++) {
    id[0] = (char)('0' + k); id[1] = '\0';
    tack_copy(path, sizeof(path), "build/m2-process/m2_job_");
    tack_cat(path, sizeof(path), id);
    tack_cat(path, sizeof(path), ".c");
    sv_push(&srcs, path);
  }
  m2_pool_probe_begin(2, 0);
  rc = compile_sources(self, &srcs, "build/m2-process/probe_obj",
                       "build/m2-process/probe_dep",
                       0, 0, 0, 0, PROF_DEBUG, 0, 0, 1, 2, 0, &objs, &touched);
  m2_pool_probe_end();
  m2_assert(rc == 0 && objs.count == 4 &&
            m2_pool_attempts == 4 && m2_pool_starts == 4 &&
            m2_pool_reaps == 4 && m2_pool_live == 0 &&
            m2_pool_peak == 2 && !m2_pool_violation,
            "P17 real child overlap and -j 2 strict live upper bound");
  for (k = 0; k < objs.count; k++)
    m2_assert(file_exists(objs.items[k]), "P17 four compiler output objects");
  sv_free(&srcs); sv_free(&objs);

  m2_assert(m2_write("build/m2-process/m2_order_slow.c", "int m2_order_slow;\n") == 0,
            "P19 slow completion fixture");
  m2_assert(m2_write("build/m2-process/m2_order_fast.c", "int m2_order_fast;\n") == 0,
            "P19 fast completion fixture");
  for (k = 0; k < 2; k++) {
    (void)remove("build/m2-process/order-fast.done");
    (void)remove("build/m2-process/order-slow.saw-fast");
    (void)rm_rf_contents("build/m2-process/order_obj");
    (void)rm_rf_contents("build/m2-process/order_dep");
    sv_init(&srcs); sv_init(&objs);
    if (k == 0) {
      sv_push(&srcs, "build/m2-process/m2_order_slow.c");
      sv_push(&srcs, "build/m2-process/m2_order_fast.c");
    } else {
      sv_push(&srcs, "build/m2-process/m2_order_fast.c");
      sv_push(&srcs, "build/m2-process/m2_order_slow.c");
    }
    m2_pool_probe_begin(2, 0);
    rc = compile_sources(self, &srcs, "build/m2-process/order_obj",
                         "build/m2-process/order_dep",
                         0, 0, 0, 0, PROF_DEBUG, 0, 0, 1, 2, 0, &objs, &touched);
    m2_pool_probe_end();
    m2_assert(rc == 0 && objs.count == 2 &&
              file_exists("build/m2-process/order-fast.done") &&
              file_exists("build/m2-process/order-slow.saw-fast") &&
              m2_pool_starts == 2 && m2_pool_reaps == 2 &&
              m2_pool_live == 0 && m2_pool_peak == 2 &&
              !m2_pool_violation,
              "P19 child-enforced fast-before-slow finish, both launch orders");
    if (objs.count == 2)
      m2_assert(file_exists(objs.items[0]) && file_exists(objs.items[1]),
                "P19 successful objects for both reversed launch orders");
    sv_free(&srcs); sv_free(&objs);
  }

  (void)rm_rf_contents("build/m2-process/fault_obj");
  (void)rm_rf_contents("build/m2-process/fault_dep");
  (void)remove("build/m2-process/hold-ready");
  (void)remove("build/m2-process/hold-release");
  (void)remove("build/m2-process/hold-complete");
  m2_assert(m2_write(hold_src, "int m2_hold;\n") == 0,
            "P20 hold source fixture");
  sv_init(&srcs); sv_init(&objs);
  sv_push(&srcs, hold_src);
  sv_push(&srcs, "build/m2-process/m2_third.c");
  sanitize_path_to_id(obj_name, sizeof(obj_name), hold_src);
  tack_copy(meta_name, sizeof(meta_name), obj_name);
  tack_cat(meta_name, sizeof(meta_name), ".meta");
  path_join(meta_path, sizeof(meta_path), "build/m2-process/fault_dep", meta_name);
  m2_pool_probe_begin(2, 2);
  rc = compile_sources(self, &srcs, "build/m2-process/fault_obj",
                       "build/m2-process/fault_dep",
                       0, 0, 0, 0, PROF_DEBUG, 0, 0, 1, 2, 0, &objs, &touched);
  m2_pool_probe_end();
  m2_assert(rc != 0 && m2_pool_attempts == 2 &&
            m2_pool_starts == 1 && m2_pool_reaps == 1 &&
            m2_pool_live == 0 && m2_pool_peak == 1 &&
            m2_pool_hold_observed && !m2_pool_violation &&
            file_exists("build/m2-process/hold-complete") &&
            !file_exists(meta_path),
            "P20 second spawn fails while first runs, then drains without metadata");
  sv_free(&srcs); sv_free(&objs);
}
int main(int argc, char **argv) {
  int rc;
  if (argc > 1) {
    rc = m2_child(argc, argv);
    if (rc >= 0) return rc;
  }
  if (!argv || !argv[0] || !argv[0][0]) return 1;
  ensure_dir_recursive("build/m2-process");
  m2_process_cases(argv[0]);
  m2_pool_and_cwd(argv[0]);
  m2_parallel_probe_cases(argv[0]);
  if (m2_failed) {
    fprintf(stderr, "process_execution_test: %d failures\n", m2_failed);
    return 1;
  }
  puts("process_execution_test: ok");
  return 0;
}
