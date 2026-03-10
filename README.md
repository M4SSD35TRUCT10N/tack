# tack — Tiny ANSI-C Kit (v0.7.17)

---

> **DE/EN README**
> 
> DE: Ein schlanker **Build‑Driver** für C (C89/ANSI‑C), inspiriert von Cargo‑Workflows: `new`, `init`, `list`, `build`, `run`, `test` — ohne Make/CMake/Ninja‑Stack.  
> EN: A tiny **build driver** for C (C89/ANSI‑C), inspired by Cargo workflows: `new`, `init`, `list`, `build`, `run`, `test` — without a Make/CMake/Ninja stack.

**Backlinks:** [FAQ](FAQ.md) • [Roadmap](ROADMAP.md) • [Release Notes](RELEASENOTES.md)

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

## Features (v0.7.17)

- Single-File Build-Driver (C89/ANSI‑C)
- Kein Make/CMake/Ninja
- Rekursives Scanning: `src/**/*.c`, `tools/<name>/**/*.c`, `tests/**/*_test.c`
- Target-Discovery: `app` + `tool:<name>` (aus `tools/`, per Config abschaltbar)
- Deklarative Targets: hinzufügen/ändern/deaktivieren/entfernen (via `tack.ini` und/oder `tackfile.c`)
- `tack list` zeigt Targets (Name + id + src + core + enabled)
- Robuste Prozessausführung (kein `system()` für Builds)
- Paralleles Kompilieren: `-j N`
- Depfiles (`.d`, `tack-deps-v1`): tack scannt `#include "..."` **und** `#include <...>` rekursiv über Include-Pfade für Incremental Builds (Header-Änderungen triggern Rebuilds)
- Optionaler Compile-Cache in `.tack-cache/` für schnellere Incremental Builds (abschaltbar mit `--no-cache`)
- Diagnose: `--why`/`--explain` erklärt Rebuild-Entscheidungen („why rebuild“)
- Windows: robustere Depfile-/Pfadbehandlung (custom Depfiles, normalisierte Pfade) → weniger unnötige Rebuilds
- Help-Passthrough: `tack build --help` / `-h` zeigt die Kommando-Hilfe
- Strict Mode: `--strict` aktiviert zusätzlich `-Wunsupported`
- Echte Target-Konfiguration: Includes/Defines/CFLAGS/LDFLAGS/LIBS pro Target
- Profil-spezifische Target-Overrides in `tack.ini` (`[target "...".debug]` / `[target "...".release]`)
- Shared Core Code: `src/core/` wird 1× pro Profil gebaut und optional gelinkt
- `tack bom`: erzeugt ein Build-Manifest (BOM) als `build/bom.md` und `build/bom.html`.
- `tack sbom`: erzeugt eine **Build-Input-SBOM** in mehreren Formaten (Default: tack‑JSON unter `build/sbom.json`; CycloneDX/SPDX mit formatabhängigen Defaults; `--all-targets` schreibt je Target eine eigene JSON-Datei).
- `tack doc`: erzeugt offline HTML-Doku in `build/doc/` (Wrapper um Markdown) für **alle Root-Markdowns** (`*.md` im Projekt-Root) sowie optional alle `docs/**/*.md` (wenn vorhanden) und verlinkt die BOM.
- `tack init`: legt bei Bedarf (nicht destruktiv) `templates/` inkl. Standard-CSS/Template sowie eine Start-`tack.ini` an.
- Optional: HTML-Templates + CSS für DOC/BOM via `tack.ini` (`[doc]`/`[bom]`: `template`, `css`).
- HTML-Ausgabe: stabile Template-Ankerpunkte (Marker + IDs) für CSS-Hooks und optionales Post-Processing.
- **Konfiguration / Layering**:
  - `--config <path>` (explizite INI-Datei; **höchste Priorität**)
  - `tack.ini` (runtime, data-only, auto-load)
  - `tackfile.c` (optional, runtime → generiertes INI-Layer; niedrigere Priorität als `tack.ini`)
  - Built-ins in `tack.c` (Fallback)
- **Hardening (v0.6.1–v0.7.0)**: fail-fast Bounds-Checks, sicherere FS-Traversierung (Depth/Symlink/Reparse-Guards), harte Input-Limits, robustere Token-Parsing-Regeln, plus `--no-code-config` für CI/Teams.

## Ökosystem: Software, die gut mit tack zusammenspielt

