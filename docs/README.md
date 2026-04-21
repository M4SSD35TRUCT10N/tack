# tack docs (DE/EN) — v0.8.0-dev

**Backlinks:** [README](../README.md) • [FAQ](../FAQ.md) • [Roadmap](../ROADMAP.md) • [Release Notes](../RELEASENOTES.md)

---

## Deutsch

Dieses Verzeichnis ist ab der **0.8-Serie** der feste Ort für:

- **Spezifikationen** (`docs/specs/`) als nachvollziehbare Vorab-Festlegung,
- **Architekturentscheidungen** (`docs/adr/`) als dokumentierte Richtungsentscheidungen,
- spätere ergänzende Leitfäden, sobald ein Themengebiet stabil genug ist.

Ziel ist nicht, `README.md`, `FAQ.md`, `ROADMAP.md` oder `RELEASENOTES.md` zu ersetzen. Diese Root-Dokumente bleiben die primären Einstiegspunkte. `docs/` ergänzt sie um den strukturierten Nachweis, **warum** eine Änderung eingeführt wird und **welche Grenzen** dabei gelten.

### Startpunkt der 0.8-Serie

- [ADR 0001 – Eröffnung der 0.8-Serie](adr/0001-open-v0.8-series.md)
- [SPEC 0001 – Bilinguale Dokumentationspolitik](specs/0001-bilingual-documentation-policy.md)
- [SPEC 0002 – Grundlage für eine INI-first-Toolchain-Politik](specs/0002-ini-first-toolchain-policy-foundation.md)
- [SPEC 0003 – Trennung von Compilerwahl und Produktpolitik](specs/0003-compiler-selection-and-policy-split.md)
- [SPEC 0004 – CI-Modus und Summary-Grundlage](specs/0004-ci-mode-and-summary-foundation.md) *(angenommen / accepted)*
- [SPEC 0005 – Events-JSONL-Grundlage](specs/0005-events-jsonl-foundation.md) *(angenommen / accepted)*
- [SPEC 0006 – TAP-Report-Grundlage](specs/0006-tap-report-foundation.md) *(angenommen / accepted)*
- [SPEC 0007 – JUnit-XML-Grundlage](specs/0007-junit-xml-foundation.md) *(angenommen / accepted)*

---

## English

Starting with the **0.8 series**, this directory becomes the fixed home for:

- **specifications** (`docs/specs/`) as traceable up-front design work,
- **architecture decisions** (`docs/adr/`) as recorded directional decisions,
- later supporting guides once a topic is stable enough.

The goal is not to replace `README.md`, `FAQ.md`, `ROADMAP.md`, or `RELEASENOTES.md`. Those root documents remain the primary entry points. `docs/` complements them with a structured record of **why** a change is introduced and **which boundaries** apply.

### 0.8 series starting set

- [ADR 0001 – Open the 0.8 series](adr/0001-open-v0.8-series.md)
- [SPEC 0001 – Bilingual documentation policy](specs/0001-bilingual-documentation-policy.md)
- [SPEC 0002 – Foundation for an INI-first toolchain policy](specs/0002-ini-first-toolchain-policy-foundation.md)
- [SPEC 0003 – Split between compiler selection and product policy](specs/0003-compiler-selection-and-policy-split.md)
- [SPEC 0004 – CI mode and summary foundation](specs/0004-ci-mode-and-summary-foundation.md) *(accepted)*
- [SPEC 0005 – Events JSONL foundation](specs/0005-events-jsonl-foundation.md) *(accepted)*
- [SPEC 0006 – TAP report foundation](specs/0006-tap-report-foundation.md) *(accepted)*
- [SPEC 0007 – JUnit XML foundation](specs/0007-junit-xml-foundation.md) *(accepted)*

## CI-Grundlagenblock / CI foundation block

DE: Die Spezifikationen 0004 bis 0007 sind als **angenommene Grundlagen** markiert, weil `--ci`, `TACK_SUMMARY`, `--events-jsonl`, `--report-tap` und `--report-junit` im aktuellen Stand umgesetzt sind.

EN: Specifications 0004 through 0007 are marked as **accepted foundations** because `--ci`, `TACK_SUMMARY`, `--events-jsonl`, `--report-tap`, and `--report-junit` are implemented in the current state.
