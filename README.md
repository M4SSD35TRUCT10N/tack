# tack — Tiny ANSI-C Kit (v0.6.0)

---

> **DE/EN README**
> 
> DE: Ein schlanker **Build‑Driver** für C (C89/ANSI‑C), inspiriert von Cargo‑Workflows: `init`, `list`, `build`, `run`, `test` — ohne Make/CMake/Ninja‑Stack.  
> EN: A tiny **build driver** for C (C89/ANSI‑C), inspired by Cargo workflows: `init`, `list`, `build`, `run`, `test` — without a Make/CMake/Ninja stack.

**Links:** [FAQ](FAQ.md) • [Roadmap](ROADMAP.md)

---

# Deutsch

`tack` ist ein **Build- und Projekt-Werkzeug in einer einzigen C-Datei** (C89/ANSI‑C).
Es ist für Projekte gedacht, die **ohne Make/CMake/Ninja** auskommen sollen und trotzdem:

- mehrere Targets (App + Tools + eigene Targets) sauber bauen,
- Shared Code (Core) wiederverwenden,
- **data-only** konfigurierbar bleiben (`tack.ini`) – ohne fremden C-Code im Repo,
- optional „Power-Konfiguration“ per `tackfile.c` nutzen (Code, aber kontrolliert und fail-fast),
- und sich hervorragend mit **tcc** (Tiny C Compiler) fahren lassen.

**Leitidee:** *Build-Logik ist Code.* Wenn dein Projekt C ist, darf auch die Build-Pipeline C sein.

## Warum tack (und warum nicht Make/CMake/Ninja)?

### Warum es Make/CMake/Ninja etc. überhaupt gibt
- **Make**: pragmatisches, zeitstempelbasiertes Rebuild-System.
- **CMake**: Generator („Makefiles/IDE-Projekte für alles“).
- **Ninja**: schneller Executor für große (meist generierte) Build-Graphen.
- **jam/b2**: alternative Regel-/Graph-Modelle.
- **mk (Plan 9/9front)**: sehr elegant, aber nicht überall vorhanden.

### Warum tack trotzdem sinnvoll ist
- Du willst **keinen Build-Stack** (Generator → Executor → Toolchain).
- Du willst eine **schmale Pipeline**: `tcc -run src/tack.c ...`
- Du willst Build-Logik **wie C-Code debuggen**.
- Du willst **Portabilität** (C89) und einfache Verteilung (eine Datei oder ein kleines `tack.exe`).

### Was tack ist
- Ein **einzelnes C‑Programm** als Build‑Tool (C89), das Targets findet und baut.
- Fokus: **DX** (einheitliche Kommandos), **Portabilität**, **nachvollziehbare Builds**.
- `tack.ini` als Standard‑Konfiguration (data‑only); optional `tackfile.c` für Code‑Konfig.

### Was tack nicht ist
- **Kein Package Manager** (kein Resolver/Registry/Lockfile).  
- Kein IDE‑Projektgenerator wie CMake (bewusst).

## Features (v0.6.0)

- Single-file Build Driver (C89)
- Kein Make/CMake/Ninja
- Recursive Scanning: `src/**/*.c`, `tools/<name>/**/*.c`, `tests/**/*_test.c`
- Target Discovery: `app` + `tool:<name>` (aus `tools/`, per Config abschaltbar)
- Declarative Targets: add/modify/disable/remove (via `tack.ini` und/oder `tackfile.c`)
- `tack list` zeigt Targets (Name + id + src + core + enabled)
- Robuste Prozessausführung (kein `system()` für Builds)
- Parallel Compile: `-j N`
- Depfiles (`-MD -MF`) für Incremental Builds
- Strict Mode: `--strict` aktiviert zusätzlich `-Wunsupported`
- Echte Target-Konfiguration: Includes/Defines/CFLAGS/LDFLAGS/LIBS pro Target
- Shared Core Code: `src/core/` wird 1× pro Profil gebaut und optional gelinkt
- **Konfiguration / Layering**:
  - `tack.ini` (runtime, data-only, auto-load; höchste Priorität)
  - `tackfile.c` (optional, runtime → generiertes INI-Layer; niedrigere Priorität als `tack.ini`)
  - Built-ins in `tack.c` (Fallback)

## Repository-Struktur (wie im Repo)