- **Compiler/Toolchain:** TinyCC (tcc), GCC/MinGW-w64, Clang/LLVM (über `TACK_CC`). Das Debug-Profil nutzt compilerbewusste Basis-Flags: `-g` und `-DDEBUG=1` allgemein, `-bt20` nur für tcc/TinyCC.
- **Versionsverwaltung:** Git, Fossil (tack `init` legt u. a. `.gitignore` + Fossil-Ignore an).
- **CI/CD:** GitHub Actions, GitLab CI, Jenkins, Buildkite, … (einfach `tack build` / `tack test` aufrufen).
- **Editor/IDE:** VS Code (Tasks), Vim/Neovim, Emacs, CLion/IntelliJ (External Tools), Visual Studio (External Tools).
- **Qualität/Checks:** `clang-format`, `clang-tidy`, `cppcheck`, `include-what-you-use`.
- **SBOM/BOM-Weiterverarbeitung:** CycloneDX CLI (Validierung/Konvertierung), SPDX Online Tools / `spdx-tools` (Validierung), GitHub `actions/attest-sbom` (SBOM-Attestations), OWASP Dependency-Track (CycloneDX-Ingestion/Analyse).

## Software, die tack bereits nutzt

- **Tablinum** — strikt C89, Single-Binary „Document Hub“ (paperless-style); Build/Tests werden per tack gefahren.  
  <https://github.com/M4SSD35TRUCT10N/tablinum>
- **Dein Projekt hier** — schick eine PR (oder Issue), wenn du gelistet werden möchtest.

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

## Compilerbewusste Profil-Flags (ab v0.7.15)

- **Debug**: tack setzt immer `-g` und `-DDEBUG=1`.
- **TinyCC/tcc**: zusätzlich wird `-bt20` gesetzt, weil dieser Schalter tcc-spezifisch ist.
- **GCC/Clang**: erhalten **kein** `-bt20`; damit bleibt `TACK_CC=gcc` bzw. `TACK_CC=clang` im Debug-Profil portabel.
- Zusätzliche `cflags` aus `tack.ini` oder `tackfile.c` bleiben davon getrennt; tack liefert weiterhin seine eingebauten Basis-Profil-Flags.

## BOM, SBOM und DOC – was ist was?

tack kann drei Arten von „Dokumentation“ ausgeben, die oft verwechselt werden:

- **DOC**: eine kleine, offline-fähige Projekt-Doku-Site aus allen Root-Markdowns (`*.md` im Projekt-Root) sowie optional `docs/**/*.md` (wenn vorhanden).  
  Das ist **keine** automatisch generierte API-Dokumentation wie bei `cargo doc`/`rustdoc`.
- **BOM**: ein **Build-Manifest** für das konkrete Build (Targets, Inputs, Flags, Toolchain/OS, Output-Pfade).  
  Zweck: Debugging, Nachvollziehbarkeit, Reproduzierbarkeit.
- **SBOM**: eine **Software Bill Of Materials** im Supply-Chain-Sinne (Komponenten + Abhängigkeiten, z.B. CycloneDX/SPDX).  
  `tack sbom` erzeugt daraus bewusst eine **deterministische Build-Input-SBOM**: erfasst werden bekannte Quellpfade, Include-Pfade, Defines, Compiler-/Linker-Flags und weitere direkt beobachtbare Build-Eingaben.  
  Standardmäßig ist das Format **tack‑spezifisch** (`format: "tack-sbom-1"`) und landet als JSON unter `build/sbom.json`.  
  CycloneDX (`specVersion` 1.4) und SPDX (`SPDX-2.3`) werden unterstützt, aber **ohne Resolver/Package-Manager keine Versionsauflösung fremder Komponenten**. Die Ausgabe ist also bewusst eher „Input-SBOM“ als vollständige Supply-Chain-SBOM mit Komponenten-Versionen und Beziehungen.  
  Über `[sbom]` in `tack.ini` kannst du Format, Spec-Version und den **single-target**-Ausgabepfad `output` steuern (siehe Konfiguration). Für mehrere Targets gibt es `tack sbom --all-targets`, das standardmäßig `build/sbom.<target>.json` bzw. `.cdx.json` / `.spdx.json` schreibt.  
  Es werden **keine** Versions‑Ratespiele aus Linker‑Flags betrieben (z.B. kein OpenSSL‑Guess aus `-lssl`).

### Suche / „cargo-like UI“
Die erzeugten HTML-Seiten sind bewusst **CSS-first** und funktionieren offline.  
Eine echte Volltextsuche ist ohne JavaScript nur sehr eingeschränkt möglich; ohne JS gilt: Index + Browser-Suche (Strg+F).  
Eine optionale, kleine JS-Suche (Progressive Enhancement) ist denkbar, bleibt aber optional.

## Quickstart

### Option A: direkt aus `tack.c` laufen lassen (tcc)

Windows:
```bat
tcc -run src/tack.c init
tcc -run src/tack.c list
tcc -run src/tack.c build debug -v -j 8
tcc -run src/tack.c build debug --why -j 8
tcc -run src/tack.c run debug -- --hello Berlin
```

Linux/BSD:
```sh
tcc -run src/tack.c init
tcc -run src/tack.c list
tcc -run src/tack.c build debug -v -j 8
tcc -run src/tack.c build debug --why -j 8
tcc -run src/tack.c run debug -- --hello Berlin
```

