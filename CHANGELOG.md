# Changelog / Änderungsverlauf

## Deutsch
### v0.8.5 finaler lokaler Release-Kandidat (HWM freigegeben, Veröffentlichung ausstehend)
- Windows RC0.7 vollständig bestätigt: GCC Release 25 PASS/1 SKIP, TinyCC Debug 24 PASS/2 SKIP, Ratchet/Docs PASS, echter clang-format-E2E mit Leerzeichen und Exitcodes 0/2/0/0/1/0 PASS.
- Linux GCC/Clang Strict C89, Release 25 PASS/1 SKIP, Ratchet/Docs/Formatter E2E PASS. Owner hat Windows 3577/5906 (60,56553 %) und Linux 3505/5710 (61,38354 %) zur HWM-Annahme freigegeben; Linux CLI-Annahme und Check PASS, Windows Post-Adoption-Check noch offen.
- Historische RC0.4–RC0.6-Prüfstände nach `docs/releases/archive/0.8.5/` verschoben. `docs/releases/0.8.5.md` ist der kanonische Bericht. Kein GitHub-Push/Merge/Tag ohne separate Freigabe.


### v0.8.5 RC0.7 (Versionsanhebung und Release-Dokumentation, NICHT FREIGEGEBEN)
- Auf Basis der Windows-RC0.6-Owner-Abnahme: GCC Release 25 PASS/1 SKIP, TinyCC Debug 24 PASS/2 SKIP, externes `clang-format 23.1.5` mit Leerzeichen und Exitcodes 0/2/0/0/1/0 PASS; `git fetch` erfolgreich.
- Zentrale Programmversion auf 0.8.5 angehoben, mitgelieferte `tack.ini` und DE/EN-Dokumentation konsolidiert. Historische RC-Protokolle bleiben erhalten. Keine neuen Features, keine Änderung der Formatter-Logik oder Test-Assertions.
- Windows RC0.6 3577/5906 und Linux RC0.6 3505/5710 nur als Coverage-Messkandidaten; Windows- und Linux-HWM unverändert. Finale Tests auf RC0.7 und separate Owner-Freigaben erforderlich. Kein Commit/Push/Merge/Tag. Siehe `docs/releases/0.8.5-rc0.7-validation.md`.

### v0.8.5-dev RC0.6 (Formatter-Platzhalter-Terminierung, NICHT FREIGEGEBEN)
- Owner-RC0.5: GCC Release und TinyCC Debug brechen bei echten Windows-Kindprozess-Assertions ab; 13 PASS, 1 FAIL, 12 NOT RUN. Coverage ohne gültige aktuelle Messung. Der Formatter-E2E wurde nicht erneut ausgeführt.
- Eng begrenzter ISO-C89-Fix: `fmt_replace_all_alloc()` terminiert nun auch Platzhalter-Ersetzungen am Tokenende mit `\0`; die Windows-`CreateProcessA`-Implementierung bleibt unverändert. Sechs zusätzliche plattformübergreifende Regressionen, alle bisherigen Tests erhalten.
- Linux GCC/Clang Strict C89 PASS; GCC Release 25 PASS, 1 SKIP; Dokumentation und isolierter Formatter E2E PASS. GCC/gcov 3505/5710 (61,38354 %) gemessen; HWM bleibt BLOCKED.
- Beide angenommenen HWMs unverändert, Windows RC0.6 NOT RUN, Version 0.8.4, kein Commit/Push/Merge/Tag. Details: `docs/releases/archive/0.8.5/0.8.5-dev-RC0.6-validation.md`.

