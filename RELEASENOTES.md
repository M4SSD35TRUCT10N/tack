# tack Release Notes / Changelog (DE/EN)

**Backlinks:** [README](README.md) • [FAQ](FAQ.md) • [Roadmap](ROADMAP.md)

---

## Deutsch (Release Notes)

> Hinweis: Offizielle GitHub-Releases werden erst erstellt, sobald reale Projekte (z.B. ft2-clone, nuklear, imgui‑Variante) sauber mit tack gebaut werden können.
> Diese Datei dokumentiert bis dahin die Versionen/Milestones.

### v0.7.10
- Feature: `tack fmt` – Orchestrator für externe Formatter (Policy in `tack.ini` via `[fmt]` / `[fmt "NAME"]`).
  - `tack fmt` formatiert in-place.
  - `tack fmt --check` ändert nichts und liefert Exit-Code 2, wenn Dateien neu formatiert werden müssten.
  - `--rule/--target/--list/--no-defaults/--diff` (Diff best effort via `git diff --no-index` oder `diff -u`).

### v0.7.9
- Bugfix: Rebuild-Entscheidung nutzt nicht mehr die reine Reihenfolge `depfile mtime > object mtime`; stattdessen werden gespeicherte Abhängigkeits-Metadaten (`mtime`/Größe/FNV-1a) geprüft, inklusive Depfile-Fingerprint zur Erkennung geänderter Dependency-Graphen. Dadurch keine False-Positives nach Cache-Restores, bei denen Objekt und Depfile nacheinander geschrieben werden.
- Depfile-Scan erfasst jetzt auch `#include <...>` und löst diese gegen die effektiven Include-Pfade auf; Header-Änderungen über `-I` triggern damit korrekt Recompiles.
- Dokumentation auf neuen Stand gebracht (README/FAQ/ROADMAP/RELEASENOTES) und Version auf `v0.7.9` angehoben.

### v0.7.8
- INI: Profil-spezifische Target-Overrides via `[target "NAME".debug]` / `[target "NAME".release]` für `core`, `includes`, `defines`, `cflags`, `ldflags`, `libs`.

### v0.7.7
- Bugfix-Release: C89-Compile-Fixes nach fehlerhaftem Merge (SBOM-Pfad/Variablen/Braces bereinigt).
- `tack sbom`: CycloneDX/SPDX-Ausgabe vollständig implementiert; `[sbom]` unterstützt `format`, `spec_version`, `output` inkl. formatabhängiger Defaults.

### v0.7.6
- Dokumentations-Sync: README/FAQ/ROADMAP/Release Notes auf den aktuellen Implementationsstand (Features/Optionen) abgeglichen.
- `tack sbom`: Mehrformat-Ausgabe (Default `tack`), Format/Output steuerbar via `[sbom]` in `tack.ini` (CycloneDX/SPDX-Optionen dokumentiert, noch nicht implementiert).

### v0.7.5
- `tack sbom`: deterministischer SBOM-Export als JSON (`build/sbom.json`, Format `tack-sbom-1`).

### v0.7.4
- Bugfix-Release: Compile-Cache validiert Abhängigkeiten jetzt über `mtime` + Dateigröße + Content-Hash (32-bit FNV-1a), um false positives auf Dateisystemen mit grober Timestamp-Auflösung zu vermeiden.
- Cache-Entries werden atomarer geschrieben (Tempfiles + `rename()`), um partielle Einträge zu vermeiden.
- Striktere C89/ANSI-C-Kompatibilität (kein `snprintf`/`vsnprintf`; stabiler 32-bit Cache-Key).

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

### v0.7.10
- Feature: `tack fmt` – orchestrator for external formatters (policy in `tack.ini` via `[fmt]` / `[fmt "NAME"]`).
  - `tack fmt` formats in-place.
  - `tack fmt --check` never modifies files and returns exit code 2 if changes would be required.
  - `--rule/--target/--list/--no-defaults/--diff` (diff best effort via `git diff --no-index` or `diff -u`).

### v0.7.9
- Bugfix: rebuild decisions no longer rely on raw `depfile mtime > object mtime`; they now use recorded dependency metadata (`mtime`/size/FNV-1a), including a depfile fingerprint to detect dependency-graph changes, avoiding false stale detections after cache restore where object and depfile are written sequentially.
- Depfile scanning now also tracks `#include <...>` by resolving them against effective include search paths; header changes through `-I` now trigger recompilation correctly.
- Documentation refreshed (README/FAQ/ROADMAP/RELEASENOTES) and version bumped to `v0.7.9`.

### v0.7.8
- INI: profile-specific target overrides via `[target "NAME".debug]` / `[target "NAME".release]` for `core`, `includes`, `defines`, `cflags`, `ldflags`, `libs`.

### v0.7.7
- Bugfix release: C89 compile fixes after a faulty merge (SBOM path/variables/braces cleaned up).
- `tack sbom`: CycloneDX/SPDX output fully implemented; `[sbom]` supports `format`, `spec_version`, `output` with format-specific defaults.

### v0.7.6
- Documentation sync: aligned README/FAQ/ROADMAP/Release Notes with the current implementation (features/options).
- `tack sbom`: multi-format output (default `tack`), format/output controlled via `[sbom]` in `tack.ini` (CycloneDX/SPDX options documented, not implemented yet).

### v0.7.5
- `tack sbom`: deterministic SBOM export as JSON (`build/sbom.json`, format `tack-sbom-1`).

### v0.7.4
- Bugfix release: compile cache now validates dependencies via `mtime` + file size + content hash (32-bit FNV-1a) to avoid false positives on file systems with coarse timestamp resolution.
- Cache entries are written more atomically (temp files + `rename()`) to reduce partial entries.
- Stricter C89/ANSI-C compatibility (no `snprintf`/`vsnprintf`; stable 32-bit cache key).

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
