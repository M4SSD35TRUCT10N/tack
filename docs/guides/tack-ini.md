# tack.ini: vom ersten Build zur reproduzierbaren Pipeline / From first build to reproducible pipelines

> **Stand / Scope:** tack 0.8.4, Dokumentationsmeilenstein 0.8.5-dev. Dies ist eine Anleitung, keine neue Parser- oder CLI-Funktion. / Current implementation: tack 0.8.4; documentation milestone: 0.8.5-dev. This guide does not introduce new CLI or parser functionality.
>
> **Nachschlagen / Reference:** [vollständige Schlüsselreferenz](../reference/tack-ini.md) · [Coverage-Ratchet](coverage-ratchet.md) · [README](../../README.md).

---

# Deutsch

## 1. Das Modell in drei Sätzen

`tack.ini` ist eine reine Datendatei; sie enthält keine ausführbaren Build-Skripte. Aus ihr ermittelt tack Projektdefaults, Targets, Compiler, Profil-Overrides sowie DOC/BOM/SBOM/Formatter- und Coverage-Einstellungen. `tack` aus dem **Projekt-Root** starten und die Konfiguration im Team/CI ausdrücklich angeben:

```bat
tack --config .\tack.ini list
```

Die globale Option `--config` steht **vor** dem Kommando. Das vermeidet Verwechslungen zwischen globaler Konfiguration und Target-Argumenten.

## 2. Ein einfaches C89-Projekt

Projektbaum:

```text
hello/
  tack.ini
  src/main.c
  include/
  tests/smoke_test.c
```

`tack.ini`:

```ini
[project]
default_target = app
compiler = gcc
compiler_policy = auto
allow_unsafe_paths = no

[target "app"]
src = src
bin = hello
id = app
core = no
includes = include

[target "app".debug]
cflags = -std=c89;-pedantic-errors;-Wextra;-Werror

[target "app".release]
cflags = -std=c89;-pedantic-errors;-Wextra;-Werror
```

Windows-CMD, aus dem Verzeichnis `hello`:

```bat
tack --config .\tack.ini doctor release --target app
tack --config .\tack.ini build debug --target app --rebuild -j 1
tack --config .\tack.ini build release --target app --rebuild -j 1
tack --config .\tack.ini run release --target app
tack --config .\tack.ini test release --ci --rebuild -j 1
```

`compiler = gcc` setzt voraus, dass GCC im `PATH` ist. Ohne Projekt- oder Umgebungs-Override ist der eingebaute Compilerdefault **tcc**. Für vorhandene Tests kann der projektspezifische Test-Ratchet zusätzliche Anforderungen erzwingen; der oben gezeigte allgemeine Minimalaufbau enthält kein versioniertes Inventar.

## 3. Was v0.8.2-dev zusätzlich möglich gemacht hat

Seit v0.8.2-dev lassen sich **Compilerprogramm und Compiler-Policy getrennt pro Target und Profil** festlegen. Die Compiler-Auswahl lautet:

```text
TACK_CC (Umgebung)
  > [target "NAME".debug|release|coverage] compiler
  > [project] compiler
  > Built-in tcc
```

Die Policy `auto|tcc|gcc|clang|generic` bestimmt, welche eingebauten compilerspezifischen Flags tack verwendet. Sie ist **nicht** das Compilerprogramm. `auto` leitet sie aus dem effektiven Compilernamen ab.

```ini
[project]
compiler = tcc
compiler_policy = auto

[target "app".debug]
compiler = tcc
compiler_policy = tcc

[target "app".release]
compiler = gcc
compiler_policy = auto
cflags = -std=c89;-pedantic-errors;-Wextra;-Werror
```

```bat
tack --config .\tack.ini doctor debug --target app
tack --config .\tack.ini doctor release --target app
set TACK_CC=gcc
tack --config .\tack.ini doctor debug --target app
set TACK_CC=
```

