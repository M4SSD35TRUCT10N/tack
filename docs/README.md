# Docs (DE/EN)

**Backlinks:** [README](../README.md) • [FAQ](../FAQ.md) • [Roadmap](../ROADMAP.md) • [Release Notes](../RELEASENOTES.md)

---

## Deutsch
**Stand 17.11.2026:** [v0.8.5 Abschlussdokument](releases/0.8.5.md). Windows RC0.7 und Linux-Referenz grün; beide höheren Coverage-HWMs vom Owner freigegeben, Linux-Check nach CLI-Annahme PASS. Windows-Check mit neuer HWM und GitHub-Veröffentlichung stehen noch aus. Die folgenden RC0.7-Angaben dokumentieren den Stand *vor* der HWM-Freigabe.


### 0.8-Status
`v0.8.0-dev` ist als CI-Grundlagenblock abgeschlossen.
`v0.8.4` ist zur Veröffentlichung freigegeben und umfasst die v0.8.2/v0.8.3-Entwicklungsschritte. Linux-GCC/gcov-Referenz-HWM und Windows/GCC-Release- sowie TinyCC-Debug-Tests sind ownerbestätigt. Windows-Coverage wurde vom Owner mit 3192/5827 gemessen, als eigene HWM versioniert und anschließend mit vollständigem Testlauf auf PASS geprüft. Für v0.8.5 liegt RC0.7 mit Versionsanhebung als **nicht freigegebener Kandidat** vor; die Windows-RC0.6-Gates wurden vom Owner bestätigt. Neue Windows-/Linux-HWMs und GitHub-Freigabe stehen noch aus. Die BA-STD-C89-Gruppennorm bleibt Entwurf.

- [v0.8.5 RC0.7: Release-Prüfstand und Windows-CMD-Abnahme](releases/0.8.5-rc0.7-validation.md)

### Architekturentscheidungen
- [ADR 0001 – Eröffnung der 0.8-Serie](adr/0001-open-v0.8-series.md)
- [ADR 0002 – Compiler je Targetprofil](adr/0002-profile-compiler-ownership.md)

### Spezifikationen
- [SPEC 0001 – Bilinguale Dokumentationspolitik](specs/0001-bilingual-documentation-policy.md)
- [SPEC 0002 – Grundlage für eine INI-first-Toolchain-Politik](specs/0002-ini-first-toolchain-policy-foundation.md)
- [SPEC 0003 – Trennung von Compilerwahl und Produktpolitik](specs/0003-compiler-selection-and-policy-split.md)
- [SPEC 0004 – CI-Modus und Summary-Grundlage](specs/0004-ci-mode-and-summary-foundation.md)
- [SPEC 0005 – Events-JSONL-Grundlage](specs/0005-events-jsonl-foundation.md)
- [SPEC 0006 – TAP-Report-Grundlage](specs/0006-tap-report-foundation.md)
- [SPEC 0007 – JUnit-XML-Grundlage](specs/0007-junit-xml-foundation.md)
- [SPEC 0008 – tack-doc-UX-Grundlage](specs/0008-doc-ux-foundation.md)
- [SPEC 0009 – Optionale JS-Such-Grundlage](specs/0009-optional-js-search-foundation.md)
- [SPEC 0010 – Compiler je Targetprofil](specs/0010-profile-compiler-selection.md)
- [SPEC 0011 – Test- und Coverage-Ratchets](specs/0011-ba-std-c89-ratchet-adoption.md)
- [SPEC 0012 – Formatter-INI-Parser: Abschnittstrennung](specs/0012-fmt-ini-parser-separation.md)