Dieses Repo legt `tack` unter `src/tack.c` ab. Du kannst es aber auch in die Repo-Root legen – wichtig ist nur, dass du `tack` aus dem Projekt-Root startest (weil relative Pfade wie `src/`, `tools/`, `build/` verwendet werden).

### Projektstruktur (Konventionen)

- **App**
  - Standard: `src/`
  - Optional: `src/app/` (wenn vorhanden, nimmt tack bevorzugt `src/app`)
- **Shared Core**
  - `src/core/` (wird einmal pro Profil kompiliert)
- **Tools**
  - `tools/<name>/` → Target `tool:<name>` (1 Ebene tief; Quellen darunter rekursiv)
- **Tests**
  - `tests/**/*_test.c` → wird gebaut und ausgeführt

## Quickstart

### Option A: direkt aus `tack.c` laufen lassen (tcc)

Windows:
```bat
tcc -run src/tack.c init
tcc -run src/tack.c list
tcc -run src/tack.c build debug -v -j 8
tcc -run src/tack.c run debug -- --hello "Berlin"
```

Linux/BSD:
```sh
tcc -run src/tack.c init
tcc -run src/tack.c list
tcc -run src/tack.c build debug -v -j 8
tcc -run src/tack.c run debug -- --hello Berlin
```

### Option B: `tack.exe` bauen (für CI/Teams)

Windows (tcc):
```bat
tcc src/tack.c -o tack.exe
tack.exe init
tack.exe build debug -j 8 -v
```

## Globale Optionen (müssen vor dem Kommando stehen)

```bat
tack.exe --config tack.ci.ini build release
tack.exe --no-config build debug
tack.exe --no-auto-tools list
```

- `--config <path>`: explizite INI-Datei laden (höchste Priorität)
- `--no-config`: **alle** Konfiguration deaktivieren (`tack.ini` und `tackfile.c`)
- `--no-auto-tools`: Auto-Discovery von `tools/<name>` deaktivieren (praktisch für rein deklarative Builds)

## Kommandos

- `help`, `version`, `doctor`
- `init` – Grundstruktur & Hello-World erzeugen
- `list` – Targets anzeigen
- `build [debug|release] ...` – Target bauen
- `run [debug|release] ... -- <args...>` – Target bauen + ausführen
- `test [debug|release] ...` – `_test.c` bauen + ausführen
- `clean` – Inhalt von `build/` löschen, Ordner bleibt
- `clobber` – `build/` komplett löschen

### Warum “clean” und “clobber” (statt distclean)?
`distclean` stammt aus Make-Welten („putze auch generierte Konfig“).  
Bei tack ist es klar getrennt:
- **clean**: „Baureste weg, Struktur bleibt“
- **clobber**: „alles weg“

## Konfiguration

### 1) `tack.ini` — Data-only Konfiguration (empfohlen)

Wenn `tack.ini` vorhanden ist (oder per `--config PATH` gesetzt wird), lädt tack sie automatisch – außer du setzt `--no-config`.

**Sektionen**
- `[project]`
- `[target "NAME"]` (oder ohne Quotes: `[target tool:foo]`)

**Schlüssel in `[project]`**
- `default_target = app`
- `disable_auto_tools = yes|no`

**Schlüssel in `[target ...]`**
- `src = <dir>`        (rekursiver `.c`-Scan)
- `bin = <name>`       (Exe-Base-Name)
- `id = <safe_id>` (optional; Ordnername unter `build/<id>/...`)
- `enabled = yes|no`
- `remove = yes|no`
- `core = yes|no`
- `includes = a;b;c`   (ohne `-I`, tack setzt `-I` selbst)
- `defines  = A=1;B=2` (ohne `-D`, tack setzt `-D` selbst)
- `cflags   = ...`     (Tokens, per `;` getrennt)
- `ldflags  = ...`     (Tokens, per `;` getrennt)
- `libs     = ...`     (Tokens, per `;` getrennt)

**Listen-Format:** Semikolon-getrennt (`;`).  
Leerzeichen um Tokens herum sind ok, aber Tokens sollten keine eingebetteten Leerzeichen enthalten.

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

### 2) `tackfile.c` — Code-Konfiguration (optional, runtime, fail-fast)

Wenn `tackfile.c` im Projekt-Root existiert, dann:

1. tack kompiliert automatisch einen kleinen Generator unter `build/_tackfile/`
2. der Generator erzeugt `build/_tackfile/tackfile.generated.ini`
3. tack lädt diese generierte INI als **Low-Priority-Layer** (unterhalb von `tack.ini`)

