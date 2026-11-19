# tack — Agentenvertrag / Agent contract

## Deutsch

**Scope:** Nur der beauftragte Milestone. SPEC, ADR, `docs/agents/STATE.md` und Task vor Änderungen lesen. Einzeldatei-Implementierung in striktem ISO C89 und `tack.ini` als einzige Builddefinition erhalten. Keine neuen Make-/CMake-/Ninja-/Python-/Node-Abhängigkeiten. Flags gehören in die INI; `TACK_CC` wählt nur das Compilerprogramm.

**Sicherheit:** Fail-closed Parsing, begrenzte Pfade und Operationen, keine versteckten Warnungsunterdrückungen. Tests und Coverage-High-Water-Werte nicht abschwächen; keine ungemessenen Baselines erfinden. Compiler- oder Windows-Gates niemals als bestanden ausgeben, wenn sie nicht tatsächlich liefen.

**Agenten:** Codex und Claude Code folgen demselben Vertrag. `CLAUDE.md` verweist ausschließlich hierher. Architektur, Lizenz und Release-Status werden vom Owner freigegeben.

**Lieferung:** Vollständiges Quellen-ZIP, Änderungsprotokoll, Conventional-Commit-Nachricht, SHA-256-Prüfung und für jedes Gate PASS/FAIL/NOT RUN. Windows-Abnahme nur anhand von Owner-Logs. tack 0.8.5: RC0.7-GCC/TinyCC/Formatter-E2E durch Owner bestanden, Linux-Referenz ebenfalls bestanden. Owner hat die HWM-Anhebungen Windows 3577/5906 und Linux 3505/5710 freigegeben; Linux-CLI-Annahme/Check PASS, Windows-Check mit der neuen HWM noch ausstehend. Lokaler Abschlusskandidat, **nicht veröffentlicht**. Kein Push, Merge oder Tag ohne separate ausdrückliche Owner-Freigabe. Siehe `docs/releases/0.8.5.md`.

## Aktuelle Freigaberegel für tack v0.8.6 (Owner-Vorgabe 19.11.2026)

**Größere fachliche Batches, gleiche Qualität:** Pro Auftrag weiterhin genau ein Milestone, aber mehrere eng verwandte Fehler plus Regressionen in einer überprüfbaren Lieferung bündeln. Gezielt testen, dann auf derselben Quellrevision vollständig abnehmen. Kein historisches PASS für einen geänderten Kandidaten übernehmen.

**Pflichtmatrix vor v0.8.6-Freigabe:** Windows TinyCC Debug, GCC Release, **Clang/MSVC Release**, GCC/gcov; Linux GCC Release, **Clang Release**, GCC/gcov. Jeweils echte Runtime-PASS, ISO C89 und `-Werror`. **>=90 %** ausführbare Produktionszeilen mit unabhängigen Windows-/Linux-HWMs; >=90 % testbare Requirements und 100 % kritische Sicherheits-/Integritätsregeln. Alle Ratchets nur nach neuer echter Messung und ausdrücklicher Owner-Freigabe, niemals absenken.

**M2-Umfang:** `test -j N` muss nach eigener Umsetzung tatsächlich begrenzt parallele Compiler-/Link-Jobs starten; bis dahin ist der Schalter im Test-Runner wirkungslos. Testausführung bleibt bis zur Isolierung gemeinsamer Fixtures sequenziell; gcov nach allen Tests. OS-Dateisystempfade und kanonische Quellidentitäten sind getrennt zu behandeln, `ba_core` nur nach eigenem ADR/Tests als möglicher wiederverwendbarer Baustein, keine neue tack-Abhängigkeit. Host-Artefakte nach `build/host/`, CI-Dateien nach `build/ci/`/`build/artifacts/`, Root-Binaries nur nach Sichtprüfung lokal löschen.

**v0.8.7 getrennt:** Großes Code-/Dokumentationsreview, quellbelegte ADR-/SPEC-Status, Neuordnung README/Guides/Referenz nach Lesefluss. Kein vorzeitiges Umsetzen von v0.8.7 im laufenden M2. Siehe `docs/plans/0.8.6-execution-contract.md` und `docs/plans/0.8.7-code-documentation-review.md`.

## English

**Scope:** Only the assigned milestone. Read SPEC, ADR, `docs/agents/STATE.md` and the task before editing. Preserve the single-file strict ISO C89 implementation and `tack.ini` as the sole build definition. No new Make/CMake/Ninja/Python/Node dependencies. Flags belong in the INI; `TACK_CC` selects the compiler executable only.

**Security:** Fail-closed parsing, bounded paths and operations, no hidden warning suppression. Never weaken tests or coverage high-water marks, or invent an unmeasured baseline. Never claim compiler or Windows gates passed unless they actually ran.

**Agents:** Codex and Claude Code have equal authority under this contract. `CLAUDE.md` only points here. Architecture, licensing and release status require owner approval.

**Delivery:** Complete source ZIP, changelog, Conventional Commit message, SHA-256 verification and actual PASS/FAIL/NOT RUN gate evidence. Windows gates require owner logs. v0.8.5 RC0.7 GCC/TinyCC/real formatter E2E passed on Windows; Linux reference passed. Owner approved increased Windows 3577/5906 and Linux 3505/5710 HWMs; Linux CLI acceptance/check PASS, Windows post-adoption check pending. Final local candidate, **not published**. No push, merge, or tag without separate explicit owner approval. See `docs/releases/0.8.5.md`.

## Current tack v0.8.6 release contract (Owner directive 19 Nov 2026)

Use larger coherent batches **within a single milestone**, with targeted probes before an actual full release matrix on the **same code revision**. Mandatory: Windows TinyCC Debug, GCC Release, **Clang/MSVC Release**, GCC/gcov; Linux GCC Release, **Clang Release**, GCC/gcov. All required lanes use strict ISO C89, `-Werror` and true runtime tests; GCC/gcov retains per-OS >=90% executable production line coverage goal. Also >=90% verified testable requirements and 100% critical security/integrity requirements. Never auto-accept/lower test inventory or HWM ratchets.

In M2, `tack test -j N` must be made into real bounded parallel test compile/link (currently parsed but ignored); keep runtime tests serial until shared fixtures are isolated and collect gcov after all tests. Distinguish native filesystem paths from canonical source IDs. `ba_core` remains a separately approved reusable-code candidate, **not** an automatic new dependency. Bootstrap and CI reports under `build/`, no new root binaries; clean old local ignored files only after inspection. v0.8.7 is a separate post-0.8.6 source-to-doc review and README information architecture overhaul. See execution/review plans under `docs/plans/`.