### Option B: `tack.exe` bauen (CI/Teams)

```bat
tcc src/tack.c -o tack.exe
tack.exe new hello
cd hello

tack.exe build debug -j 8 -v
```

## Globale Optionen (müssen vor dem Kommando stehen)

```bat
tack.exe --config tack.ci.ini build release
tack.exe --no-config build debug
tack.exe --no-code-config build debug
tack.exe --no-auto-tools list
tack.exe --no-cache build debug
```

- `--config <pfad>`: explizites laden der INI-Datei (höchste Priorität)
- `--no-config`: **alle** Konfiguration deaktivieren (`tack.ini` und `tackfile.c`)
- `--no-code-config`: **nur** `tackfile.c` deaktivieren, laden der INI-Datei bleibt aktiv (CI/Team‑Modus)
- `--no-auto-tools`: `tools/<name>` Auto-Discovery deaktivieren (für vollständig deklarative Builds)
- `--no-cache`: Compile-Cache deaktivieren (`.tack-cache/`)

## Compile-Cache (.tack-cache/)

`tack` kann Kompilergebnisse cachen, um wiederholte Builds zu beschleunigen. Der Cache liegt im Projekt-Root:

- `.tack-cache/`

### Was wird gecacht?
Pro Compile-Schritt (pro Ziel/Profil):

- Objektdatei (`.o`)
- Depfile (`.d`)
- Meta-Datei (`.meta`) mit Abhängigkeits-Fingerprints

### Was steht im Depfile?

`tack` schreibt pro Compile-Schritt eine `.d` Datei im Format **tack-deps-v1**:

- erste Zeile: `# tack-deps-v1`
- danach: **ein aufgelöster Pfad pro Zeile**
- enthält die `.c` Datei **und** alle rekursiv gefundenen Header aus `#include "..."` und `#include <...>`

**Hinweise / Grenzen:**
- Quoted (`"..."`) **und** Angle-Bracket Includes (`<...>`) werden über die effektiven Include-Pfade aufgelöst und erfasst.
- Keine Makro-/Conditional-Auswertung: tack sammelt einfache `#include` Zeilen (Whitespace vor `#` ist ok).


### Wie wird die Gültigkeit geprüft?
Damit Cache-Hits **und** normale Incremental-Rebuild-Checks auch auf Dateisystemen mit grober Timestamp-Auflösung zuverlässig sind, validiert `tack` die Abhängigkeiten aus dem Depfile zweistufig:

- **normale Dependencies**: `mtime` (Modifikationszeit) + Dateigröße + Content-Hash (schneller 32-bit FNV-1a; nicht kryptografisch)
- **Depfile selbst** (zur Erkennung geänderter Dependency-Graphen): **Dateigröße + Content-Hash**

Der Unterschied ist bewusst: Nach `tack clean` wird das Depfile vor einem möglichen Cache-Restore neu geschrieben. Würde seine `mtime` zwingend mit dem alten Cache-Eintrag übereinstimmen müssen, entstünden timing-abhängige Cache-Misses trotz identischem Inhalts.

### Cache deaktivieren
- `--no-cache`

### Cache löschen (Reset)
- `tack clean --cache` (löscht zusätzlich `.tack-cache/`)
- `tack clobber` (löscht `build/` **und** `.tack-cache/`)

## Kommandos

- `help`, `version`, `doctor`
- `init` – Grundstruktur & Hello-World erzeugen (legt auch `.gitignore` + Fossil-Ignore an, ohne zu überschreiben)
- `list` – Targets anzeigen
- `fmt [--check] [--diff] [--list] [--rule NAME] [--target NAME] [--no-defaults] [-v] [--strict] [-- PATH...]` – Quellcode formatieren (externe Formatter; Policy via `[fmt]`/`[fmt "NAME"]`)
- `build [debug|release] [--target NAME] [-v] [--why|--explain] [--rebuild] [-j N] [--strict] [--no-core]` – Target bauen
- `run [debug|release] [--target NAME] [-v] [--why|--explain] [--rebuild] [-j N] [--strict] [--no-core] [-- <args...>]` – bauen + ausführen
- `test [debug|release] [--target NAME] [-v] [--why|--explain] [--rebuild] [-j N] [--strict] [--no-core]` – bauen + `_test.c` ausführen
- `clean [--cache]` – Inhalte von `build/` löschen (Ordner bleibt bestehen)
  - `--cache` löscht zusätzlich `.tack-cache/` (Compile-Cache-Reset)
- `clobber` – `build/` komplett löschen (löscht auch `.tack-cache/`)
- `bom [debug|release] [--target NAME] [--outdir DIR] [-v] [--strict] [--no-core]` – Build-Manifest als Markdown/HTML
- `sbom [debug|release] [--target NAME | --all-targets] [--outdir DIR] [-v] [--strict] [--no-core]` – deterministische Build-Input-SBOM (ein Ziel oder Batch-Export je Target)
- `doc [debug|release] [--target NAME] [--outdir DIR] [-v] [--strict] [--no-core]` – Offline-HTML-Doku (Root-`*.md` + optional `docs/**/*.md` + BOM)

