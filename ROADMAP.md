# tack ROADMAP (DE/EN) — v0.8.0-dev

Links: **[README](README.md)** · **[FAQ](FAQ.md)** · **[Release Notes](RELEASENOTES.md)** · **[Docs](docs/README.md)**

---

## Deutsch (Roadmap)

### 0.8-Serie eröffnet
- `docs/` ist jetzt der feste Ort für Spezifikationen und Architekturentscheidungen.
- Vor größeren 0.8-Codeänderungen sollen erst die Leitplanken in `docs/specs/` bzw. `docs/adr/` dokumentiert werden.
- Die ersten 0.8-Commits haben die Leitplanken dokumentiert; Commit 0003 bringt jetzt den ersten kleinen Produktions-Code-Schritt.
- Dieser Schritt setzt bewusst nur die Grundlage für **Compilerwahl vs. INI-Politik vs. Built-ins** um: `[project] compiler` und `[project] compiler_policy`.
- Die Benutzerführung wird in kleinen Schritten nachgezogen: Hilfeausgabe und `tack doctor` sollen diese Trennung klar und knapp sichtbar machen.
- 0.8 verfolgt bewusst **keine** generische Toolchain-DSL und kein Meta-Buildsystem als Zielbild.

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

### Erledigt in v0.7.14
- `tack new <name>`: erstellt das Projektverzeichnis `<name>/` und führt darin `tack init` aus (One‑Step‑Scaffold).

### Erledigt in v0.7.15
- Compilerbewusste Debug-Profil-Flags: `-g` und `-DDEBUG=1` bleiben generisch, `-bt20` wird nur noch für tcc/TinyCC gesetzt.
- GCC/Clang-Starts über `TACK_CC` scheitern im Debug-Profil nicht mehr an einem tcc-spezifischen Schalter.
- Regressionstest für BOM-/Flag-Verhalten von tcc vs. gcc ergänzt.
- README/FAQ konkretisieren die effektiven eingebauten Profil-Flags.

### Erledigt in v0.7.16
- Cache-Restores nach `tack clean` hängen nicht mehr von der neu geschriebenen Depfile-`mtime` ab.
- Das Depfile selbst wird für die Cache-Validierung gezielt über Größe + FNV-1a geprüft; normale Dependencies bleiben bei `mtime`/Größe/FNV-1a.
- Regressionstest für den `clean`/Restore-Pfad ergänzt.

### Erledigt in v0.7.17
- `tack sbom` wird in der Doku jetzt präziser als **Build-Input-SBOM** positioniert; CycloneDX/SPDX bleiben Exportformate, ohne Resolver/Paketmanager werden Fremdversionsangaben bewusst nicht erfunden.
- Neuer CLI-Batchmodus: `tack sbom --all-targets` schreibt je aktiviertem Target eine eigene JSON-Datei (`build/sbom.<target>.json`, `.cdx.json`, `.spdx.json`).
- `[sbom] output` bleibt bewusst single-target-spezifisch; Einzel-Export und Batch-Export bleiben klar getrennt.
- Regressionstest `tests/sbom_all_targets_test.c` ergänzt.

### Erledigt in v0.7.18
- GCC/Clang-Testhärtung für `-Werror`: eingebettete `#include "../src/tack.c"`-Tests nutzen jetzt einen kleinen Wrapper, der `-Wunused-function` nur im Testkontext unterdrückt.
- `functional_smoke_test` prüft den Cache-Pfad jetzt ohne erzwungenes Rebuild, damit die Erwartung zur tatsächlichen Cache-Semantik passt (`force` umgeht den Compile-Cache absichtlich).
- `path_join_test.c` definiert testlokale Helfer nur dort, wo sie wirklich verwendet werden.
- Produktionscode und Single-File-Ansatz bleiben unverändert; die Anpassung ist bewusst auf den Testkontext begrenzt.

### Erledigt in v0.7.19
- README/FAQ enthalten jetzt eine deutlich konkretere Windows-Long-Path-Guidance.
- Dokumentiert sind flache Workspace-Pfade, die Aktivierung von `LongPathsEnabled` bzw. der Gruppenrichtlinie sowie der Hinweis, dass danach ein Neustart/Reboot nötig sein kann.
- Die Doku stellt jetzt ausdrücklich klar, dass Long-Paths nur helfen, wenn das jeweilige Programm selbst **long-path-aware** ist; externe Tool-Grenzen bleiben relevant.

