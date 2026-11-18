# tack v0.8.6 M1 RC0.4 – Win32 enumeration-fault regression candidate

**Date:** 2026-11-18. **Status:** IMPLEMENTED / Windows GCC + TinyCC OWNER RETEST REQUIRED. Not an accepted milestone. M1-only change, ISO C89, no additional build dependency.

## Trigger and defect

Owner accepted Windows HWM 3708/5996 and Linux HWM 3604/5790 for the *previous* RC0.3 source/test revision. Real Windows junction scan and `clean`/`clobber`, `--unsafe-paths` CLI, test inventory and Windows coverage check passed on RC0.3. Critical requirement S15 still lacked forced Win32 directory-enumeration failures.

Inspection found that `rm_rf_depth` on Windows fell through to `RemoveDirectoryA` when `FindFirstFileA` failed, even when the failure was ACCESS_DENIED rather than the expected empty-directory FILE_NOT_FOUND. On an empty directory, that could report success after an enumeration failure. **RC0.4 fixes this:** accept only `ERROR_FILE_NOT_FOUND`/`ERROR_NO_MORE_FILES` as an empty enumeration; any other error records failure and returns 1 before removal.

## Deterministic Windows tests

`tests/path_filesystem_safety_test.c` now intercepts `FindFirstFileA` and `FindNextFileA` **inside its embedded C89 test translation unit only**. Win32 API wrappers normally pass through to the real APIs. In armed probes they inject one `ERROR_ACCESS_DENIED` using `SetLastError` and return failure. The actual production code is exercised, not a separate imitation.

- **Scanner:** child-process probes `--scan-enum-first` and `--scan-enum-next` each must exit 2 through `tack_die`.
- **`rm_rf_contents`:** injected first and next enumeration errors each must return 1, confirm the wrapper fired exactly once, and preserve the directory as appropriate.
- **`rm_rf_depth`:** injected first-error on an **empty directory** must return 1 **without removing it**; injected next-error must return 1. The normal clean-up after injection must succeed.
- External sentinel file must remain intact; all fixtures are scoped to isolated `build/m1_fs_inside_<pid>`.
- Positive, negative, symlink, junction, lock and existing assertions from RC0.3 are not weakened or removed.

**Boundary:** the fault-injection wrappers are not installed in the shipping `tack.exe` (they exist only in the test). API-level error injection is deterministic; this is not proof of race-free file-system traversal. Concurrent link replacement (TOCTOU) remains outside M1.

## Actual validations performed

| Gate | Status | Evidence |
| --- | --- | --- |
| Linux GCC 14.2.0 strict `-std=c89 -pedantic-errors -Wall -Wextra -Werror` | PASS | Production and modified M1 test compile |
| Linux Clang strict C89, M1 test | PASS | M1 test compiles and runs |
| Linux direct M1 regression | PASS | `path_filesystem_safety_test: ok` |
| Linux direct GCC C89 inventory | PASS / 1 platform SKIP | All 27 test sources compile; 26 execute PASS; `path_join_test` exits 77 (Windows-only) |
| Windows GCC 10.3.0 M1 S15 injected probes | NOT RUN | Owner must execute native Windows test |
| Windows TinyCC 0.9.27 M1 S15 injected probes | NOT RUN | Owner must execute native Windows test |
| Official Windows/Linux release, coverage, HWM check with RC0.4 | BLOCKED / NOT RUN | New test/source fingerprint requires Owner review and fresh measurement |

**Ratchet rule:** RC0.3 owner-accepted Windows and Linux HWM files and test inventory are deliberately preserved **byte-for-byte** in RC0.4. The changed M1 test source intentionally causes `ratchet tests` to block until Owner reviews and explicitly accepts the new fingerprint. Because `src/tack.c` also changed, both OS coverage profiles must be remeasured; never silently lower either HWM. No `coverage accept` is authorized by this candidate. No Git commits, pushes, merges or tags performed.

## Windows CMD acceptance sequence (after installing RC0.4 files)

```bat
cd /d C:\Projects\tack
set "TACK_CC=gcc"
gcc -std=c89 -pedantic-errors -Wall -Wextra -Werror -I. -Iinclude -Isrc tests\path_filesystem_safety_test.c -o build\m1_s15_gcc_probe.exe && build\m1_s15_gcc_probe.exe
set "TACK_CC="
tcc -I. -Iinclude -Isrc -o build\m1_s15_tcc_probe.exe tests\path_filesystem_safety_test.c && build\m1_s15_tcc_probe.exe
```

Expected both: `path_filesystem_safety_test: ok` and exit 0. Only after reviewing the test diff and both direct probes, Owner may run `tack --config .\tack.ini ratchet tests --owner-approve`, then `tack --config .\tack.ini ratchet tests`. Run TinyCC debug, GCC release and GCC coverage as full CI gates. Record *new* covered/total and platform fingerprint data. The existing accepted Windows HWM 3708/5996 and Linux HWM 3604/5790 are a nondecreasing baseline, not automatic new approvals.
