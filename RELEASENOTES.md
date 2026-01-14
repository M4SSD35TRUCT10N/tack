# tack Release Notes / Changelog (DE/EN)

**Backlinks:** [README](README.md) • [FAQ](FAQ.md) • [Roadmap](ROADMAP.md)

---

## Deutsch (Release Notes)

> Hinweis: Offizielle GitHub-Releases werden erst erstellt, sobald reale Projekte (z.B. ft2-clone, nuklear, imgui‑Variante) sauber mit tack gebaut werden können.
> Diese Datei dokumentiert bis dahin die Versionen/Milestones.

### v0.7.0
- `tack init` erzeugt optional eine `.gitignore` sowie `.fossil-settings/ignore-glob`.
- **Non-destructive:** Existierende Dateien werden **nicht überschrieben**; ein klar markierter tack‑Block wird nur angehängt, wenn er fehlt.
- Zweck: Standardmäßig keine Build‑Artefakte, Cache‑Ordner oder Generator‑Outputs im VCS einchecken.

### v0.6.x (Hardening-Serie, Überblick)
- „Fail-fast“ als Default, mit besseren Fehlermeldungen und robusteren Checks.
- Konsistentere, sicherere String-/Path-Operationen (Bounds-/Truncation‑Checks).
- Konfiguration via `tack.ini` (declarative) plus optionaler Code‑Pfad (`tackfile.c` → generierte INI) mit Verifikations‑Makros.
- Schalter zum Abschalten von Code‑Konfig und Auto‑Tool‑Discovery (Team/CI‑Sicherheit).

---

## English (Release Notes)

> Note: We will only cut official GitHub Releases once real-world projects (e.g., ft2-clone, nuklear, an imgui C variant) build cleanly with tack.
> Until then, this file tracks versions/milestones.

### v0.7.0
- `tack init` can provision a `.gitignore` and `.fossil-settings/ignore-glob`.
- **Non-destructive:** existing files are **not overwritten**; a clearly marked tack block is appended only if missing.
- Rationale: keep build artifacts, caches and generator outputs out of version control by default.

### v0.6.x (Hardening series, overview)
- Fail-fast defaults with improved diagnostics.
- More consistent, safer string/path handling (bounds & truncation checks).
- `tack.ini` for declarative configuration plus an optional code path (`tackfile.c` → generated INI) with verification macros.
- Switches to disable code config and auto tool discovery for stricter team/CI setups.

---

**Backlinks:** [README](README.md) • [FAQ](FAQ.md) • [Roadmap](ROADMAP.md)

