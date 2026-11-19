# ADR 0004 – Clang-MSVC-UCRT und ISO C89 / Clang MSVC CRT and ISO C89

**Datum / Date:** 2026-11-19
**Status:** RC0.6 Candidate, NOT RUN / Owner-Abnahme ausstehend
**Scope:** tack v0.8.6 M2 only; no release, no M3–M8.

## Deutsch

### Problem und Owner-Nachweis

Das vom Assistenten als Linux-Gate vorgesehene Clang wurde vom Owner unter Windows ausgeführt. `clang 23.1.5` meldet `Target: x86_64-pc-windows-msvc`. Microsofts UCRT annotiert C89-Standardfunktionen `fopen`, `getenv`, `strerror` als veraltet (`[-Wdeprecated-declarations]`). Unter unverändertem `-Werror` endete der erste Windows-Clang-Testlauf mit 20 derartigen Diagnosen und `TACK_SUMMARY status=failed total=28 compiled=1 passed=0 failed=1 skipped=0 not_run=27`. Dieser Lauf ist keine Linux-Validierung. Die umgebenden Windows-GCC-Debug/Release/Coverage-Läufe waren jeweils `28 compiled / 27 PASS / 0 FAIL / 1 SKIP`; Windows HWM `3845/6075` blieb PASS.

### Entscheidung

ISO C89 bleibt verbindlich. Standard-C89-APIs werden **nicht** durch herstellerspezifische beziehungsweise spätere `_s`-APIs (`fopen_s`, `_dupenv_s`, `strerror_s`) ersetzt.

Bei `defined(_WIN32) && defined(_MSC_VER) && defined(__clang__)` wird die Microsoft-UCRT-Option `_CRT_SECURE_NO_WARNINGS` ausdrücklich **vor allen CRT- bzw. Windows-Systemheadern** definiert, sofern noch nicht gesetzt. Dies schaltet ausschließlich die Microsoft's vendor-only 'unsafe'-Deprecation-Markierung ab, nicht etwa `-Werror`, `-Wall`, andere Compiler-Diagnosen, Bounds-Checks oder Projekt-Assertions. Die Ausnahme ist mit Kommentar an jeder Verwendungsstelle offen gelegt, nicht versteckt. Sie ist mit GCC, TinyCC und Linux inaktiv.

Die Einzeldatei `src/tack.c` erhält den Guard ohne weitere Includes, um den Single-File-Contract beizubehalten. Für die Embedded-Tests steht der Guard vor dem ersten Include in `include/tests/tack_test_embed.h`; drei bestehende Testdateien mit eigenem frühem CRT/Windows-Include erhalten ihn zusätzlich ganz am Anfang: `tests/docs_bilingual_contract_test.c`, `tests/path_filesystem_safety_test.c` und `tests/process_execution_test.c`. Keine Prozessimplementierung und keine Testassertion wird geändert.

### RC0.6a – Getrennte Windows-/POSIX-Namenszuordnung in Cache-Test, 19.11.2026

Owner-Folgeprobe nach Checkout von `2528f131da1b472f35989d3aea20c5448e281eaa`: `git diff --check 65544f2..HEAD` ohne Befund. Die direkte Prüfung `clang -std=c89 -pedantic-errors -Wextra -Werror -c src\tack.c -o build\tack-clang-msvc-probe.obj` **PASS** (keine Fehlermeldung). Die direkte Probe von `tests/cache_restore_after_clean_test.c` mit `-Iinclude -Isrc` **FAIL** an `getcwd` (UCRT `_CRT_NONSTDC_DEPRECATE(_getcwd)`; `[-Werror,-Wdeprecated-declarations]`). Das zeigt, dass das vorherige Clang-UCRT-`fopen/getenv/strerror`-Problem beim eigenständigen Produktionscode beseitigt ist, **nicht**, dass die gesamte Clang-Test-Suite bereits PASS ist.

**Entscheidung:** Im Cache-Restoration-Test ersetzt eine C89-konforme `read_cwd_local`-Hilfsfunktion den unbedingten Aufruf des POSIX-Namens `getcwd`: unter `_WIN32` `_getcwd` mit `capacity <= INT_MAX` und explizitem Cast auf `int`, unter POSIX weiterhin `getcwd(..., size_t)`. Das ist bereits das projekttypische Muster bei anderen Tests. Kein globales `_CRT_NONSTDC_NO_WARNINGS`, kein Wegdrücken von POSIX-/CRT-Warnungen, keine Änderung an `src/tack.c`, Prozesslogik oder Assertions.

**Gate-Status:** Direkte Clang-Kompilierung von `src/tack.c` auf RC0.6 **PASS**; direkte Kompilierung des Cache-Tests auf RC0.6 **FAIL**. **RC0.6a Änderung zunächst NOT RUN**. Vollständige Windows- und Linux-Suites sowie neue Coverage weiterhin NOT RUN. Das geänderte Testinventar bleibt Owner-freigabepflichtig, die bestehende Windows-HWM 3845/6075 und Linux-HWM 3604/5790 unverändert.
### Prüf- und Freigabegates

Der erste Clang-MSC-Fehlerbericht enthält nur die ersten 20 Diagnosen; weitere Compiler-/ABI-/Linkerfehler sind möglich. **RC0.6 Windows TinyCC/GCC/Clang und Linux GCC/Clang/gcov sind NOT RUN.** Die Quell-/Teständerungen ändern Ratchet-Fingerprints. Die Besitzer-freigegebene Windows-HWM **3845/6075** und Linux-HWM **3604/5790** sowie das 28er-Inventar bleiben **unverändert**, bis echte neue Messungen, diff-Review und separate explizite Owner-Annahme vorliegen. Kein HWM-Absenken, keine stillen Annahmen, kein PR-Merge oder Release.

## English

Windows Clang 23.1.5 uses the MSVC UCRT, whose vendor deprecation annotations on standard C89 CRT APIs fail under `-Werror`. The narrow, explicit `_WIN32 && _MSC_VER && __clang__` CRT compatibility guard defines `_CRT_SECURE_NO_WARNINGS` **before all CRT headers**, preserving ISO C89 standard APIs and all language warnings. This opt-out suppresses only Microsoft's vendor-specific CRT deprecation annotations, and is documented rather than hidden. Standalone tack and embedded/early-header tests are guarded; normal C89 builds and assertions are unchanged. RC0.6 has not been tested. Existing Windows/Linux coverage HWMs and owner-gated ratchets remain unchanged and must not be automatically rewritten.

### English: RC0.6a Windows getcwd test compatibility

Owner direct compilation of `src/tack.c` under Windows Clang 23.1.5 with `-std=c89 -pedantic-errors -Wextra -Werror` passed. The first embedded test then failed because MSVC UCRT deprecates the POSIX alias `getcwd`. The cache test now selects native `_getcwd` on Windows with a checked `size_t` to `int` conversion and `getcwd` on POSIX, without turning off `_CRT_NONSTDC_DEPRECATE` warnings globally. This candidate has not yet been rerun. Full Windows/Linux suites and fresh test/coverage ratchets are still outstanding.
