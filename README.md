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

`tack` is a **single-file build & project tool written in C** (C89/ANSI‑C). It targets projects that want to **avoid Make/CMake/Ninja** while still getting:

- clean multi-target builds (app + tools + custom targets),
- shared code reuse (core),
- **data-only** configuration via `tack.ini`,
- optional “power config” via `tackfile.c` (compile-time),
- and a smooth workflow with **tcc** (Tiny C Compiler).

**Core idea:** *Build logic is code.* If your project is C, your build pipeline can be C too.

## Features (v0.5.0)

- Single-file build driver (C89)
- No Make/CMake/Ninja
- Recursive scanning: `src/**/*.c`, `tools/<name>/**/*.c`, `tests/**/*_test.c`
- Target discovery: `app` + `tool:<name>` (from `tools/`, can be disabled)
- Declarative targets: add/modify/disable/remove (via `tack.ini` or `tackfile.c`)
- `tack list` prints targets (name + id + src + core + enabled)
- Robust process execution (no `system()` for builds)
- Parallel compile: `-j N`
- Depfiles (`-MD -MF`) for incremental builds
- Strict mode: `--strict` enables `-Wunsupported`
- Per-target configuration (includes/defines/cflags/ldflags/libs/core)
- Shared core code: `src/core/` built once per profile, optionally linked

## Installation / Usage

Put `tack.c` into your repo root.

### Option A: run from source (tcc)
```bat
tcc -run tack.c init
tcc -run tack.c list
tcc -run tack.c build debug -v -j 8
tcc -run tack.c run debug -- --hello "Berlin"
```

### Option B: build `tack.exe`
```bat
tcc tack.c -o tack.exe
tack.exe init
tack.exe build debug -j 8 -v
```

### Compiler override
Set `TACK_CC` to pick a different compiler.

## License
MIT
