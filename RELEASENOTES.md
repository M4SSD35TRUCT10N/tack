# tack Release Notes / Changelog (DE/EN)

**Backlinks:** [README](README.md) • [FAQ](FAQ.md) • [Roadmap](ROADMAP.md)

---

## Deutsch (Release Notes)

> Hinweis: Offizielle GitHub-Releases werden erst erstellt, sobald reale Projekte (z.B. ft2-clone, nuklear, imgui‑Variante) sauber mit tack gebaut werden können.
> Diese Datei dokumentiert bis dahin die Versionen/Milestones.

### v0.7.24
- `tack init` erzeugt `src/main.c` jetzt versionsneutral mit `puts("Hello from tack!")` statt mit einer hart verdrahteten Altversion; künftige Releases müssen das Scaffold damit nicht mehr pro Version anfassen.
- `TACK_INIT_DEFAULT_TACK_INI` referenziert `TACK_VERSION` jetzt direkt statt eine Versionsnummer zu duplizieren; die generierte `tack.ini` bleibt dadurch enger an der echten Tool-Version.
- `tests/init_scaffold_version_test.c` prüft jetzt sowohl die per `tack init` generierte `tack.ini` als auch das Standard-`src/main.c` auf Versionsdrift.
- Aufräumarbeit im Quelltext: der interne Altkommentar `tackfile.c auto-config (v0.6.0)` wurde in eine versionsneutrale Form gebracht, um unnötige Drift-Signale in Re-Reviews zu vermeiden.

### v0.7.23
- Clang/LLVM-Buildpfad repariert: tcc-spezifische Warn-Flags (`-Wunsupported` / `-Wno-unsupported`) werden nur noch für tcc/TinyCC gesetzt; `TACK_CC=clang` bricht damit nicht mehr an unbekannten Warn-Optionen unter `-Werror`.
- Sichere Target-Pfade als Default eingeführt: `id` und `bin` werden als einfache, pfadsichere Tokens validiert; `src` bleibt repo-relativ ohne `..`-Traversal. Für kontrollierte Sonderfälle gibt es ein explizites Opt-in via `--unsafe-paths` oder `[project] allow_unsafe_paths = yes`.
- CycloneDX-/SPDX-Metadaten fachlich nachgezogen: CycloneDX schreibt jetzt `serialNumber` und `metadata.timestamp`; SPDX schreibt ein echtes UTC-`creationInfo.created` und einen eindeutigeren `documentNamespace`. `SOURCE_DATE_EPOCH` wird für reproduzierbare Timestamps respektiert.
- Versions-/Scaffold-Hygiene repariert: `TACK_VERSION`, die von `tack init` erzeugte `tack.ini` und die mitgelieferte Beispiel-`tack.ini` zeigen wieder denselben Versionsstand.
- Neue Regressionstests: `path_safety_test.c`, `sbom_metadata_test.c`, `init_scaffold_version_test.c`; `compiler_profile_flags_test.c` prüft jetzt zusätzlich das compilerbewusste Warn-Flag-Gating.

### v0.7.22
- Test-Fix für den bislang übergangenen Nebenbefund `cache dir missing` im `functional_smoke_test`.
- Der Cache-Test entfernt jetzt vor der Prüfung gezielt die target-spezifischen Build-Ausgaben unter `build/<target>/<profile>/{obj,dep,bin}`, damit ein echter Build-/Restore-Pfad entsteht und nicht bloß ein Up-to-date-Short-Circuit.
- Dadurch prüft der Smoke-Test wieder belastbar, dass `.tack-cache/` bei cache-fähigen Builds angelegt wird und der Warm-Cache-Pfad nach entfernten Build-Ausgaben weiter funktioniert.
- Der `--no-cache`-Pfad wird mit derselben Ausgangslage gegengeprüft; Produktionscode bleibt unverändert.

### v0.7.21
- Abschlussprüfung der DE/EN-Doku: README, FAQ, ROADMAP, Release Notes und `templates/README.md` wurden inhaltlich angeglichen statt nur markerbasiert geprüft.
- README EN enthält jetzt die zuvor nur auf Deutsch dokumentierten Konfigurationsdetails (`tack.ini`, `[doc]`/`[bom]`, Flag-Semantik, Beispiel-INI, clean/clobber-Abgrenzung).
- FAQ EN wurde in dieselbe Themenreihenfolge wie FAQ DE gebracht; `templates/README.md` enthält nun auch im englischen Teil dieselben Layout-/Template-Hinweise.

