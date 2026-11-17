# SPEC 0012 — Formatter INI Section Isolation / Trennung der Formatter-INI-Sektionen

**Status:** 0.8.5-dev RC0.2 IMPLEMENTED ON LINUX / WINDOWS NOT RUN / RELEASE NOT APPROVED.

## Deutsch

### RC0.3-Nachprüfung (17.11.2026)
Windows TDM-GCC 10.3.0 Release/Debug wurden vom Owner mit je 26 kompiliert, 25 PASS und 1 SKIP bestätigt. Die Windows-Coverage-HWM 3434/5827 (58,93256 %) wurde vom Owner nach echtem GCC/gcov-Lauf angenommen und mit erneutem PASS bestätigt. Externe Formatterausführung auf Linux wurde isoliert mit echtem Prozess und Check-vorher/Formatieren/Check-nachher geprüft; Windows externes E2E bleibt NOT RUN. Siehe [Integrationsleitfaden](../guides/formatter-external-validation.md). Die Linux-HWM bleibt unverändert (3228/5708); 3470/5708 ist ein gemessener Kandidat.


### Fehler und Vertragsgrenze

In tack 0.8.4 lagen die Schlüsselbehandlungen von `[fmt]` und `[fmt "NAME"]` innerhalb des Blocks `if (sec == SEC_SBOM)`, also hinter einer gegenseitig ausschließenden Bedingung. Ein gültiger Formatter-INI-Abschnitt wurde daher zwar als Sektion erkannt, seine Schlüssel aber stillschweigend ignoriert. `fmt --list --strict` endete mit `missing cmd`.

### Verbindliches Verhalten

- `[sbom]`, `[fmt]` und `[fmt "NAME"]` sind getrennte, sich nicht überlappende Parserzweige. Ein gültiger Abschnitt darf Werte anderer Sektionen nicht überschreiben.
- Die existierenden Formatter-Schlüssel `enabled`, `fail_fast`, `defaults`, `rules`, `exclude`, `tool`, `mode`, `include`, `cmd`, `requires`, `workdir`, `check_cmd`, `diff_cmd` und `env.*` werden gemäß bestehender Parserstruktur verarbeitet.
- `fmt --list --strict` muss mit einer vollständigen gültigen Formatterkonfiguration erfolgreich sein, ohne einen Formatter zu starten oder Quellcode zu verändern.
- Unbekannte Schlüssel und ungültige `mode`-Angaben werden unter `--strict` abgewiesen. Testdateinamen mit Leerzeichen dürfen nicht fehlerhaft zerlegt werden.
- `check_cmd`, `diff_cmd`, `env.*` und `fail_fast = no` werden durch diese Parserkorrektur **nicht** zu neuen ausführbaren Funktionen. Externe Formatter bleiben optionale, explizite Werkzeuge.
- Das Testinventar wird nur nach vollständigem Diff-Review aktualisiert; eine gemessene Coverage-HWM darf nicht abgesenkt werden. OS-Gates bleiben unabhängig.

### Testnachweis und noch offene Gates

`tests/fmt_ini_parser_test.c` prüft vollständiges `[fmt]`-Parsing, getrenntes `[sbom]`, `fmt --list --strict`, Ausschlussmuster, Leerzeichen in Kandidatenpfaden sowie Negativfälle. Linux GCC 14.2.0: Release 26 kompiliert / 25 PASS / 1 SKIP / 0 FAIL. Instrumentierter Linux-GCC/gcov-Kandidat: 3470/5708 (60,79187 %), höhere HWM **nicht angenommen**; Coverage-Gate `BLOCKED improved coverage requires HWM increase`. Windows/TDM-GCC und echte externe Formatterausführung: `NOT RUN`. Kein Release-Tag oder neuer Versionseintrag behauptet.

## English

### RC0.3 follow-up verification (2026-11-17)
Owner Windows TDM-GCC 10.3.0 release/debug suites each compiled 26 tests with 25 PASS and one SKIP. Owner accepted measured Windows coverage HWM 3434/5827 (58.93256%) and confirmed PASS in a fresh run. Real external formatter execution passed on Linux with isolated list/check-before/write/check-after; external Windows E2E remains NOT RUN. See [integration guide](../guides/formatter-external-validation.md). Linux accepted HWM remains 3228/5708; the measured 3470/5708 improvement is a candidate.


### Defect and scope

In tack 0.8.4, `[fmt]` and `[fmt "NAME"]` key handlers were accidentally nested inside `if (sec == SEC_SBOM)`, a mutually exclusive branch. Although the formatter section name was recognized, its keys were silently dropped; `fmt --list --strict` then failed with `missing cmd`.

### Required behavior

- `[sbom]`, `[fmt]` and `[fmt "NAME"]` have independent, non-overlapping parse branches. A valid section must not overwrite another section's values.
- Existing formatter keys `enabled`, `fail_fast`, `defaults`, `rules`, `exclude`, `tool`, `mode`, `include`, `cmd`, `requires`, `workdir`, `check_cmd`, `diff_cmd` and `env.*` are parsed as implemented.
- `fmt --list --strict` must succeed with a complete valid rule without spawning an external formatter or modifying source files.
- `--strict` rejects unknown keys and invalid `mode` values. Candidate filenames containing spaces must remain intact.
- This parser fix **does not** implement runtime support for `check_cmd`, `diff_cmd`, `env.*` or guaranteed continue-after-error with `fail_fast = no`. External formatter tools stay optional.
- Refresh the test inventory only after reviewing the exact test diff; never lower accepted coverage high-water marks. Windows and Linux gates remain separate.

### Evidence and remaining gates

`tests/fmt_ini_parser_test.c` covers real `[fmt]` parsing, independent `[sbom]`, `fmt --list --strict`, excludes, filenames with spaces and negative validation. Linux GCC 14.2.0 release: 26 compiled / 25 PASS / one platform SKIP / zero failures. Instrumented Linux GCC/gcov improvement candidate: 3470/5708 (60.79187%), HWM **not yet accepted**, coverage gate `BLOCKED improved coverage requires HWM increase`. Windows/TDM-GCC and real external formatter execution: `NOT RUN`. No release/tag/version promotion is claimed.
