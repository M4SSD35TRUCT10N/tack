# tack — Tiny ANSI-C Kit (v0.5.0)

> **DE/EN README**

# Deutsch

`tack` ist ein **Build- und Projekt-Werkzeug in einer einzigen C-Datei** (C89/ANSI‑C). Es ist für Projekte gedacht, die **ohne Make/CMake/Ninja** auskommen sollen und trotzdem:

- mehrere Targets (App + Tools + eigene Targets) sauber bauen,
- Shared Code (Core) wiederverwenden,
- **data-only** konfigurierbar bleiben (`tack.ini`) – ohne fremden C-Code im Repo,
- optional „Power-Konfiguration“ per `tackfile.c` nutzen (compile-time),
- und sich hervorragend mit **tcc** (Tiny C Compiler) fahren lassen.

**Leitidee:** *Build-Logik ist Code.* Wenn dein Projekt C ist, darf auch die Build-Pipeline C sein.

## Warum tack (und warum nicht Make/CMake/Ninja)?

### Warum es Make/CMake/Ninja etc. überhaupt gibt
- **Make**: pragmatisches, zeitstempelbasiertes Rebuild-System.
- **CMake**: Generator („Makefiles/IDE-Projekte für alles“).
- **Ninja**: schneller Executor für große Build-Graphen (meist generiert).
- **jam/b2**: alternative Regel-/Graph-Modelle.
- **mk (Plan 9/9front)**: sehr elegant, aber nicht überall vorhanden.

### Warum tack trotzdem sinnvoll ist
- Du willst **keinen Build-Stack** (Generator → Executor → Toolchain).
- Du willst eine **schmale Pipeline**: `tcc -run tack.c ...`
- Du willst Build-Logik **wie C-Code debuggen**.
- Du willst **Portabilität** (C89) und einfache Verteilung (eine Datei oder ein kleines `tack.exe`).

## Features (v0.5.0)

- Single-file Build Driver (C89)
- Kein Make/CMake/Ninja
- Recursive Scanning: `src/**/*.c`, `tools/<name>/**/*.c`, `tests/**/*_test.c`
- Target Discovery: `app` + `tool:<name>` (aus `tools/`, abschaltbar)
- Eigene Targets deklarativ definieren/ändern/abschalten/entfernen (via `tack.ini` oder `tackfile.c`)
- `tack list` zeigt Targets (Name + id + src + core + enabled)
- Robuste Prozessausführung (kein `system()` für Builds)
- Parallel Compile: `-j N`
- Depfiles (`-MD -MF`) für Incremental Builds
- Strict Mode: `--strict` aktiviert zusätzlich `-Wunsupported`
- Echte Target-Konfiguration (Includes/Defines/CFLAGS/LDFLAGS/LIBS pro Target)
- Shared Core Code: `src/core/` wird 1× pro Profil gebaut und optional gelinkt
- **Konfiguration**
  - `tack.ini` (runtime, data-only, auto-load; Highest Priority)
  - `tackfile.c` (compile-time include, optional; Middle Priority)
  - Built-ins in `tack.c` (Fallback)

## Installation / Verwendung

Lege `tack.c` in die Repo-Root.

### Option A: direkt aus `tack.c` laufen lassen (tcc)
Windows:
```bat
tcc -run tack.c init
tcc -run tack.c list
tcc -run tack.c build debug -v -j 8
tcc -run tack.c run debug -- --hello "Berlin"
```

Linux/BSD:
```sh
tcc -run tack.c init
tcc -run tack.c list
tcc -run tack.c build debug -v -j 8
tcc -run tack.c run debug -- --hello Berlin
```

### Option B: `tack.exe` bauen (für CI/Teams)
Windows (tcc):
```bat
tcc tack.c -o tack.exe
tack.exe init
tack.exe build debug -j 8 -v
```

Mit optionalem `tackfile.c` (compile-time):
```bat
tcc -DTACK_USE_TACKFILE tack.c -o tack.exe
```

## Compiler wählen (Env)

Standard ist `tcc`. Du kannst den Compiler pro Umgebung überschreiben:

- `TACK_CC` → Compiler-Binary (muss im PATH sein)

Beispiele:
```bat
set TACK_CC=clang
tcc -run tack.c doctor
```

```sh
export TACK_CC=cc
tcc -run tack.c doctor
```

