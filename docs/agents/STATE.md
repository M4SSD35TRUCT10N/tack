# Aktueller Projektstand / Current agent handoff

## Deutsch

- Ausgangsbasis: `tack.zip` vom Owner, Stand `v0.8.3-dev` mit Compilerprofil-Änderungen aus 0.8.2-dev und BA-STD-C89-Struktur.
- Testkorrekturen: `functional_smoke_test.c` und `help_doctor_output_test.c` an neue Funktionssignaturen angepasst; zwei POSIX-`fileno()`-Deklarationen für strenges C89 ergänzt.
- Linux/GCC und Linux/Clang: Debug- und Release-Suites mit jeweils 24/24 Tests bestanden; Details in `docs/releases/0.8.3-dev.md`. Windows-only Testpfade auf Linux nicht geprüft.
- Windows: Owner meldete vor der Reparatur einen Compilerfehler; Windows-Rerun der korrigierten Quellen noch NICHT DURCHGEFÜHRT.
- TinyCC: NICHT DURCHGEFÜHRT. Coverage: noch keine akzeptierte, gemessene High-Water-Baseline und kein aktiviertes Ratchet-Gate.
- `tack init` und `tack new` behalten generisches, nicht destruktives Scaffolding; Compilerprofil-Beispiele ergänzt, keine stille BA-STD-Migration.
- Lizenz: Bestehende MIT-Lizenz unverändert. Release: NICHT FREIGEGEBEN; Owner-Abnahme steht aus.

## English

- Input baseline: owner-provided `tack.zip`, `v0.8.3-dev` with 0.8.2-dev profile compiler changes and BA-STD-C89 structure.
- Test corrections: updated `functional_smoke_test.c` and `help_doctor_output_test.c` for new signatures; added two POSIX `fileno()` declarations for strict C89.
- Linux/GCC and Linux/Clang: debug and release suites passed 24/24 tests each; see `docs/releases/0.8.3-dev.md`. Windows-only paths are not verified on Linux.
- Windows: owner reported an earlier compilation error; rerun of the corrected sources NOT RUN yet.
- TinyCC: NOT RUN. Coverage: no accepted measured high-water baseline and no active ratchet gate.
- `tack init`/`tack new` remain generic and non-destructive; added compiler profile examples without silent BA-STD migration.
- License: existing MIT license unchanged. Release: NOT APPROVED; owner acceptance outstanding.