### Erledigt in v0.7.20
- `templates/tack_doc.css` enthält jetzt Grundunterstützung für `prefers-contrast: more` und `forced-colors: active`, damit DOC/BOM-Ausgaben unter High-Contrast/Forced-Colors systemfreundlicher bleiben.
- Lesbarkeit/Orientierung leicht verbessert: stärkere Rahmen/Unterstreichungen bei mehr Kontrast, sichtbarer aktueller Nav-Eintrag und klarere Fokusmarkierung ohne JavaScript.
- `templates/README.md`, README und FAQ dokumentieren die neue Accessibility-Feinabstimmung ausdrücklich.

### Erledigt in v0.7.21
- Abschlussprüfung der Doku auf Zweisprachigkeit: README, FAQ, ROADMAP, Release Notes und `templates/README.md` enthalten jetzt denselben Sachstand in DE/EN.
- README EN dokumentiert jetzt die bislang nur im deutschen Teil vorhandenen Konfigurationsdetails und die Abgrenzung `clean`/`clobber`.
- FAQ EN wurde an dieselbe Themenreihenfolge wie FAQ DE angepasst; `templates/README.md` wurde ebenfalls inhaltlich angeglichen.

### Erledigt in v0.7.22
- `functional_smoke_test` setzt den Cache-Pfad jetzt robuster auf: Vor der Cache-Prüfung werden Build-Ausgaben gezielt entfernt, damit wirklich ein neuer Compile-/Restore-Pfad statt eines Up-to-date-No-op getestet wird.
- Der Test deckt damit wieder zwei reale Erwartungen ab: Cache-Verzeichnis wird bei einem echten cache-fähigen Build angelegt, und ein zweiter Build kann mit warmem Cache nach entfernten Build-Ausgaben erneut erfolgreich laufen.
- Der `--no-cache`-Pfad wird weiterhin separat geprüft, jetzt mit derselben Ausgangslage wie der Cache-Fall.

### Erledigt in v0.7.23
- Clang/LLVM-Pfad wieder portabel gemacht: tcc-spezifische Warn-Flags (`-Wunsupported` / `-Wno-unsupported`) werden nur noch für tcc/TinyCC gesetzt; GCC/Clang behalten die gemeinsamen Warn-Flags.
- Sichere Target-Pfade als Default: `id` und `bin` werden als einfache, pfadsichere Tokens validiert; `src` bleibt repo-relativ ohne `..`-Traversal. Explizites Opt-in für Alt-/Power-Fälle: `--unsafe-paths` oder `[project] allow_unsafe_paths = yes`.
- SBOM-Metadaten fachlich korrigiert: CycloneDX erhält `serialNumber` + `metadata.timestamp`, SPDX ein echtes UTC-`creationInfo.created` und einen eindeutigeren `documentNamespace`. Für reproduzierbare Pipelines wird `SOURCE_DATE_EPOCH` respektiert.
- Versions-/Scaffold-Hygiene nachgezogen: `TACK_VERSION`, die per `tack init` erzeugte `tack.ini` und die mitgelieferte Beispiel-`tack.ini` sind jetzt wieder auf demselben Stand.
- Regressionstests ergänzt: Compiler-Flag-Gating, Pfadsicherheit, SBOM-Metadaten und Scaffold-Version.

### Erledigt in v0.7.24
- `tack init` erzeugt `src/main.c` jetzt bewusst versionsneutral (`Hello from tack!`) statt mit einer hart verdrahteten Altversionsnummer; damit bleibt das Scaffold über künftige Releases stabiler.
- `TACK_INIT_DEFAULT_TACK_INI` bindet `TACK_VERSION` jetzt direkt ein, damit die generierte Start-`tack.ini` nicht erneut durch eine duplizierte Versionszeichenkette driftet.
- `tests/init_scaffold_version_test.c` deckt jetzt sowohl die generierte `tack.ini` als auch das Standard-`src/main.c` ab, damit Versionsdrift im Scaffold nicht erneut unbemerkt durchrutscht.
- Interner Altkommentar bereinigt: `tackfile.c auto-config (v0.6.0)` wurde versionsneutral formuliert, um Re-Evaluationen kein unnötiges Drift-Rauschen zu liefern.