### Leitfäden
- [tack.ini: Praxiseinstieg, Compilerprofile, Targets und Coverage](guides/tack-ini.md)
- [Externe Formatter-Ausführung: Integrationsprüfung](guides/formatter-external-validation.md)
- [tack.ini: vollständige Schlüsselreferenz](reference/tack-ini.md)
- [v0.8.6: Coverage-Ziele und Testmatrix (Plan)](plans/0.8.6-coverage.md)
- [v0.8.6: Größere Arbeitspakete, Clang-Freigabe und Testparallelisierung](plans/0.8.6-execution-contract.md)
- [M2 RC0.8: begrenzter Compiler-/Linker-Pool, Coverage-Fix (NICHT GETESTET)](plans/M2-RC08-TASK.md)
- [ADR 0006: Pfadidentitäten, native Pfade und Parallelisierung (Vorschlag)](adr/0006-paths-parallelism-ba-core.md)
- [Host-Artefakte: Build-Binary und CI-Ausgaben unter build/](guides/host-artifacts.md)
- [v0.8.7: Code-/Dokumentationsreview und Lesefluss (DRAFT)](plans/0.8.7-code-documentation-review.md)
- [v0.8.6 M1 RC0.3: Abnahmeakte (Windows/Linux HWMs akzeptiert, S15 offen)](plans/M1-VALIDATION.md)
- [v0.8.6 M1 RC0.3: Lieferung und Integration](plans/M1-DELIVERY.md)
- [v0.8.6 M1: Linux-Coverage-Nachweis ohne Root-Rechte](plans/M1-LINUX-EVIDENCE.txt)
- [v0.8.6 M1: Windows Unsafe-Opt-in-CLI-E2E](plans/M1-WINDOWS-CLI-EVIDENCE.txt)
- [v0.8.6 M1: Kritische Anforderungen S01–S16](plans/M1-REQUIREMENTS.tsv)
- [SPEC 0013: Pfad- und Dateisystemsicherheit](specs/0013-m1-path-filesystem-safety.md)
- [GitHub self-hosted runners – Workflow-Starter](guides/github-self-hosted-runners.md)
- [GitHub self-hosted runner installation unter Windows x64](guides/github-runner-install-windows.md)
- [GitHub self-hosted runner installation unter Linux ARM64](guides/github-runner-install-linux-arm64.md)
- [`tack init`/`tack new` und BA-STD-C89](guides/init-new-and-ba-std-c89.md)
- [Test- und Coverage-Ratchet: Messung und Freigabe](guides/coverage-ratchet.md)
- [GitHub-Veröffentlichung v0.8.4](guides/publish-v0.8.4.md)

### Versionsstände
- [0.8.2-dev: Profilcompiler](releases/0.8.2-dev.md)
- [0.8.3-dev: BA-STD-C89-Kandidat](releases/0.8.3-dev.md)
- [0.8.4: Test- und Coverage-Ratchet](releases/0.8.4.md)
- [Windows-HWM: Abnahmenachweis 0.8.4](releases/0.8.4-windows-hwm.md)
- [0.8.5-dev: Dokumentation und Formatter-Validierung](releases/archive/0.8.5/0.8.5-dev.md)

Unterverzeichnisse unterhalb von `docs/` werden von `tack doc` als eigene Dokumentgruppen in Navigation und Index behandelt.

---

## English
**M1 RC0.3 status (2026-11-18):** Owner-accepted HWMs Windows **3708/5996** and Linux **3604/5790**, both reference coverage checks PASS; Windows unsafe-paths CLI E2E 2/0/0/2 PASS. **Critical requirement S15 remains partially verified** pending deterministic Win32 enumeration-failure injection. See [M1 acceptance record](plans/M1-VALIDATION.md), [Windows CLI evidence](plans/M1-WINDOWS-CLI-EVIDENCE.txt), and [requirements trace](plans/M1-REQUIREMENTS.tsv). Earlier status statements below are historical snapshots.

**As of 2026-11-17:** [v0.8.5 final record](releases/0.8.5.md). Windows RC0.7 and Linux reference passed; both higher coverage HWMs owner-approved, Linux post-acceptance check PASS. Windows HWM post-adoption check and GitHub publication remain pending. RC0.7 text below records the earlier state.


