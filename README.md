# tack — Tiny ANSI-C Kit (v0.4.3)

> **DE/EN README**

# Deutsch

`tack` ist ein **Build- und Projekt-Werkzeug in einer einzigen C-Datei** (C89/ANSI‑C). Es ist für Projekte gedacht, die **ohne Make/CMake/Ninja** auskommen sollen und trotzdem:

- mehrere Targets (App + Tools + Demos) sauber bauen,
- Shared Code (Core) wiederverwenden,
- konfigurierbar bleiben (inkl. optionaler Projektdatei `tackfile.c`),
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
- Du willst **Portabilität** (C89) und einfache Verteilung (eine Datei).

## Features (v0.4.3)

- Single-file Build Driver (C89)
- Kein Make/CMake/Ninja
- Recursive Scanning: `src/**/*.c`, `tools/<name>/**/*.c`, `tests/**/*_test.c`
- Target Discovery: `app` + `tool:<name>` (aus `tools/`)
- `tack list` zeigt Targets
- Robuste Prozessausführung (kein `system()` für Builds)
- Parallel Compile: `-j N`
- Depfiles (`-MD -MF`) für Incremental Builds
- Strict Mode: `--strict` aktiviert zusätzlich `-Wunsupported`
- Echte Target-Konfiguration (Includes/Defines/Libs pro Target)
- Shared Core Code: `src/core/` wird 1× pro Profil gebaut und gelinkt
- `tackfile.c` (optional):
  - Overrides auslagern
  - Targets hinzufügen/ändern (Upsert)
  - Targets deaktivieren/entfernen
  - Auto Tool Discovery abschaltbar (voll deklarativ)

## Installation / Verwendung

Lege `tack.c` in die Repo-Root.

### Windows (tcc)
```bat
tcc -run tack.c init
tcc -run tack.c list
tcc -run tack.c build debug -v -j 8
tcc -run tack.c run debug -- --hello "Berlin"
```

### Linux/BSD
```sh
tcc -run tack.c init
tcc -run tack.c list
tcc -run tack.c build debug -v -j 8
tcc -run tack.c run debug -- --hello Berlin
```

## Projektstruktur (Konventionen)

- **App**
  - Standard: `src/`
  - optional: `src/app/` (wenn vorhanden, nimmt tack bevorzugt `src/app`)
- **Shared Core**
  - `src/core/` (wird einmal pro Profil kompiliert)
- **Tools**
  - `tools/<name>/` → Target `tool:<name>`
- **Tests**
  - `tests/**/*_test.c` → wird gebaut und ausgeführt

## Kommandos

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
Listet Targets (inkl. id, src, core yes/no).
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
- `--target NAME`    → Target wählen
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

## Echte Target-Konfiguration (Overrides)

`tack.c` enthält eine Override-Tabelle pro Target:
- extra Includes (`-I...`)
- Defines (`-D...`)
- zusätzliche CFLAGS / LDFLAGS / LIBS
- `use_core` (Core linken ja/nein)

Beispiel: Tool bekommt Define + Lib
```c
static const char *foo_defines[] = { "TOOL_FOO=1", 0 };
static const char *foo_libs[]    = { "-lws2_32", 0 };

static const TargetOverride g_overrides[] = {
  { "app",      0, 0, 0, 0, 0, 1 },
  { "tool:foo", 0, foo_defines, 0, 0, foo_libs, 1 },
  { 0,0,0,0,0,0,0 }
};
```

## Shared Core (src/core)

**Wofür?** App und Tools teilen sich oft Logik (Parser/IO/Protokolle/Utilities).  
Mit `src/core/` gibt’s eine klare Trennung: Core wird einmal gebaut und mehrfach gelinkt.

**Wie?**  
- Objekte landen unter `build/_core/<profile>/obj/...`
- Targets mit `use_core=1` linken diese Objekte dazu

## tackfile.c (optional, empfohlen für Projekte)

`tackfile.c` lagert Projekt-Konfiguration aus. Aktiviert wird es explizit:

```bat
tcc -DTACK_USE_TACKFILE -run tack.c list
tcc -DTACK_USE_TACKFILE -run tack.c build debug
```

### 1) Auto Tool Discovery deaktivieren (voll deklarativ)
```c
#define TACKFILE_DISABLE_AUTO_TOOLS 1
```

### 2) Default Target setzen
```c
#define TACKFILE_DEFAULT_TARGET "app"
```

### 3) Targets hinzufügen/ändern/disable/remove