Tipp: `tack build --help` (oder `-h`) zeigt die Hilfe **für dieses Sub‑Kommando**.

### Warum “clean” und “clobber” (statt distclean)?
`distclean` stammt aus Make-Welten („putze auch generierte Konfig“).  
Bei tack ist es klar getrennt:
- **clean**: „Baureste weg, Struktur bleibt“
- **clobber**: „alles weg“

### Exit-Codes
- **0** – Erfolg
- **1** – Laufzeit-/Build-Fehler (Compiler/Linker, I/O, Asset-Copy, …)
- **2** – Usage-/Config-Fehler (CLI, ungültiges INI, fehlendes Template/CSS, fehlender Template-Token)

## Konfiguration

### 1) `tack.ini` — Data-only Konfiguration (empfohlen)

Wenn `tack.ini` vorhanden ist (oder per `--config PATH` gesetzt wird), lädt tack sie automatisch – außer du setzt `--no-config`.

**Sektionen**
- `[project]`
- `[target "NAME"]` (oder ohne Quotes: `[target tool:foo]`)
- `[doc]` (optional: HTML-Template/CSS für `tack doc`)
- `[bom]` (optional: HTML-Template/CSS für `tack bom`)
- `[sbom]` (Format/Output für `tack sbom`)

**Schlüssel in `[project]`**
- `default_target = app`
- `disable_auto_tools = yes|no`

**Schlüssel in `[target ...]`**
- `src = <dir>`        (rekursiver `.c`-Scan)
- `bin = <name>`       (Exe-Base-Name)
- `id = <safe_id>`     (optional; Ordnername unter `build/<id>/...`)
- `enabled = yes|no`
- `remove = yes|no`
- `core = yes|no`
- `includes = a;b;c`   (ohne `-I`, tack setzt `-I` selbst)
- `defines  = A=1;B=2` (ohne `-D`, tack setzt `-D` selbst)
- `cflags   = ...`     (Tokens, per `;` getrennt)
- `ldflags  = ...`     (Tokens, per `;` getrennt)
- `libs     = ...`     (Tokens, per `;` getrennt)

**Profil-spezifische Overrides in `tack.ini`**
Du kannst pro Target zusätzliche Overrides für `debug`/`release` definieren:

- `[target "NAME".debug]`
- `[target "NAME".release]`

In diesen Profil-Sektionen gelten nur die Extra-Listen + `core`:

- `core = yes|no`
- `includes = ...`
- `defines  = ...`
- `cflags   = ...`
- `ldflags  = ...`
- `libs     = ...`

**Merge-Regel:** Profilwerte überschreiben die Basis-Listen aus `[target "NAME"]` (oder aus `tackfile.c`/Built-ins) pro Profil. Nicht gesetzte Felder bleiben unverändert.

**Defines vs. CFLAGS (Profil-Overrides)**  
`defines = FOO=1` ist semantisch identisch zu `cflags = -DFOO=1`: tack wandelt `defines` intern in `-D`‑Flags um.  
Für Präprozessor‑Makros ist `defines` die klarere/lesbarere Variante; `cflags` ist für allgemeine Compiler‑Flags gedacht.

Beispiel (äquivalent):
```ini
[target "app".release]
defines = TACK_RELEASE=1
; oder:
; cflags = -DTACK_RELEASE=1
```

**Schlüssel in `[sbom]`**
- `format = tack|tack-sbom-1|cyclonedx|cyclonedx-1.4|spdx|spdx-2.3` (Default: `tack-sbom-1`)
- `spec_version = ...` (optional; z. B. `1.4` für CycloneDX oder `2.3` für SPDX; für tack wird daraus `tack-sbom-<spec>`)
- `output = <pfad>` (optional; **nur für Single-Target-Export**; Default: `build/sbom.json`, `build/sbom.cdx.json`, `build/sbom.spdx.json` je nach Format)
- Mehrziel-Export: `tack sbom --all-targets [--outdir DIR]` schreibt standardmäßig `build/sbom.<target>.json` bzw. `.cdx.json` / `.spdx.json`.

**Flag-Semantik (CFLAGS/DFLAGS/LFLAGS)**  
- `includes`, `defines`, `cflags`, `ldflags`, `libs` sind **Extra-Listen**. tack ergänzt sie zu seinen internen Basis-Flags (Warnungen + Profil-Flags wie `-g`/`-O2`).  
- Pro Target gilt: Werte aus `tack.ini` **ersetzen** die entsprechenden Extra-Listen aus `tackfile.c`/Built-ins (es wird nicht „zusammenaddiert“).  
- Built-ins bleiben als Fallback aktiv, solange in `tack.ini` kein Target-Override für den betreffenden Schlüssel gesetzt ist.
- Profil-Sektionen überschreiben die Basis-Listen nur für das jeweilige Profil (`debug`/`release`).

