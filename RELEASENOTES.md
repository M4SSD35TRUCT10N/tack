# tack Release Notes / Changelog (DE/EN)

**Backlinks:** [README](README.md) • [FAQ](FAQ.md) • [Roadmap](ROADMAP.md)

---

## Deutsch (Release Notes)

> Hinweis: Offizielle GitHub-Releases werden erst erstellt, sobald reale Projekte (z.B. ft2-clone, nuklear, imgui‑Variante) sauber mit tack gebaut werden können.
> Diese Datei dokumentiert bis dahin die Versionen/Milestones.

### v0.7.3
- Optionaler Compile-Cache (`.tack-cache/`) für schnellere Incremental Builds, abschaltbar via `--no-cache`.

### v0.7.2
- `--why` / `--explain`: kurze Diagnoseausgaben, *warum* tack einen Compile- oder Link‑Schritt ausführt (z.B. "output missing", "source newer", "dependency newer", "forced (--rebuild)").

### v0.7.1
- `tack bom`: erzeugt ein Build‑Manifest (BOM) als `build/bom.md` und `build/bom.html`.
- `tack doc`: erzeugt offline HTML‑Doku in `build/doc/` (Wrapper um Markdown) und verlinkt die BOM.
- Optional: HTML-Templates + CSS für DOC/BOM via `tack.ini` (`[doc]`/`[bom]`: `template`, `css`).
- HTML-Ausgabe: stabile Template-Ankerpunkte (Marker + IDs) für CSS-Hooks und optionales Post-Processing.
- Fixes/Clarifications (v0.7.1 Patch):
  - Fail-fast: fehlende `template`/`css` Dateien oder fehlendes `{{TACK_CONTENT_HTML}}` → Exitcode 2.
  - `--no-code-config` greift jetzt tatsächlich.
  - `doc`/`bom` Profil-Parsing korrigiert (keine falschen Argument-Offsets mehr bei globalen Optionen).
  - INI: Layering über mehrere Quellen funktioniert; `[target ...]` wird korrekt geparst.
  - Template-Rendering: Token-Längen korrigiert (keine stray `}` Artefakte); Marker-Vertrag präzisiert (genau einmal im Output).

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

### v0.7.3
- Optional compile cache (`.tack-cache/`) for faster incremental builds, disable with `--no-cache`.

### v0.7.2
- `--why` / `--explain`: short diagnostics that explain *why* tack runs a compile/link step (e.g. "output missing", "source newer", "dependency newer", "forced (--rebuild)").

### v0.7.1
- `tack bom`: writes a build manifest (BOM) as `build/bom.md` and `build/bom.html`.
- `tack doc`: writes offline HTML docs into `build/doc/` (Markdown wrappers) and links the BOM.
- Optional: HTML templates + CSS for DOC/BOM via `tack.ini` (`[doc]`/`[bom]`: `template`, `css`).
- HTML output: stable template anchor markers (markers + IDs) for CSS hooks and optional post-processing.
- Fixes/Clarifications (v0.7.1 patch):
  - Fail-fast: missing `template`/`css` files or missing `{{TACK_CONTENT_HTML}}` → exit code 2.
  - `--no-code-config` now actually takes effect.
  - `doc`/`bom` profile parsing fixed (no wrong argument offsets with global options).
  - INI: multi-layer loading works; `[target ...]` is parsed correctly.
  - Template rendering: token lengths fixed (no stray `}`); marker contract clarified (exactly once in output).


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
