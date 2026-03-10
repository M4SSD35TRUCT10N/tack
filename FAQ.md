# tack FAQ / FQA (DE/EN) — v0.7.19

**Backlinks:** [README](README.md) • [Roadmap](ROADMAP.md) • [Release Notes](RELEASENOTES.md)

---

## Deutsch (FAQ)

### Nutzt tack einen Compile-Cache?
Ja. `tack` kann Kompilergebnisse in `.tack-cache/` ablegen, um wiederholte Builds zu beschleunigen.

Die Cache-Validierung nutzt bewusst zwei Regeln:
- **normale Dependencies**: `mtime` (Modifikationszeit) + Dateigröße + Content-Hash (32-bit FNV-1a; nicht kryptografisch)
- **Depfile selbst**: Dateigröße + Content-Hash

So bleibt ein Cache-Restore nach `tack clean` deterministisch, obwohl das Depfile vor dem Restore neu geschrieben wird.

### Wie kann ich den Cache deaktivieren?
Nutze:
- `--no-cache`

### Wie kann ich den Cache löschen?

- `tack clean --cache`
- `tack clobber`

### Ist tack ein Ersatz für CMake/make?
tack ersetzt für viele Projekte das klassische Build-Skript (Makefile/CMakeLists), indem es eine feste, simple Konvention nutzt und die üblichen Tasks (`build/run/test/clean`) anbietet. Für extrem komplexe Toolchains ist make/cmake weiterhin besser geeignet.

### Erzeugt tack eine `.gitignore` oder Fossil-Ignore-Regeln?
Ja: `tack init` legt, falls nicht vorhanden, eine sinnvolle `.gitignore` sowie `.fossil-settings/ignore-glob` an. Zusätzlich erzeugt es (nicht destruktiv) `templates/` inkl. Standard-CSS/Template und eine Start-`tack.ini`. Wenn die Dateien bereits existieren, **überschreibt tack nichts**, sondern hängt nur einen klar markierten `tack`-Block an, sofern er noch fehlt.
Das ist bewusst „non-destructive“, damit bestehende Projektregeln erhalten bleiben.

### Welche Compiler funktionieren?
Standard ist **tcc**. Über `TACK_CC` kannst du z.B. `gcc` oder `clang` nutzen, solange sie „klassische“ C‑Kommandozeilen verstehen. Im Debug-Profil setzt tack die Basis-Flags compilerbewusst: `-g` und `-DDEBUG=1` allgemein, `-bt20` nur für tcc/TinyCC.  
**Wichtig:** `TACK_CC` ist der **Compiler**, nicht „Compiler + Flags“. Flags gehören in `tack.ini`.

### Warum lehnt tack `TACK_CC="clang -std=c89"` ab?
Weil tack den Compiler als argv[0] startet. Flags würden als Teil des Programnamens verstanden.  
Lösung: `TACK_CC=clang` und `cflags = -std=c89` in `tack.ini`.

### Wie nutze ich profil-spezifische Target-Overrides?
Du kannst in `tack.ini` pro Target eigene Overrides für `debug` und `release` definieren, z.B.:

```ini
[target "app"]
core = yes
includes = include; src

[target "app".debug]
defines = APP_DEBUG=1
cflags = -O0

[target "app".release]
cflags = -O3
```

In den Profil-Sektionen sind nur `core`, `includes`, `defines`, `cflags`, `ldflags`, `libs` erlaubt.  
Profilwerte überschreiben die Basis-Listen aus `[target "NAME"]` (oder aus `tackfile.c`/Built-ins) für das jeweilige Profil.

**Defines vs. CFLAGS:**  
`defines = FOO=1` ist funktional identisch zu `cflags = -DFOO=1`. tack wandelt `defines` intern in `-D`‑Flags um.  
Für Präprozessor‑Makros ist `defines` die klarere Variante; `cflags` ist für allgemeine Compiler‑Flags gedacht.

Beispiel (äquivalent):
```ini
[target "app".release]
defines = TACK_RELEASE=1
; oder:
; cflags = -DTACK_RELEASE=1
```