**Listen-Format:** Primär Semikolon-getrennt (`;`). Ab **v0.6.5** werden zusätzlich **Whitespace** als Trenner sowie **quotierte Tokens** unterstützt (z. B. Pfade mit Leerzeichen). Empfehlung: `;` nutzen, weil es am klarsten ist.  
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

[target "tool:gen".debug]
cflags = -DTACK_DEBUG_TOOLS=1

[target "tool:old"]
enabled = no

[target "tool:tmp"]
remove = yes
```

### 2) `tackfile.c` — Code-Konfiguration (optional, runtime, fail-fast)

Wenn `tackfile.c` im Projekt-Root existiert, dann:

> **CI/Team-Hinweis:** Nutze `--no-code-config`, wenn `tackfile.c` im Repo liegen darf, aber in CI/Team-Umgebungen **nicht** ausgeführt werden soll.

1. tack kompiliert automatisch einen kleinen Generator unter `build/_tackfile/`
2. der Generator erzeugt `build/_tackfile/tackfile.generated.ini`
3. tack lädt diese generierte INI als **Low-Priority-Layer** (unterhalb von `tack.ini`)

Wenn `tackfile.c` **nicht** kompiliert oder ausgeführt werden kann, bricht tack mit Fehler ab (fail-fast).

#### `[doc]` / `[bom]` — HTML-Template und CSS (optional)

Diese Sektionen sind optional. Ohne Eintrag nutzt tack das eingebaute HTML-Layout. Ein Template wird nur verwendet, wenn `template = ...` gesetzt ist.

**Fail-Fast:** Wenn `template` oder `css` gesetzt ist, die Datei aber fehlt oder nicht gelesen werden kann, bricht tack mit **Exitcode 2** ab.

**Empfehlung:** Templates als `templates/` neben `src/` im Repo ablegen (Assets, kein Quellcode).

**Schlüssel**
- `template = PATH` (HTML-Datei; wird gelesen, Standardlimit max. 1 MiB; nicht in den Output kopiert)
- `css = PATH` (CSS-Datei; wird in den Output kopiert und per `<link>` eingebunden)

**Fallback-Regel für BOM:** Wenn `[bom]` nicht gesetzt ist, nutzt BOM die Werte aus `[doc]`.

Beispiel:
```ini
[doc]
template = templates/tack_template_min.html
css      = templates/tack_doc.css

[bom]
; optional: wenn nicht gesetzt, greift der Fallback auf [doc]
template = templates/tack_template_min.html
css      = templates/tack_doc.css
```

**Template-Platzhalter**
- `{{TACK_PAGE_TITLE}}` (escaped)
- `{{TACK_PROJECT_TITLE}}` (escaped)
- `{{TACK_HEAD_ASSETS}}`
- `{{TACK_NAV_HTML}}`
- `{{TACK_TOC_HTML}}`
- `{{TACK_CONTENT_HTML}}` (**Pflicht**, sonst Fehler)
- `{{TACK_FOOTER_HTML}}`

Der Output enthält stabile Marker (`<!-- TACK:BEGIN ... -->`) und IDs (`#tack-nav`, `#tack-content`, `#tack-footer`) als Vertrag für CSS-Hooks und optionales Post-Processing.

- **IDs** werden immer von tack erzeugt (NAV/CONTENT/FOOTER).
- **Marker** kommen entweder aus dem eingebauten Layout **oder** aus dem Template. Wenn ein eigenes Template verwendet wird und Marker benötigt werden, müssen sie im Template um die Platzhalter liegen (die shipped Templates machen das so).

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

## Sicherheitslage (Security posture)

`tack` ist ein Entwickler‑Tool: es scannt dein Repo und startet Compiler/Tools. Es ist **kein** Security‑Produkt, aber ab v0.6.1+ deutlich robuster (fail‑fast).

Praktische Regeln:

- **Untrusted Repos:** `tack` nicht als Admin/root auf fremden Repositories ausführen.
- **Code‑Konfiguration:** `tackfile.c` ist *bewusst* ausführbarer Code (Generator → `tackfile.generated.ini`).  
  Für CI/Teams: `--no-code-config` (nur INI), oder `--no-config` (alles aus).
- **Eingaben:** `tack.ini`, ENV (`TACK_CC`) und CLI werden mit harten Limits geprüft; tack bricht bei Überschreitung ab.
- **Filesystem:** Traversierung ist depth‑limitiert; Symlinks/Junctions/Reparse Points werden nicht verfolgt (Loop‑Schutz).
- **Supply‑Chain:** externe Libs versionieren/pinnen und Herkunft/Lizenzen dokumentieren.