### v0.8.5-dev RC0.5 (Windows-Formatter-Prozessstart, NICHT FREIGEGEBEN)
- Owner-RC0.4: Windows GCC Release **26 kompiliert, 25 PASS, 1 SKIP**; TinyCC Debug **26 kompiliert, 24 PASS, 2 SKIP**; Test-Ratchet und `tack doc --strict` PASS. GCC/gcov: **3459/5859 (59,03738 %)**, `coverage check` korrekt **BLOCKED**, nicht FAIL der funktionalen Tests.
- Tatsächlicher Windows-Formatter-E2E bleibt RC0.4-**FAIL**: LIST 0; CHECK_BEFORE 1 (statt 2); ORIGINAL_UNCHANGED 0; FORMAT 1 (statt 0); CHANGED 0 (statt 1); CHECK_AFTER 1 (statt 0). Dateinamen werden nicht mehr gespalten, enthalten aber beschädigte Suffixe. `clang-format 23.1.5` installiert. Das RC0.4-Quoting vor `_spawnvp` reicht nicht.
- Enger C89-Fixkandidat: **nur Formatter** nutzt unter Windows `CreateProcessA` mit selbst aufgebauter, begrenzter, NUL-terminierter Win32-Kommandozeile und echtem Wait/Exitcode; allgemeiner tack-Build-Runner bleibt unverändert. Keine Shell, keine neue Tool-Abhängigkeit. `fmt_ini_parser_test` erweitert um **drei echte Windows-Kindprozesse** mit Leerzeichen im Executable-Pfad und Leerzeichen/Quotes/abschließendem Backslash im Argument; bestehende fünf Quote-Assertions bleiben erhalten.
- Linux GCC und Clang Strict C89 BUILD PASS; GCC-Release: **26 kompiliert, 25 PASS, 1 SKIP**; Linux Formatter E2E in Ordner mit Leerzeichen PASS; Coverage frisch **3470/5709 (60,78122 %)**, HWM-Gate weiterhin **BLOCKED** bis Owner-Freigabe. Beide angenommenen HWM-Dateien unverändert.
- Windows RC0.5-Tests **NICHT AUSGEFÜHRT**. Git-SSH-Probe `ssh -T git@github.com` inzwischen PASS, `git fetch` noch nicht erfolgreich wiederholt. Programmversion weiter **0.8.4**; kein Git-Push, Merge oder Tag. Siehe `docs/releases/archive/0.8.5/0.8.5-dev-RC0.5-validation.md`.

### v0.8.5-dev RC0.4 (Windows-Formatter-Argumentpfade, NICHT FREIGEGEBEN)
- Owner-RC0.3-Befund: `clang-format` vorhanden, `fmt --list` PASS, tatsächliches `--check` und Write FAIL durch Aufspaltung der übergebenen Pfade mit Leerzeichen; Original bleibt unverändert.
- Enge C89-Korrektur des Formatter-Ausführungspfads: Quoting der Windows-CRT-Kindargumente vor `_spawnvp`, einschließlich Quotes und abschließender Backslashes. Globaler Build-Prozessstarter unverändert, keine neuen Abhängigkeiten.
- Fünf zusätzliche Windows-Regressionen in `fmt_ini_parser_test.c`, nach Diff-Review aktualisierter Fingerprint im Testinventar. Linux GCC Release: 26 kompiliert, 25 PASS, 1 SKIP. Linux-Formatter-E2E: Exitcodes 0/2/0/0 im Ordner mit Leerzeichen. Linux-Coverage neu gemessen: 3470/5709 (60,78122 %), `BLOCKED` bis zur Owner-HWM-Freigabe.
- Angenommene Windows-/Linux-HWMs bytegleich erhalten; Windows-Retest und Gesamt-Release-Abnahme offen. Versionsnummer bleibt 0.8.4, kein Push/Tag.


### v0.8.5-dev RC0.3 (Windows-HWM und externe Linux-Formatter-Prüfung, NICHT FREIGEGEBEN)
- Owner-Prüfung: Windows GCC Release und TinyCC Debug je 26 kompiliert, 25 PASS, 1 SKIP; GCC/gcov nach neuer HWM-Annahme 3434/5827 (58,93256 %) PASS, 26 Testquellen und keine Fehler. Die vorherige Windows-HWM war 3192/5827.
- Erster echter externer Formatter-CLI-Lauf auf Linux: Liste, Check-Differenz mit unveränderter Quelldatei, Formatieren und anschließender Check-PASS; Dateiname mit Leerzeichen. Windows externer Formatter bleibt NOT RUN.
- Kein Produktionscode oder Test verändert; Linux 3470/5708 (60,79187 %) bleibt gemessener, noch nicht angenommener Kandidat. Die Ausführungsversionsnummer bleibt 0.8.4; kein Tag/Push.

### v0.8.5-dev RC0.2 (Formatter-Parserkorrektur, NICHT FREIGEGEBEN)
- Compilerprogramm und Compiler-Policy je Targetprofil (seit 0.8.2-dev) jetzt direkt im README-Hauptteil erklärt.
- Bilinguale INI-Anleitung und Parser-Schlüsselreferenz mit nachvollziehbaren Beispielen ergänzt.
- Den aus 0.8.4 bekannten verschachtelten `[sbom]`-Zweig repariert; `[fmt]` und `[fmt "NAME"]` funktionieren jetzt als INI-Konfiguration. C89-Parser-/CLI-Regressionstest und versioniertes Testinventar ergänzt. Linux GCC Release: 25 PASS, 1 SKIP. Gemessener Linux-Coverage-Kandidat: 3470/5708 (60,79187 %), Freigabe der höheren HWM noch offen.
- Separate Planungsgrundlage für 0.8.6 mit 90-%-Line-Coverage pro Plattform und unabhängiger Anforderungsabdeckung; angenommene HWMs unverändert, Testinventar für den neuen Regressionstest erhöht.


