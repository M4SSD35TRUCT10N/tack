# M2 RC0.5 – Targeted P17/P19/P20 test hardening task

**Date:** 2026-11-18
**Version / milestone:** tack v0.8.6 M2 only; one milestone, RC0.5 candidate.
**Start/base:** `feature/v0.8.6-m2-process-execution` @ `ca9090dda0aa302d404784cb78a0716b5289a059`.
**Goal:** Reproduce and verify real active compiler process bounds (P17), output-completion reordering (P19), and a spawn failure while a sibling is demonstrably running (P20). Strict ISO C89.
**Files in scope:** `tests/process_execution_test.c`; M2 SPEC 0014, requirement matrix, M2-VALIDATION, STATE and coverage documentation. No production code changes are initially authorized or needed by present evidence.
**References read:** AGENTS.md, docs/agents/TASK-TEMPLATE.md, docs/specs/0014-m2-process-execution.md, docs/plans/M2-REQUIREMENTS.tsv, docs/plans/M2-VALIDATION.md, src/tack.c `proc_spawn_nowait/proc_wait/compile_sources`, current M2 test. ADR 0003 remains authoritative for Windows process API.
**Non-goals:** No new build tools or dependencies; no Python, CMake, Make, JS runtime, release, merge, M3–M8, workflow/CI changes, unsupported concurrency semantics, shell wrappers, or weakening previous tests.
**Safety and limits:** Bounded max 256 compiler jobs, strictly bounded marker polling, shell-free child launch, fail-fast fixture operations, no success `.meta` on failed spawn, no leaked handles or zombies, preserve exact argv boundaries.
**Test design:** Parent-side OS process-lifecycle probe; real child pairwise start barriers; two fast-before-slow completion-marker test orders; deterministic second-spawn failure injection gated by first child READY and RELEASE. No success statuses before platform execution.
**Existing acceptance:** Owner Windows RC0.4 TinyCC/GCC test suites PASS (27/0/1), 28 test sources owner-approved, Windows HWM `3844/6075` owner-accepted and versioned, Linux HWM `3604/5790` unchanged.
**Gates:** RC0.5 Windows TinyCC debug NOT RUN, GCC release NOT RUN, GCC gcov NOT RUN, 28-test ratchet BLOCKED until Owner approval. Linux GCC/Clang tests NOT RUN and Linux coverage NOT RUN. M2 acceptance BLOCKED.
**Owner actions after retrieval:** inspect test diff, explicitly `ratchet tests --owner-approve`, run Windows debug/release/coverage with `tack --config .\tack.ini`; inspect actual metrics; explicitly accept improved HWM if and only if validated. Repeat Linux reference gates, do not lower baseline.
**Delivery:** GitHub-only signed Conventional Commit on existing M2 feature branch and Draft PR #13. No ZIP as explicitly agreed for this workflow; no merge until separate Owner approval. Commit SHA and diff are the integrity record; no fabricated SHA256 release manifest.
**Sign-off:** `Signed-off-by: Enrico Lefass <enrico.lefass@mailbox.org>`.

## English

A targeted M2 RC0.5 strict-C89 test-only task implementing bounded native process overlap accounting, deterministic output completion-order fixtures, and spawn-failure drain while the first child has verifiably started. No production changes or accepted coverage-baseline changes. All RC0.5 gate claims remain NOT RUN pending Owner actual Windows/Linux rerun and ratchet acceptance.
