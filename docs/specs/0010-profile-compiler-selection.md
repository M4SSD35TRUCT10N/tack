# SPEC 0010 — Per-target profile compiler / Compiler je Targetprofil

**Version / Version:** 0.8.2-dev  
**Status / Status:** implementation candidate; NOT ACCEPTED / Implementierungskandidat, NICHT ABGENOMMEN

## Deutsch

Ein Release-Target darf einen anderen Compiler verwenden als das Debug-Target. Die Auswahl bleibt eine deklarative Eigenschaft von `tack.ini` und wird nicht mit Compiler-Flags oder Skriptlogik vermischt.

### Syntax

```ini
[project]
compiler = tcc
compiler_policy = auto

[target "app".release]
compiler = gcc
compiler_policy = gcc
cflags = -std=c89;-pedantic-errors;-Wextra;-Werror
```

Die Compilerwahl erfolgt in der Reihenfolge:

1. Nichtleeres `TACK_CC` (Programmname oder Pfad, niemals ein Flag-Container).
2. Nichtleeres `[target "NAME".PROFILE] compiler` des aktiven Targets.
3. Nichtleeres `[project] compiler`.
4. Eingebauter Default `tcc`.

Bei `[target "NAME".PROFILE] compiler_policy` gilt die explizite Target-Policy; `auto` erkennt die Policy aus dem wirksamen Compilernamen. Andernfalls greift die bisherige Projekt-Policy. Ein explizites `compiler_policy = gcc` bleibt auch bei `TACK_CC=tcc` wirksam; deshalb darf eine Umgebungsüberschreibung nur mit einer passenden Policy verwendet werden. Fehlerhafte Kombinationen müssen durch Compiler-Gates entdeckt werden.

`build`, `run`, `bom`, `sbom` und `doc` binden die Compilerwahl an das aktive Target und Profil. `test` verwendet dieselbe Auswahl wie das mit `--target` ausgewählte Target (standardmäßig `default_target`) und übernimmt dessen Profil-`cflags`; der Compiler wird nicht unabhängig aus `[project]` bestimmt. `doctor release --target app` zeigt die wirksame Auswahl an. Normale Ausführung von `doctor` zeigt das Debug-Profil des Default-Targets.

**Grenzen:** Keine neue Toolchain-DSL, keine Änderung des `tackfile.c`-Vertrags, keine automatischen Installer, keine Agentensteuerung. Der Wechsel des Compilers innerhalb desselben Profils erfordert bis zur gesonderten Cache-Verbesserung `--rebuild`, da die bestehende inkrementelle Prüfung keinen Wechsel allein anhand des Compilernamens erkennt.

## English

A release target may use a different compiler from its debug target. Compiler selection stays declarative in `tack.ini`, separate from flags and scripts.

Selection precedence: non-empty `TACK_CC` > non-empty active `[target "NAME".PROFILE] compiler` > non-empty `[project] compiler` > built-in `tcc`. Target `compiler_policy` overrides project policy; `auto` detects the effective compiler. A forced non-auto policy remains forced even when `TACK_CC` changes the program, so users must keep the combination consistent. `build`, `run`, `bom`, `sbom`, `doc`, `doctor` and `test` resolve the active target/profile. `test` inherits that target/profile's extra `cflags` as well. `doctor release --target app` reports the selection.

No generic build DSL, installers, agent management, or `tackfile.c` API expansion. Until cache identity is extended, use `--rebuild` when overriding a compiler within the same profile.

## Acceptance evidence / Abnahme

Regression source: `tests/compiler_target_profile_selection_test.c` (environment precedence, profile fallback, policy, target isolation). The existing compiler selection and compiler flag regression sources remain normative. **No tests executed for this draft.** Windows/TinyCC, TDM-GCC, GCC and Clang gates remain open.
