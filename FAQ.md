# tack FAQ / FQA (DE/EN) — v0.7.6

**Backlinks:** [README](README.md) • [Roadmap](ROADMAP.md) • [Release Notes](RELEASENOTES.md)

---

## Deutsch (FAQ)

### Nutzt tack einen Compile-Cache?
Ja. `tack` kann Kompilergebnisse in `.tack-cache/` ablegen, um wiederholte Builds zu beschleunigen.

Die Cache-Validierung nutzt:
- `mtime` (Modifikationszeit)
- Dateigröße
- Content-Hash (32-bit FNV-1a; nicht kryptografisch)

### Wie kann ich den Cache deaktivieren?
Nutze:
- `--no-cache`

### Wie kann ich den Cache löschen?
Lösche den Ordner:
- `.tack-cache/`

### Ist tack ein Ersatz für CMake/make?
tack ersetzt für viele Projekte das klassische Build-Skript (Makefile/CMakeLists), indem es eine feste, simple Konvention nutzt und die üblichen Tasks (`build/run/test/clean`) anbietet. Für extrem komplexe Toolchains ist make/cmake weiterhin besser geeignet.

### Erzeugt tack eine `.gitignore` oder Fossil-Ignore-Regeln?
Ja: `tack init` legt, falls nicht vorhanden, eine sinnvolle `.gitignore` sowie `.fossil-settings/ignore-glob` an.
Wenn die Dateien bereits existieren, **überschreibt tack nichts**, sondern hängt nur einen klar markierten `tack`-Block an, sofern er noch fehlt.
Das ist bewusst „non-destructive“, damit bestehende Projektregeln erhalten bleiben.

### Welche Compiler funktionieren?
Standard ist **tcc**. Über `TACK_CC` kannst du z.B. `gcc` oder `clang` nutzen, solange sie „klassische“ C‑Kommandozeilen verstehen.  
**Wichtig:** `TACK_CC` ist der **Compiler**, nicht „Compiler + Flags“. Flags gehören in `tack.ini`.

### Warum lehnt tack `TACK_CC="clang -std=c89"` ab?
Weil tack den Compiler als argv[0] startet. Flags würden als Teil des Programnamens verstanden.  
Lösung: `TACK_CC=clang` und `cflags = -std=c89` in `tack.ini`.

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
tack baut Pfade dynamisch, nutzt aber trotzdem harte Limits (fail-fast). Wenn dein Windows Setup Long-Paths unterstützt, hilft das. Bei extrem langen Repo-Pfaden bekommst du eine klare Fehlermeldung.

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
`tack sbom` erzeugt eine **deterministische JSON-SBOM** (`build/sbom.json`) aus den bekannten Build-Inputs.  
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

### Erzeugt `tack doc` API-Dokumentation wie `cargo doc` / rustdoc?

Nein. `tack doc` erzeugt eine kleine, offline-fähige Projekt-Doku-Site (README/FAQ/ROADMAP/RELEASENOTES usw.).  
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

Cache validation uses:
- `mtime` (modification time)
- file size
- content hash (32-bit FNV-1a; not cryptographic)

### How do I disable the cache?
Use:
- `--no-cache`

### How do I clear the cache?
Delete the folder:
- `.tack-cache/`

### Is tack a replacement for CMake/make?
For many projects, yes: tack replaces custom build scripts by using conventions and providing `build/run/test/clean`. For very complex toolchains, make/cmake may still be the better fit.

### Does tack create a `.gitignore` or Fossil ignore rules?
Yes: `tack init` will provision a sensible `.gitignore` and `.fossil-settings/ignore-glob` if they don't exist.
If the files already exist, tack **won't overwrite** them; it only appends a clearly marked `tack` block if missing.
This is intentionally non-destructive to preserve existing project rules.

### Which compilers work?
Default is **tcc**. You can set `TACK_CC` to `gcc`/`clang` etc. as long as they behave like classic C compilers.  
Important: `TACK_CC` is the compiler program, not “compiler + flags”. Put flags into `tack.ini`.

### Why does tack reject `TACK_CC="clang -std=c89"`?
Because tack starts the compiler as argv[0]. Flags would be part of the program name.  
Fix: `TACK_CC=clang` and `cflags = -std=c89` in `tack.ini`.

### `--no-config` vs `--no-code-config`?
- `--no-config`: ignore **INI + tackfile.c**
- `--no-code-config`: ignore **only tackfile.c**, still load INI

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
`tack sbom` emits a **deterministic JSON SBOM** (`build/sbom.json`) from known build inputs.  
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

### Does `tack doc` generate API docs like `cargo doc` / rustdoc?

No. `tack doc` is a small, offline project documentation site (README/FAQ/ROADMAP/RELEASENOTES, etc.).  
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
These extra lists are then appended to tack’s internal base flags (warnings + profile flags such as `-g` / `-O2`).

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