### v0.7.20
- CSS-Feinschliff für die mitgelieferten DOC/BOM-Templates: `templates/tack_doc.css` enthält jetzt Grundunterstützung für `prefers-contrast: more` und `forced-colors: active`.
- Lesbarkeit/Orientierung in kontraststarken Modi verbessert: Links werden unterstrichen, Rahmen etwas deutlicher gezeichnet, der aktuelle Nav-Eintrag fällt klarer auf, und der Tastaturfokus ist sichtbarer.
- Dokumentation in README/FAQ/templates/README ergänzt; in diesem Schritt wurde bewusst kein Produktions-C-Code geändert.

### v0.7.19
- Windows-Long-Path-Guidance in README/FAQ deutlich erweitert: konkrete Praxisempfehlungen für flache Workspace-Pfade, Aktivierung von `LongPathsEnabled` bzw. Gruppenrichtlinie und Hinweis auf erforderlichen Neustart/Reboot.
- Doku stellt jetzt ausdrücklich klar, dass Long-Paths unter Windows nur helfen, wenn das jeweilige Programm selbst **long-path-aware** ist; externe Tool-Limits bleiben also relevant.
- Keine Produktionscode-Änderung in diesem Schritt; es handelt sich bewusst um einen kleinen Doku-/Erfahrungs-Commit.

### v0.7.18
- Test-Hardening für GCC/Clang mit `-Werror`: eingebettete `#include "../src/tack.c"`-Tests nutzen jetzt einen kleinen Wrapper, der `-Wunused-function` bewusst nur im Testkontext unterdrückt.
- `functional_smoke_test` prüft den Cache-Pfad ohne forcierten Rebuild; damit entspricht die Testerwartung wieder der realen Cache-Semantik (`force` umgeht den Compile-Cache absichtlich).
- `path_join_test.c` definiert testlokale Hilfsfunktionen jetzt nur noch dort, wo sie tatsächlich gebraucht werden.
- Produktionscode und Single-File-Ansatz bleiben unverändert; der Fix bleibt absichtlich testlokal.

### v0.7.17
- `tack sbom` fachlich präzisiert: Die Doku beschreibt die Ausgabe jetzt ausdrücklich als **Build-Input-SBOM**. CycloneDX/SPDX bleiben Exportformate, aber ohne Resolver/Package-Manager gibt es bewusst keine erfundenen Fremdkomponenten-Versionen.
- Neuer CLI-Batchmodus: `tack sbom --all-targets` schreibt je aktiviertem Target eine eigene JSON-Datei (`build/sbom.<target>.json`, `build/sbom.<target>.cdx.json`, `build/sbom.<target>.spdx.json`).
- `[sbom] output` bleibt absichtlich Single-Target-spezifisch; so bleiben Einzel-Export und Batch-Export semantisch sauber getrennt.
- Neuer Regressionstest `tests/sbom_all_targets_test.c` prüft den Mehrziel-Export inklusive Dateinamen und Target-Zuordnung.

### v0.7.16
- Cache-Validierung nach `tack clean` deterministischer gemacht: Das Depfile selbst wird für Cache-Restores jetzt über **Dateigröße + Content-Hash** statt über eine harte `mtime`-Gleichheit geprüft.
- Normale Dependencies bleiben weiterhin über `mtime` + Größe + FNV-1a validiert; nur der Depfile-Fingerprint wurde gezielt entschärft.
- Damit verschwinden timing-abhängige Cache-Misses nach `clean`, ohne Änderungen am Dependency-Graphen zu übersehen.
- Neuer Regressionstest `tests/cache_restore_after_clean_test.c` simuliert den `clean`/Restore-Fall und prüft zugleich, dass echte Depfile-Inhaltsänderungen den Cache weiterhin verwerfen.

### v0.7.15
- Compilerbewusste Debug-Profil-Flags: tack setzt `-g` und `-DDEBUG=1` allgemein, `-bt20` aber nur noch für tcc/TinyCC.
- `TACK_CC=gcc` bzw. `TACK_CC=clang` funktionieren damit im Debug-Profil portabler; der im Paper reproduzierte `-bt20`-Fehler wird vermieden.
- Doku präzisiert die effektiven Basis-Profil-Flags in README/FAQ.
- Neuer Regressionstest `tests/compiler_profile_flags_test.c` prüft BOM/Flag-Verhalten für tcc vs. gcc.

