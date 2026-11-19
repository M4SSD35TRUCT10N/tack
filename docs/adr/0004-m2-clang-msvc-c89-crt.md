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

### Prüf- und Freigabegates

Der erste Clang-MSC-Fehlerbericht enthält nur die ersten 20 Diagnosen; weitere Compiler-/ABI-/Linkerfehler sind möglich. **RC0.6 Windows TinyCC/GCC/Clang und Linux GCC/Clang/gcov sind NOT RUN.** Die Quell-/Teständerungen ändern Ratchet-Fingerprints. Die Besitzer-freigegebene Windows-HWM **3845/6075** und Linux-HWM **3604/5790** sowie das 28er-Inventar bleiben **unverändert**, bis echte neue Messungen, diff-Review und separate explizite Owner-Annahme vorliegen. Kein HWM-Absenken, keine stillen Annahmen, kein PR-Merge oder Release.

## English

Windows Clang 23.1.5 uses the MSVC UCRT, whose vendor deprecation annotations on standard C89 CRT APIs fail under `-Werror`. The narrow, explicit `_WIN32 && _MSC_VER && __clang__` CRT compatibility guard defines `_CRT_SECURE_NO_WARNINGS` **before all CRT headers**, preserving ISO C89 standard APIs and all language warnings. This opt-out suppresses only Microsoft's vendor-specific CRT deprecation annotations, and is documented rather than hidden. Standalone tack and embedded/early-header tests are guarded; normal C89 builds and assertions are unchanged. RC0.6 has not been tested. Existing Windows/Linux coverage HWMs and owner-gated ratchets remain unchanged and must not be automatically rewritten.