## Projektstruktur (Konventionen)

- **App**
  - Standard: `src/`
  - optional: `src/app/` (wenn vorhanden, nimmt tack bevorzugt `src/app`)
- **Shared Core**
  - `src/core/` (wird einmal pro Profil kompiliert)
- **Tools**
  - `tools/<name>/` → Target `tool:<name>` (nur 1 Ebene tief; Quellen darunter rekursiv)
- **Tests**
  - `tests/**/*_test.c` → wird gebaut und ausgeführt

## Kommandos

### Wichtige Regel: globale Optionen stehen **vor** dem Kommando
`tack` parst globale Optionen zuerst. Beispiele:
```bat
tcc -run tack.c --no-config build debug
tcc -run tack.c --config tack.ci.ini build release
tcc -run tack.c --no-auto-tools list
```

Als `tack.exe`:
```bat
tack.exe --config tack.ini build debug
```

### help / version / doctor
```bat
tcc -run tack.c help
tcc -run tack.c version
tcc -run tack.c doctor
```

### init
Erzeugt (wenn nicht vorhanden) Grundstruktur & Hello-World.
```bat
tcc -run tack.c init
```

### list
Listet Targets (inkl. id, src, core yes/no, enabled yes/no).
```bat
tcc -run tack.c list
```

### build
```bat
tcc -run tack.c build debug   -j 8 -v
tcc -run tack.c build release --target tool:foo
```

Optionen:
- `-v` / `--verbose` → zeigt Compiler-Kommandos
- `--rebuild`        → erzwingt Neuaufbau
- `-j N`             → paralleles Kompilieren
- `--strict`         → aktiviert zusätzlich `-Wunsupported`
- `--target NAME`    → Target wählen (Name **oder** `id`)
- `--no-core`        → Core für diesen Aufruf nicht linken

### run
Alles hinter `--` wird an das Programm durchgereicht.
```bat
tcc -run tack.c run debug -- --hello "Berlin"
```

### test
```bat
tcc -run tack.c test debug -v
```

### clean / clobber
- **clean**: Inhalt von `build/` löschen, Ordner bleibt
- **clobber**: `build/` komplett löschen
```bat
tcc -run tack.c clean
tcc -run tack.c clobber
```

## Warum “clean” und “clobber” (statt distclean)?

`distclean` stammt aus Make-Welten („putze auch generierte Konfig“). Bei tack ist es klar getrennt:
- **clean**: „Baureste weg, Struktur bleibt“
- **clobber**: „alles weg“

## Konfiguration: `tack.ini` (Data) und `tackfile.c` (Code)

### `tack.ini` — Data-only Konfiguration (empfohlen)
Wenn `tack.ini` vorhanden ist (oder per `--config PATH` gesetzt wird), lädt tack sie automatisch – außer du setzt `--no-config`.

**Sektionen:**
- `[project]`
- `[target "<name>"]` (oder ohne Quotes: `[target tool:foo]`)

**Schlüssel in `[project]`:**
- `default_target = app`
- `disable_auto_tools = yes|no`

**Schlüssel in `[target ...]`:**
- `src = <dir>`        (rekursiver `.c`-Scan)
- `bin = <name>`       (Exe-Base-Name)
- `id = <id>`          (optional, build/<id>/...)
- `enabled = yes|no`
- `remove = yes|no`
- `core = yes|no`      (Core linken für dieses Target)
- `includes = a;b;c`   (ohne `-I`, tack setzt `-I` selbst)
- `defines  = A=1;B=2` (ohne `-D`, tack setzt `-D` selbst)
- `cflags   = ...`     (Tokens, per `;` getrennt)
- `ldflags  = ...`     (Tokens, per `;` getrennt)
- `libs     = ...`     (Tokens, per `;` getrennt)

**Listen-Format:** Semikolon-getrennt (`;`). Leerzeichen um Tokens herum sind ok, aber Tokens sollten keine eingebetteten Leerzeichen enthalten.

Beispiel `tack.ini`:
```ini
[project]
default_target = app
disable_auto_tools = no

[target "app"]
core = yes
includes = include; src
defines = FEATURE_X=1

[target "tool:gen"]
src = tools/gen
bin = gen
core = yes
libs = -lws2_32

[target "tool:old"]
enabled = no

[target "tool:tmp"]
remove = yes
```