### Erledigt in v0.7.25
- P0-Fix für die Link-Correctness: Wenn in einem Lauf ein Objekt neu kompiliert oder aus dem Cache restauriert wurde, wird der Link-Schritt jetzt zuverlässig ausgeführt.
- Zusätzliche `.linkmeta`-Metadatei pro Binary eingeführt, damit stale Binaries auch dann erkannt werden, wenn Zeitstempel ungünstig zusammenfallen oder aus früheren Läufen stammen.
- Regressionstest `incremental_link_test.c` ergänzt, der die Header-only-Änderung mit absichtlich gleichgezogenen Output-Timestamps nachstellt.
- Striktes C89/C90 nachgezogen: eingebettete Init-Templates als String-Listen, neuer Regressionstest `strict_c89_compile_test.c` für `-std=c89 -pedantic -Werror`.
- `tack doctor` zeigt jetzt zusätzlich den Compiler-Fundstatus; verbose DOC-Logging wieder mit sauberen Zeilenumbrüchen.

### Nächste sinnvolle Schritte (v0.7.x Idee)
- Mehr Beispiel‑Repos + „Schema‑F“ Walkthroughs

### Paketmanagement (Idee / Untersuchungen)
C hat kein Standard‑Paketmanagement wie Rust. Ein tack‑eigenes System wäre ein USP, aber nur mit sehr klarer Scope‑Definition:
- vendoring (Git submodules / subtree / copy)
- lockfile‑ähnliche Reproducibility
- offline‑first / minimal dependencies  
Das bleibt bewusst offen, bis die Build‑Basis in der Praxis sitzt.

---

## English (Roadmap)

### 0.8 series opened
- `docs/` is now the fixed home for specifications and architecture decisions.
- Before larger 0.8 code changes, the guardrails should first be documented in `docs/specs/` and `docs/adr/`.
- The previous implementation baseline remained **v0.7.25**; commit 0003 now lands the first small real 0.8 code step.
- This first step introduces `[project] compiler` and `[project] compiler_policy` as a small, reviewable foundation without turning tack into a meta-build system.
- User guidance is being tightened in small steps: help output and `tack doctor` should surface that split clearly and concisely.
- 0.8 explicitly does **not** target a generic toolchain DSL or a meta-build system.

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

### Done in v0.7.14
- `tack new <name>`: creates the project directory `<name>/` and runs `tack init` inside it (one-step scaffold).

### Done in v0.7.15
- Compiler-aware debug profile flags: `-g` and `-DDEBUG=1` stay generic, while `-bt20` is now emitted only for tcc/TinyCC.
- GCC/Clang invocations via `TACK_CC` no longer fail in debug builds because of a tcc-only switch.
- Added a regression test for BOM/flag behavior across tcc vs. gcc.
- README/FAQ now state the effective built-in profile flags more explicitly.

### Done in v0.7.16
- Cache restores after `tack clean` no longer depend on the rewritten depfile `mtime`.
- For cache validation, the depfile itself is intentionally checked via size + FNV-1a while normal dependencies still use `mtime`/size/FNV-1a.
- Added a regression test for the `clean`/restore path.

### Done in v0.7.17
- `tack sbom` is now documented more precisely as a **build-input SBOM**. CycloneDX/SPDX remain export formats; without a resolver/package manager tack intentionally does not invent third-party component versions.
- New CLI batch mode: `tack sbom --all-targets` writes one JSON file per enabled target (`build/sbom.<target>.json`, `.cdx.json`, `.spdx.json`).
- `[sbom] output` intentionally remains single-target specific, keeping single-export and batch-export semantics clearly separated.
- Added regression test `tests/sbom_all_targets_test.c`.

### Done in v0.7.18
- GCC/Clang test hardening for `-Werror`: embedded `#include "../src/tack.c"` tests now use a small wrapper that suppresses `-Wunused-function` only in the test context.
- `functional_smoke_test` now checks the cache path without a forced rebuild so the expectation matches actual cache semantics (`force` intentionally bypasses the compile cache).
- `path_join_test.c` now defines test-local helpers only where they are actually used.
- Production code and the single-file approach remain unchanged; the adjustment intentionally stays local to the test context.

### Done in v0.7.19
- README/FAQ now contain much more concrete Windows long-path guidance.
- The docs cover shallow workspace roots, enabling `LongPathsEnabled` / Group Policy, and the note that restarting the shell or even rebooting may be required afterwards.
- The docs now explicitly state that long paths only help when the affected program is itself **long-path aware**; external tool limits still matter.

