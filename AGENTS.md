# tack — Agentenvertrag / Agent contract

## Deutsch

**Scope:** Nur der beauftragte Milestone. SPEC, ADR, `docs/agents/STATE.md` und Task vor Änderungen lesen. Einzeldatei-Implementierung in striktem ISO C89 und `tack.ini` als einzige Builddefinition erhalten. Keine neuen Make-/CMake-/Ninja-/Python-/Node-Abhängigkeiten. Flags gehören in die INI; `TACK_CC` wählt nur das Compilerprogramm.

**Sicherheit:** Fail-closed Parsing, begrenzte Pfade und Operationen, keine versteckten Warnungsunterdrückungen. Tests und Coverage-High-Water-Werte nicht abschwächen; keine ungemessenen Baselines erfinden. Compiler- oder Windows-Gates niemals als bestanden ausgeben, wenn sie nicht tatsächlich liefen.

**Agenten:** Codex und Claude Code folgen demselben Vertrag. `CLAUDE.md` verweist ausschließlich hierher. Architektur, Lizenz und Release-Status werden vom Owner freigegeben.

**Lieferung:** Vollständiges Quellen-ZIP, Änderungsprotokoll, Conventional-Commit-Nachricht, SHA-256-Prüfung und für jedes Gate PASS/FAIL/NOT RUN. Windows-Abnahme erfolgt erst durch Owner-Logs. tack v0.8.4 ist zur Veröffentlichung freigegeben. Die Linux-GCC/gcov-Referenz-HWM ist vom Owner angenommen; Windows/GCC Release und TinyCC Debug sind ownergeprüft. Windows-Coverage ist nicht gemessen, und ein GitHub-Push/Tag ist nicht bestätigt.

## English

**Scope:** Only the assigned milestone. Read SPEC, ADR, `docs/agents/STATE.md` and the task before editing. Preserve the single-file strict ISO C89 implementation and `tack.ini` as the sole build definition. No new Make/CMake/Ninja/Python/Node dependencies. Flags belong in the INI; `TACK_CC` selects the compiler executable only.

**Security:** Fail-closed parsing, bounded paths and operations, no hidden warning suppression. Never weaken tests or coverage high-water marks, or invent an unmeasured baseline. Never claim compiler or Windows gates passed unless they actually ran.

**Agents:** Codex and Claude Code have equal authority under this contract. `CLAUDE.md` only points here. Architecture, licensing and release status require owner approval.

**Delivery:** Complete source ZIP, change log, Conventional Commit message, SHA-256 verification and PASS/FAIL/NOT RUN for each gate. Owner-provided logs are required for Windows acceptance. tack v0.8.4 is approved for publication. The Linux GCC/gcov reference HWM is owner-accepted and Windows/GCC release and TinyCC debug test suites are owner-validated. Windows coverage is not measured, and GitHub push/tag has not been confirmed.
