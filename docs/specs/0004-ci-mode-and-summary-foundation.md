# SPEC 0004 – CI mode and stable summary foundation / CI-Modus und stabile Summary-Grundlage

**Status:** draft / in Umsetzung  
**Serie:** v0.8.0-dev  
**Typ:** normative Spezifikation / normative specification

---

## DE

## Ziel

`tack` erhält mit der 0.8-Serie einen **kleinen, plattformneutralen CI-Grundmodus**, ohne den normalen CLI-Charakter zu verlieren.

Dieser erste Schritt führt deshalb nur zwei Dinge ein:

1. `--ci` als bewussten Schalter für deterministischere CI-Ausgabe
2. einen stabilen `TACK_SUMMARY`-Abschlussblock für `build`, `run` und `test`

## Anforderungen

1. **Default-Ausgabe bleibt menschlich.**  
   Ohne `--ci` verhält sich `tack` weiter wie bisher.

2. **`--ci` bleibt klein.**  
   Der Schalter aktiviert keine eigene Build-Pipeline, keinen Runner-Modus und keine fremde CI-DSL.

3. **Summary ist stabil und grep-freundlich.**  
   Die maschinenlesbare Kurzfassung wird als einzelne Zeile auf `stdout` ausgegeben und beginnt mit `TACK_SUMMARY`.

4. **Tests bleiben fail-fast.**  
   Die Summary spiegelt die bestehende Testphilosophie wider und führt noch keine komplexe Skip-/Retry-Logik ein.

5. **Die Schicht ist nur Grundlage.**  
   JSONL-Events, TAP, JUnit XML oder SARIF bauen später auf dieser Basisschicht auf.

## Vorerst kein Ziel

- kein generisches CI-Framework
- keine GitHub-spezifischen Annotationen in diesem Schritt
- keine vollumfängliche Test-Report-Engine

---

## EN

## Goal

With the 0.8 series, `tack` gains a **small, platform-neutral CI foundation** without losing its normal CLI character.

This first step therefore introduces only two things:

1. `--ci` as an explicit switch for more deterministic CI output
2. a stable `TACK_SUMMARY` trailer line for `build`, `run`, and `test`

## Requirements

1. **Default output remains human-oriented.**  
   Without `--ci`, `tack` continues to behave as before.

2. **`--ci` stays small.**  
   The switch does not introduce its own build pipeline, runner mode, or foreign CI DSL.

3. **Summary output is stable and grep-friendly.**  
   The machine-readable short form is emitted as a single line on `stdout` and starts with `TACK_SUMMARY`.

4. **Tests remain fail-fast.**  
   The summary reflects the existing test philosophy and does not yet add complex skip/retry logic.

5. **This layer is only a foundation.**  
   JSONL events, TAP, JUnit XML, or SARIF can be layered on top later.

## Not a goal yet

- no generic CI framework
- no GitHub-specific annotations in this step
- no full-blown test reporting engine