`TACK_CC` hat Vorrang vor einer profilierten Auswahl. Beim Wechsel des Compilers **im gleichen Profil** `--rebuild` verwenden, weil die Compileridentität im aktuellen Cache nicht vollständig isoliert ist. Das eingebaute Debug-Profil setzt `-g` und `-DDEBUG=1`; `-bt20` erhält ausschließlich TinyCC, nicht GCC/Clang. `cflags` **ergänzen** die eingebauten Profilflags.

**Achtung:** `compiler` und `compiler_policy` in `[target "NAME"]` (ohne `.debug`/`.release`/`.coverage`) sind keine implementierten Basisschlüssel. Sie gehören in `[project]` oder in eine *profilierte* Target-Sektion.

## 4. Mehrere Targets und Shared Core

```ini
[target "app"]
src = src/app
bin = server
id = app
core = yes
includes = include;src

[target "tool:dbgen"]
src = tools/dbgen
bin = dbgen
id = dbgen
core = yes
includes = include;src

[target "tool:dbgen".release]
defines = DBGEN_RELEASE=1
```

Mit `tack ... list` die erkannten Targets prüfen, anschließend `tack ... build release --target tool:dbgen`. Quellen für Shared Core liegen standardmäßig unter `src/core/`; `core = yes` verlinkt sie in das Target. Tool-Discovery kann mit `[project] disable_auto_tools = yes` deaktiviert werden. Ein Target kann mit `enabled = no` deaktiviert oder mit `remove = yes` aus den vorhandenen Definitionen entfernt werden.

`src` ist relativ zum Projekt-Root. `id` und `bin` sind standardmäßig einfache sichere Namen; `..`/absolute Pfade werden abgewiesen. `--unsafe-paths` und `allow_unsafe_paths = yes` nur nach bewusstem Sicherheitsreview nutzen.

## 5. Listen und Layering

Konfigurationsreihenfolge: **explizit gewählte INI** (`--config`) beziehungsweise Projekt-INI über einem optional aus `tackfile.c` generierten Layer, darüber hinaus eingebaute Fallbacks. `--no-code-config` unterbindet die *Ausführung* der Codekonfiguration, nicht die INI. `--no-config` deaktiviert beide.

Werte für `includes`, `defines`, `cflags`, `ldflags`, `libs` sind zusätzliche Tokenlisten. Bevorzugt `;` als Trenner; zitierte Tokens können Leerzeichen enthalten:

```ini
[target "app".release]
includes = include;"vendor headers"
defines = FEATURE=1;BUILD_ID=42
cflags = -std=c89;-Wextra;-Werror
ldflags = -s
libs = -lm
```

**Wichtig:** Ein nichtleeres Feld ersetzt die entsprechende *Extra-Liste* des darunterliegenden Layers; Profilfelder ersetzen die jeweiligen Basis-Extra-Listen. Es ersetzt **nicht** die eingebauten Basisflags. Ein leeres `cflags =` ist **kein zuverlässiger Befehl zum Löschen** einer geerbten Liste, weil beim Materialisieren leere Listen teilweise als „nicht gesetzt“ behandelt werden. Ebenso sollte `core` in eigenen Target-Overrides explizit gesetzt werden, wenn Shared Core wichtig ist.

## 6. DOC, BOM und SBOM

```ini
[doc]
template = templates/tack_template_min.html
css = templates/tack_doc.css
allow_js_search = yes

[bom]
template = templates/tack_template_min.html
css = templates/tack_doc.css

[sbom]
format = cyclonedx-1.4
output = build/sbom.cdx.json
```

`tack ... doc` schreibt HTML nach `build/doc/`; `tack ... bom` erstellt ein Build-Manifest, `tack ... sbom` erstellt eine **Build-Input-SBOM**, keine vollständige Dependency-Auflösung. `output` in `[sbom]` gilt nur für Single-Target-Exporte; bei `--all-targets` erzeugt tack getrennte Dateien. Fehlende konfigurierte HTML-Templates/CSS führen zu einem Fehler, nicht zu stillschweigendem Fallback. DOC bleibt ohne JavaScript lesbar; die Suche ist optional.