### Was ist der Unterschied zwischen `--no-config` und `--no-code-config`?
- `--no-config`: ignoriert **INI + tackfile.c** (alles aus)
- `--no-code-config`: ignoriert **nur tackfile.c**, INI bleibt aktiv (CI/Team‑Modus)

### Wie deaktiviere ich Auto-Tool-Discovery?
- CLI: `--no-auto-tools`
- INI: `[project] disable_auto_tools = yes`
- tackfile.c (Makro): `#define TACKFILE_DISABLE_AUTO_TOOLS 1`

### Wie kann ich Targets deaktivieren oder entfernen?
In INI oder tackfile.c (als generierte INI):
```ini
[target "tool:foo"]
enabled = no
```
oder
```ini
[target "tool:foo"]
remove = yes
```
`enabled=no` lässt das Target existieren, aber „aus“. `remove=yes` entfernt es aus dem Graph.

### Wie aktiviere/deaktiviere ich `src/core/`?
- Pro Target in INI: `core = yes|no`
- Für einen Lauf per CLI: `--no-core`

### Was bedeutet „fail-fast“ in tack?
tack bricht bewusst ab, wenn:
- Pfade/Strings zu lang werden
- INI‑Zeilen abgeschnitten wären (zu lang)
- Rekursionstiefe überschritten wird (Scan/RM)
- Token-/Listen-Limits überschritten werden
- tackfile.c Generator fehlschlägt

Das ist Absicht: lieber **klarer Fehler** statt undefiniertes Verhalten.


### Wie erkennt tack Header-Abhängigkeiten (Rebuild bei `.h`-Änderungen)?

`tack` erzeugt vor jedem Compile-Schritt ein Depfile (`.d`) im Format **tack-deps-v1** und nutzt es für Incremental Builds sowie für die Cache-Validierung. Für normale Dependencies werden `mtime`/Größe/Hash geprüft; das Depfile selbst wird bewusst nur über Größe/Hash validiert, damit Cache-Restores nach `clean` nicht vom Timestamp-Timing abhängen.

- Erfasst werden `#include "..."` (quoted) **und** `#include <...>` Includes **rekursiv** (auch Header, die wiederum Header includen).
- Die Suche folgt der üblichen Reihenfolge: Verzeichnis der includenden Datei → effektive `-I` Pfade (Built-ins + `includes = ...`).
- Für `<...>` wird gegen die effektiven Include-Pfade (`-I`) aufgelöst; wenn eine Datei dort gefunden wird, landet sie im Depfile/Cache-Metadaten.

Wenn du exotische Include-Muster verwendest (Makros/Generatoren), nutze im Zweifel `--rebuild` oder `tack clobber`.

### Wie finde ich heraus, warum tack neu baut („why rebuild“)?

Nutze `--why` (Alias: `--explain`). Das ist rein diagnostisch und löst **keinen** Rebuild aus.

Beispiele:
```bat
tack build debug --why -j 8
tack build release --why -j 8
```

Typische Gründe sind z.B. „missing output“, „input newer than output“, „depfile missing/changed“ oder „forced (--rebuild)“.

### Warum funktioniert `tack build --help` (oder `-h`)?

`tack help` zeigt die allgemeine Hilfe. Mit `tack build --help` (oder `-h`) wird die Hilfe **für dieses Sub‑Kommando** angezeigt.

Beispiele:
```bat
tack build --help
tack run --help
tack test --help
```

### Windows: Was ist mit langen Pfaden?
`tack` baut Join-Pfade dynamisch, bleibt aber bewusst **fail-fast**. Das hilft gegen starre interne Puffer, ersetzt aber keine Windows- oder Toolchain-Grenzen.

Praktische Checkliste:

