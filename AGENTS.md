# tack — Agentenvertrag / Agent contract

## Deutsch

**Scope:** Nur der beauftragte Milestone. SPEC, ADR, `docs/agents/STATE.md` und Task vor Änderungen lesen. Einzeldatei-Implementierung in striktem ISO C89 und `tack.ini` als einzige Builddefinition erhalten. Keine neuen Make-/CMake-/Ninja-/Python-/Node-Abhängigkeiten. Flags gehören in die INI; `TACK_CC` wählt nur das Compilerprogramm.

**Sicherheit:** Fail-closed Parsing, begrenzte Pfade und Operationen, keine versteckten Warnungsunterdrückungen. Tests und Coverage-High-Water-Werte nicht abschwächen; keine ungemessenen Baselines erfinden. Compiler- oder Windows-Gates niemals als bestanden ausgeben, wenn sie nicht tatsächlich liefen.

**Agenten:** Codex und Claude Code folgen demselben Vertrag. `CLAUDE.md` verweist ausschließlich hierher. Architektur, Lizenz und Release-Status werden vom Owner freigegeben.

**Lieferung:** Vollständiges Quellen-ZIP, Änderungsprotokoll, Conventional-Commit-Nachricht, SHA-256-Prüfung und für jedes Gate PASS/FAIL/NOT RUN. Windows-Abnahme nur anhand von Owner-Logs. tack 0.8.5: RC0.7-GCC/TinyCC/Formatter-E2E durch Owner bestanden, Linux-Referenz ebenfalls bestanden. Owner hat die HWM-Anhebungen Windows 3577/5906 und Linux 3505/5710 freigegeben; Linux-CLI-Annahme/Check PASS, Windows-Check mit der neuen HWM noch ausstehend. Lokaler Abschlusskandidat, **nicht veröffentlicht**. Kein Push, Merge oder Tag ohne separate ausdrückliche Owner-Freigabe. Siehe `docs/releases/0.8.5.md`.

## English

**Scope:** Only the assigned milestone. Read SPEC, ADR, `docs/agents/STATE.md` and the task before editing. Preserve the single-file strict ISO C89 implementation and `tack.ini` as the sole build definition. No new Make/CMake/Ninja/Python/Node dependencies. Flags belong in the INI; `TACK_CC` selects the compiler executable only.

**Security:** Fail-closed parsing, bounded paths and operations, no hidden warning suppression. Never weaken tests or coverage high-water marks, or invent an unmeasured baseline. Never claim compiler or Windows gates passed unless they actually ran.

**Agents:** Codex and Claude Code have equal authority under this contract. `CLAUDE.md` only points here. Architecture, licensing and release status require owner approval.

**Delivery:** Complete source ZIP, changelog, Conventional Commit message, SHA-256 verification and actual PASS/FAIL/NOT RUN gate evidence. Windows gates require owner logs. v0.8.5 RC0.7 GCC/TinyCC/real formatter E2E passed on Windows; Linux reference passed. Owner approved increased Windows 3577/5906 and Linux 3505/5710 HWMs; Linux CLI acceptance/check PASS, Windows post-adoption check pending. Final local candidate, **not published**. No push, merge, or tag without separate explicit owner approval. See `docs/releases/0.8.5.md`.