### v0.7.14
- `tack new <name>`: erstellt ein Verzeichnis `<name>/` und führt darin automatisch `tack init` aus (Projekt-Scaffold in einem Schritt).

### v0.7.13
- `tack clobber`: räumt jetzt immer zusätzlich `.tack-cache/` ab (Big Hammer: Build + Cache weg).
- `tack clean --cache`: löscht zusätzlich `.tack-cache/` (Cache-Reset ohne manuelle Dateisystem-Operationen).

### v0.7.12
- `tack doc`: keine festen Dateinamen mehr. Stattdessen werden alle `*.md` im Projekt-Root als HTML in `build/doc/` gerendert (z.B. `README.md`, `CHANGELOG.md`, `RELEASENOTES.md`, … – je nachdem, was im Repo existiert).
  - Navigation/Index werden aus dieser dynamischen Root-Liste erzeugt (kein Dead-Link-Nav mehr).
- `tack init`: legt zusätzlich (nicht destruktiv) `templates/` inkl. Standard-CSS/Template sowie eine Start-`tack.ini` an.
- Convenience: Wenn keine CSS-Datei konfiguriert ist, nutzt `tack doc`/`tack bom` automatisch `templates/tack_doc.css`, falls vorhanden.

### v0.7.11
- `tack doc`: berücksichtigt optional einen `docs/`-Ordner (alle `docs/**/*.md`) und erzeugt dafür HTML-Seiten unter `build/doc/docs/`.
- `build/doc/index.html` zeigt bei vorhandenen `docs/**/*.md` zusätzlich eine „Docs“-Sektion (Anchor `#docs`) und die Seiten enthalten einen passenden Nav-Link.

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

### v0.7.24
- `tack init` now writes `src/main.c` in a version-neutral form with `puts("Hello from tack!")` instead of a hard-coded legacy version string, so future releases no longer need to touch the scaffold per version.
- `TACK_INIT_DEFAULT_TACK_INI` now references `TACK_VERSION` directly instead of duplicating a version string, keeping the generated `tack.ini` closer to the real tool version.
- `tests/init_scaffold_version_test.c` now checks both the `tack.ini` generated by `tack init` and the default `src/main.c` scaffold for version drift.
- Source cleanup: the internal legacy comment `tackfile.c auto-config (v0.6.0)` was rewritten in a version-neutral form to avoid noisy drift signals in re-reviews.

### v0.7.23
- Repaired the Clang/LLVM build path: tcc-specific warning flags (`-Wunsupported` / `-Wno-unsupported`) are now emitted only for tcc/TinyCC, so `TACK_CC=clang` no longer fails on unknown warning options under `-Werror`.
- Introduced safe target paths by default: `id` and `bin` are validated as simple filesystem-safe tokens; `src` stays repo-relative without `..` traversal. Controlled special cases now require explicit opt-in via `--unsafe-paths` or `[project] allow_unsafe_paths = yes`.
- Tightened CycloneDX/SPDX metadata semantics: CycloneDX now writes `serialNumber` and `metadata.timestamp`; SPDX writes a real UTC `creationInfo.created` value and a more unique `documentNamespace`. `SOURCE_DATE_EPOCH` is respected for reproducible timestamps.
- Repaired version/scaffold hygiene: `TACK_VERSION`, the `tack.ini` generated by `tack init`, and the shipped sample `tack.ini` now show the same version again.
- New regression tests: `path_safety_test.c`, `sbom_metadata_test.c`, `init_scaffold_version_test.c`; `compiler_profile_flags_test.c` now also checks compiler-aware warning-flag gating.

### v0.7.22
- Test fix for the previously parked `cache dir missing` side finding in `functional_smoke_test`.
- Before asserting cache behavior, the smoke test now removes the target-specific build outputs under `build/<target>/<profile>/{obj,dep,bin}`, so it exercises a real build/restore path instead of an up-to-date short-circuit.
- This makes the smoke test reliably verify again that `.tack-cache/` is created for cache-capable builds and that the warm-cache path still works after build outputs were removed.
- The `--no-cache` path is checked from the same starting conditions; production code is unchanged.

### v0.7.21
- Final DE/EN documentation pass: README, FAQ, ROADMAP, Release Notes, and `templates/README.md` were aligned by content rather than by markers only.
- README EN now includes the configuration details that were previously documented only in German (`tack.ini`, `[doc]`/`[bom]`, flag semantics, example INI, clean/clobber distinction).
- FAQ EN now follows the same topic order as FAQ DE; `templates/README.md` now carries the same layout/template hints in the English section.