- Repo/Workspace möglichst kurz halten, z. B. `C:\src\hello` oder `C:\w\proj` statt tiefer Benutzerprofil-Pfade.
- Wenn du die Maschine kontrollierst, Win32-Long-Paths aktivieren:
  ```powershell
  New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" `
  -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force
  ```
- Alternativ per Gruppenrichtlinie: `Computer Configuration > Administrative Templates > System > Filesystem > Enable Win32 long paths`
- Shell/IDE danach neu starten; Microsoft weist darauf hin, dass je nach Prozesszustand auch ein Reboot nötig sein kann.
- Wichtig: Das hilft nur Programmen, die selbst **long-path-aware** sind. tack kann Grenzen externer Werkzeuge (Git, Compiler, Archiver, Explorer, Shell) nicht umgehen.
- Wenn weiterhin Pfadprobleme auftreten: zuerst den Checkout-/Build-Pfad verkürzen und dann erneut testen.

### Windows: Warum findet tack meine `tack.ini` nicht, wenn tack im `PATH` liegt?
Wenn `tack.exe` aus einem Tool-Ordner im `PATH` gestartet wird und bei `--config` nur ein Dateiname ohne Pfad angegeben wird (z.B. `--config tack.ini`), kann die Config-Datei – je nach Aufruf/Setup – im **falschen Verzeichnis** gesucht werden (z.B. im Tool-Ordner statt im Projektordner).

Abhilfe: den Pfad explizit angeben.

- relativ zum aktuellen Ordner: `tack --config .\tack.ini build release -j 8`
- oder absolut: `tack --config C:\path\to\tack.ini build release -j 8`

### Windows: Warum wurde bei mir immer neu gebaut?

Wenn tack bei unveränderten Quellen in jedem Lauf neu baut, ist das fast immer ein Depfile-Problem (Pfadformat/Parsing) oder ein Zeitstempel-Thema.

- Ab **v0.7.2** werden typische Windows-Depfile-Pfade (z.B. `C:\...` / Backslashes) robuster verarbeitet.
- Wenn es trotzdem passiert: einmal mit `--why` laufen lassen und die Diagnose-Zeile anschauen (z.B. „depfile missing/changed“, „input newer than output“).

### Was ist der Unterschied zwischen BOM und SBOM?

**tack BOM** ist ein *Build-Manifest*: es beschreibt, *wie ein konkretes Build erzeugt wurde* (Targets, Inputs, Flags, Toolchain/OS, Outputs).

Eine **SBOM** (Software Bill of Materials) ist ein Supply-Chain-Artefakt (Komponenten + Abhängigkeiten, z.B. CycloneDX/SPDX).  
`tack sbom` erzeugt daraus bewusst eine **deterministische Build-Input-SBOM** aus den bekannten Build-Inputs (Default: `tack-sbom-1` unter `build/sbom.json`).  
CycloneDX/SPDX werden unterstützt, aber ohne Resolver/Package-Manager löst tack **keine** fremden Komponenten-Versionen auf. Die Ausgabe ist also eher „Input-SBOM“ als vollständige Supply-Chain-SBOM mit Versionsauflösung.  
Über `[sbom]` in `tack.ini` kannst du Format, Spec-Version und den **Single-Target**-Ausgabepfad `output` steuern (Standard-Dateien: `build/sbom.cdx.json` und `build/sbom.spdx.json`).  
Das tack-BOM vermeidet bewusst Ratespiele (z.B. keine Versionsableitung aus Linker-Flags).

**Aufbau der SBOM relativ zum Quellbaum (Beispiel):**

Angenommen, dein Projekt hat folgende Struktur:

```
src/
  app/main.c
  util/log.c
src/core/
  core.c
include/
  app.h
tools/pack/
  pack.c