`TargetDef` (v0.4.3):
```c
typedef struct {
  const char *name;      /* CLI: "app", "tool:foo", "demo:hi" ... */
  const char *src_dir;   /* recursive .c scan */
  const char *bin_base;  /* exe base name */
  const char *id;        /* optional build id; if 0: derived */
  int enabled;           /* action mode: 1 enable, 0 disable */
  int remove;            /* action mode: 1 remove */
} TargetDef;
```

**Upsert (add/modify):**
```c
static const TargetDef my_targets[] = {
  { "demo:hi", "demos/hi", "hi", "demo_hi", 1, 0 },
  { 0,0,0,0,0,0 }
};
#define TACKFILE_TARGETS my_targets
```

**Action-Modus (disable/enable/remove):**  
Gilt, wenn `src_dir/bin_base/id` **alle 0** sind.
```c
static const TargetDef my_targets[] = {
  /* disable */
  { "tool:old", 0,0,0, 0, 0 },
  /* enable */
  { "tool:old", 0,0,0, 1, 0 },
  /* remove */
  { "tool:tmp", 0,0,0, 0, 1 },
  { 0,0,0,0,0,0 }
};
#define TACKFILE_TARGETS my_targets
```

### 4) Overrides aus tackfile.c
```c
#define TACKFILE_OVERRIDES my_overrides

static const char *gen_defines[] = { "TOOL_GEN=1", 0 };

static const TargetOverride my_overrides[] = {
  { "tool:gen", 0, gen_defines, 0, 0, 0, 1 },
  { 0,0,0,0,0,0,0 }
};
```

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
- **`keep undeclared`** o.ä.: oft Kommentar versehentlich beendet (`*/` in `build/*/*` etc.).
- **Pfade mit Leerzeichen**: tack nutzt `spawn/exec`, daher deutlich weniger Quoting-Probleme.

## Lizenz
MIT

---

# English

`tack` is a **single-file build & project tool written in C** (C89/ANSI‑C). It is designed for projects that want to **avoid Make/CMake/Ninja** while still getting:

- clean multi-target builds (app + tools + demos),
- shared code reuse (core),
- configuration (optionally via a project file `tackfile.c`),
- and a smooth workflow with **tcc** (Tiny C Compiler).

**Core idea:** *Build logic is code.* If your project is C, your build pipeline can be C too.

## Why tack (instead of Make/CMake/Ninja)?

### Why those tools exist
- **Make**: pragmatic timestamp-based rebuilds.
- **CMake**: a generator (“Makefiles/IDE projects for everything”).
- **Ninja**: a fast executor for large generated build graphs.
- **jam/b2**: alternative rule/graph models.
- **mk (Plan 9/9front)**: elegant, but not universally available.

### Why tack can be the better fit
- You want **no build stack** (generator → executor → toolchain).
- You want a **thin pipeline**: `tcc -run tack.c ...`
- You want to **debug build logic as C code**.
- You want **portability** (C89) and easy distribution (one file).

## Features (v0.4.3)

- Single-file build driver (C89)
- No Make/CMake/Ninja
- Recursive scanning: `src/**/*.c`, `tools/<name>/**/*.c`, `tests/**/*_test.c`
- Target discovery: `app` + `tool:<name>` (from `tools/`)
- `tack list` prints targets
- Robust process execution (no `system()` for builds)
- Parallel compile: `-j N`
- Depfiles (`-MD -MF`) for incremental builds
- Strict mode: `--strict` enables extra `-Wunsupported`
- Real per-target configuration (includes/defines/libs per target)
- Shared core code: `src/core/` built once per profile and linked into targets
- Optional `tackfile.c`:
  - externalize overrides
  - add/modify targets (upsert)
  - disable/remove targets
  - disable auto tool discovery (fully declarative builds)

## Install / Use

Put `tack.c` in your repo root.

### Windows (tcc)
```bat
tcc -run tack.c init
tcc -run tack.c list
tcc -run tack.c build debug -v -j 8
tcc -run tack.c run debug -- --hello "Berlin"
```

### Linux/BSD
```sh
tcc -run tack.c init
tcc -run tack.c list
tcc -run tack.c build debug -v -j 8
tcc -run tack.c run debug -- --hello Berlin
```

## Project layout conventions

- **App**
  - default: `src/`
  - optional: `src/app/` (preferred when present)
- **Shared core**
  - `src/core/` (compiled once per profile)
- **Tools**
  - `tools/<name>/` → target `tool:<name>`
- **Tests**
  - `tests/**/*_test.c` → built and executed

## Commands

### help / version / doctor
```bat
tcc -run tack.c help
tcc -run tack.c version
tcc -run tack.c doctor
```

### init
Creates a baseline structure and hello-world (if missing).
```bat
tcc -run tack.c init
```

