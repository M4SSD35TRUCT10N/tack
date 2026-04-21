# SPEC 0007 — JUnit-XML-Grundlage / JUnit XML foundation

**Status:** angenommen / accepted  
**Serie / series:** v0.8.0-dev

## Deutsch

### Ziel

**Umsetzungsstand / implementation status:**

`--report-junit FILE` ist für `tack test` umgesetzt. Die hier definierte kleine JUnit-Teilmenge gilt damit als angenommene CI-Ausgabegrundlage für Systeme, die JUnit XML erwarten.


`--report-junit FILE` ergänzt `tack test` um einen konservativen JUnit-XML-Bericht.
Die Funktion baut auf `--ci`, `TACK_SUMMARY`, `tack.events.v1` und dem TAP-Adapter auf.

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

### Begründung

1. Viele CI-Systeme erwarten JUnit XML als kleinsten gemeinsamen Nenner.
2. `tack` soll dafür keine fremde XML-Bibliothek benötigen.
3. Die Ausgabe bleibt fail-fast-kompatibel: auch bei frühem Abbruch wird ein konsistenter Bericht geschrieben.
4. JUnit XML bleibt Adapter, nicht Primärschicht. Die interne Maschinenbasis bleibt `tack.events.v1`.

### Formatregeln

- nur `tack test` akzeptiert `--report-junit FILE`
- die Datei wird komplett von tack geschrieben
- Zeiten werden als Sekunden mit Millisekunden-Nachkommastellen ausgegeben
- Dateipfade und Namen werden XML-escaped
- erfolgreiche Tests erhalten keinen leeren `<failure>`-Knoten

### Ergebnis

Mit SPEC 0007 ist `tack` für lokale Runner und klassische CI-Systeme deutlich anschlussfähiger, ohne den Kern in Richtung XML-zentriertes Framework zu verschieben.

---

## English

**Implementation status:**

`--report-junit FILE` is implemented for `tack test`. The small JUnit subset defined here is therefore accepted as the CI output foundation for systems that expect JUnit XML.


### Goal

`--report-junit FILE` adds a conservative JUnit XML report to `tack test`.
The feature builds on top of `--ci`, `TACK_SUMMARY`, `tack.events.v1`, and the TAP adapter.

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

### Rationale

1. Many CI systems consume JUnit XML as the smallest common denominator.
2. `tack` should not need a foreign XML library for this.
3. The output stays compatible with fail-fast execution: even on early abort, tack still writes a consistent report.
4. JUnit XML remains an adapter, not the primary layer. The internal machine-facing base stays `tack.events.v1`.

### Format rules

- only `tack test` accepts `--report-junit FILE`
- the file is written fully by tack
- durations are emitted as seconds with millisecond precision
- file paths and names are XML-escaped
- successful tests do not get empty `<failure>` nodes

### Result

With SPEC 0007, `tack` becomes easier to plug into local runners and classic CI systems without pushing the core toward an XML-centric framework.