Empfehlung: Für Teams ist `tack.ini` der Default. `tackfile.c` nur bewusst nutzen.

## Fehlersuche

- Warnings aus `stdio.h` und fehlende `.exe`: `--strict` nur aktivieren, wenn das gewollt ist.
- „… undeclared …“: häufig ist ein Kommentar versehentlich „kaputt“ (vorzeitig beendet).
- Pfade mit Leerzeichen: tack nutzt spawn/exec statt Shell-`system()`; Quoting-Probleme sind dadurch reduziert.

## ROADMAP
Weitere Infos, wie es mit tack weitergehen wird, stehen [hier](ROADMAP.md).

## FAQ
Eine detaillierte FAQ ist [hier](FAQ.md).

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

## Features (v0.7.17)

- single‑file build driver (C89)
- No Make/CMake/Ninja
- Recursive scanning: `src/**/*.c`, `tools/<name>/**/*.c`, `tests/**/*_test.c`
- Target discovery: `app` + `tool:<name>` (from `tools/`, can be disabled)
- Declarative targets: add/modify/disable/remove (via `tack.ini` and/or `tackfile.c`)
- `tack list` prints targets (name + id + src + core + enabled)
- Robust process execution (no `system()` for builds)
- Parallel compile: `-j N`
- Depfiles (`.d`, `tack-deps-v1`): tack scans `#include "..."` **and** `#include <...>` recursively via include paths for incremental builds (header changes trigger rebuilds)
- Optional compile cache in `.tack-cache/` for faster incremental builds (disable with `--no-cache`)
- Diagnostics: `--why`/`--explain` explains rebuild decisions (“why rebuild”)
- Windows: more robust depfile/path handling (custom depfiles, normalized paths) → fewer unnecessary rebuilds
- Help passthrough: `tack build --help` / `-h` shows sub-command help
- strict mode: `--strict` enables `-Wunsupported` (default suppresses it)
- real per‑target config: includes/defines/cflags/ldflags/libs/core
- profile-specific target overrides in `tack.ini` (`[target "...".debug]` / `[target "...".release]`)
- Shared core code: `src/core/` built once per profile, optionally linked
- `tack bom`: generates a build manifest (BOM) as `build/bom.md` and `build/bom.html`.
- `tack sbom`: emits a **build-input SBOM** in multiple formats (default: tack JSON at `build/sbom.json`; CycloneDX/SPDX with format-specific defaults; `--all-targets` writes one JSON file per target).
- `tack doc`: generates offline HTML docs in `build/doc/` (wrapper around Markdown) for all root-level `*.md` files plus optional `docs/**/*.md` (if present), and links the BOM.
- `tack init`: non-destructively creates `templates/` with default CSS/template and a starter `tack.ini` when needed.
- Optional: HTML templates + CSS for DOC/BOM via `tack.ini` (`[doc]`/`[bom]`: `template`, `css`).
- HTML output: stable template anchor markers (markers + IDs) for CSS hooks and optional post-processing.
- **Configuration layering**:
  - `--config <path>` (explicit INI file; **highest priority**)
  - `tack.ini` (runtime, data‑only, auto‑load)
  - `tackfile.c` (optional, runtime → generated INI layer; lower priority than `tack.ini`)
  - built‑ins in `tack.c` (fallback)

## Ecosystem: software that pairs well with tack

- **Compilers/toolchains:** TinyCC (tcc), GCC/MinGW-w64, Clang/LLVM (via `TACK_CC`). The debug profile uses compiler-aware base flags: `-g` and `-DDEBUG=1` generically, `-bt20` only for tcc/TinyCC.
- **Version control:** Git, Fossil (tack `init` provisions `.gitignore` and Fossil ignore settings, among other things).
- **CI/CD:** GitHub Actions, GitLab CI, Jenkins, Buildkite, … (just call `tack build` / `tack test`).
- **Editors/IDEs:** VS Code (Tasks), Vim/Neovim, Emacs, CLion/IntelliJ (External Tools), Visual Studio (External Tools).
- **Quality checks:** `clang-format`, `clang-tidy`, `cppcheck`, `include-what-you-use`.
- **SBOM/BOM post-processing:** CycloneDX CLI (validation/conversion), SPDX Online Tools / `spdx-tools` (validation), GitHub `actions/attest-sbom` (SBOM attestations), OWASP Dependency-Track (CycloneDX ingestion/analysis).

## Software using tack

- **Tablinum** — strict C89 single-binary “document hub” (paperless-style); builds/tests are driven by tack.  
  <https://github.com/M4SSD35TRUCT10N/tablinum>
- **Your project here** — open a PR (or issue) if you want to be listed.

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

## Compiler-aware profile flags (since v0.7.15)