### list
Shows targets (including id, src dir, core yes/no).
```bat
tcc -run tack.c list
```

### build
```bat
tcc -run tack.c build debug   -j 8 -v
tcc -run tack.c build release --target tool:foo
```

Options:
- `-v` / `--verbose` → prints compiler commands
- `--rebuild`        → forces rebuild
- `-j N`             → parallel compilation
- `--strict`         → enables `-Wunsupported`
- `--target NAME`    → choose a target
- `--no-core`        → do not link shared core for this invocation

### run
Everything after `--` is forwarded to the built program.
```bat
tcc -run tack.c run debug -- --hello "Berlin"
```

### test
```bat
tcc -run tack.c test debug -v
```

### clean / clobber
- **clean**: remove contents of `build/`, keep the directory
- **clobber**: remove the entire `build/` directory
```bat
tcc -run tack.c clean
tcc -run tack.c clobber
```

## Why “clean” and “clobber” (instead of distclean)?

`distclean` is a Make-era convention (“also remove generated configuration”).  
`tack` uses clearer semantics:
- **clean**: remove build artifacts, keep structure
- **clobber**: remove everything (fresh start)

## Real per-target configuration (Overrides)

`tack.c` contains a per-target override table:
- extra includes (`-I...`)
- defines (`-D...`)
- extra CFLAGS / LDFLAGS / LIBS
- `use_core` (link core yes/no)

Example: add a define and a Windows library for a tool
```c
static const char *foo_defines[] = { "TOOL_FOO=1", 0 };
static const char *foo_libs[]    = { "-lws2_32", 0 };

static const TargetOverride g_overrides[] = {
  { "app",      0, 0, 0, 0, 0, 1 },
  { "tool:foo", 0, foo_defines, 0, 0, foo_libs, 1 },
  { 0,0,0,0,0,0,0 }
};
```

## Shared core (src/core)

**Why?** Apps and tools often share logic (parsers/IO/protocol/utilities).  
Place it in `src/core/` so it is compiled once and linked into multiple executables.

**How?**
- objects end up under `build/_core/<profile>/obj/...`
- targets with `use_core=1` link those objects

## tackfile.c (optional, recommended for real projects)

`tackfile.c` externalizes project configuration. Enable it explicitly:

```bat
tcc -DTACK_USE_TACKFILE -run tack.c list
tcc -DTACK_USE_TACKFILE -run tack.c build debug
```

### 1) Disable auto tool discovery (fully declarative builds)
```c
#define TACKFILE_DISABLE_AUTO_TOOLS 1
```

### 2) Set default target
```c
#define TACKFILE_DEFAULT_TARGET "app"
```

### 3) Add/modify/disable/remove targets

`TargetDef` (v0.4.3):
```c
typedef struct {
  const char *name;
  const char *src_dir;
  const char *bin_base;
  const char *id;
  int enabled;
  int remove;
} TargetDef;
```

Upsert (add/modify):
```c
static const TargetDef my_targets[] = {
  { "demo:hi", "demos/hi", "hi", "demo_hi", 1, 0 },
  { 0,0,0,0,0,0 }
};
#define TACKFILE_TARGETS my_targets
```

Action mode (disable/enable/remove):  
Triggered when `src_dir/bin_base/id` are all `0`.
```c
static const TargetDef my_targets[] = {
  { "tool:old", 0,0,0, 0, 0 }, /* disable */
  { "tool:old", 0,0,0, 1, 0 }, /* enable */
  { "tool:tmp", 0,0,0, 0, 1 }, /* remove */
  { 0,0,0,0,0,0 }
};
#define TACKFILE_TARGETS my_targets
```

### 4) Overrides from tackfile.c
```c
#define TACKFILE_OVERRIDES my_overrides

static const char *gen_defines[] = { "TOOL_GEN=1", 0 };

static const TargetOverride my_overrides[] = {
  { "tool:gen", 0, gen_defines, 0, 0, 0, 1 },
  { 0,0,0,0,0,0,0 }
};
```

## Strict mode (`--strict`)

On Windows, system headers may contain GCC attributes (`format`, `nonnull`).  
Combined with `-Werror`, those can break builds if `-Wunsupported` is enabled.

Default behavior:
- `-Wno-unsupported`

Enable strict explicitly:
```bat
tcc -run tack.c build debug --strict
```

## Troubleshooting (short)

- **Warnings from stdio.h** + no `.exe`: don’t enable strict (default is correct).
- **`keep undeclared`** and similar: usually a block comment accidentally ended (`*/` inside `build/*/*` etc.).
- **Paths with spaces**: tack uses `spawn/exec`, much less quoting pain than `system()`.

## License
MIT
