/* M2 process execution: real child-received argv, errors and -j cleanup.
 * ISO C89; fault wrappers are compiled only into this test. */
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <errno.h>
static int m2_win_fail_create, m2_win_fail_wait, m2_win_fail_exit, m2_win_fail_cwait;
static int m2_win_closes;
static BOOL WINAPI m2_wrap_create(LPCSTR app, LPSTR cmd,
    LPSECURITY_ATTRIBUTES proc_attr, LPSECURITY_ATTRIBUTES thread_attr,
    BOOL inherit, DWORD flags, LPVOID env, LPCSTR cwd,
    LPSTARTUPINFOA startup, LPPROCESS_INFORMATION info) {
  if (m2_win_fail_create) { SetLastError(ERROR_ACCESS_DENIED); return FALSE; }
  return CreateProcessA(app, cmd, proc_attr, thread_attr, inherit, flags,
                        env, cwd, startup, info);
}
static DWORD WINAPI m2_wrap_wait(HANDLE process, DWORD timeout) {
  if (m2_win_fail_wait) { SetLastError(ERROR_INVALID_HANDLE); return WAIT_FAILED; }
  return WaitForSingleObject(process, timeout);
}
static BOOL WINAPI m2_wrap_exit(HANDLE process, LPDWORD exit_code) {
  if (m2_win_fail_exit) { SetLastError(ERROR_INVALID_HANDLE); return FALSE; }
  return GetExitCodeProcess(process, exit_code);
}
static BOOL WINAPI m2_wrap_close(HANDLE handle) {
  m2_win_closes++;
  return CloseHandle(handle);
}
static intptr_t m2_wrap_cwait(int *status, intptr_t process, int action) {
  if (m2_win_fail_cwait) { errno = ECHILD; return -1; }
  return _cwait(status, process, action);
}
#define CreateProcessA m2_wrap_create
#define WaitForSingleObject m2_wrap_wait
#define GetExitCodeProcess m2_wrap_exit
#define CloseHandle m2_wrap_close
#define _cwait m2_wrap_cwait
#endif
#ifndef _WIN32
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
static int m2_fail_fork, m2_eintr_once, m2_fail_wait;
static pid_t m2_fork(void) {
  if (m2_fail_fork) { errno = EAGAIN; return (pid_t)-1; }
  return fork();
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
  return waitpid(pid, status, flags);
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
#undef _cwait
#endif
#ifndef _WIN32
#undef fork
#undef waitpid
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
static int m2_child(int argc, char **argv) {
  static const char *expected[] = {
    "space here", "tab\there", "", "\"quoted\"", "C:\\spaced path\\tail\\",
    "backslash\\\"quote", "&|;$\x60<>", "one", "two"
  };
  int i;
  if (argc < 2) return -1;
  if (!strcmp(argv[1], "--m2-args")) {
    if (argc != 2 + (int)(sizeof(expected)/sizeof(expected[0]))) return 61;
    for (i = 0; i < (int)(sizeof(expected)/sizeof(expected[0])); i++)
      if (strcmp(argv[i + 2], expected[i])) return 62 + i;
    return 0;
  }
#ifdef _WIN32
  if (!strcmp(argv[1], "--m2-win-limit-ok-child"))
    return (argc == 3 && strlen(argv[2]) == 16000u) ? 0 : 69;
  if (!strcmp(argv[1], "--m2-win-limit") ||
      !strcmp(argv[1], "--m2-win-total") ||
      !strcmp(argv[1], "--m2-win-limit-ok")) {
    char *big = (char*)xmalloc(16002u);
    char *cmd[6];
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
    rc = fmt_run_win32(cmd, 0);
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
  char *args[] = {(char*)self, "--m2-args", "space here", "tab\there", "",
                  "\"quoted\"", "C:\\spaced path\\tail\\",
                  "backslash\\\"quote", "&|;$\x60<>", "one", "two", 0};
  char *ok[] = {(char*)self, "--m2-exit0", 0};
  char *fail[] = {(char*)self, "--m2-exit13", 0};
  char *exit127[] = {(char*)self, "--m2-exit127", 0};
  char *over[] = {(char*)self, "--m2-overflow", 0};
  char *missing[] = {"build/m2-process/never_exists", 0};
  char *none[] = {0};
  char *empty[] = {"", 0};
  Argv av;
  Proc p;
  int i;
  m2_assert(run_argv_wait(args, 0) == 0, "child sees exact argv and literal metacharacters");
  m2_assert(run_argv_wait(ok, 0) == 0, "exit zero");
  m2_assert(run_argv_wait(fail, 0) == 13, "nonzero child exit");
  m2_assert(run_argv_wait(exit127, 0) == 127, "real child exit 127");
  m2_assert(proc_spawn_nowait(0, &p) != 0 && errno == EINVAL, "NULL argv");
  m2_assert(proc_spawn_nowait(none, &p) != 0 && errno == EINVAL, "NULL program");
  m2_assert(proc_spawn_nowait(empty, &p) != 0 && errno == EINVAL, "empty program");
  m2_assert(proc_spawn_nowait(ok, 0) != 0 && errno == EINVAL, "NULL process result");
  m2_assert(run_argv_wait(0, 1) != 0, "verbose NULL argv handled");
  m2_assert(run_argv_wait(missing, 0) != 0, "missing executable fails");
  av_init(&av); av_push(&av, "executable"); av_push(&av, ""); av_terminate(&av);
  m2_assert(av.n == 3 && av.a[2] == 0 && !strcmp(av.a[1], ""),
            "empty argument separated from NULL sentinel");
  av_free(&av);
  av_init(&av);
  for (i = 0; i < 8191; i++) av_push(&av, "x");
  av_terminate(&av);
  m2_assert(av.n == 8192 && av.a[8191] == 0, "bounded NULL terminated argv");
  av_free(&av);
  m2_assert(run_argv_wait(over, 0) == 2, "oversize argv rejected");
#ifdef _WIN32
  m2_assert(fmt_run_win32(args, 0) == 0, "CreateProcessA exact received argv");
  m2_assert(fmt_run_win32(fail, 0) != 0, "formatter child error propagated");
  {
    char *long_bad[] = {(char*)self, "--m2-win-limit", 0};
    char *total_bad[] = {(char*)self, "--m2-win-total", 0};
    char *long_ok[] = {(char*)self, "--m2-win-limit-ok", 0};
    m2_assert(run_argv_wait(long_bad, 0) == 2,
              "Windows argument longer than 16000 rejected");
    m2_assert(run_argv_wait(total_bad, 0) == 2,
              "Windows aggregate command length rejected");
    m2_assert(run_argv_wait(long_ok, 0) == 0,
              "Windows 16000-byte argument reaches child intact");
  }
  {
    char self_path[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, self_path, MAX_PATH);
    const char *spaced = "build/m2-process/space dir/m2 child.exe";
    if (n > 0u && n < MAX_PATH) {
      char *spaced_argv[] = {(char*)spaced, "--m2-args", "space here",
                            "tab\there", "", "\"quoted\"", "C:\\spaced path\\tail\\",
                            "backslash\\\"quote", "&|;$\x60<>", "one", "two", 0};
      ensure_dir_recursive("build/m2-process/space dir");
      m2_assert(CopyFileA(self_path, spaced, FALSE) != 0,
                "prepare executable with spaced path");
      m2_assert(fmt_run_win32(spaced_argv, 0) == 0,
                "CreateProcessA starts executable whose path contains spaces");
      m2_assert(DeleteFileA(spaced) != 0, "remove spaced child executable fixture");
    } else m2_assert(0, "query own Windows executable path");
  }
  m2_win_fail_create = 1;
  m2_win_closes = 0;
  m2_assert(fmt_run_win32(ok, 0) != 0 && m2_win_closes == 0,
            "CreateProcessA forced failure without leaked handles");
  m2_win_fail_create = 0;
  m2_win_fail_wait = 1;
  m2_win_closes = 0;
  m2_assert(fmt_run_win32(ok, 0) != 0 && m2_win_closes == 2,
            "WaitForSingleObject forced error closes both handles");
  m2_win_fail_wait = 0;
  m2_win_fail_exit = 1;
  m2_win_closes = 0;
  m2_assert(fmt_run_win32(ok, 0) != 0 && m2_win_closes == 2,
            "GetExitCodeProcess forced error closes both handles");
  m2_win_fail_exit = 0;
  m2_win_closes = 0;
  m2_assert(fmt_run_win32(ok, 0) == 0 && m2_win_closes == 2,
            "successful formatter closes both handles");
  {
    Proc invalid;
    invalid.pid = -1;
    m2_win_fail_cwait = 1;
    m2_assert(proc_wait(&invalid) != 0, "injected _cwait failure");
    m2_win_fail_cwait = 0;
  }
#else
  {
    char *sig[] = {(char*)self, "--m2-signal", 0};
    char *noexec[] = {"build/m2-process/noexec", 0};
    m2_assert(proc_spawn_nowait(missing, &p) != 0 && errno == ENOENT,
              "failed execvp ENOENT differs from real exit 127");
    m2_assert(m2_write(noexec[0], "not executable") == 0, "permission fixture write");
    m2_assert(chmod(noexec[0], 0600) == 0, "permission fixture mode");
    m2_assert(proc_spawn_nowait(noexec, &p) != 0 && errno == EACCES,
              "failed execvp EACCES");
    m2_assert(remove(noexec[0]) == 0, "remove permission fixture");
    m2_fail_fork = 1;
    m2_assert(proc_spawn_nowait(ok, &p) != 0 && errno == EAGAIN,
              "injected fork failure propagates");
    m2_fail_fork = 0;
    if (proc_spawn_nowait(ok, &p) == 0) {
      m2_eintr_once = 1;
      m2_assert(proc_wait(&p) == 0 && m2_eintr_once == 0,
                "waitpid EINTR retry");
    } else m2_assert(0, "spawn EINTR fixture");
    if (proc_spawn_nowait(ok, &p) == 0) {
      m2_fail_wait = 1;
      m2_assert(proc_wait(&p) != 0 && m2_fail_wait == 0,
                "injected wait failure propagates");
    } else m2_assert(0, "spawn wait error fixture");
    m2_assert(run_argv_wait(sig, 0) == 128 + SIGTERM,
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
  if (m2_failed) {
    fprintf(stderr, "process_execution_test: %d failures\n", m2_failed);
    return 1;
  }
  puts("process_execution_test: ok");
  return 0;
}