- **Debug**: tack always emits `-g` and `-DDEBUG=1`.
- **TinyCC/tcc**: tack additionally emits `-bt20`, because that switch is tcc-specific.
- **GCC/Clang**: do **not** receive `-bt20`; this keeps `TACK_CC=gcc` and `TACK_CC=clang` portable in debug builds.
- Extra `cflags` from `tack.ini` or `tackfile.c` remain separate from this; tack still provides the built-in base profile flags.

## BOM, SBOM and DOC — what is what?

tack can output three kinds of “documentation” that are often mixed up:

- **DOC**: a small, offline-friendly project documentation site from all root-level `*.md` files plus optional `docs/**/*.md` (if present).  
  This is **not** automatically generated API documentation like `cargo doc`/`rustdoc`.
- **BOM**: a **build manifest** for a specific build (targets, inputs, flags, toolchain/OS, output paths).  
  Purpose: debugging, traceability, reproducibility.
- **SBOM**: a **Software Bill of Materials** in the supply-chain sense (components + dependencies, e.g. CycloneDX/SPDX).  
  `tack sbom` therefore emits a **deterministic build-input SBOM**: known source paths, include paths, defines, compiler/linker flags, and other directly observed build inputs are recorded.  
  By default the format is **tack-specific** (`format: "tack-sbom-1"`) and is written as JSON to `build/sbom.json`.  
  CycloneDX (`specVersion` 1.4) and SPDX (`SPDX-2.3`) are supported, but **without a resolver/package manager tack does not invent third-party component versions**. The result is intentionally closer to an “input SBOM” than a complete supply-chain SBOM with resolved component versions and dependency relationships.  
  Use `[sbom]` in `tack.ini` to control format, spec version, and the **single-target** output path `output` (see configuration). For multiple targets use `tack sbom --all-targets`, which writes `build/sbom.<target>.json` (or `.cdx.json` / `.spdx.json`) by default.  
  It intentionally avoids guesswork (e.g. it does not try to infer exact OpenSSL versions from `-lssl`).

### Search / “cargo-like UI”
The generated HTML is intentionally **CSS-first** and works offline.  
Full-text search without JavaScript is very limited; without JS use the index page + browser search (Ctrl+F).  
An optional minimal JS search (progressive enhancement) is possible, but remains optional.

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
tack.exe new hello
cd hello

tack.exe build debug -j 8 -v
```

## Global options (must come before the command)

```bat
tack.exe --config tack.ci.ini build release
tack.exe --no-config build debug
tack.exe --no-code-config build debug
tack.exe --no-auto-tools list
tack.exe --no-cache build debug
```

- `--config <path>`: load explicit INI (highest priority)
- `--no-config`: disable **all** configuration (`tack.ini` and `tackfile.c`)
- `--no-code-config`: disable **only** `tackfile.c` (INI stays active)
- `--no-auto-tools`: disable `tools/<name>` auto discovery (useful for fully declarative builds)
- `--no-cache`: disable the compile cache (`.tack-cache/`)

## Compile cache (.tack-cache/)

`tack` can cache compile outputs to speed up repeated builds. The cache lives in the project root:

- `.tack-cache/`

### What is cached?
Per compile step (per target/profile):

- object file (`.o`)
- depfile (`.d`)
- meta file (`.meta`) with dependency fingerprints

### What is in the depfile?

Per compile step, `tack` writes a `.d` file in the **tack-deps-v1** format:

- first line: `# tack-deps-v1`
- then: **one resolved path per line**
- includes the `.c` file **and** all recursively discovered headers from `#include "..."` and `#include <...>`

**Notes / limits:**
- Quoted (`"..."`) **and** angle-bracket includes (`<...>`) are resolved and tracked via the effective include search paths.
- No macro/conditional evaluation: tack collects simple `#include` lines (leading whitespace before `#` is fine).


### How is validity checked?
To avoid false cache hits **and** false up-to-date decisions in normal incremental rebuild checks on file systems with coarse timestamp resolution, `tack` validates dependencies listed in the depfile in two layers:

- **normal dependencies**: `mtime` (modification time) + file size + content hash (fast 32-bit FNV-1a; not cryptographic)
- **the depfile itself** (to detect dependency-graph changes): **file size + content hash**

That distinction is intentional: after `tack clean`, the depfile is rewritten before a cache restore can happen. Requiring the depfile `mtime` to match the old cache entry would create timing-dependent cache misses even when the depfile content is unchanged.

### Disable cache
- `--no-cache`

### Reset / clear cache
- `tack clean --cache` (also deletes `.tack-cache/`)
- `tack clobber` (deletes `build/` **and** `.tack-cache/`)

## Commands

