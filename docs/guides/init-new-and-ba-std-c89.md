# `tack init` und `tack new` / Scaffolding and BA-STD-C89

## Deutsch

`tack init` legt fehlende Verzeichnisse, eine minimale `tack.ini`, DOC/BOM-Templates, `src/main.c`, einen Smoke-Test sowie Ignore-Regeln an. Bestehende Projektdateien werden nicht überschrieben; vorhandene Ignore-Dateien erhalten nur den abgegrenzten tack-Block. `tack new NAME` erzeugt ein neues Verzeichnis und ruft darin dieselbe Initialisierung auf.

Seit v0.8.2-dev kann die Compilerwahl pro Target und Profil erfolgen. Die generierte `tack.ini` enthält ab dieser Korrektur einen **auskommentierten** Beispielblock für Debug/TinyCC und Release/GCC. Dieser ist absichtlich nicht automatisch aktiv, weil Projekte ihre verfügbare Toolchain selbst festlegen. Bei Compilerwechsel innerhalb eines Profils `--rebuild` verwenden.

BA-STD-C89-spezifische Dateien wie `AGENTS.md`, `CLAUDE.md`, Coverage-High-Water-Baseline, ADRs und Security-Vertrag werden **nicht** generisch erzeugt: Normative Projektregeln benötigen Owner-Entscheidung und eigene Spezifikationen. Eine spätere explizite BA-STD-Scaffold-Option würde einen eigenen ADR, Tests und eine bewusste CLI-Freigabe benötigen. Kein automatisches Kopieren fremder Policy und keine stille Anhebung des Compliance-Status.

## English

`tack init` creates missing directories, a minimal `tack.ini`, DOC/BOM templates, `src/main.c`, a smoke test and ignore rules. It does not overwrite existing project files; existing ignore files only receive the bounded tack block. `tack new NAME` creates a new directory and invokes the same initialization inside it.

Since v0.8.2-dev, compiler choice can be made per target and profile. The generated `tack.ini` now includes a **commented-out** example for debug/TinyCC and release/GCC. It is intentionally not active by default, because each project must choose an available toolchain. Use `--rebuild` when switching compilers within a profile.

BA-STD-C89-specific files such as `AGENTS.md`, `CLAUDE.md`, a coverage high-water baseline, ADRs and a security contract are **not** generated generically: normative project rules require owner decisions and dedicated specifications. A future explicit BA-STD scaffold option would need its own ADR, tests and deliberate CLI approval. No copying another project's policy or silently claiming compliance.
