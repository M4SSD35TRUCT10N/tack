# SPEC 0011 — tack: Test- und Coverage-Ratchets / Test and coverage ratchets

**Status:** Implementierungskandidat v0.8.4-dev; noch nicht freigegeben / implementation candidate v0.8.4-dev, not released.

## Deutsch

### Testvertrag

`tack ratchet tests` überprüft das versionierte `coverage/test-inventory.txt` auf Datei- und Byteänderungen an `_test.c`. Das Inventar wird nur mit `tack ratchet tests --owner-approve` verändert. Fingerprints sind **kein** kryptografischer oder semantischer Nachweis: Der Owner muss fachlich prüfen, ob bestehende positive, negative und Regressionstests und ihre Assertions in ihrer Aussagekraft erhalten bleiben. Ein geänderter Test darf nicht allein durch geänderte Zähler als akzeptiert gelten.

### Instrumentierung und Quellenumfang

`[target "app".coverage]` in der `tack.ini` konfiguriert GCC, `--coverage` und die Instrumentierungsflags für den Befehl `tack test coverage --ci --rebuild`. C89-Code ruft `gcov` ohne Shell auf, liest erzeugte Zeileninformationen und vereinigt mehrfach auftretende Header-Zeilen nach Quelldatei und Zeilennummer. Gemessen werden ausführbare Produktionszeilen in `src/` und `include/` (ohne `include/tests/`), nicht der Testcode oder Buildartefakte. Nicht gemessene neue Produktionsquellen blockieren die Auswertung.

Nach dem vollständigen instrumentierten Testlauf wird `coverage/current.txt` mit echten Zähler-/Nenner-Werten, Quellumfang, Profil-ID, Test- und Flag-Fingerprints geschrieben. Die Messung ist bei späteren Produktions-/Test-/Flag-Änderungen **veraltet**, ein frischer Durchlauf ist dann Pflicht. `coverage check` verlangt eine explizit angenommene plattformspezifische Baseline `coverage/hwm-linux.txt` beziehungsweise `coverage/hwm-windows.txt`.

### High-Water-Gate

Bei gleicher oder höherer gemessener Coverage ist die HWM zu halten beziehungsweise im **gleichen akzeptierten Stand** anzuheben. Ein bloßer Messfortschritt gilt als `BLOCKED`, bis der Owner nach Prüfung `tack coverage accept --owner-approve` ausführt. Messregressionen sind `FAIL`; inkompatible Profile oder nicht gemessene Referenzumgebungen sind `BLOCKED`. Eine Baseline darf nicht zur Schönung zurückgesetzt oder verringert werden. Für Windows sind Profilkennung und gcc/gcov-Version gesondert zu messen und abzugrenzen.

**Evidenz:** Ein echter Linux/GCC/gcov-Lauf wurde durchgeführt; die resultierende Baseline ist **noch nicht akzeptiert**, daher endet das Coverage-Gate bewusst mit `BLOCKED`. Linux/GCC Debug/Release: 25 Testquellen kompiliert, 24 Tests bestanden und ein Windows-only-Test übersprungen. Windows/TDM-GCC und TinyCC-Nachtest des 0.8.4-Stands sind `NOT RUN`. Die dokumentierte Quelle ist [Versionsbericht 0.8.4](../releases/0.8.4-dev.md).

## English

### Test contract

`tack ratchet tests` validates versioned `coverage/test-inventory.txt` for changes to `_test.c` filenames and bytes. Only `tack ratchet tests --owner-approve` can update the manifest. Fingerprints are **neither cryptographic nor semantic proof**: the owner must review that accepted positive, negative and regression tests and their assertions retain their strength. An altered test is not accepted merely because the count increased.

### Instrumentation and scope

`[target "app".coverage]` in `tack.ini` specifies GCC and `--coverage` instrumentation flags for `tack test coverage --ci --rebuild`. C89 code invokes gcov without a shell, parses executed-line data and merges duplicate header lines by file and line number. Production executable lines in `src/` and `include/` are counted, excluding `include/tests/`, test sources and build outputs. Missing measurements of new production modules block the run.

After the full instrumented test run, `coverage/current.txt` records actual covered/total counts, included scope, profile ID, test/flag fingerprints. A production, test or compiler flag change makes the measurement **stale** and mandates a fresh run. `coverage check` requires an explicitly accepted platform-specific `coverage/hwm-linux.txt` or `coverage/hwm-windows.txt`.

### High-water gate

Equal coverage retains the HWM; improved coverage must raise it in the **same accepted revision**. Improvement alone stays `BLOCKED` until the owner explicitly runs `tack coverage accept --owner-approve` after review. A regression is `FAIL`; incomparable profiles or missing reference measurements are `BLOCKED`. Never reset or reduce the baseline merely to green a build. Windows needs a separately measured and identified gcc/gcov profile.

**Evidence:** A real Linux GCC/gcov run was performed, but its baseline is **not accepted**, so the coverage gate deliberately ends `BLOCKED`. Linux/GCC Debug/Release: 25 test sources compiled, 24 passed, one Windows-only skip. Windows/TDM-GCC and TinyCC reruns for v0.8.4 are `NOT RUN`. See the [v0.8.4 release record](../releases/0.8.4-dev.md).
