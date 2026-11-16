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


### Abnahme und Nachweise

Regression: `tests/compiler_target_profile_selection_test.c` prüft Umgebungspriorität, Profilfallback, Policy und Target-Isolation. Die vorhandenen Tests zu Compilerwahl und Flags gelten weiterhin. Nach der Korrektur von v0.8.3 wurden die Linux/GCC-Debug- und Release-Suites ausgeführt. Windows/TinyCC, TDM-GCC und gegebenenfalls weitere Compiler bleiben getrennte Gates; die historische 0.8.2-Implementierung war zuvor nicht unabhängig getestet.

## English

A release target may use a different compiler from its debug target. Compiler selection remains declarative in `tack.ini`, separate from compiler flags or script logic.

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

Compiler selection priority is:

1. Non-empty `TACK_CC` (executable name or path, never a container for flags).
2. Non-empty `[target "NAME".PROFILE] compiler` for the active target/profile.
3. Non-empty `[project] compiler`.
4. Built-in default `tcc`.

A target-profile `compiler_policy` overrides project policy. `auto` detects policy from the effective compiler program. Otherwise the project policy is used. An explicitly forced `compiler_policy = gcc` remains in effect even under `TACK_CC=tcc`; environment overrides therefore require a matching policy. Incompatible combinations must be caught by compiler gates.

`build`, `run`, `bom`, `sbom` and `doc` bind compiler selection to the active target and profile. `test` uses the compiler of the `--target` target (or `default_target`) and inherits that target/profile's extra `cflags`; it does not independently choose the project compiler. `doctor release --target app` reports the effective selection. Plain `doctor` shows the debug profile of the default target.

**Limits:** No generic toolchain DSL, no changes to the `tackfile.c` contract, no auto-installers or agent control. Switching compilers within a profile requires `--rebuild` until separate cache improvements include compiler identity in the cache key.

### Acceptance and evidence

Regression source: `tests/compiler_target_profile_selection_test.c` covers environment precedence, profile fallback, policy and target isolation. The existing compiler-selection and compiler-flags tests remain applicable. Linux/GCC debug and release suites were executed after the v0.8.3 fixes. Windows/TinyCC, TDM-GCC and any other required compilers remain separate gates; the historical 0.8.2 implementation had not been independently tested before that correction.