### v0.8.4 (zur GitHub-Veröffentlichung freigegeben)
- **RC0.6:** Echte Owner-Messung Windows/TDM-GCC/gcov 10.3.0 mit **3192/5827 (54,77947 %)** in `coverage/hwm-windows.txt` festgeschrieben. Getrennte Linux-HWM 3228/5708 bleibt unverändert. HWM-Zustandsregeln DE/EN und Abnahmenachweis ergänzt; finaler Windows-Coverage-PASS vor GitHub-Push noch erforderlich.
- **RC0.5, vor dem GitHub-Tag:** Zwei falsche Unix-Pfad-Erwartungen im `coverage_ratchet_test` ließen Windows-GCC Release/Coverage vor der eigentlichen gcov-Auswertung abbrechen. Die Regression prüft jetzt native Windows-/POSIX-Pfade **und** deren kanonische `build/`-Position; aussagekräftige Ist-/Soll-Diagnosen ergänzt. Das Testinventar wurde für den erweiterten Teststand aktualisiert, ohne eine Coverage-HWM abzusenken. Die spätere RC0.5-Windows-Abnahme ist unter RC0.6 dokumentiert; bestätigender Windows-PASS nach der HWM-Übernahme steht aus.
- Windows-Coverage-Fix vor Tag: Testquellen werden zuerst in `build/tests/coverage/obj/` zu `.o`/`.gcno` kompiliert und danach separat gelinkt. `gcov` liest nur die neue Objektablage; fehlende Notizen führen zu FAIL. Die vorherige `coverage/current.txt` wird vor jeder neuen Messung invalidiert, angenommene HWMs bleiben erhalten.
- GitHub-Vorbereitung: rückwärtskompatible `id_windows` für getrennte Linux-/Windows-HWM sowie genaue Windows-Abnahmeschritte (Windows-Messung seit RC0.5 nachgewiesen und in RC0.6 als HWM festgeschrieben; Nachtest steht aus).

- RC0.2: Testinventar mit kanonischen Windows-Pfaden und LF-/CRLF-Fingerprints; echter Testinhalt bleibt ratchet-geschützt.
- POSIX-only-Testhelfer für Windows/GCC strikt mit Präprozessor-Guards abgetrennt; Plattform-SKIP (77) wird nicht als PASS gezählt.
- Versionsgebundenes Testinventar mit Owner-Freigabe und Regressionstest ergänzt.
- Getrenntes GCC-/gcov-Coverage-Profil, gemessene Line-Coverage, Quellenzeilen-Deduplikation und HWM-Prüfung implementiert.
- RC0.2 durch den Owner unter Windows/GCC Release und Windows/TinyCC Debug bestätigt: jeweils 25 kompiliert, 24 PASS, 1 SKIP, 0 FAIL.
- Frische Linux/GCC-14.2.0/gcov-14.2.0-Referenzmessung für den Releasequellstand als versionierte `coverage/hwm-linux.txt` angenommen; Ratchet-Prüfung PASS. Windows-Coverage wurde später in RC0.5 gemessen und in RC0.6 separat versioniert.
- `TACK_VERSION` und Startkonfiguration auf `0.8.4` gesetzt; Veröffentlichungs- und Prüfnachweise aktualisiert. GitHub-Upload/Tag sind noch nicht erfolgt.

### v0.8.3-dev (nicht freigegeben)

- BA-STD-C89-Repositorystruktur und gemeinsamer Testheader unter `include/tests/`.
- Reparierte Test-Signaturen und C89/POSIX-Testdeklarationen; überholten Testheader entfernt.
- DE/EN-Dokumentation und Profilcompiler-Beispiele in `tack init`/`tack new` synchronisiert.
- Linux/GCC Debug/Release geprüft; Windows, TinyCC und Coverage-Ratchet offen.

### v0.8.2-dev (nicht freigegeben)

- `compiler` und `compiler_policy` für Targetprofile ergänzt. Tests folgen dem Zielprofil.

### v0.8.1-dev

- Abgeschlossener Basisstand. Details in [RELEASENOTES.md](RELEASENOTES.md).

