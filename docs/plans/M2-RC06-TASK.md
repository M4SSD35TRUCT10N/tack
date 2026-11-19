# M2 RC0.6 – Windows Clang/MSVC-UCRT C89 compatibility task

**2026-11-19 | Scope: M2 only | Base:** `65544f2fa56a978979c6ee4e3564432612d0f5e8`
**Status:** untested candidate. ADR 0004 describes decision.

**Goal:** Preserve ISO C89 CRT calls and strict warnings with Clang 23.1.5 targeting Windows MSVC UCRT, which deprecates `fopen/getenv/strerror`. User log: Windows clang 1/28 compiled, 1 fail, 27 not run, 20 deprecated warnings. Windows GCC RC0.5 still 27 PASS/0 FAIL/1 SKIP and accepted 3845/6075. Not a Linux run.

**Changes permitted:** Before any CRT/WinSDK headers, define `_CRT_SECURE_NO_WARNINGS` only under `_WIN32 && _MSC_VER && __clang__` in standalone `src/tack.c`, shared test embed and three early-header test sources. No change to tests' behavior, -Werror, process execution, build tool, dependencies, or M3–M8.

**Owner gates:** review diff; new `ratchet tests` mismatch is expected until `ratchet tests --owner-approve`. Windows TinyCC debug, GCC release, Windows Clang-MSVC release (not Linux), GCC coverage `--ci --rebuild -j 1`, inspect outputs and comparability; only on Owner approval accept any measured increase. True Linux GCC/Clang suites, Linux GCC 14.2 gcov and platform-specific ratchets remain mandatory. A changed denominator or regression is a hard failure, never auto-accept or lower HWM.

**Delivery:** GitHub feature branch, signed Conventional Commit, Draft PR #13; no merge/tag/release and no automatic ratchet adoption.

## English

Use a documented MSVC-CRT-only opt-out instead of replacing ISO C89 functions or globally disabling warnings. Clang/MSVC and Linux gates remain NOT RUN until real owner execution, all ratchets require fresh explicit approval.

## RC0.6a first direct Clang probe, 2026-11-19

Owner at `2528f13`: standalone strict C89 Clang/MSVC `src/tack.c` compilation **PASS**. Embedded `tests/cache_restore_after_clean_test.c` compilation **FAIL** on POSIX `getcwd` deprecated by MSVC UCRT (`_CRT_NONSTDC_DEPRECATE(_getcwd)`). Candidate RC0.6a selects `_getcwd` under `_WIN32` with `capacity <= INT_MAX` guard; POSIX continues `getcwd`. No `_CRT_NONSTDC_NO_WARNINGS`, no `-Werror` change, no weakening of assertions. **RC0.6a not yet run**, full Windows and Linux tests, coverage and ratchet acceptance pending; current accepted HWM Windows 3845/6075, Linux 3604/5790 unchanged.
