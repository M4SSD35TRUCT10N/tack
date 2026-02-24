# tack ROADMAP (DE/EN) — v0.7.13

Links: **[README](README.md)** · **[FAQ](FAQ.md)** · **[Release Notes](RELEASENOTES.md)**

---

## Deutsch (Roadmap)

### Aktueller Fokus: Real‑World‑Ports & Stabilität
Bevor „große“ Releases und Social‑Media‑Ankündigungen kommen, steht eine echte Praxisprüfung an:

- Port‑Tests mit realen Projekten (z.B. **ft2-clone**, **nuklear**, **imgui** in einer C‑geeigneten Variante)
- Dokumentation per **Projekt‑Port‑Report** (1 Seite pro Projekt)
- Mini‑Matrix: Projekt × OS × Compiler × Status

### Erledigt in v0.7.0
- `tack init` legt optional `.gitignore` sowie `.fossil-settings/ignore-glob` an und **überschreibt nichts** (non-destructive, tack-Block wird nur angehängt, wenn er fehlt).

### Erledigt in v0.7.1
- `tack bom`: erzeugt ein Build‑Manifest (BOM) als `build/bom.md` und `build/bom.html`.
- `tack doc`: erzeugt offline HTML‑Doku in `build/doc/` (Wrapper um Markdown) und verlinkt die BOM.
- Optional: HTML-Templates + CSS für DOC/BOM via `tack.ini` (`[doc]`/`[bom]`: `template`, `css`).
- HTML-Ausgabe: stabile Template-Ankerpunkte (Marker + IDs) für CSS-Hooks und optionales Post-Processing.

### Erledigt in v0.7.2
- Bessere Diagnose-Ausgaben: `--why`/`--explain` erklärt Rebuild-Entscheidungen („why rebuild“).

### Erledigt in v0.7.4
- Optional: schnelleres Incremental-Building via Compile-Cache (`.tack-cache/`).

### Erledigt in v0.7.5
- `tack sbom`: deterministischer SBOM-Export als JSON (tack/CycloneDX/SPDX; formatabhängige Default-Ausgaben), ohne Versions-Ratespiele aus Linker-Flags.

### Erledigt in v0.7.6
- Dokumentations-Sync: README/FAQ/ROADMAP/Release Notes konsistent zur Implementation gehalten.

### Erledigt in v0.7.7
- Fehlerbereinigung nach vorschnellen Pull-Request-Merges

### Erledigt in v0.7.8
- Profil-spezifische Target-Overrides in `tack.ini` dokumentiert (z. B. `[target "app".debug]` / `[target "app".release]`).

### Erledigt in v0.7.9
- Depfile-Scan erfasst jetzt auch `#include <...>` und löst diese gegen die effektiven Include-Pfade auf; Header-Änderungen über `-I` triggern damit korrekt Recompiles.
- Rebuild-Entscheidungen prüfen gespeicherte Abhängigkeits-Metadaten (`mtime`/Größe/FNV-1a) **plus** Depfile-Fingerprint zur Erkennung geänderter Dependency-Graphen (robust gegen Cache-Restores / Write-Order-False-Positives).
- Dokumentation aktualisiert (README/FAQ/ROADMAP/RELEASENOTES).

### Erledigt in v0.7.10
- Feature: `tack fmt` – Orchestrator für externe Formatter (Policy via `[fmt]`/`[fmt "NAME"]`).
  - `tack fmt` formatiert in-place; `tack fmt --check` ändert nichts und liefert Exit-Code 2 bei Abweichungen.
  - Regeln, Globs, Excludes, Reporting; `--rule`/`--target`/`--list`/`--no-defaults`/`--diff` (Diff best effort).

### Erledigt in v0.7.11
- `tack doc`: berücksichtigt optional einen `docs/`-Ordner (alle `docs/**/*.md`) und erzeugt HTML-Seiten unter `build/doc/docs/`.
- `build/doc/index.html`: bei vorhandenen `docs/**/*.md` zusätzliche „Docs“-Sektion (Anchor `#docs`) und Nav-Link.

### Erledigt in v0.7.12
- `tack doc`: rendert alle `*.md` im Projekt-Root dynamisch (keine festen Dateinamen mehr) und erzeugt Nav/Index aus dieser Liste.
- `tack init`: erzeugt zusätzlich (nicht destruktiv) `templates/` inkl. Standard-CSS/Template sowie eine Start-`tack.ini`.
- `tack doc`/`tack bom`: nutzen automatisch `templates/tack_doc.css`, falls keine CSS-Datei konfiguriert ist und die Datei existiert.

### Erledigt in v0.7.13
- `tack clobber`: räumt jetzt immer zusätzlich `.tack-cache/` ab (Big Hammer: Build + Cache weg).
- `tack clean --cache`: löscht zusätzlich `.tack-cache/` (Cache-Reset ohne manuelle Dateisystem-Operationen).
- Sicherheits-Defaults: Löschoperationen folgen keinen Symlinks/Junctions und bleiben durch das bestehende Recursion-Limit begrenzt.