```

Wenn du `tack sbom --target app` ausführst, schreibt tack eine JSON-Datei mit:

- `target.src`: **Root** für die Target-Sourcen (`src/app` oder `src/` je nach Konvention).
- `sources.target`: **alle `.c`-Dateien** unter dem Target-Root (rekursiv), z.B. `src/app/main.c`, `src/util/log.c`.
- `sources.core`: **alle `.c`-Dateien** unter `src/core/`, sofern Core aktiv ist.
- `flags.includes`: **Include-Pfade**, die tack effektiv nutzt (z.B. `include`, `src/app`, `src`, `src/core`).

Wichtig: Die SBOM enthält nur **bekannte Build-Inputs** (Dateipfade, Flags, libs).  
Sie versucht **nicht**, Abhängigkeitsversionen zu erraten oder fremde Komponenten zu „auflösen“.

### Wie erzeuge ich SBOMs für alle Targets?

Nutze:
- `tack sbom --all-targets`
- optional mit Profil: `tack sbom release --all-targets`
- optional mit anderem Zielordner: `tack sbom --all-targets --outdir out/sbom`

Dann schreibt tack standardmäßig **eine Datei pro aktiviertem Target**:
- `build/sbom.app.json`
- `build/sbom.tool_pack.json`
- bei CycloneDX/SPDX entsprechend `*.cdx.json` bzw. `*.spdx.json`

Wichtig: `[sbom] output = ...` bleibt bewusst ein **Single-Target-Pfad**. Für Batch-Exports ist `--all-targets` zuständig.

### Erzeugt `tack doc` API-Dokumentation wie `cargo doc` / rustdoc?

Nein. `tack doc` erzeugt eine kleine, offline-fähige Projekt-Doku-Site aus **allen Root-Markdowns** (`*.md` im Projekt-Root) sowie optional `docs/**/*.md` (wenn vorhanden) und verlinkt die BOM.  
Automatische API-Dokumentation für C würde einen Parser/Indexer erfordern und ist aktuell nicht Teil des `doc`-Features.

### Wird das erzeugte HTML Themes (Hell/Dunkel/Hoher Kontrast) und Templates unterstützen?

Ja, weitgehend.

- Hell/Dunkel: CSS-first über `prefers-color-scheme`.
- Templates + CSS: ab v0.7.1 via `tack.ini` (`[doc]`/`[bom]`: `template`, `css`).
- Hoher Kontrast: funktioniert grundsätzlich mit systemweiten High-Contrast/Forced-Colors-Settings; Feinschliff (z.B. `forced-colors`/`prefers-contrast`) bleibt optional.

### Gibt es eine Volltextsuche in der HTML-Doku?

Nicht standardmäßig. Die HTML-Ausgabe ist so entworfen, dass sie ohne JavaScript funktioniert.  
Ohne JS gilt: Index-Seite + Browser-Suche (Strg+F). Eine optionale, kleine JS-Suche (Progressive Enhancement) ist denkbar.

### Ergänzen oder überschreiben Werte in `tack.ini` die Flags aus Built-ins / tackfile?

Pro Target ersetzen Listenwerte in `tack.ini` (z.B. `cflags`, `defines`, `ldflags`, `libs`) die entsprechenden Extra-Listen aus `tackfile.c` / Built-ins.  
Diese Extra-Listen werden anschließend zu tacks internen Basis-Flags hinzugefügt (Warnungen + Profil-Flags wie `-g` / `-O2`).

### Unterstützt die HTML-Ausgabe Templates und CSS?

Ja. Standardmäßig nutzt tack ein eingebautes HTML-Layout. Ab v0.7.1 kann die Ausgabe für `tack doc` und `tack bom` optional über ein externes Template gestaltet werden:

```ini
[doc]
template = templates/tack_template_min.html
css      = templates/tack_doc.css

[bom]
; optional: wenn nicht gesetzt, verwendet BOM die DOC-Werte als Fallback
template = templates/tack_template_min.html
css      = templates/tack_doc.css
```

Empfohlen ist ein Ordner `templates/` neben `src/` (Assets, kein Quellcode). `css` wird in den Output kopiert und per `<link>` eingebunden; `template` wird nur gelesen.

**Fail-Fast:** Wenn `template` oder `css` gesetzt ist, die Datei aber fehlt oder nicht gelesen werden kann, bricht tack mit **Exitcode 2** ab.

### Welche Platzhalter unterstützt das HTML-Template?

- `{{TACK_PAGE_TITLE}}` (escaped)
- `{{TACK_PROJECT_TITLE}}` (escaped)
- `{{TACK_HEAD_ASSETS}}`
- `{{TACK_NAV_HTML}}`
- `{{TACK_TOC_HTML}}` (derzeit leer, aber als Markerblock vorhanden)
- `{{TACK_CONTENT_HTML}}` (**Pflicht**, sonst Fehler)
- `{{TACK_FOOTER_HTML}}`

Der Output enthält stabile IDs (`#tack-nav`, `#tack-content`, `#tack-footer`) als Vertrag für CSS-Hooks.
Marker-Kommentare (`<!-- TACK:BEGIN ... -->`) liefert das eingebaute Layout oder (bei Template-Ausgabe) das Template selbst. Wenn Marker für Post-Processing benötigt werden, müssen sie im Template um die Platzhalter liegen (siehe shipped Templates).

