# Changelog / Änderungsverlauf

## Deutsch

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