### Nächste sinnvolle Schritte (v0.7.x Idee)
- Mehr Beispiel‑Repos + „Schema‑F“ Walkthroughs
- Optional: bessere Windows Long‑Path Guidance
- Optional: High-Contrast/Forced-Colors CSS-Feinschliff für DOC/BOM-Templates
- Optional: Multiple SBOM-JSONs je Target

### Paketmanagement (Idee / Untersuchungen)
C hat kein Standard‑Paketmanagement wie Rust. Ein tack‑eigenes System wäre ein USP, aber nur mit sehr klarer Scope‑Definition:
- vendoring (Git submodules / subtree / copy)
- lockfile‑ähnliche Reproducibility
- offline‑first / minimal dependencies  
Das bleibt bewusst offen, bis die Build‑Basis in der Praxis sitzt.

---

## English (Roadmap)

### Current focus: real-world ports & stability
Before “big” releases and announcements, tack should be validated against real projects:

- Port tests with real codebases (e.g. **ft2-clone**, **nuklear**, **imgui** in a C-friendly setup)
- Document each attempt with a one-page **Project Port Report**
- Maintain a small matrix: Project × OS × Compiler × Status

### Done in v0.7.0
- `tack init` optionally creates `.gitignore` and `.fossil-settings/ignore-glob` and **does not overwrite anything** (non-destructive, tack block is only appended if it is missing).

### Done in v0.7.1
- `tack bom`: generates a build manifest (BOM) as `build/bom.md` and `build/bom.html`.
- `tack doc`: generates offline HTML documentation in `build/doc/` (wrapper around Markdown) and links to the BOM.
- Optional: HTML templates + CSS for DOC/BOM via `tack.ini` (`[doc]`/`[bom]`: `template`, `css`).
- HTML output: stable template anchor markers (markers + IDs) for CSS hooks and optional post-processing.

### Done in v0.7.2
- Improved diagnostics: `--why`/`--explain` explains rebuild decisions (“why rebuild”).

### Done in v0.7.4
- Optional: faster incremental builds via a compile cache (`.tack-cache/`).

### Done in v0.7.5
- `tack sbom`: deterministic SBOM export as JSON (tack/CycloneDX/SPDX; format-specific defaults), without guessing versions from linker flags.

### Done in v0.7.6
- Documentation sync: kept README/FAQ/ROADMAP/Release Notes consistent with the implementation.

### Done in v0.7.7
- Bugfix release due to badly controlled pull request merges

### Done in v0.7.8
- Profile-specific target overrides documented in `tack.ini` (e.g. `[target "app".debug]` / `[target "app".release]`).

### Done in v0.7.9
- Depfile scanning now also captures `#include <...>` and resolves them against the effective include paths; header changes behind `-I` now correctly trigger recompiles.
- Rebuild decisions now validate stored dependency metadata (`mtime`/size/FNV-1a) **plus** a depfile fingerprint to detect changed dependency graphs (robust against cache restores / write-order false positives).
- Documentation updated (README/FAQ/ROADMAP/RELEASENOTES).

### Done in v0.7.10
- Feature: `tack fmt` – orchestrator for external formatters (policy via `[fmt]`/`[fmt "NAME"]`).
  - `tack fmt` formats in-place; `tack fmt --check` never modifies files and returns exit code 2 on differences.
  - Rules, globs, excludes, reporting; `--rule`/`--target`/`--list`/`--no-defaults`/`--diff` (diff best effort).

### Done in v0.7.11
- `tack doc`: optionally includes a `docs/` folder (all `docs/**/*.md`) and generates HTML pages under `build/doc/docs/`.
- `build/doc/index.html`: adds a “Docs” section when `docs/**/*.md` exists (anchor `#docs`) and provides a matching nav link.

### Done in v0.7.12
- `tack doc`: renders all root-level `*.md` dynamically (no fixed filenames) and builds nav/index from that list.
- `tack init`: additionally creates (non-destructive) `templates/` with default CSS/template and a starter `tack.ini`.
- `tack doc`/`tack bom`: automatically use `templates/tack_doc.css` if no CSS is configured and the file exists.

### Done in v0.7.13
- `tack clobber`: now also removes `.tack-cache/` (big hammer: build + cache reset).
- `tack clean --cache`: additionally removes `.tack-cache/` (cache reset without manual filesystem operations).
- Safety defaults: deletion does not follow symlinks/junctions and stays bounded by the existing recursion depth limit.

### Next sensible steps (v0.7.x ideas)
- More example repos + “Schema‑F” walkthroughs
- Optional: better Windows long-path guidance
- Optional: high-contrast / forced-colors CSS polish for DOC/BOM templates
- Optional: multiple SBOM-JSONs per target

### Package management (idea / research)
C has no standard package manager. A tack-native approach could be a USP, but needs strict scope:
- vendoring (submodules/subtree/copy)
- lockfile-like reproducibility
- offline-first, minimal dependencies  
This stays open until the core build workflow is proven in practice.

---

Back: **[README](README.md)** · **[FAQ](FAQ.md)** · **[Release Notes](RELEASENOTES.md)**