---

## English (FAQ)

### Does tack use a compile cache?
Yes. `tack` can store compile outputs in `.tack-cache/` to speed up repeated builds.

Cache validation deliberately uses two rules:
- **normal dependencies**: `mtime` (modification time) + file size + content hash (32-bit FNV-1a; not cryptographic)
- **the depfile itself**: file size + content hash

This keeps cache restores deterministic after `tack clean`, even though the depfile is rewritten before restore is attempted.

### How do I disable the cache?
Use:
- `--no-cache`

### How do I clear the cache?

- `tack clean --cache`
- `tack clobber`

### Is tack a replacement for CMake/make?
For many projects, yes: tack replaces custom build scripts by using conventions and providing `build/run/test/clean`. For very complex toolchains, make/cmake may still be the better fit.

### Does tack create a `.gitignore` or Fossil ignore rules?
Yes: `tack init` will provision a sensible `.gitignore` and `.fossil-settings/ignore-glob` if they don't exist.
If the files already exist, tack **won't overwrite** them; it only appends a clearly marked `tack` block if missing.
This is intentionally non-destructive to preserve existing project rules.

### Which compilers work?
Default is **tcc**. You can set `TACK_CC` to `gcc`/`clang` etc. as long as they behave like classic C compilers. In debug builds tack applies compiler-aware base flags: `-g` and `-DDEBUG=1` generically, `-bt20` only for tcc/TinyCC.  
Important: `TACK_CC` is the compiler program, not “compiler + flags”. Put flags into `tack.ini`.

### Why does tack reject `TACK_CC="clang -std=c89"`?
Because tack starts the compiler as argv[0]. Flags would be part of the program name.  
Fix: `TACK_CC=clang` and `cflags = -std=c89` in `tack.ini`.

### How do I use profile-specific target overrides?
You can define per-target overrides for `debug` and `release` in `tack.ini`, for example:

```ini
[target "app"]
core = yes
includes = include; src

[target "app".debug]
defines = APP_DEBUG=1
cflags = -O0

[target "app".release]
cflags = -O3
```

In profile sections, only `core`, `includes`, `defines`, `cflags`, `ldflags`, `libs` are allowed.  
Profile values replace the base lists from `[target "NAME"]` (or `tackfile.c`/built-ins) for that profile.

**Defines vs. CFLAGS:**  
`defines = FOO=1` is functionally identical to `cflags = -DFOO=1`. tack converts `defines` into `-D` flags internally.  
Use `defines` for preprocessor macros and `cflags` for general compiler flags.

Example (equivalent):
```ini
[target "app".release]
defines = TACK_RELEASE=1
; or:
; cflags = -DTACK_RELEASE=1
```

### `--no-config` vs `--no-code-config`?
- `--no-config`: ignore **INI + tackfile.c**
- `--no-code-config`: ignore **only tackfile.c**, still load INI

### Windows: What about long paths?
`tack` allocates joined paths dynamically, but it intentionally remains **fail-fast**. This avoids rigid internal buffers, but it does not remove Windows or toolchain limits.

Practical checklist:

- Keep the repo/workspace path short, e.g. `C:\src\hello` or `C:\w\proj`, instead of deep user-profile paths.
- If you control the machine, enable Win32 long paths:
  ```powershell
  New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" `
  -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force
  ```
- Or use Group Policy: `Computer Configuration > Administrative Templates > System > Filesystem > Enable Win32 long paths`
- Restart the shell/IDE afterwards; Microsoft notes that a reboot may still be required because the setting is cached per process.
- Important: this only helps programs that are themselves **long-path aware**. tack cannot bypass limits in external tools (Git, compiler, archiver, Explorer, shell).
- If path problems remain, shorten the checkout/build root first and test again.

### Windows: Why doesn’t tack find my `tack.ini` when tack is on `PATH`?
If `tack.exe` is started from a tools folder that’s on `PATH` and you pass only a bare file name to `--config` (e.g. `--config tack.ini`), the config file may be looked up in the **wrong directory** depending on your setup (for example the tools folder instead of your project folder).

Fix: provide an explicit path.

- relative to the current folder: `tack --config .\tack.ini build release -j 8`
- or absolute: `tack --config C:\path\to\tack.ini build release -j 8`

### How do I disable auto tool discovery?
- CLI: `--no-auto-tools`
- INI: `[project] disable_auto_tools = yes`
- tackfile.c macro: `#define TACKFILE_DISABLE_AUTO_TOOLS 1`

### Disable/remove targets?
```ini
[target "tool:foo"]
enabled = no
```
or
```ini
[target "tool:foo"]
remove = yes
```

### Enable/disable `src/core/`?
- Per target: `core = yes|no`
- Per run: `--no-core`

### What does “fail-fast” mean?
tack intentionally aborts on:
- overly long paths/strings
- truncated INI lines
- recursion depth limits (scan/rm)
- token/list limits
- tackfile.c generator failures

### Windows: Why did it rebuild every time?

If tack rebuilds on every run without source changes, it is usually a depfile path/parse issue or a timestamp issue.

- Since **v0.7.2**, typical Windows depfile paths (e.g. `C:\...` / backslashes) are handled more robustly.
- If it still happens: run once with `--why` and look at the reason (e.g. “depfile missing/changed”, “input newer than output”).

### What is the difference between BOM and SBOM?

**tack BOM** is a *build manifest*: it describes **how a specific build was produced** (targets, inputs, flags, toolchain/OS, outputs).

An **SBOM** (Software Bill of Materials) is a supply-chain artifact (components + dependencies, e.g. CycloneDX/SPDX).  
`tack sbom` intentionally emits a **deterministic build-input SBOM** from known build inputs (default: `tack-sbom-1` at `build/sbom.json`).  
CycloneDX/SPDX are supported, but without a resolver/package manager tack does **not** resolve third-party component versions. The result is therefore closer to an “input SBOM” than a fully resolved supply-chain SBOM.  
Use `[sbom]` in `tack.ini` to control the format, spec version, and the **single-target** output path `output` (default files: `build/sbom.cdx.json` and `build/sbom.spdx.json`).  
tack BOM intentionally avoids guesswork (e.g. it does not try to infer exact library versions from linker flags).

**How the SBOM maps to the source tree (example):**

Assume your project layout looks like this:

```
src/
  app/main.c
  util/log.c
src/core/
  core.c
include/
  app.h
tools/pack/
  pack.c
```

When you run `tack sbom --target app`, tack writes a JSON file with:

- `target.src`: the **root** of the target sources (`src/app` or `src/` depending on layout).
- `sources.target`: **all `.c` files** under the target root (recursive), e.g. `src/app/main.c`, `src/util/log.c`.
- `sources.core`: **all `.c` files** under `src/core/` when core is enabled.
- `flags.includes`: the **effective include paths** used by tack (e.g. `include`, `src/app`, `src`, `src/core`).

Note: The SBOM only contains **known build inputs** (file paths, flags, libs).  
It **does not** try to resolve third-party component versions or infer dependencies.

### How do I emit SBOMs for all targets?

Use:
- `tack sbom --all-targets`
- optionally with a profile: `tack sbom release --all-targets`
- optionally with another output directory: `tack sbom --all-targets --outdir out/sbom`

This writes **one file per enabled target** by default:
- `build/sbom.app.json`
- `build/sbom.tool_pack.json`
- for CycloneDX/SPDX: matching `*.cdx.json` or `*.spdx.json` files

Important: `[sbom] output = ...` intentionally remains a **single-target path**. Batch export is handled by `--all-targets`.

### Does `tack doc` generate API docs like `cargo doc` / rustdoc?

No. `tack doc` is a small, offline project documentation site from all root-level `*.md` files plus optional `docs/**/*.md` (if present) and links the BOM.  
Automatic API documentation for C would require a parser/indexer and is out of scope for the current `doc` feature.