- `help`, `version`, `doctor`
- `init` – create a minimal skeleton + hello world (also provisions `.gitignore` + Fossil ignore, non-destructive)
- `list` – show targets
- `fmt [--check] [--diff] [--list] [--rule NAME] [--target NAME] [--no-defaults] [-v] [--strict] [-- PATH...]` – format sources (external formatters; policy via `[fmt]`/`[fmt "NAME"]`)
- `build [debug|release] [--target NAME] [-v] [--why|--explain] [--rebuild] [-j N] [--strict] [--no-core]` – build target
- `run [debug|release] [--target NAME] [-v] [--why|--explain] [--rebuild] [-j N] [--strict] [--no-core] [-- <args...>]` – build + run target
- `test [debug|release] [--target NAME] [-v] [--why|--explain] [--rebuild] [-j N] [--strict] [--no-core]` – build + execute `_test.c`
- `clean [--cache]` – delete contents of `build/` (keep directory)
  - `--cache` also deletes `.tack-cache/` (compile cache reset)
- `clobber` – delete `build/` entirely (also deletes `.tack-cache/`)
- `bom [debug|release] [--target NAME] [--outdir DIR] [-v] [--strict] [--no-core]` – build manifest as Markdown/HTML
- `sbom [debug|release] [--target NAME | --all-targets] [--outdir DIR] [-v] [--strict] [--no-core]` – deterministic build-input SBOM (single target or one JSON per target)
- `doc [debug|release] [--target NAME] [--outdir DIR] [-v] [--strict] [--no-core]` – offline HTML docs (root `*.md` + optional `docs/**/*.md` + BOM)

### Exit codes

- **0** – success
- **1** – runtime failure (e.g., compiler/linker errors, I/O, asset copy failures)
- **2** – usage/config failure (CLI args, invalid INI, missing template/CSS file, missing required template token)

## Configuration

### 1) `tack.ini` — data-only config (recommended)

Auto-loaded if present (or via `--config`), unless `--no-config` is set.

See the German section above for the full key list. The format is the same.

Profile-specific target overrides are supported via sections like:
- `[target "NAME".debug]`
- `[target "NAME".release]`

These sections accept only `core`, `includes`, `defines`, `cflags`, `ldflags`, `libs`. Profile values replace the base lists for that profile when set.

**Defines vs. CFLAGS (profile overrides)**  
`defines = FOO=1` is semantically identical to `cflags = -DFOO=1`: tack turns `defines` into `-D` flags internally.  
Use `defines` for preprocessor macros and `cflags` for general compiler flags.

Example (equivalent):
```ini
[target "app".release]
defines = TACK_RELEASE=1
; or:
; cflags = -DTACK_RELEASE=1
```

**SBOM output (optional)**  
Use `[sbom]` in `tack.ini`:
- `format = tack|tack-sbom-1|cyclonedx|cyclonedx-1.4|spdx|spdx-2.3` (default: `tack-sbom-1`)
- `spec_version = ...` (optional; e.g. `1.4` for CycloneDX or `2.3` for SPDX; for tack it becomes `tack-sbom-<spec>`)
- `output = <path>` (optional; **single-target only**; default: `build/sbom.json`, `build/sbom.cdx.json`, or `build/sbom.spdx.json` by format)
- Multi-target export: `tack sbom --all-targets [--outdir DIR]` writes `build/sbom.<target>.json` (or `.cdx.json` / `.spdx.json`) by default.

**DOC/BOM templates (optional, v0.7.1)**  
`tack.ini` may also contain `[doc]` and `[bom]` sections with `template = PATH` and `css = PATH`.  
A `templates/` folder next to `src/` is recommended. The template must contain `{{TACK_CONTENT_HTML}}` (required).

Fail-fast behavior:
- If `template = PATH` is set and the file is missing or not readable, tack exits with **code 2**.
- If `css = PATH` is set and the file is missing or not readable, tack exits with **code 2**.
- If the template does not contain `{{TACK_CONTENT_HTML}}`, tack exits with **code 2**.

Marker contract:
- tack always emits stable IDs (`#tack-nav`, `#tack-content`, `#tack-footer`).
- Marker comments (`<!-- TACK:BEGIN ... -->`) are provided by the built-in layout or by your template. If you rely on markers with a custom template, wrap the placeholders with the marker comments (see the shipped templates).

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

## Security posture
`tack` is a developer tool: it scans your repo and spawns compilers/tools. It is **not** a hardened security product, but v0.6.1+ is much more robust (fail‑fast).

Practical rules:

- **Untrusted repos:** don’t run tack as Admin/root on untrusted repositories.
- **Code config:** `tackfile.c` is intentionally executable code (generator → `tackfile.generated.ini`).  
  For CI/teams: use `--no-code-config` (INI only), or `--no-config` (everything off).
- **Inputs:** `tack.ini`, env (`TACK_CC`) and CLI args are checked with hard limits; tack aborts on violations.
- **Filesystem:** traversal is depth‑limited; symlinks/junctions/reparse points are not followed (loop protection).
- **Supply chain:** pin external dependencies and document provenance/licenses.

Recommendation: for teams, prefer `tack.ini` by default; use `tackfile.c` only intentionally.

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