## English
### v0.8.5 final local release candidate (HWMs approved, publication pending)
- Owner Windows RC0.7: GCC release 25 PASS/1 SKIP, TinyCC debug 24 PASS/2 SKIP, ratchet/docs PASS, real clang-format E2E with spaces and 0/2/0/0/1/0 exit codes PASS.
- Linux GCC/Clang strict C89, release 25 PASS/1 SKIP, ratchet/docs/formatter E2E PASS. Owner approved independent Windows 3577/5906 (60.56553%) and Linux 3505/5710 (61.38354%) HWMs. Linux CLI acceptance/check PASS; Windows post-adoption rerun pending.
- Historical RC0.4–RC0.6 records archived under `docs/releases/archive/0.8.5/`. Canonical report is `docs/releases/0.8.5.md`. No GitHub push/merge/tag without separate approval.


### v0.8.5 RC0.7 (version promotion and release docs, NOT RELEASED)
- Based on owner-provided Windows RC0.6 evidence: GCC release 25 PASS/1 SKIP, TinyCC debug 24 PASS/2 SKIP, real `clang-format 23.1.5` with spaced paths and exit codes 0/2/0/0/1/0 PASS; `git fetch` succeeded.
- Central binary version promoted to 0.8.5; shipped `tack.ini` and DE/EN release documentation consolidated. Historical RC evidence retained. No new features and no formatter runtime or test assertion changes.
- Windows RC0.6 3577/5906 and Linux RC0.6 3505/5710 remain measured, unaccepted coverage candidates. Final RC0.7 retests and separate owner HWM approvals required. No commit, push, merge or tag. See `docs/releases/0.8.5-rc0.7-validation.md`.

### v0.8.5-dev RC0.6 (formatter placeholder termination, NOT RELEASED)
- Owner RC0.5 Windows GCC/TCC runs failed a real child-process regression (13 PASS, 1 FAIL, 12 NOT RUN each); no valid Windows coverage measurement was produced.
- Fixed the missing NUL terminator after a trailing formatter placeholder; six cross-platform regressions added without weakening existing assertions or changing `CreateProcessA`.
- Strict Linux GCC/Clang C89 and GCC Release 25 PASS/1 SKIP, docs and external formatter E2E passed. Linux gcov measured 3505/5710 (61.38354%); HWM remains BLOCKED.
- Existing accepted Windows/Linux HWMs unchanged. Windows RC0.6 untested; no version bump, HWM acceptance or Git publication. See `docs/releases/archive/0.8.5/0.8.5-dev-RC0.6-validation.md`.

### v0.8.5-dev RC0.5 (Windows formatter process launch, NOT RELEASED)
- Owner RC0.4 Windows: GCC release **26 compiled, 25 PASS, 1 SKIP**; TinyCC debug **26 compiled, 24 PASS, 2 SKIP**; inventory ratchet and docs PASS. GCC/gcov **3459/5859 (59.03738%)**, coverage HWM check correctly **BLOCKED** (not a functional-test failure).
- Real Windows formatter still **FAIL** on RC0.4: LIST 0; first CHECK 1 (expected 2); original unchanged 0; FORMAT 1 (expected 0); no file change; final CHECK 1 (expected 0). The file paths carry corrupted suffixes rather than split fragments. `clang-format 23.1.5` installed. CRT `_spawnvp` quoting alone was insufficient.
- Narrow C89 candidate: Windows external **formatter only** now launches via `CreateProcessA`, with a fully bounded, NUL-terminated command line, wait and exit-code verification. General tack build process launcher untouched. No shell or new dependencies. Existing five Windows quoting assertions retained; three actual child-process probes added for space-containing executable/argument paths, trailing backslash and embedded quote.
- Linux GCC and Clang strict C89 builds PASS, GCC release **26 compiled, 25 PASS, 1 SKIP**, external formatter E2E PASS, Linux coverage **3470/5709 (60.78122%)** measured, higher HWM **BLOCKED** until owner acceptance. Accepted Windows/Linux HWMs byte-identical to RC0.4.
- Windows RC0.5 **NOT RUN**, SSH authentication probe successful but Git fetch not reverified. Binary version **0.8.4**, no push/merge/tag. See `docs/releases/archive/0.8.5/0.8.5-dev-RC0.5-validation.md`.

### v0.8.5-dev RC0.4 (Windows external formatter argument paths, NOT RELEASED)
- Owner RC0.3 test: `clang-format` installed, list PASS, real check and write FAIL because space-containing paths were split; original unchanged.
- Narrow C89 fix: quote Windows CRT child arguments in the formatter runner before `_spawnvp`, including embedded quotes and trailing backslashes. The general build process runner is unchanged; no dependencies added.
- Five Windows-only regression assertions added to `fmt_ini_parser_test.c`, test inventory fingerprint updated for review. Linux GCC release: 26 compiled, 25 PASS, one SKIP; external Linux formatter E2E 0/2/0/0 with spaces in directory and file name. Linux GCC/gcov 3470/5709 (60.78122%), improved HWM candidate remains BLOCKED pending owner approval.
- Accepted Windows/Linux HWM files byte-identical to RC0.3. Windows retest and full release acceptance remain pending; executable version still 0.8.4, no push/tag.