### `tackfile.c` — Code-Konfiguration (optional)
`tackfile.c` ist **kein** Runtime-Plugin in v0.5.0, sondern wird compile-time eingebunden.

Aktivieren:
```bat
tcc -DTACK_USE_TACKFILE -run tack.c list
```

In `tackfile.c` kannst du (optional) definieren:

1) Overrides (höhere Priorität als built-ins):
```c
#define TACKFILE_OVERRIDES my_overrides
static const char *gen_defines[] = { "TOOL_GEN=1", 0 };
static const TargetOverride my_overrides[] = {
  { "tool:gen", 0, gen_defines, 0, 0, 0, 1 },
  { 0,0,0,0,0,0,0 }
};
```

2) Targets add/modify/disable/remove:
```c
#define TACKFILE_TARGETS my_targets
static const TargetDef my_targets[] = {
  { "demo:hi", "demos/hi", "hi", "demo_hi", 1, 0 }, /* upsert */
  { "tool:old", 0,0,0, 0, 0 },                      /* disable action */
  { "tool:tmp", 0,0,0, 0, 1 },                      /* remove action */
  { 0,0,0,0,0,0 }
};
```

3) Default Target:
```c
#define TACKFILE_DEFAULT_TARGET "app"
```

4) Auto Tool Discovery deaktivieren (voll deklarativ):
```c
#define TACKFILE_DISABLE_AUTO_TOOLS 1
```

### Prioritäten (wichtig)
- **Flags/Overrides:** `tack.ini` → `tackfile.c` → built-in `tack.c`
- **Target-Graph:** Discovery → `tackfile.c` → `tack.ini`

## Roadmap (v0.6.0+): `tackfile.c` als Runtime-Plugin (DLL/SO)

In **v0.5.0** ist `tackfile.c` eine **compile-time** Option (`-DTACK_USE_TACKFILE`).  
Für **v0.6.0+** ist als nächster Evolutionsschritt geplant, `tackfile.c` **zur Laufzeit** zu laden:

- `tack.exe` kompiliert `tackfile.c` automatisch zu einem Plugin (Windows: **DLL**, POSIX: **SO**)
- Plugin wird geladen und registriert Targets/Overrides via Host‑API
- Änderungen an `tackfile.c` wirken **ohne Neubau** von `tack.exe` (perfekt für CI/Dev)

**Security/CI:** zusätzlich ein Schalter `--no-code-config`, damit Teams strikt auf `tack.ini` bestehen können.

(Die sichere Default-Empfehlung bleibt: **INI als Standard**, Code-Konfig nur wenn bewusst gewünscht.)



## Shared Core (src/core)

**Wofür?** App und Tools teilen sich oft Logik (Parser/IO/Protokolle/Utilities).  
Mit `src/core/` gibt’s eine klare Trennung: Core wird einmal gebaut und mehrfach gelinkt.

**Wie?**
- Objekte landen unter `build/_core/<profile>/obj/...`
- Targets mit `core=yes` (INI) bzw. `use_core=1` (Override) linken diese Objekte dazu
- `--no-core` schaltet Core für den aktuellen Aufruf aus

## Strict Mode (`--strict`)

Unter Windows enthalten System-Header oft GCC-Attribute (`format`, `nonnull`).  
Mit `-Werror` können solche Warnungen Builds abbrechen. Darum ist Default:
- `-Wno-unsupported`

`--strict` schaltet bewusst wieder strenger:
```bat
tcc -run tack.c build debug --strict
```

## Troubleshooting (kurz)

- **Warnungen aus stdio.h** + kein `.exe`: `--strict` aus lassen (Default ist korrekt).
- **`keep undeclared`** o.ä.: oft Kommentar versehentlich beendet.
- **Pfade mit Leerzeichen**: tack nutzt `spawn/exec`, daher deutlich weniger Quoting-Probleme.

## Lizenz
MIT

---

# English

`tack` is a **single‑file build & project tool written in C** (C89/ANSI‑C). It targets projects that intentionally want to **avoid Make/CMake/Ninja** while still having:

- clean multi‑target builds (app + tools + custom targets),
- shared code reuse via a “core” (`src/core/`),
- **data‑only** configuration via `tack.ini` (no executable config required),
- optional “power config” via `tackfile.c` (compile‑time include),
- a workflow that plays very well with **tcc** (Tiny C Compiler).

