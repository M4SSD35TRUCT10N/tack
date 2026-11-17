# SPEC 0011 — tack: Test- und Coverage-Ratchets / Test and coverage ratchets

**Status:** tack v0.8.4 durch Projekt-Owner zur Veröffentlichung freigegeben / tack v0.8.4 owner-approved for publication; BA-STD-C89 group policy remains a draft.

## Deutsch

### Testvertrag

`tack ratchet tests` überprüft das versionierte `coverage/test-inventory.txt` auf Datei- und bedeutungsrelevante Textänderungen an `_test.c` (plattformneutral kanonisierte Pfade und Zeilenenden). Das Inventar wird nur mit `tack ratchet tests --owner-approve` verändert. Fingerprints sind **kein** kryptografischer oder semantischer Nachweis: Der Owner muss fachlich prüfen, ob bestehende positive, negative und Regressionstests und ihre Assertions in ihrer Aussagekraft erhalten bleiben. Ein geänderter Test darf nicht allein durch geänderte Zähler als akzeptiert gelten.

### Instrumentierung und Quellenumfang

`[target "app".coverage]` in der `tack.ini` konfiguriert GCC, `--coverage` und die Instrumentierungsflags für den Befehl `tack test coverage --ci --rebuild`. C89-Code ruft `gcov` ohne Shell auf, liest erzeugte Zeileninformationen und vereinigt mehrfach auftretende Header-Zeilen nach Quelldatei und Zeilennummer. Gemessen werden ausführbare Produktionszeilen in `src/` und `include/` (ohne `include/tests/`), nicht der Testcode oder Buildartefakte. Nicht gemessene neue Produktionsquellen blockieren die Auswertung.

Für reproduzierbare Ablage kompiliert der Coverage-Testtreiber zuerst jede Testquelle mit GCC `-c -o build/tests/coverage/obj/NAME.o`, prüft die dortige `.gcno` und linkt erst danach die ausführbare Testdatei in `build/tests/coverage/bin/`. Die `.gcda` entstehen neben den Objekten. Der gcov-Collector liest **ausschließlich** diese Objektablage; die Arbeitsergebnisse liegen unter `build/coverage-gcov/`. Alte Root-Artefakte aus dem fehlerhaften Ein-Schritt-Windows-Lauf sind nicht Teil einer gültigen Messung. Zu Beginn eines neuen Laufs wird die vorige, nicht angenommene `coverage/current.txt` verworfen, damit Fehler sie nicht als aktuelle Evidenz erscheinen lassen.

Nach dem vollständigen instrumentierten Testlauf wird `coverage/current.txt` mit echten Zähler-/Nenner-Werten, Quellumfang, Profil-ID, Test- und Flag-Fingerprints geschrieben. Die Messung ist bei späteren Produktions-/Test-/Flag-Änderungen **veraltet**, ein frischer Durchlauf ist dann Pflicht. `coverage check` verlangt eine explizit angenommene plattformspezifische Baseline `coverage/hwm-linux.txt` beziehungsweise `coverage/hwm-windows.txt`.

### High-Water-Gate

Bei gleicher oder höherer gemessener Coverage ist die HWM zu halten beziehungsweise im **gleichen akzeptierten Stand** anzuheben. Ein bloßer Messfortschritt gilt als `BLOCKED`, bis der Owner nach Prüfung `tack coverage accept --owner-approve` ausführt. Messregressionen sind `FAIL`; inkompatible Profile oder nicht gemessene Referenzumgebungen sind `BLOCKED`. Eine Baseline darf nicht zur Schönung zurückgesetzt oder verringert werden. Für Windows sind Profilkennung und gcc/gcov-Version gesondert zu messen und abzugrenzen. Mit dem optionalen `[coverage] id_windows` kann Windows ein eigenes Messprofil verwenden, während die bisherige `[coverage] id` das Linux-Referenzprofil identifiziert. Die Windows-HWM muss **tatsächlich auf Windows gemessen** und separat ownergeprüft werden; die neue INI-Option ist kein Mess- oder Freigabenachweis.

