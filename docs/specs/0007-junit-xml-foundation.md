# SPEC 0007 — JUnit-XML-Grundlage / JUnit XML foundation

**Status:** angenommen / accepted
**Serie:** v0.8.0-dev
**Typ:** Testreport-Spezifikation / test report specification

---

## Deutsch

### Ziel

`--report-junit FILE` ergänzt `tack test` um einen konservativen JUnit-XML-Bericht. Die Funktion baut auf `--ci`, `TACK_SUMMARY`, `tack.events.v1` und dem TAP-Adapter auf.

### Umsetzungsstand

`--report-junit FILE` ist für `tack test` umgesetzt. Die hier definierte kleine JUnit-Teilmenge gilt damit als angenommene CI-Ausgabegrundlage für Systeme, die JUnit XML erwarten.

### Nicht-Ziele

- kein neuer Test-Runner
- keine Erweiterung von `build` oder `run` um JUnit
- keine Dialekt-Sammlung für CI-System-spezifische Sonderfelder

### Grundsatz

`tack` gibt nur eine kleine, robuste JUnit-Teilmenge aus:

- `<testsuites>`
- `<testsuite>`
- `<testcase>`
- `<failure>` bei Fehlschlag
- `<system-out>` für kleine Zusatzinformationen wie `compiled: true|false`

### Formatregeln

- nur `tack test` akzeptiert `--report-junit FILE`
- die Datei wird komplett von tack geschrieben
- Zeiten werden als Sekunden mit Millisekunden-Nachkommastellen ausgegeben
- Dateipfade und Namen werden XML-escaped
- erfolgreiche Tests erhalten keinen leeren `<failure>`-Knoten

### Begründung

1. Viele CI-Systeme erwarten JUnit XML als kleinsten gemeinsamen Nenner.
2. `tack` soll dafür keine fremde XML-Bibliothek benötigen.
3. Die Ausgabe bleibt fail-fast-kompatibel: auch bei frühem Abbruch wird ein konsistenter Bericht geschrieben.
4. JUnit XML bleibt Adapter, nicht Primärschicht. Die interne Maschinenbasis bleibt `tack.events.v1`.

---

## English

### Goal

`--report-junit FILE` adds a conservative JUnit XML report to `tack test`. The feature builds on top of `--ci`, `TACK_SUMMARY`, `tack.events.v1`, and the TAP adapter.

### Implementation status

`--report-junit FILE` is implemented for `tack test`. The small JUnit subset defined here is therefore accepted as the CI output foundation for systems that expect JUnit XML.

### Non-goals

- no new test runner
- no JUnit support for `build` or `run`
- no collection of CI-vendor-specific dialect fields

### Principle

`tack` emits only a small, robust JUnit subset:

- `<testsuites>`
- `<testsuite>`
- `<testcase>`
- `<failure>` for failures
- `<system-out>` for small extra details such as `compiled: true|false`

### Format rules

- only `tack test` accepts `--report-junit FILE`
- the file is written completely by tack
- times are emitted as seconds with millisecond decimals
- file paths and names are XML-escaped
- successful tests do not receive empty `<failure>` nodes

### Rationale

1. Many CI systems expect JUnit XML as the smallest common denominator.
2. `tack` should not need an external XML library for this.
3. Output stays fail-fast compatible: even after an early abort, the report remains consistent.
4. JUnit XML remains an adapter, not the primary layer. The internal machine basis remains `tack.events.v1`.