**Core idea:** *Build logic is code.* If your project is C, your build pipeline can be C too.

## Why tack (instead of Make/CMake/Ninja)?

### Why those tools exist
- **Make**: pragmatic, timestamp‑based rebuild system.
- **CMake**: a generator (“Makefiles/IDE projects for everything”).
- **Ninja**: fast executor for large generated build graphs.
- **jam/b2**: alternative rule/graph models.
- **mk (Plan 9/9front)**: elegant, but not universally available.

### Why tack can be a better fit
- you want **no build stack** (generator → executor → toolchain),
- you want a **thin pipeline**: `tcc -run tack.c ...`,
- you want to **debug build logic as C code**,
- you want **portability** (C89) and easy distribution (one file or a small `tack.exe`).

## Features (v0.5.0)

- single‑file build driver (C89)
- No Make/CMake/Ninja
- Recursive scanning: `src/**/*.c`, `tools/<name>/**/*.c`, `tests/**/*_test.c`
- Target discovery: `app` + `tool:<name>` (from `tools/`, can be disabled)
- Declarative targets: add/modify/disable/remove (via `tack.ini` or `tackfile.c`)
- `tack list` prints targets (name + id + src + core + enabled)
- Robust process execution (no `system()` for builds)
- Parallel compile: `-j N`
- Depfiles (`-MD -MF`) for incremental builds
- strict mode: `--strict` enables `-Wunsupported` (default suppresses it)
- real per‑target config: includes/defines/cflags/ldflags/libs/core
- Shared core code: `src/core/` built once per profile, optionally linked
- configuration sources (priority):
  - `tack.ini` (runtime, data‑only, auto‑load; highest priority)
  - `tackfile.c` (compile‑time include, optional; middle priority)
  - built‑ins in `tack.c` (fallback)

## Quickstart

Put `tack.c` into your repo root.

### Option A: run from source (tcc)
Windows:
```bat
tcc -run tack.c init
tcc -run tack.c list
tcc -run tack.c build debug -v -j 8
tcc -run tack.c run debug -- --hello "Berlin"
```

Linux/BSD:
```sh
tcc -run tack.c init
tcc -run tack.c list
tcc -run tack.c build debug -v -j 8
tcc -run tack.c run debug -- --hello Berlin
```

### Option B: build `tack.exe` (CI/teams)
```bat
tcc tack.c -o tack.exe
tack.exe init
tack.exe build debug -j 8 -v
```

With optional `tackfile.c` (compile‑time):
```bat
tcc -DTACK_USE_TACKFILE tack.c -o tack.exe
```

## Pick a compiler (Env)

Default compiler is `tcc`. Override it with:

- `TACK_CC` → compiler binary (must be in PATH)

## Project layout conventions

- **App**
  - default: `src/`
  - optional: `src/app/` (if present, preferred)
- **Shared core**
  - `src/core/` (built once per profile)
- **Tools**
  - `tools/<name>/` → target `tool:<name>` (one level deep; sources below scanned recursively)
- **Tests**
  - `tests/**/*_test.c` (built and executed)

## Commands

### Important rule: global options come **before** the command
Examples:
```bat
tack.exe --no-config build release
tack.exe --config ci/windows.ini build release
tack.exe --no-auto-tools list
```

### help / version / doctor
```bat
tack.exe help
tack.exe version
tack.exe doctor
```

### init
Creates (if missing) a minimal project skeleton + Hello World.
```bat
tack.exe init
```

### list
Lists targets (id, src, core yes/no, enabled yes/no).
```bat
tack.exe list
```

### build
```bat
tack.exe build debug   -j 8 -v
tack.exe build release --target tool:foo
```

Options:
- `-v` / `--verbose` → print compiler/link commands
- `--rebuild`        → force rebuild
- `-j N`             → parallel compilation
- `--strict`         → re-enable `-Wunsupported`
- `--target NAME`    → select target (name **or** `id`)
- `--no-core`        → do not link shared core for this invocation

### run
Everything after `--` is forwarded to the target executable.
```bat
tack.exe run debug -- --hello "Berlin"
```

### test
Builds and runs `tests/**/*_test.c`.
```bat
tack.exe test debug -v
```

### clean / clobber
- **clean**: remove contents of `build/`, keep the directory
- **clobber**: remove `build/` itself
```bat
tack.exe clean
tack.exe clobber
```

