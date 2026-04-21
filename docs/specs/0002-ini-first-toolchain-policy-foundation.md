# SPEC 0002 — Grundlage für eine INI-first-Toolchain-Politik / Foundation for an INI-first toolchain policy

**Status:** angenommen / accepted  
**Serie:** v0.8.0-dev  
**Typ:** Architekturspezifikation / architecture specification

---

## Deutsch

### Problemstellung

`tack` unterstützt bereits mehrere Compilerpfade über `TACK_CC`, besitzt aber weiterhin eingebaute Profil- und Toolchain-Annahmen. Diese Built-ins sind praktisch und bleiben wichtig, doch ihre Rolle muss sauberer beschrieben werden.

### Ziel

Die 0.8-Serie bereitet eine **INI-first-Toolchain-Politik** vor, ohne `tack` in ein generisches Meta-Buildsystem umzubauen.

### Normative Leitplanken

1. **Der Kern bleibt im Code.**  
   Sicherheitslogik, Parsing, Layering, Fail-fast-Grenzen und andere fundamentale Regeln bleiben in `src/tack.c` bzw. im Produktionscode verankert.

2. **INI steuert Produktpolitik, nicht Sicherheitsgrenzen.**  
   `tack.ini` soll stärker ausdrücken können, welche Compiler-/Profilpolitik ein Projekt wünscht. Sie ersetzt aber keine harten Sicherheitsprüfungen und keine Grundsemantik.

3. **TCC bleibt zulässiger Default-Fall.**  
   Die bisherige TCC-Freundlichkeit wird nicht als Fehler behandelt. Sie darf jedoch künftig nicht mehr stillschweigend die gesamte Produktsemantik dominieren.

4. **Eingebaute Fallbacks bleiben erlaubt.**  
   `tack` darf weiterhin vernünftige Built-ins für Bootstrap- und Minimalfälle besitzen, solange deren Rang und Grenzen dokumentiert sind.

5. **Keine freie Toolchain-Metaprogrammierung als 0.8-Ziel.**  
   0.8 soll Klarheit und Steuerbarkeit erhöhen, nicht beliebige Pipelines, Skriptsprachen oder frei zusammensteckbare Buildsysteme einführen.

### Folge für die nächsten Commits

Spätere 0.8-Codeänderungen sollen deshalb zuerst die Policy-Schicht sauberer machen:

- explizitere Dokumentation von Compiler-/Profilannahmen,
- klarere Trennung zwischen Compilerwahl und zusätzlichen Flags,
- möglichst kleine, nachvollziehbare Schritte mit Regressionstests.

---

## English

### Problem

`tack` already supports multiple compiler paths via `TACK_CC`, but it still carries built-in profile and toolchain assumptions. Those built-ins are practical and remain important, yet their role needs to be described more clearly.

### Goal

The 0.8 series prepares an **INI-first toolchain policy** without turning `tack` into a generic meta-build system.

### Normative guardrails

1. **The core stays in code.**  
   Security logic, parsing, layering, fail-fast boundaries, and other fundamental rules stay anchored in `src/tack.c` and production code.

2. **INI expresses product policy, not security boundaries.**  
   `tack.ini` should become better at expressing which compiler/profile policy a project wants, but it does not replace hard safety checks or base semantics.

3. **TCC remains a valid default case.**  
   The current TCC friendliness is not treated as a bug. However, it should no longer silently dominate overall product semantics.

4. **Built-in fallbacks remain allowed.**  
   `tack` may continue to ship sensible built-ins for bootstrap and minimal cases, as long as their rank and limits are documented.

5. **No free-form toolchain metaprogramming as a 0.8 goal.**  
   0.8 should increase clarity and control, not introduce arbitrary pipelines, scripting layers, or freely composable build systems.

### Consequence for later commits

Later 0.8 code changes should therefore first improve the policy layer itself:

- more explicit documentation of compiler/profile assumptions,
- a clearer split between compiler selection and extra flags,
- small, traceable steps backed by regression tests.