**Evidenz:** Die reale Linux/GCC-14.2.0/gcov-14.2.0-Referenzmessung **3.228/5.708** wurde in `coverage/hwm-linux.txt` versioniert und geprüft. Linux/GCC Debug/Release: 25 Tests kompiliert, 24 PASS und ein Windows-only-SKIP. Der Owner hat auf Windows RC0.2 mit GCC Release und TinyCC Debug jeweils 25 kompiliert, 24 PASS, 1 SKIP und 0 FAIL nachgewiesen. Ein späterer Windows-Coverage-Versuch blieb wegen falsch abgelegter `.gcno` ohne verwertbare Messung **BLOCKED**; RC0.4 bedarf der Windows-Nachprüfung. Die BA-STD-C89-Gruppennorm wird dadurch nicht automatisch ratifiziert. Siehe [Versionsbericht 0.8.4](../releases/0.8.4.md).

## English

### Test contract

`tack ratchet tests` validates versioned `coverage/test-inventory.txt` for changes to `_test.c` filenames and source contents (canonicalizing path separators and line endings across platforms). Only `tack ratchet tests --owner-approve` can update the manifest. Fingerprints are **neither cryptographic nor semantic proof**: the owner must review that accepted positive, negative and regression tests and their assertions retain their strength. An altered test is not accepted merely because the count increased.

### Instrumentation and scope

`[target "app".coverage]` in `tack.ini` specifies GCC and `--coverage` instrumentation flags for `tack test coverage --ci --rebuild`. C89 code invokes gcov without a shell, parses executed-line data and merges duplicate header lines by file and line number. Production executable lines in `src/` and `include/` are counted, excluding `include/tests/`, test sources and build outputs. Missing measurements of new production modules block the run.

For deterministic placement the coverage runner compiles each test source with GCC `-c -o build/tests/coverage/obj/NAME.o`, verifies its `.gcno`, then links the executable under `build/tests/coverage/bin/`. Runtime `.gcda` stays beside its object; the gcov collector reads **only** this object directory, with scratch output under `build/coverage-gcov/`. Root artifacts from the old one-step Windows build are not valid evidence. A new instrumented run invalidates the previous, unaccepted `coverage/current.txt` before testing, preventing failed runs from appearing current.

After the full instrumented test run, `coverage/current.txt` records actual covered/total counts, included scope, profile ID, test/flag fingerprints. A production, test or compiler flag change makes the measurement **stale** and mandates a fresh run. `coverage check` requires an explicitly accepted platform-specific `coverage/hwm-linux.txt` or `coverage/hwm-windows.txt`.

### High-water gate

Equal coverage retains the HWM; improved coverage must raise it in the **same accepted revision**. Improvement alone stays `BLOCKED` until the owner explicitly runs `tack coverage accept --owner-approve` after review. A regression is `FAIL`; incomparable profiles or missing reference measurements are `BLOCKED`. Never reset or reduce the baseline merely to green a build. Windows needs a separately measured and identified gcc/gcov profile. Optional `[coverage] id_windows` allows an independent Windows ID alongside the existing Linux `[coverage] id`. This does not create a Windows measurement or authorize a baseline without actual Windows instrumentation and owner review.

**Evidence:** The real Linux GCC 14.2.0/gcov 14.2.0 measurement of **3,228/5,708** is versioned in `coverage/hwm-linux.txt` and checked. Linux GCC debug/release compiled 25 tests with 24 PASS and one Windows-only SKIP. Owner-provided Windows RC0.2 GCC release and TinyCC debug logs each show 25 compiled, 24 PASS, one SKIP, zero failures. A subsequent Windows coverage attempt was **BLOCKED** by misplaced `.gcno` files without a valid measurement; RC0.4 needs fresh Windows verification. This does not ratify the group-wide BA-STD-C89 draft. See the [v0.8.4 release record](../releases/0.8.4.md).