Wenn `tackfile.c` **nicht** kompiliert oder ausgeführt werden kann, bricht tack mit Fehler ab (fail-fast).

**Warum so?**  
Viele Teams wollen „nur Daten“ (`tack.ini`) – aber manchmal brauchst du Code, um Targets dynamisch zu definieren. Mit dem Generator-Ansatz bleibt der Host (`tack.exe`) stabil, und du bekommst trotzdem Code-Flexibilität.

#### Makros in `tackfile.c` (gleiches Format wie bisher)

**a) Overrides (per Target)**
```c
#define TACKFILE_OVERRIDES my_overrides

static const char *gen_defines[] = { "TOOL_GEN=1", 0 };

static const TargetOverride my_overrides[] = {
  { "tool:gen", 0, gen_defines, 0, 0, 0, 1 },
  { 0,0,0,0,0,0,0 }
};
```

**b) Targets add/modify/disable/remove**
```c
#define TACKFILE_TARGETS my_targets

static const TargetDef my_targets[] = {
  /* upsert / define */
  { "demo:hi", "demos/hi", "hi", "demo_hi", 1, 0 },

  /* action: disable (src/bin/id = 0) */
  { "tool:old", 0, 0, 0, 0, 0 },

  /* action: remove (remove=1) */
  { "tool:tmp", 0, 0, 0, 0, 1 },

  { 0,0,0,0,0,0 }
};
```

**c) Default Target**
```c
#define TACKFILE_DEFAULT_TARGET "app"
```

**d) Auto Tool Discovery deaktivieren**
```c
#define TACKFILE_DISABLE_AUTO_TOOLS 1
```

### Prioritäten / Layering (wichtig)

„Höchste Priorität gewinnt“:
1. `--config <path>` / `tack.ini`
2. generiertes `tackfile.generated.ini` (aus `tackfile.c`)
3. built-ins in `tack.c`

### Legacy/Lockdown: compile-time `-DTACK_USE_TACKFILE`

Falls du **gar keine** dynamische Code-Konfiguration zur Laufzeit willst (z.B. in sehr strikten Umgebungen), kannst du `tackfile.c` auch compile-time einbinden:

```bat
tcc -DTACK_USE_TACKFILE src/tack.c -o tack.exe
```

In diesem Modus wird die runtime-Generator-Variante nicht verwendet.

## Shared Core (`src/core/`)

**Wofür?** App und Tools teilen sich oft Logik (Parser/IO/Protokolle/Utilities).  
Core wird einmal gebaut und in mehrere Targets gelinkt.

- Objekte landen unter `build/_core/<profile>/obj/...`
- Targets mit `core=yes` (INI) bzw. `use_core=1` (Override) linken diese Objekte dazu
- `--no-core` schaltet Core für den aktuellen Aufruf aus

## Strict Mode (`--strict`)

Unter Windows enthalten System-Header oft GCC-Attribute (`format`, `nonnull`). Mit `-Werror` können solche Warnungen Builds abbrechen.

Darum ist Default: `-Wno-unsupported`  
`--strict` schaltet bewusst wieder strenger:

```bat
tcc -run src/tack.c build debug --strict
```

## ROADMAP
Weitere Infos, wie es mit tack weitergehen wird, ist [hier](ROADMAP.md).

## FAQ
Eine detailierte FAQ ist [hier](FAQ.md).

## Lizenz
MIT

---

# English

`tack` is a **single‑file build & project tool written in C** (C89/ANSI‑C).
It targets projects that intentionally want to **avoid Make/CMake/Ninja** while still having:

- clean multi‑target builds (app + tools + custom targets),
- shared code reuse via a “core” (`src/core/`),
- **data‑only** configuration via `tack.ini` (no executable config required),
- optional “power config” via `tackfile.c` (code, but controlled and fail-fast),
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
- you want a **thin pipeline**: `tcc -run src/tack.c ...`,
- you want to **debug build logic as C code**,
- you want **portability** (C89) and easy distribution (one file or a small `tack.exe`).

## Features (v0.6.0)