### v0.8.5-dev RC0.3 (Windows HWM and Linux external formatter verification, NOT RELEASED)
- Owner-verified Windows GCC release and TinyCC debug: 26 compiled, 25 PASS, one SKIP, zero failures; GCC/gcov PASS after acceptance of 3434/5827 (58.93256%), raised from 3192/5827.
- Real external formatter process validated on Linux using CLI list, check-before, in-place formatting and check-after with a filename containing spaces. Windows external formatter E2E NOT RUN.
- Production and test sources unchanged; Linux 3470/5708 (60.79187%) remains measured but not yet approved as HWM. Binary version remains 0.8.4; no tag/push.

### v0.8.5-dev RC0.2 (formatter parser fix, NOT RELEASED)
- README now explains the per-target/profile compiler-program and compiler-policy split added in 0.8.2-dev.
- Added bilingual INI walkthrough and complete parser key reference, including tested examples.
- Fixed the 0.8.4 `[sbom]` nesting defect so `[fmt]` and `[fmt "NAME"]` parse correctly; added strict C89 parser/CLI regression and updated test inventory. Linux release 25 PASS/1 SKIP; measured 60.79187% Linux coverage improvement remains a candidate until accepted.
- Planned separate 0.8.6 quality targets (90% line coverage per platform and independent requirements coverage); accepted HWMs remain unchanged, test inventory extended for the new regression test.


### v0.8.4 (approved for GitHub publication)
- **RC0.6:** Version the owner-measured Windows TDM-GCC/gcov 10.3.0 HWM of **3192/5827 (54.77947%)** in `coverage/hwm-windows.txt`; preserve separate Linux HWM 3228/5708. Document HWM states and evidence in DE/EN. New Windows PASS after baseline installation remains required before GitHub push.
- **Pre-tag RC0.5:** Correct two Unix-only path expectations in `coverage_ratchet_test` that caused Windows GCC release and coverage runs to abort before gcov collection. Assert native Windows/POSIX paths **and** canonical `build/` locations, with actionable actual/expected diagnostics. Update the versioned test inventory for the reviewed changes without lowering any coverage HWM. RC0.5 owner validation subsequently completed; the versioned Windows HWM still requires a fresh PASS after adoption.
- Pre-tag Windows gcov correction: compile instrumented tests to explicit objects/notes under `build/tests/coverage/obj/` before linking; collect only from that directory. Missing notes fail closed, and stale `coverage/current.txt` is invalidated before a new run without changing accepted HWMs.
- Publishing preparation: backwards-compatible platform-specific `id_windows` and documented Windows measurement/acceptance; no Windows HWM is claimed before an actual run.

- RC0.2: Canonical Windows paths and LF/CRLF test fingerprints; meaningful test content remains ratchet-protected.
- Fully guarded POSIX-only test helpers for Windows/GCC; platform SKIP (77) no longer counts as PASS.
- Added versioned test inventory with explicit owner acceptance and regression coverage.
- Added separate GCC/gcov instrumentation profile, measured line coverage, source-line deduplication and HWM gate.
- RC0.2 owner validation on Windows/GCC release and Windows/TinyCC debug: each 25 compiled, 24 PASS, one SKIP and zero failures.
- Accepted a fresh Linux GCC 14.2.0/gcov 14.2.0 production line measurement for the release source as versioned `coverage/hwm-linux.txt`; ratchet check PASS. Windows coverage was later measured during RC0.5 and independently versioned in RC0.6.
- Aligned `TACK_VERSION` and starter configuration with `0.8.4`, and updated release evidence. GitHub upload/tag remains to be done.

### v0.8.3-dev (not released)

- BA-STD-C89 repository structure and shared test header under `include/tests/`.
- Repaired test signatures and strict C89/POSIX test declarations; removed obsolete test header.
- Aligned DE/EN documentation and profile compiler examples in `tack init`/`tack new`.
- Linux/GCC debug/release tested; Windows, TinyCC and coverage ratchet remain open.

### v0.8.2-dev (not released)

- Added target/profile `compiler` and `compiler_policy`; tests follow the target profile.

### v0.8.1-dev

- Completed baseline. Details in [RELEASENOTES.md](RELEASENOTES.md).