## 7. Formatter-Regeln: bekannter Defekt in tack 0.8.4

**WARNUNG: Nicht als funktionsfähiges Beispiel freigegeben.** Die folgende INI zeigt die *vorgesehene* Syntax. Im untersuchten 0.8.4-Produktionscode liegt die Verarbeitung von `[fmt]` und `[fmt "NAME"]` irrtümlich innerhalb eines unerreichbaren `[sbom]`-Zweigs (`src/tack.c`, `ini_load_file`). Deshalb werden selbst `rules`, `include` und `cmd` **nicht wirksam übernommen**. Mit einer ausdrücklich deklarierten Regel endet `fmt --list --strict` reproduzierbar mit `tack: fmt: rule=c: missing cmd`. Der Fehler muss in einem eigenen C89-Korrekturschritt mit Test-Ratchet-/Coverage-Abnahme repariert werden, **bevor** Formatter-INI als 0.8.5-Funktion dokumentiert und veröffentlicht wird.

```ini
[fmt]
enabled = yes
defaults = off
rules = c
exclude = build/**;.git/**

[fmt "c"]
tool = clang-format
mode = file
include = src/**/*.c;include/**/*.h;tests/**/*.c
cmd = clang-format -i --style=file @FILE@
requires = clang-format
```

```bat
tack --config .\tack.ini fmt --list --rule c --strict
tack --config .\tack.ini fmt --check --rule c --strict
```

Nach **Behebung des Parserdefekts** soll `--list` die Dateiauswahl zeigen und `--check` Dateien unverändert lassen; Exitcode 2 kann Formatabweichung *oder* Konfigurations-/Regelfehler bedeuten. `mode=file` benötigt `@FILE@`; `mode=batch` benötigt `@RSP@`. Der Formatter selbst ist eine **optionale externe Abhängigkeit**, kein Buildtool-Zwang.

**Zusätzliche Implementierungsgrenze nach Reparatur des Parserzweigs:** `check_cmd`, `diff_cmd` und `env.NAME` werden im Code zwar als Felder erkannt, aber nicht als eigene Kommandos beziehungsweise Umgebungsvariablen angewandt. Auch `fail_fast = no` garantiert derzeit kein Fortsetzen nach Regelversagen. Diese Felder **nicht** als fertige Features planen; siehe Referenz.

## 8. Coverage für Linux und Windows getrennt

```ini
[target "app".coverage]
compiler = gcc
compiler_policy = gcc
cflags = -std=c89;-pedantic-errors;-Wextra;-Werror;--coverage;-O0
ldflags = --coverage

[coverage]
id = linux-gcc-14.2.0-gcov-14.2.0-coverage-v1
id_windows = windows-gcc-10.3.0-gcov-10.3.0-coverage-v1
```

Ein Messprofil muss zur **tatsächlich verwendeten** OS-/Compiler-/gcov-Kombination passen. Die IDs oben sind **Beispiele für die im tack-Repository verwendeten konkreten Referenzumgebungen**, keine universell gültigen Defaults.

```bat
set TACK_CC=gcc
tack --config .\tack.ini test coverage --ci --rebuild -j 1
type coverage\current.txt
tack --config .\tack.ini coverage check
```

Beim ersten vollständigen Durchlauf ohne angenommene Baseline ist `BLOCKED no accepted HWM` erwartet. Nach fachlichem Review erfolgt `tack ... coverage accept --owner-approve`, wodurch die eigene plattformspezifische HWM versioniert werden kann. Eine fehlende Messung, Regression oder ein inkompatibles Profil darf **nie** per Herabsetzen einer HWM geheilt werden. Siehe [Coverage-Anleitung](coverage-ratchet.md).