### Done in v0.7.20
- `templates/tack_doc.css` now includes baseline support for `prefers-contrast: more` and `forced-colors: active` so DOC/BOM output behaves better in system high-contrast / forced-colors modes.
- Small readability polish: stronger borders/underlines at higher contrast, a clearer current-nav state, and more visible keyboard focus without JavaScript.
- `templates/README.md`, README, and FAQ now document this accessibility polish explicitly.

### Done in v0.7.21
- Final bilingual documentation pass: README, FAQ, ROADMAP, Release Notes, and `templates/README.md` now carry the same current-state information in DE/EN.
- README EN now documents the configuration details and the `clean`/`clobber` distinction that had previously only been spelled out in the German section.
- FAQ EN was reordered to the same topic sequence as FAQ DE; `templates/README.md` was aligned as well.

### Done in v0.7.22
- `functional_smoke_test` now sets up the cache path more robustly: before asserting cache behavior it explicitly removes build outputs, so the test exercises a real compile/restore path instead of an up-to-date no-op.
- This restores two concrete expectations: a cache-capable build creates `.tack-cache/`, and a second build can succeed again with a warm cache after build outputs were removed.
- The `--no-cache` path is still covered separately, now from the same starting conditions as the cache case.

### Done in v0.7.23
- Restored Clang/LLVM portability: tcc-specific warning flags (`-Wunsupported` / `-Wno-unsupported`) are now emitted only for tcc/TinyCC; GCC/Clang keep the common warning set.
- Safe target paths by default: `id` and `bin` are validated as simple filesystem-safe tokens; `src` stays repo-relative without `..` traversal. Explicit opt-in for legacy/power-user cases: `--unsafe-paths` or `[project] allow_unsafe_paths = yes`.
- Corrected SBOM metadata semantics: CycloneDX now gets `serialNumber` + `metadata.timestamp`, SPDX a real UTC `creationInfo.created` value and a more unique `documentNamespace`. `SOURCE_DATE_EPOCH` is respected for reproducible pipelines.
- Tightened version/scaffold hygiene: `TACK_VERSION`, the `tack.ini` generated by `tack init`, and the shipped sample `tack.ini` are back on the same version.
- Added regression coverage for compiler-flag gating, path safety, SBOM metadata, and scaffold versioning.

### Done in v0.7.24
- `tack init` now writes a deliberately version-neutral `src/main.c` scaffold (`Hello from tack!`) instead of a hard-coded legacy version string, so the scaffold stays stable across future releases.
- `TACK_INIT_DEFAULT_TACK_INI` now embeds `TACK_VERSION` directly so the generated starter `tack.ini` does not drift again through a duplicated version string.
- `tests/init_scaffold_version_test.c` now covers both the generated `tack.ini` and the default `src/main.c` scaffold so scaffold drift does not slip through again.
- Internal legacy comment cleaned up: `tackfile.c auto-config (v0.6.0)` was rewritten in version-neutral form to avoid unnecessary drift noise in re-evaluations.

### Erledigt in v0.7.25
- P0-Fix für die Link-Correctness: Wenn in einem Lauf ein Objekt neu kompiliert oder aus dem Cache restauriert wurde, wird der Link-Schritt jetzt zuverlässig ausgeführt.
- Zusätzliche `.linkmeta`-Metadatei pro Binary eingeführt, damit stale Binaries auch dann erkannt werden, wenn Zeitstempel ungünstig zusammenfallen oder aus früheren Läufen stammen.
- Regressionstest `incremental_link_test.c` ergänzt, der die Header-only-Änderung mit absichtlich gleichgezogenen Output-Timestamps nachstellt.
- Striktes C89/C90 nachgezogen: eingebettete Init-Templates als String-Listen, neuer Regressionstest `strict_c89_compile_test.c` für `-std=c89 -pedantic -Werror`.
- `tack doctor` zeigt jetzt zusätzlich den Compiler-Fundstatus; verbose DOC-Logging wieder mit sauberen Zeilenumbrüchen.

### Next sensible steps (v0.7.x ideas)
- More example repos + “Schema‑F” walkthroughs

### Package management (idea / research)
C has no standard package manager. A tack-native approach could be a USP, but needs strict scope:
- vendoring (submodules/subtree/copy)
- lockfile-like reproducibility
- offline-first, minimal dependencies  
This stays open until the core build workflow is proven in practice.

---

Back: **[README](README.md)** · **[FAQ](FAQ.md)** · **[Release Notes](RELEASENOTES.md)**
