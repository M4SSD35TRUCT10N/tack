# SPEC 0005 — Events-JSONL-Grundlage / Events JSONL foundation

**Status:** angenommen / accepted  
**Serie / series:** v0.8.0-dev

## DE

`tack` ergänzt die menschenlesbare CLI-Ausgabe um eine kleine versionierte Maschinenebene:

- Schalter: `--events-jsonl FILE`
- Geltung: `build`, `run`, `test`
- Schemakennung: `tack.events.v1`
- Format: eine JSON-Zeile pro Ereignis

Ziel ist eine robuste Grundlage für lokale Runner und spätere Adapter wie TAP/JUnit, ohne den Kern von `tack` an ein fremdes CI-Format zu koppeln.

In dieser ersten Stufe werden nur wenige Ereignisse garantiert:

**Umsetzungsstand / implementation status:**

`--events-jsonl FILE` ist für `build`, `run` und `test` umgesetzt. Die hier definierte kleine Ereignisschicht gilt damit als angenommene Grundlage für lokale Runner und weitere Adapter.


- `run_started`
- `run_finished`
- `test_plan`
- `test_started`
- `test_finished`

Die Standardausgabe bleibt unverändert menschenlesbar. `--events-jsonl` ist rein optional.

## EN

`tack` adds a small versioned machine-facing layer alongside the human-readable CLI output:

- switch: `--events-jsonl FILE`
- applies to: `build`, `run`, `test`
- schema id: `tack.events.v1`
- format: one JSON line per event

The goal is a robust foundation for local runners and later adapters such as TAP/JUnit without coupling the tack core to a foreign CI format.

At this first stage only a small event set is guaranteed:

**Implementation status:**

`--events-jsonl FILE` is implemented for `build`, `run`, and `test`. The small event layer defined here is therefore accepted as the foundation for local runners and further adapters.

- `run_started`
- `run_finished`
- `test_plan`
- `test_started`
- `test_finished`

Default stdout remains human-readable. `--events-jsonl` is purely optional.
