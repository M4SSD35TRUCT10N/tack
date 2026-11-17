# Changelog / Änderungsverlauf

## Deutsch

### v0.8.4-dev (nicht freigegeben)

- POSIX-only-Testhelfer für Windows/GCC strikt mit Präprozessor-Guards abgetrennt; Plattform-SKIP (77) wird nicht als PASS gezählt.
- Versionsgebundenes Testinventar mit Owner-Freigabe und Regressionstest ergänzt.
- Getrenntes GCC-/gcov-Coverage-Profil, gemessene Line-Coverage, Quellenzeilen-Deduplikation und HWM-Prüfung implementiert.
- Erste Messung nur als Kandidat; HWM-Übernahme und Windows/TDM-GCC-Abnahme stehen aus.

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

### v0.8.4-dev (not released)

- Fully guarded POSIX-only test helpers for Windows/GCC; platform SKIP (77) no longer counts as PASS.
- Added versioned test inventory with explicit owner acceptance and regression coverage.
- Added separate GCC/gcov instrumentation profile, measured line coverage, source-line deduplication and HWM gate.
- Initial measurement remains a candidate; HWM approval and Windows/TDM-GCC validation are pending.

### v0.8.3-dev (not released)

- BA-STD-C89 repository structure and shared test header under `include/tests/`.
- Repaired test signatures and strict C89/POSIX test declarations; removed obsolete test header.
- Aligned DE/EN documentation and profile compiler examples in `tack init`/`tack new`.
- Linux/GCC debug/release tested; Windows, TinyCC and coverage ratchet remain open.

### v0.8.2-dev (not released)

- Added target/profile `compiler` and `compiler_policy`; tests follow the target profile.

### v0.8.1-dev

- Completed baseline. Details in [RELEASENOTES.md](RELEASENOTES.md).