## 9. Diagnosen in der richtigen Reihenfolge

1. `tack --config .\tack.ini doctor release --target app`: Compilerprogramm, Herkunft und Policy ansehen.
2. `tack --config .\tack.ini list`: Targetname und Aktivierung verifizieren.
3. `tack --config .\tack.ini build release --target app --rebuild -j 1`: Fehler reproduzierbar machen.
4. Bei Compilerwechsel `--rebuild`, bei Cache-Verdacht `--no-cache` **vor** dem Kommando setzen.
5. Bei Coverage zuerst `coverage/current.txt` und `TACK_SUMMARY` prüfen, erst dann eine HWM akzeptieren.

---

# English

## 1. Mental model and first build

`tack.ini` is declarative configuration, not an executable build script. Start tack from the **repository root** and explicitly select the config in shared scripts or CI:

```sh
tack --config ./tack.ini list
```

The global `--config` option goes **before** the command. A minimal GCC-based C89 program with `src/main.c` and optional tests:

```ini
[project]
default_target = app
compiler = gcc
compiler_policy = auto
allow_unsafe_paths = no

[target "app"]
src = src
bin = hello
id = app
core = no
includes = include

[target "app".release]
cflags = -std=c89;-pedantic-errors;-Wextra;-Werror
```

```sh
tack --config ./tack.ini doctor release --target app
tack --config ./tack.ini build release --target app --rebuild -j 1
tack --config ./tack.ini run release --target app
tack --config ./tack.ini test release --ci --rebuild -j 1
```

GCC must be installed. With no overrides the built-in compiler default is **tcc**. A repository-specific test inventory may impose additional requirements not present in this generic minimal example.

## 2. What v0.8.2-dev added

Compiler program and built-in compiler **policy** can be selected independently per target **profile**:

```text
TACK_CC environment
  > [target "NAME".debug|release|coverage] compiler
  > [project] compiler
  > built-in tcc
```

`compiler_policy = auto|tcc|gcc|clang|generic` selects toolchain-specific built-in behavior; `auto` infers the policy from the selected compiler name. It does **not** select a compiler program.

```ini
[project]
compiler = tcc
compiler_policy = auto

[target "app".debug]
compiler = tcc
compiler_policy = tcc

[target "app".release]
compiler = gcc
compiler_policy = auto
cflags = -std=c89;-pedantic-errors;-Wextra;-Werror
```

Use `tack ... doctor debug --target app` and `doctor release --target app` to inspect the effective choice. `TACK_CC=gcc` overrides the profile compiler. Rebuild when switching compilers within the same profile, as compiler identity is not yet completely isolated in cache keys. Debug adds `-g` and `-DDEBUG=1` across compilers; `-bt20` is TinyCC-only. `cflags` are **extras** in addition to built-in flags.

A `compiler` key in an **unprofiled** `[target "NAME"]` section is not implemented. Use `[project]` or a profiled `[target "NAME".release]` instead.

## 3. Multi-target and shared core

```ini
[target "app"]
src = src/app
bin = server
id = app
core = yes
includes = include;src

[target "tool:dbgen"]
src = tools/dbgen
bin = dbgen
id = dbgen
core = yes
includes = include;src

[target "tool:dbgen".release]
defines = DBGEN_RELEASE=1
```

Run `tack ... list` before `tack ... build release --target tool:dbgen`. Shared core sources default to `src/core/`, linked when `core = yes`. `disable_auto_tools = yes` disables automatic discovery. `enabled = no` disables a target; `remove = yes` removes it from the assembled definitions. Keep `src` repository-relative and `id`/`bin` safe tokens. Unsafe-path opt-ins are for reviewed exceptional cases only.

## 4. Layering and token lists

The explicit `--config` file or repository `tack.ini` overrides the optional INI generated from `tackfile.c`, with built-ins as fallbacks. `--no-code-config` stops the code generator but retains the INI; `--no-config` suppresses both.

