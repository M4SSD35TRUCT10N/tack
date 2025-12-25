# tack ROADMAP (DE/EN) — v0.6.6

Links: **[README](README.md)** · **[FAQ](FAQ.md)**

---

## Deutsch (Roadmap)

### Aktueller Fokus: Real‑World‑Ports & Stabilität
Bevor „große“ Releases und Social‑Media‑Ankündigungen kommen, steht eine echte Praxisprüfung an:

- Port‑Tests mit realen Projekten (z.B. **ft2-clone**, **nuklear**, **imgui** in einer C‑geeigneten Variante)
- Dokumentation per **Projekt‑Port‑Report** (1 Seite pro Projekt)
- Mini‑Matrix: Projekt × OS × Compiler × Status

### Nächste sinnvolle Schritte (v0.7.x Idee)
- Mehr Beispiel‑Repos + „Schema‑F“ Walkthroughs
- Optional: bessere Windows Long‑Path Guidance
- Optional: schnellere Incremental‑Builds (Caching / Rebuild-Reasons)
- Optional: bessere Diagnose-Ausgaben (z.B. „why rebuild“)

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

### Next sensible steps (v0.7.x ideas)
- More example repos + “Schema‑F” walkthroughs
- Optional: better Windows long-path guidance
- Optional: faster incremental builds (caching / rebuild reasons)
- Optional: improved diagnostics (“why rebuild”)

### Package management (idea / research)
C has no standard package manager. A tack-native approach could be a USP, but needs strict scope:
- vendoring (submodules/subtree/copy)
- lockfile-like reproducibility
- offline-first, minimal dependencies  
This stays open until the core build workflow is proven in practice.

---

Back: **[README](README.md)** · **[FAQ](FAQ.md)**
