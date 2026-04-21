# SPEC 0003 — Trennung von Compilerwahl und Produktpolitik / Split between compiler selection and product policy

**Status:** angenommen / accepted  
**Serie:** v0.8.0-dev  
**Typ:** Architekturspezifikation / architecture specification

---

## Deutsch

### Problemstellung

Der aktuelle Sprachgebrauch rund um `TACK_CC`, eingebaute Profil-Flags und projektbezogene `tack.ini`-Einträge ist funktional, aber noch nicht scharf genug getrennt. Dadurch entsteht die Gefahr, dass drei unterschiedliche Ebenen vermischt werden:

1. **Welches Compiler-Programm gestartet wird**
2. **Welche eingebauten tack-Basisannahmen gelten**
3. **Welche Projektpolitik ein Repository zusätzlich festlegt**

Für die 0.8-Serie muss diese Trennung vor dem ersten Codeumbau präzise dokumentiert werden.

### Entscheidung

`tack` behandelt diese Ebenen künftig ausdrücklich getrennt:

1. **Compilerwahl**  
   `TACK_CC` oder ein später gleichwertiger INI-Wert bestimmen, *welches Compilerprogramm* gestartet wird.

2. **Eingebaute tack-Basispolitik**  
   tack darf weiterhin kleine, dokumentierte Built-ins besitzen, etwa für Standard-Profile oder minimalen Bootstrap.

3. **Projektpolitik über INI**  
   `tack.ini` beschreibt zusätzliche Projekt- und Toolchain-Politik, ohne die Kernsemantik oder Sicherheitsgrenzen zu ersetzen.

### Normative Regeln

1. **`TACK_CC` ist ein Programm, kein Flag-Container.**  
   Compiler-Flags gehören nicht in `TACK_CC`. Zusätzliche Flags bleiben in `tack.ini` oder anderen ausdrücklich vorgesehenen Konfigurationswegen.

2. **INI erweitert, aber ersetzt keine Built-ins pauschal.**  
   0.8 soll die eingebaute Politik transparenter machen, nicht alle Built-ins aus dem Produkt entfernen.

3. **Die Toolchain-Politik bleibt klein.**  
   0.8 ist kein Freibrief für generische Toolchain-DSLs, beliebige Runner-Kaskaden oder skriptartige Metaprogrammierung.

4. **Dokumentierte Priorität vor Implementierung.**  
   Bevor Produktionscode an Compiler-/Profil-Logik geändert wird, muss die Priorität zwischen Compilerprogramm, Built-ins und INI in der Doku klar benannt sein.

5. **C89- und fail-fast-Rahmen bleiben unangetastet.**  
   Zusätzliche Steuerbarkeit darf nicht zu indirekter, schwer prüfbarer oder abweichend abgesicherter Build-Logik führen.

### Konsequenz für die nächsten Schritte

Spätere 0.8-Commits sollen deshalb klein bleiben und zuerst genau diese Grenzstellen verbessern:

- klarere Benutzerbegriffe in README/FAQ,
- explizitere Roadmap-Sprache zur Toolchain-Politik,
- erst danach gezielte Produktionscode-Änderungen mit Regressionstests.

---

## English

### Problem

The current wording around `TACK_CC`, built-in profile flags, and project-level `tack.ini` entries is functional, but not sharply separated enough. This risks mixing three different layers:

1. **Which compiler program is launched**
2. **Which built-in tack baseline assumptions apply**
3. **Which additional product policy a repository declares**

The 0.8 series should document this split precisely before the first code refactor lands.

### Decision

`tack` will treat these layers as explicitly separate:

1. **Compiler selection**  
   `TACK_CC` or a later equivalent INI value decides *which compiler program* is launched.

2. **Built-in tack baseline policy**  
   tack may continue to ship small, documented built-ins, for example for standard profiles or minimal bootstrap behavior.

3. **Project policy through INI**  
   `tack.ini` describes additional project and toolchain policy without replacing core semantics or safety boundaries.

### Normative rules

1. **`TACK_CC` is a program, not a flag container.**  
   Compiler flags do not belong in `TACK_CC`. Extra flags stay in `tack.ini` or other explicitly supported configuration paths.

2. **INI extends policy; it does not automatically replace all built-ins.**  
   0.8 should make built-in policy more transparent, not strip all built-ins out of the product.

3. **Toolchain policy stays small.**  
   0.8 is not a license for generic toolchain DSLs, arbitrary runner cascades, or script-like metaprogramming.

4. **Documented priority before implementation.**  
   Before production code changes the compiler/profile logic, the documentation must name the priority between compiler program, built-ins, and INI clearly.

5. **The C89 and fail-fast frame remains untouched.**  
   Additional configurability must not lead to indirect, hard-to-review, or differently hardened build logic.

### Consequence for the next steps

Later 0.8 commits should therefore stay small and improve these boundaries first:

- clearer user-facing terminology in README/FAQ,
- more explicit roadmap language for toolchain policy,
- only then targeted production code changes with regression tests.
