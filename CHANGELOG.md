# Changelog / Änderungsverlauf

## Deutsch

### v0.8.4 (zur GitHub-Veröffentlichung freigegeben)

- RC0.2: Testinventar mit kanonischen Windows-Pfaden und LF-/CRLF-Fingerprints; echter Testinhalt bleibt ratchet-geschützt.
- POSIX-only-Testhelfer für Windows/GCC strikt mit Präprozessor-Guards abgetrennt; Plattform-SKIP (77) wird nicht als PASS gezählt.
- Versionsgebundenes Testinventar mit Owner-Freigabe und Regressionstest ergänzt.
- Getrenntes GCC-/gcov-Coverage-Profil, gemessene Line-Coverage, Quellenzeilen-Deduplikation und HWM-Prüfung implementiert.
- RC0.2 durch den Owner unter Windows/GCC Release und Windows/TinyCC Debug bestätigt: jeweils 25 kompiliert, 24 PASS, 1 SKIP, 0 FAIL.
- Frische Linux/GCC-14.2.0/gcov-14.2.0-Referenzmessung für den Releasequellstand als versionierte `coverage/hwm-linux.txt` angenommen; Ratchet-Prüfung PASS. Windows-Coverage ist noch nicht gemessen.
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

### v0.8.4 (approved for GitHub publication)

- RC0.2: Canonical Windows paths and LF/CRLF test fingerprints; meaningful test content remains ratchet-protected.
- Fully guarded POSIX-only test helpers for Windows/GCC; platform SKIP (77) no longer counts as PASS.
- Added versioned test inventory with explicit owner acceptance and regression coverage.
- Added separate GCC/gcov instrumentation profile, measured line coverage, source-line deduplication and HWM gate.
- RC0.2 owner validation on Windows/GCC release and Windows/TinyCC debug: each 25 compiled, 24 PASS, one SKIP and zero failures.
- Accepted a fresh Linux GCC 14.2.0/gcov 14.2.0 production line measurement for the release source as versioned `coverage/hwm-linux.txt`; ratchet check PASS. Windows coverage is not measured.
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