### v0.7.20
- Shipped CSS polish for the DOC/BOM templates: `templates/tack_doc.css` now includes baseline support for `prefers-contrast: more` and `forced-colors: active`.
- Improved readability/orientation in higher-contrast modes: links are underlined, borders become stronger, the current nav item is more obvious, and keyboard focus is easier to see.
- Documentation updated in README/FAQ/templates/README; this step intentionally does not change production C code.

### v0.7.19
- Expanded Windows long-path guidance in README/FAQ: concrete advice for shallow workspace roots, enabling `LongPathsEnabled` / Group Policy, and restarting the shell or rebooting afterwards.
- The docs now explicitly state that long paths only help when the affected program is itself **long-path aware**; external tool limits still matter.
- No production-code change in this step; this is intentionally a small documentation/operability commit.

### v0.7.18
- Test hardening for GCC/Clang with `-Werror`: embedded `#include "../src/tack.c"` tests now use a tiny wrapper that suppresses `-Wunused-function` only in the test context.
- `functional_smoke_test` now checks the cache path without a forced rebuild so the test expectation matches actual cache semantics (`force` intentionally bypasses the compile cache).
- `path_join_test.c` now defines test-local helpers only where they are actually used.
- Production code and the single-file approach remain unchanged; this fix intentionally stays local to the tests.

### v0.7.17
- `tack sbom` is now positioned more precisely in the docs as a **build-input SBOM**. CycloneDX/SPDX remain export formats, but without a resolver/package manager tack intentionally does not invent third-party component versions.
- New CLI batch mode: `tack sbom --all-targets` writes one JSON file per enabled target (`build/sbom.<target>.json`, `build/sbom.<target>.cdx.json`, `build/sbom.<target>.spdx.json`).
- `[sbom] output` intentionally remains single-target specific, keeping single-export and batch-export semantics clearly separated.
- New regression test `tests/sbom_all_targets_test.c` covers multi-target export, filenames, and target mapping.

### v0.7.16
- Made cache validation after `tack clean` deterministic: for cache restores, the depfile itself is now validated via **file size + content hash** instead of requiring strict `mtime` equality.
- Normal dependencies still use `mtime` + size + FNV-1a; only the depfile fingerprint rule was relaxed on purpose.
- This removes timing-dependent cache misses after `clean` without masking real dependency-graph changes.
- New regression test `tests/cache_restore_after_clean_test.c` simulates the `clean`/restore path and also verifies that real depfile content changes still invalidate the cache.

### v0.7.15
- Compiler-aware debug profile flags: tack always emits `-g` and `-DDEBUG=1`, but now only emits `-bt20` for tcc/TinyCC.
- This keeps `TACK_CC=gcc` and `TACK_CC=clang` portable in debug builds and avoids the `-bt20` failure reproduced in the paper.
- Documentation now states the effective built-in profile flags more precisely in README/FAQ.
- New regression test `tests/compiler_profile_flags_test.c` checks BOM/flag behavior for tcc vs. gcc.

### v0.7.14
- `tack new <name>`: creates a `<name>/` directory and automatically runs `tack init` inside it (one-step project scaffold).

### v0.7.13
- `tack clobber`: now always deletes `.tack-cache/` as well (big hammer: build + cache reset).
- `tack clean --cache`: also deletes `.tack-cache/` (cache reset without manual filesystem operations).

### v0.7.12
- `tack doc`: no more fixed root filenames. Instead, all `*.md` in the project root are rendered to HTML under `build/doc/` (e.g., `README.md`, `CHANGELOG.md`, `RELEASENOTES.md`, … — whatever exists in the repo).
  - Navigation/index is generated from that dynamic root list (no dead-link nav).
- `tack init`: additionally (non-destructively) provisions `templates/` with default CSS/template plus a starter `tack.ini`.
- Convenience: if no CSS is configured, `tack doc`/`tack bom` automatically uses `templates/tack_doc.css` when present.

### v0.7.11
- `tack doc`: optionally includes a `docs/` folder (all `docs/**/*.md`) and generates HTML pages under `build/doc/docs/`.
- `build/doc/index.html` adds a “Docs” section when `docs/**/*.md` exists (anchor `#docs`), and pages include a matching nav link.

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
