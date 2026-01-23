# tack ROADMAP (DE/EN) — v0.7.2

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

### Nächste sinnvolle Schritte (v0.7.x Idee)
- Mehr Beispiel‑Repos + „Schema‑F“ Walkthroughs
- Optional: bessere Windows Long‑Path Guidance
- Optional: High-Contrast/Forced-Colors CSS-Feinschliff für DOC/BOM-Templates
- Optional: schnellere Incremental‑Builds (Caching)

### Geplant: SBOM-Export
- CycloneDX JSON Export (Komponenten + Abhängigkeitsgraph), konfigurierbar via `tack.ini`.
- Deterministische Ausgabe (keine Versions-Ratespiele aus Linker-Flags).
- Optional: Datei-Hashes (z.B. SHA-256) und Lizenz-Hinweise, sofern vorhanden.
- SPDX Export ggf. später (höhere Komplexität).

---

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

### Next sensible steps (v0.7.x ideas)
- More example repos + “Schema‑F” walkthroughs
- Optional: better Windows long-path guidance
- Optional: high-contrast / forced-colors CSS polish for DOC/BOM templates
- Optional: faster incremental builds (caching)

### Planned: SBOM export
- CycloneDX JSON export (components + dependency graph), configurable via `tack.ini`.
- Deterministic output (avoid guessing versions from linker flags).
- Optional file hashes (e.g. SHA-256) and license hints when present.
- SPDX export may follow later (higher complexity).

### Package management (idea / research)
C has no standard package manager. A tack-native approach could be a USP, but needs strict scope:
- vendoring (submodules/subtree/copy)
- lockfile-like reproducibility
- offline-first, minimal dependencies  
This stays open until the core build workflow is proven in practice.

---

Back: **[README](README.md)** · **[FAQ](FAQ.md)** · **[Release Notes](RELEASENOTES.md)**