## Why “clean” and “clobber” (instead of distclean)?

`distclean` is a Make‑ism (“also remove generated configuration”). In tack it’s explicit:
- **clean**: “remove build artefacts, keep structure”
- **clobber**: “remove everything under build root”

## Configuration: `tack.ini` (data) and `tackfile.c` (code)

### `tack.ini` — data-only (recommended)
If `tack.ini` exists (or is set via `--config PATH`), tack loads it automatically unless you pass `--no-config`.

Sections:
- `[project]`
- `[target "<name>"]` (quotes optional; e.g. `[target tool:foo]`)

Keys in `[project]`:
- `default_target = app`
- `disable_auto_tools = yes|no`

Keys in `[target ...]`:
- `src = <dir>`        (recursive `.c` scan)
- `bin = <name>`       (executable base name)
- `id = <id>`          (optional: controls `build/<id>/...`)
- `enabled = yes|no`
- `remove = yes|no`
- `core = yes|no`      (link shared core for this target)
- `includes = a;b;c`   (without `-I`)
- `defines  = A=1;B=2` (without `-D`)
- `cflags   = ...`     (`;`-separated tokens)
- `ldflags  = ...`     (`;`-separated tokens)
- `libs     = ...`     (`;`-separated tokens)

**List format:** `;`-separated. Whitespace around tokens is fine; avoid embedded spaces inside a token.

Example `tack.ini`:
```ini
[project]
default_target = app
disable_auto_tools = no

[target "app"]
core = yes
includes = include; src
defines = FEATURE_X=1

[target "tool:gen"]
src = tools/gen
bin = gen
core = yes
libs = -lws2_32

[target "tool:old"]
enabled = no

[target "tool:tmp"]
remove = yes
```

### `tackfile.c` — optional code config (compile-time)
In v0.5.0, `tackfile.c` is **not** a runtime plugin; it’s a compile‑time include.

Enable it with:
```bat
tcc -DTACK_USE_TACKFILE tack.c -o tack.exe
```

Optional definitions inside `tackfile.c`:
- custom overrides array via `TACKFILE_OVERRIDES`
- custom targets/upserts/disable/remove via `TACKFILE_TARGETS` (using `TargetDef`)
- `TACKFILE_DEFAULT_TARGET`
- `TACKFILE_DISABLE_AUTO_TOOLS` (fully declarative builds)

### Priorities (important)
- **Overrides/flags:** `tack.ini` → `tackfile.c` → built‑in `tack.c`
- **Target graph:** discovery → `tackfile.c` → `tack.ini`

## Shared Core (src/core)

**Why?** Apps and tools often share logic (IO, parsers, utilities).  
`src/core/` gives you a clear split: core is built once per profile and linked into multiple targets.

- core objects live under `build/_core/<profile>/obj/...`
- targets with `core=yes` (INI) or `use_core=1` (override) link them
- `--no-core` disables core linking for the current invocation

## Strict mode (`--strict`)

On Windows, system headers may contain GCC-style attributes (`format`, `nonnull`). With `-Werror` this can break builds. Default behaviour is:
- suppress unsupported warnings (`-Wno-unsupported`)

Use `--strict` to intentionally re-enable them:
```bat
tack.exe build debug --strict
```

## Troubleshooting

- warnings from `stdio.h` and missing `.exe`: don’t enable `--strict` unless you want those warnings.
- strange compile errors like `... undeclared`: often a comment accidentally ended early.
- paths with spaces: tack uses spawn/exec rather than shell `system()`, so quoting issues are reduced.

## Roadmap (v0.6.0+): tackfile.c runtime plugin (DLL/SO)

In **v0.5.0**, `tackfile.c` is **compile‑time** only (`-DTACK_USE_TACKFILE`).  
For **v0.6.0+**, the next step is a runtime plugin model:

- `tack.exe` compiles `tackfile.c` into a plugin (Windows: **DLL**, POSIX: **SO**)
- loads it automatically and registers targets/overrides through a host API
- changes take effect **without rebuilding** `tack.exe` (great for CI/dev workflows)

**Security/CI:** add `--no-code-config` so teams can enforce INI‑only policies.

(Default recommendation remains: **INI first**, code config only when explicitly desired.)

## License
MIT
