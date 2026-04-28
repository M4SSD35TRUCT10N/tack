# SPEC 0006 — TAP-Report-Grundlage / TAP report foundation

**Status:** angenommen / accepted
**Serie:** v0.8.0-dev
**Typ:** Testreport-Spezifikation / test report specification

---

## Deutsch

### Ziel

`tack` ergänzt `tack test` um einen kleinen optionalen TAP-Adapter:

- Schalter: `--report-tap FILE`
- Geltungsbereich: nur `tack test`
- Ziel: eine einfache, breit verständliche Testbericht-Datei ohne XML- oder Framework-Zwang

Diese Erweiterung baut bewusst auf den vorhandenen 0.8-Bausteinen auf: `--ci`, `TACK_SUMMARY` und `--events-jsonl FILE`.

### Umsetzungsstand

`--report-tap FILE` ist für `tack test` umgesetzt. TAP bleibt damit ein optionaler Adapter auf angenommener CI-Grundlage, nicht die Primärschnittstelle von `tack`.

### Leitplanken

1. **Optional, nicht erzwungen.**
   Ohne `--report-tap` bleibt das Verhalten von `tack test` unverändert.

2. **Nur für Tests.**
   `--report-tap` gilt bewusst nicht für `build` oder `run`.

3. **Fail-fast bleibt erhalten.**
   Bei Compile- oder Testfehlern endet `tack test` weiter sofort. Der TAP-Bericht markiert den fehlerhaften Test mit `not ok` und beendet die Folge mit `Bail out!`.

4. **Kleine Teilmenge.**
   Für 0.8 genügt eine konservative TAP-v13-Grundlage:
   - Planzeile `1..N`
   - `ok` / `not ok`
   - knappe Diagnosezeilen mit `# ...`
   - `Bail out!` bei fail-fast-Abbruch

5. **Keine Verdrängung anderer Formate.**
   TAP ersetzt weder `tack.events.v1` noch JUnit-/SARIF-Adapter.

### Beispiel

```text
1..2
ok 1 - smoke_test.c
# file: tests/smoke_test.c
# compiled: true
# duration_ms: 4
not ok 2 - fail_test.c
# file: tests/fail_test.c
# compiled: true
# duration_ms: 2
Bail out! tack fail-fast after test failure
```

---

## English

### Goal

`tack` adds a small optional TAP adapter to `tack test`:

- switch: `--report-tap FILE`
- scope: only `tack test`
- goal: a simple, widely understood test report file without XML or framework requirements

This extension deliberately builds on the existing 0.8 blocks: `--ci`, `TACK_SUMMARY`, and `--events-jsonl FILE`.

### Implementation status

`--report-tap FILE` is implemented for `tack test`. TAP therefore remains an optional adapter on top of the accepted CI foundation, not the primary interface of `tack`.

### Guardrails

1. **Optional, not forced.**
   Without `--report-tap`, `tack test` behaves as before.

2. **Tests only.**
   `--report-tap` intentionally does not apply to `build` or `run`.

3. **Fail-fast remains intact.**
   On compile or test failure, `tack test` still exits immediately. The TAP report marks the failing test with `not ok` and ends the sequence with `Bail out!`.

4. **Small subset.**
   For 0.8, a conservative TAP v13 base is enough:
   - plan line `1..N`
   - `ok` / `not ok`
   - concise diagnostic lines with `# ...`
   - `Bail out!` on fail-fast abort

5. **No displacement of other formats.**
   TAP does not replace `tack.events.v1` or JUnit/SARIF adapters.

### Example

```text
1..2
ok 1 - smoke_test.c
# file: tests/smoke_test.c
# compiled: true
# duration_ms: 4
not ok 2 - fail_test.c
# file: tests/fail_test.c
# compiled: true
# duration_ms: 2
Bail out! tack fail-fast after test failure
```