### Will the generated HTML support themes (light/dark/high-contrast) and templates?

Yes, mostly.

- Light/dark: CSS-first via `prefers-color-scheme`.
- Templates + CSS: available since v0.7.1 via `tack.ini` (`[doc]`/`[bom]`: `template`, `css`).
- High-contrast: works with system high-contrast / forced-colors settings; extra polish (e.g. `forced-colors`/`prefers-contrast`) remains optional.

### Is there full-text search in the generated docs?

Not by default. The HTML output is designed to work without JavaScript.  
Without JS, use the index page + browser search (Ctrl+F). An optional minimal JS search (progressive enhancement) may be added later.

### Do values in `tack.ini` extend or replace flags from built-ins / tackfile?

For a given target, list values in `tack.ini` (e.g. `cflags`, `defines`, `ldflags`, `libs`) **replace** the corresponding extra lists from `tackfile.c` / built-ins.  
These extra lists are then appended to tack’s internal base flags (warnings + profile flags such as `-g` / `-O2`; `-bt20` is added only for tcc/TinyCC debug builds).

### Does the HTML output support templates and CSS?

Yes. By default, tack uses a built-in HTML layout. As of v0.7.1, `tack doc` and `tack bom` may optionally use an external template:

```ini
[doc]
template = templates/tack_template_min.html
css      = templates/tack_doc.css

[bom]
; optional: if not set, BOM falls back to DOC values
template = templates/tack_template_min.html
css      = templates/tack_doc.css
```

A `templates/` folder next to `src/` is recommended (assets, not source code). `css` is copied into the output and referenced via `<link>`; `template` is read only.

Fail-fast: If `template` or `css` is set but the file is missing or not readable, tack exits with **code 2**.

### Which placeholders does the HTML template support?

- `{{TACK_PAGE_TITLE}}` (escaped)
- `{{TACK_PROJECT_TITLE}}` (escaped)
- `{{TACK_HEAD_ASSETS}}`
- `{{TACK_NAV_HTML}}`
- `{{TACK_TOC_HTML}}` (currently empty, but emitted as a marker block)
- `{{TACK_CONTENT_HTML}}` (**required**, otherwise error)
- `{{TACK_FOOTER_HTML}}`

The output always includes stable IDs (`#tack-nav`, `#tack-content`, `#tack-footer`) as a contract for CSS hooks.
Marker comments (`<!-- TACK:BEGIN ... -->`) are provided by the built-in layout or by your template. If you rely on markers with a custom template, wrap the placeholders with the marker comments (see the shipped templates).


### How does tack track header dependencies (rebuilds on `.h` changes)?

Before each compile step, `tack` writes a depfile (`.d`) in the **tack-deps-v1** format and uses it for incremental rebuilds and cache validation. Normal dependencies use `mtime`/size/hash metadata; the depfile itself is intentionally validated via size/hash only so post-`clean` cache restores do not depend on timestamp timing.

- It tracks quoted `#include "..."` **and** angle-bracket `#include <...>` includes **recursively** (including headers that include other headers).
- Resolution follows the usual order: directory of the including file → effective `-I` paths (built-ins + `includes = ...`).
- For `<...>`, tack resolves against the effective include search paths (`-I`); resolved files are written to depfiles/cache metadata.

If you rely on exotic include patterns (macros/generators), use `--rebuild` or `tack clobber` as a safe fallback.

### How do I see why tack rebuilds (“why rebuild”)?

Use `--why` (alias: `--explain`). It is diagnostics-only and does **not** trigger a rebuild by itself.

Examples:
```bat
tack build debug --why -j 8
tack build release --why -j 8
```

Typical reasons include “missing output”, “input newer than output”, “depfile missing/changed”, or “forced (--rebuild)”.

### Why does `tack build --help` (or `-h`) work?

`tack help` prints the general help. `tack build --help` (or `-h`) prints the help **for that sub-command**.

Examples:
```bat
tack build --help
tack run --help
tack test --help
```

---

**Backlinks:** [README](README.md) • [Roadmap](ROADMAP.md) • [Release Notes](RELEASENOTES.md)