### 0.8 status
`v0.8.0-dev` is complete as the CI foundation block.
`v0.8.4` is approved for publication, incorporating the v0.8.2/v0.8.3 development steps. The Linux GCC/gcov reference HWM and Windows/GCC release plus TinyCC debug tests have owner approval. Windows coverage has been owner-measured at 3192/5827, versioned as its own HWM and verified by a fresh full PASS run. v0.8.5 RC0.7 is an **unapproved release candidate** with the promoted executable version. The owner confirmed RC0.6 Windows gates; fresh RC0.7 Windows/Linux gates and separate HWM/ GitHub approvals remain pending. The BA-STD-C89 group standard remains a draft.

- [v0.8.5 RC0.7: release gates and Windows CMD acceptance](releases/0.8.5-rc0.7-validation.md)

### Architecture decisions
- [ADR 0001 – Open the 0.8 series](adr/0001-open-v0.8-series.md)
- [ADR 0002 – Compiler per target profile](adr/0002-profile-compiler-ownership.md)

### Specifications
- [SPEC 0001 – Bilingual documentation policy](specs/0001-bilingual-documentation-policy.md)
- [SPEC 0002 – INI-first toolchain policy foundation](specs/0002-ini-first-toolchain-policy-foundation.md)
- [SPEC 0003 – Compiler selection and policy split](specs/0003-compiler-selection-and-policy-split.md)
- [SPEC 0004 – CI mode and summary foundation](specs/0004-ci-mode-and-summary-foundation.md)
- [SPEC 0005 – Events JSONL foundation](specs/0005-events-jsonl-foundation.md)
- [SPEC 0006 – TAP report foundation](specs/0006-tap-report-foundation.md)
- [SPEC 0007 – JUnit XML foundation](specs/0007-junit-xml-foundation.md)
- [SPEC 0008 – tack doc UX foundation](specs/0008-doc-ux-foundation.md)
- [SPEC 0009 – Optional JS search foundation](specs/0009-optional-js-search-foundation.md)
- [SPEC 0010 – Compiler per target profile](specs/0010-profile-compiler-selection.md)
- [SPEC 0011 – Test and coverage ratchets](specs/0011-ba-std-c89-ratchet-adoption.md)
- [SPEC 0012 – Formatter INI parser section isolation](specs/0012-fmt-ini-parser-separation.md)

### Guides
- [tack.ini: guided configuration walkthrough](guides/tack-ini.md)
- [Formatter: external integration check](guides/formatter-external-validation.md)
- [tack.ini: complete key reference](reference/tack-ini.md)
- [v0.8.6: coverage targets and traceability plan](plans/0.8.6-coverage.md)
- [v0.8.6: larger batches, Clang release gates and test parallelism](plans/0.8.6-execution-contract.md)
- [M2 RC0.8: bounded parallel compiler/link pool and coverage identity (NOT RUN)](plans/M2-RC08-TASK.md)
- [ADR 0006: native paths, source identities and parallel jobs (proposal)](adr/0006-paths-parallelism-ba-core.md)
- [Bootstrap and CI output under build/](guides/host-artifacts.md)
- [v0.8.7: code/documentation consistency and reading flow (DRAFT)](plans/0.8.7-code-documentation-review.md)
- [GitHub self-hosted runners – workflow starter](guides/github-self-hosted-runners.md)
- [GitHub self-hosted runner installation on Windows x64](guides/github-runner-install-windows.md)
- [GitHub self-hosted runner installation on Linux ARM64](guides/github-runner-install-linux-arm64.md)
- [`tack init`/`tack new` and BA-STD-C89](guides/init-new-and-ba-std-c89.md)
- [Test and coverage ratchets: measurement and acceptance](guides/coverage-ratchet.md)
- [Publish v0.8.4 on GitHub](guides/publish-v0.8.4.md)

### Release records
- [0.8.2-dev: profile compiler](releases/0.8.2-dev.md)
- [0.8.3-dev: BA-STD-C89 candidate](releases/0.8.3-dev.md)
- [0.8.4: test and coverage ratchets](releases/0.8.4.md)
- [Windows HWM: 0.8.4 acceptance evidence](releases/0.8.4-windows-hwm.md)
- [0.8.5-dev: documentation and formatter verification](releases/archive/0.8.5/0.8.5-dev.md)

Subdirectories below `docs/` are treated by `tack doc` as separate document groups in navigation and index.
