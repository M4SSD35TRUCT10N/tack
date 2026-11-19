# M2 RC0.7 – Windows-Coverage-Produktionsstate-Fingerprint reparieren

**Milestone:** ausschließlich tack v0.8.6 M2 / P32.
**Datum:** 19. November 2026.
**Baseline:** RC0.6a Windows-Gates PASS auf Quellenstand `1ba915f` und versioniertem Owner-Inventar `7233bed7f6edfea524eb605d46d37d39eca4be60`.
**Status:** RC0.7-Quell-/Testkandidat, **alle neuen Gates NOT RUN**.

## Deutsch

### Fehlerbild

Windows-GCC/gcov ergibt bei echten 28 Tests **3845/6075** (27 PASS / 0 FAIL / 1 SKIP), aber `source_state=811c9dc5` bleibt exakt der initiale FNV-Wert, selbst nach Quelländerungen. Scannerpfade werden mit Backslashes gebaut, `cov_is_production()` filtert nur `src/` und `include/`. Dadurch fehlen Produktions-Bytes in der Windows-Frischeprüfung. Siehe ADR 0005.

### Einziger erlaubter Eingriff

- In `src/tack.c` die interne `cov_is_production`-Klassifizierung auf beide Separatoren `/` und `\` erweitern, `include/tests` weiterhin ausschließen.
- In `tests/coverage_ratchet_test.c` die Positiv-/Negativtests für Windows- und POSIX-Pfade sowie die tatsächliche nichtleere Produktions-Workspace-State-Berechnung ergänzen.
- Keine Änderung an Build-, Spawn-/Wait-/Pool-Algorithmen, C89, Clang/MSVC-C89-Guards oder bisherigen Assertions.
- Keine Compilerflags oder neue Abhängigkeiten, keine neuen Milestones.

### Owner-Gates

1. Owner prüft `git diff` dieser zwei Quelldateien und alle vier noch vorhandenen Testfingerprints.
2. `tack --config .\tack.ini ratchet tests` muss zunächst wegen `coverage_ratchet_test.c` FAIL melden; **nur der Owner** darf danach `ratchet tests --owner-approve` ausführen.
3. Windows TinyCC Debug, GCC Release, Clang/MSVC Release und GCC Coverage jeweils `test ... --ci --rebuild -j 1`, `27 PASS / 0 FAIL / 1 SKIP` erwartet, nicht als nachgewiesen ausgeben.
4. `coverage/current.txt` muss auf Windows echte Messung sowie **`source_state != 811c9dc5`** nachweisen. Regression der Ratio wird nicht akzeptiert. Vergleiche aktuelle HWM 3845/6075, lasse das HWM bei unveränderter Ratio unangetastet; bei messbarer Steigerung nur separate Owner-Freigabe der HWM.
5. Erneute Linux-GCC/Clang-Referenzsuite und gcc/gcov 14.2.0-Messung, Linux-HWM 3604/5790 weiterhin geschützt.
6. Dokumentiere jeweilige PASS/FAIL/SKIP/NOT RUN; PR #13 bleibt Draft und M2 bleibt offen, bis alle kritischen Gates bestanden sind.

## English

Fix the Windows production source freshness fingerprint by accepting native Windows and POSIX separators in `cov_is_production` and regression-testing the actual nonempty workspace state. Owner tests, test inventory acceptance and gcov measurements are required; the accepted Windows and Linux HWMs remain untouched. No C89, warning, test or build-system relaxation; no automatic merge/release.