- single‑file build driver (C89)
- No Make/CMake/Ninja
- Recursive scanning: `src/**/*.c`, `tools/<name>/**/*.c`, `tests/**/*_test.c`
- Target discovery: `app` + `tool:<name>` (from `tools/`, can be disabled)
- Declarative targets: add/modify/disable/remove (via `tack.ini` and/or `tackfile.c`)
- `tack list` prints targets (name + id + src + core + enabled)
- Robust process execution (no `system()` for builds)
- Parallel compile: `-j N`
- Depfiles (`-MD -MF`) for incremental builds
- strict mode: `--strict` enables `-Wunsupported` (default suppresses it)
- real per‑target config: includes/defines/cflags/ldflags/libs/core
- Shared core code: `src/core/` built once per profile, optionally linked
- **Configuration layering**:
  - `tack.ini` (runtime, data‑only, auto‑load; highest priority)
  - `tackfile.c` (optional, runtime → generated INI layer; lower priority than `tack.ini`)
  - built‑ins in `tack.c` (fallback)

## Repo layout

This repo keeps tack at `src/tack.c`. You may also place it in the repo root — just run tack from the project root, because it uses relative paths like `src/`, `tools/`, and `build/`.

### Project layout conventions

- **App**
  - default: `src/`
  - optional: `src/app/` (if present, preferred)
- **Shared core**
  - `src/core/` (built once per profile)
- **Tools**
  - `tools/<name>/` → target `tool:<name>` (one level deep; sources below scanned recursively)
- **Tests**
  - `tests/**/*_test.c` (built and executed)

## Quickstart

### Option A: run from source (tcc)
Windows:
```bat
tcc -run src/tack.c init
tcc -run src/tack.c list
tcc -run src/tack.c build debug -v -j 8
tcc -run src/tack.c run debug -- --hello "Berlin"
```

Linux/BSD:
```sh
tcc -run src/tack.c init
tcc -run src/tack.c list
tcc -run src/tack.c build debug -v -j 8
tcc -run src/tack.c run debug -- --hello Berlin
```

### Option B: build `tack.exe` (CI/teams)

```bat
tcc src/tack.c -o tack.exe
tack.exe init
tack.exe build debug -j 8 -v
```

## Global options (must come before the command)

```bat
tack.exe --config tack.ci.ini build release
tack.exe --no-config build debug
tack.exe --no-auto-tools list
```

- `--config <path>`: load explicit INI (highest priority)
- `--no-config`: disable **all** configuration (`tack.ini` and `tackfile.c`)
- `--no-auto-tools`: disable `tools/<name>` auto discovery (useful for fully declarative builds)

## Commands

- `help`, `version`, `doctor`
- `init` – create a minimal skeleton + hello world
- `list` – show targets
- `build [debug|release] ...` – build target
- `run [debug|release] ... -- <args...>` – build + run target
- `test [debug|release] ...` – build + execute `_test.c`
- `clean` – delete contents of `build/` (keep directory)
- `clobber` – delete `build/` entirely

## Configuration

### 1) `tack.ini` — data-only config (recommended)

Auto-loaded if present (or via `--config`), unless `--no-config` is set.

See the German section above for the full key list. The format is the same.

### 2) `tackfile.c` — optional code config (runtime, fail-fast)

If `tackfile.c` exists in the project root:

1. tack compiles a small generator under `build/_tackfile/`
2. the generator writes `build/_tackfile/tackfile.generated.ini`
3. tack loads that generated INI as a **low-priority layer** (below `tack.ini`)

If `tackfile.c` cannot be compiled or executed, tack exits with an error (fail-fast).

#### Macros in `tackfile.c` (same format as before)

```c
#define TACKFILE_DEFAULT_TARGET "app"
#define TACKFILE_DISABLE_AUTO_TOOLS 1
#define TACKFILE_TARGETS my_targets
#define TACKFILE_OVERRIDES my_overrides
```

### Layering / priorities (highest wins)

1. `--config <path>` / `tack.ini`
2. generated `tackfile.generated.ini` (from `tackfile.c`)
3. built-ins in `tack.c`

### Legacy/lockdown: compile-time `-DTACK_USE_TACKFILE`

```bat
tcc -DTACK_USE_TACKFILE src/tack.c -o tack.exe
```

When enabled, the runtime generator path is not used.

## Shared core (`src/core/`)

Core is built once per profile and linked into targets with `core = yes`.
Use `--no-core` to skip core linking for the current invocation.

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

## ROADMAP
A detailed roadmap on how tack is evolving is available [here](ROADMAP.md).

## FAQ
A detailed FAQ is available [here](FAQ.md).


## License
MIT

