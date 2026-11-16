# tack — agent contract / Agentenvertrag

**Work scope / Arbeitsbereich:** only the explicitly assigned milestone; no unrelated refactoring. Read relevant SPEC/ADR before editing. Preserve the single-file ISO C89 implementation and `tack.ini` as the sole build definition. Do not add Make, CMake, Ninja, Python or Node as project build dependencies. Keep compiler flags in `tack.ini`, not environment variables. `TACK_CC` is an executable selector only.

**Before edits / Vor Änderungen:** review `docs/agents/STATE.md`, `docs/specs/0010-profile-compiler-selection.md`, relevant ADR, tests and active task. Limit broad searching and tool output. Source files only; no testing unless instructed for this milestone.

**C89 / Security:** strict ISO C89, fail-closed parsing, safe paths, bounded operations, no hidden warning suppression. Keep test and coverage ratchets; never lower accepted baselines or invent numbers. Do not fabricate a PASS or Windows check.

**Parallel agents / Agenten:** Codex and Claude Code have identical authority. `CLAUDE.md` only points here; no additional normative instructions. An authorized owner explicitly approves architecture changes, licensing and release status.

**Delivery / Lieferung:** complete source archive, change log, Conventional Commit message, SHA-256 verification, explicit PASS/FAIL/NOT RUN per check. Actual Windows execution is OWNER VERIFIED only after logs are provided. Until then, report NOT RUN.

**State:** tack v0.8.3-dev BA-STD-C89 candidate, no release approval. The v0.8.2 target/profile compiler feature is not independently regression-tested. No accepted initial coverage high-water mark for this repository yet. Do not present the BA-STD migration as completed without a measured baseline and green gates.