The `includes`, `defines`, `cflags`, `ldflags`, `libs` fields are **extra token lists**, preferably semicolon-separated, with quotes for tokens containing spaces:

```ini
[target "app".release]
includes = include;"vendor headers"
defines = FEATURE=1;BUILD_ID=42
cflags = -std=c89;-Wextra;-Werror
ldflags = -s
libs = -lm
```

Nonempty per-profile lists replace their matching underlying **extra** list, not tack's built-in base flags. An **empty** `cflags =` is not a reliable way to clear an inherited list because empty override lists may be materialized as absent. Explicitly keep `core = yes|no` when core behavior matters.

## 5. DOC, BOM, SBOM and formatters

```ini
[doc]
template = templates/tack_template_min.html
css = templates/tack_doc.css
allow_js_search = yes

[bom]
template = templates/tack_template_min.html
css = templates/tack_doc.css

[sbom]
format = cyclonedx-1.4
output = build/sbom.cdx.json
```

DOC generates offline-readable HTML; BOM describes build inputs; SBOM is a **build-input SBOM**, not a package resolver. The SBOM `output` setting affects a single-target export; `--all-targets` writes separate per-target files. Configured missing DOC/BOM templates or CSS fail rather than silently falling back.

**Known tack 0.8.4 defect:** The `[fmt]` and `[fmt "NAME"]` key handlers are accidentally nested inside the `[sbom]` conditional in `ini_load_file` and are unreachable. The following is the **intended syntax, not a working 0.8.4 configuration**. `fmt --list --strict` with an explicit rule currently reports `missing cmd`. Repair needs a separate C89 test and independent coverage gates before 0.8.5 release.

```ini
[fmt]
enabled = yes
defaults = off
rules = c
exclude = build/**;.git/**

[fmt "c"]
tool = clang-format
mode = file
include = src/**/*.c;include/**/*.h;tests/**/*.c
cmd = clang-format -i --style=file @FILE@
requires = clang-format
```

Once the parser defect is fixed, `fmt --list --rule c --strict` should preview selection and `fmt --check --rule c --strict` should preserve sources. In 0.8.4 explicit rule configuration currently fails; exit code 2 may also denote a configuration or rule error. `mode=file` requires `@FILE@`; `mode=batch` requires `@RSP@`.

**Further limitation after a parser fix:** `check_cmd`, `diff_cmd`, and `env.NAME` exist as source-level fields but are not applied in current formatter execution paths. `fail_fast = no` does not currently guarantee continuing after a formatter error. Do not treat these as operational features.

## 6. Separate platform-specific coverage ratchets

```ini
[target "app".coverage]
compiler = gcc
compiler_policy = gcc
cflags = -std=c89;-pedantic-errors;-Wextra;-Werror;--coverage;-O0
ldflags = --coverage

[coverage]
id = linux-gcc-14.2.0-gcov-14.2.0-coverage-v1
id_windows = windows-gcc-10.3.0-gcov-10.3.0-coverage-v1
```

These are **actual tack-reference toolchain examples**, not portable IDs for arbitrary installations. The measured profile must match the real GCC/gcov/OS combination. Run `test coverage --ci --rebuild -j 1`, inspect `coverage/current.txt`, then `coverage check`. The initial complete run without an accepted HWM blocks by design. Only after reviewing a real measurement should the owner run `coverage accept --owner-approve`. Never lower an accepted HWM to hide regression. See [full gate guide](coverage-ratchet.md).

## 7. Troubleshooting

Check `doctor` (compiler/policy), then `list` (target), then a reproducible `build release --target app --rebuild -j 1`. Put `--no-cache` before the subcommand when diagnosing cache behavior. Inspect the test summary and current measured coverage before approving any HWM. Exact supported keys and implementation caveats are in the [INI reference](../reference/tack-ini.md).
