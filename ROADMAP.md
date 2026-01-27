# tack ROADMAP (DE/EN) — v0.7.6

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

### Nächste sinnvolle Schritte (v0.7.x Idee)
- Mehr Beispiel‑Repos + „Schema‑F“ Walkthroughs
- Optional: bessere Windows Long‑Path Guidance
- Optional: High-Contrast/Forced-Colors CSS-Feinschliff für DOC/BOM-Templates

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

### Next sensible steps (v0.7.x ideas)
- More example repos + “Schema‑F” walkthroughs
- Optional: better Windows long-path guidance
- Optional: high-contrast / forced-colors CSS polish for DOC/BOM templates

### Package management (idea / research)
C has no standard package manager. A tack-native approach could be a USP, but needs strict scope:
- vendoring (submodules/subtree/copy)
- lockfile-like reproducibility
- offline-first, minimal dependencies  
This stays open until the core build workflow is proven in practice.

---

Back: **[README](README.md)** · **[FAQ](FAQ.md)** · **[Release Notes](RELEASENOTES.md)**
