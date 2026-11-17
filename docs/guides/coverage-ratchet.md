# Test- und Coverage-Ratchet / Test and coverage ratchets

**Stand / status:** v0.8.4, Owner-Freigabe zur GitHub-Publikation / owner-approved for GitHub publication (push/tag pending).

## Deutsch

### 1. Test-Ratchet (separat von Coverage)

Das versionierte `coverage/test-inventory.txt` erfasst Namen und Fingerprints aller `_test.c`-Dateien. Ab RC0.2 werden Pfadtrenner (`\\` oder `/`) als `/` und Text-Zeilenenden (CRLF, LF) als LF kanonisiert. Sonstige Dateiänderungen bleiben sperrend; die kanonische Darstellung ist kein Freibrief zum Abschwächen von Tests. Prüfen:

```cmd
tack --config .\tack.ini ratchet tests
```

Änderungen dürfen nur nach Diff- und Assertion-Review akzeptiert werden. Ein Fingerprint beweist **keine** fachliche Gleichwertigkeit. Explizit nach Owner-Entscheid:

```cmd
tack --config .\tack.ini ratchet tests --owner-approve
```

### 2. Messprofil (INI-first)

```ini
[target "app".coverage]
compiler = gcc
compiler_policy = gcc
cflags = -std=c89;-pedantic-errors;-Wextra;-Werror;--coverage;-O0
ldflags = --coverage

[coverage]
id = linux-gcc-14.2.0-gcov-14.2.0-coverage-v1
id_windows = windows-gcc-10.3.0-gcov-10.3.0-coverage-v1
```

Die Profil-ID ist **kein** automatischer Versionsnachweis. Der Owner gleicht sie mit dem tatsächlich verwendeten `gcc --version` und `gcov --version`, der Plattform und der Messdefinition ab. Für Windows muss eine eigene `windows-...`-Profil-ID verwendet werden. Ab dem vorbereiteten 0.8.4-Windows-HWM-Nachtrag kann `id_windows` zusätzlich zu `id` eingetragen werden. Auf Windows gewinnt `id_windows`, auf Linux bleibt `id` aktiv. Der Linux-Eintrag wird **nicht** für Windows überschrieben. Compilerflags dürfen ausschließlich in `tack.ini` stehen. Unter Linux ohne installiertes TinyCC kann für **Compilerwahl, nicht für Compilerflags**, `TACK_CC=gcc` beim Testlauf gesetzt werden; sonst schlägt der generische `functional_smoke_test` zu Recht wegen fehlendem Compiler fehl.

### 3. Messen, prüfen, annehmen

```cmd
tack --config .\tack.ini test coverage --ci --rebuild -j 1
tack --config .\tack.ini coverage check
```

Der erste Befehl misst Produktionszeilen (`src/` und `include/`, ohne `include/tests/`) mittels GCC/gcov und schreibt das lokal erzeugte, nicht zu commitende `coverage/current.txt`. Für das Linux-Referenzprofil existiert ab v0.8.4 eine vom Owner angenommene `coverage/hwm-linux.txt`. Auf einem passenden Linux-GCC/gcov-Referenzsystem kann der Coverage-Lauf nun **PASS** liefern. Fehlt eine HWM für ein anderes Profil, bleibt dessen Coverage-Gate **BLOCKED**, selbst wenn Tests bestehen. Ein niedrigerer Wert als eine akzeptierte HWM ist FAIL; ein höherer bleibt bis zur Owner-Freigabe BLOCKED. `--rebuild` und frische Messdaten verhindern veraltete Instrumentierung.

Nach Prüfung des angezeigten `covered/total`, der Testresultate, Quell-/Toolchain-Filter und eines dokumentierten Ownersentscheids:

```cmd
tack --config .\tack.ini coverage accept --owner-approve
tack --config .\tack.ini coverage check
```

Die Anhebung ist Teil **desselben freigegebenen Commits** wie die zugehörige höhere Messung. Ein neues Messprofil braucht Vergleichsmessung und dokumentierte Migration; das Absenken der bisherigen HWM ist verboten. `coverage/current.txt` ist **kein** angenommenes HWM und bleibt als lokaler Messbericht von Commit-Artefakten getrennt.

### 4. Angenommener Linux-Referenzstand

Der Owner hat die HWM für das Linux-Referenzprofil ausdrücklich freigegeben. Die nach der Windows-Profilergänzung neu erstellte Messung **3.228/5.708 (56,55221 %)** ist in `coverage/hwm-linux.txt` versioniert. Maßgeblich sind die **tatsächlich** gespeicherten `covered`/`total`-Werte; die Messung vom RC0.2 ist nur historische Evidenz. Der Ratchet prüft Quell-/Test-/Flag-Fingerprints und blockiert geänderte oder inkompatible Messungen. Der einmal angenommene HWM-Wert darf nicht abgesenkt werden.

### 5. Windows-HWM erstmals messen und annehmen (CMD, vor GitHub-Push)

**Vorbedingung:** Das Repository enthält den aktualisierten Quellstand mit optionalem `[coverage] id_windows`. Die Windows-GCC-/TinyCC-Tests allein sind **keine** gemessene Coverage. Niemals einen Linux-Wert in `coverage/hwm-windows.txt` kopieren oder einen Test-SKIP als PASS umdeuten.

**Windows-Korrektur für den Coverage-Lauf:** Frühere 0.8.4-Kandidaten kompilierten und linkten instrumentierte Tests mit einem einzigen GCC-Aufruf. TDM-GCC 10.3.0 konnte deshalb `.gcno`/`.gcda` im Repository-Root schreiben, während `tack` die Notizen unter `build/tests/coverage/bin/` erwartete (`0 of 25`). Im korrigierten Stand wird **jeder Test zuerst nach `build/tests/coverage/obj/<test>.o` kompiliert und anschließend getrennt nach `build/tests/coverage/bin/<test>.exe` gelinkt**. Die Instrumentierungsdateien gehören nach `build/tests/coverage/obj/`; gcov-Zwischenberichte entstehen unter `build/coverage-gcov/`. Ein fehlendes `.gcno` wird beim Kompilieren als Fehler behandelt. `coverage/current.txt` wird vor jedem neuen instrumentierten Lauf gelöscht, sodass eine gescheiterte Messung nicht als aktuelle Evidenz übernommen werden kann. Die akzeptierten `hwm-*.txt` werden dabei niemals gelöscht.

**RC0.5-Nachtrag zu `coverage_ratchet_test: 2 failures`:** Nach der Root-Artefakt-Korrektur enthielt RC0.4 noch zwei Tests mit hartcodierten Unix-Pfaden (`build/tests/coverage/obj/sample_test.o` und `.gcno`). `path_join()` setzt auf Windows stattdessen `\\` als neuen Separator ein. Das führte im Owner-Protokoll zu exakt zwei Testfehlern **vor** der gcov-Auswertung, weshalb `coverage/current.txt` zu Recht fehlte. RC0.5 prüft für jedes Artefakt den nativen Windows-/POSIX-Pfad und den nach `/` kanonisierten Pfad. Alle vier Assertions bleiben aktiv. Eine fehlende Messdatei nach `coverage_ratchet_test: 2 failures` ist daher **noch kein** Hinweis auf ein erneutes `.gcno`-Collector-Problem. Zuerst mit der neu gebauten RC0.5-Executable `test release` ausführen und `coverage_ratchet_test: ok` verlangen; erst danach `test coverage` starten. Die Windows-HWM ist bis zur echten Messung weiterhin **BLOCKED**.

**Einmalige Bereinigung alter Root-Artefakte (nur nach Sichtprüfung):** Vor dem nächsten Test in `C:\Projects\tack` mit `dir /b *.gcda *.gcno *.gcov` nachsehen. Nur die alten **direkt im Projekt-Root** erzeugten Instrumentierungsdateien mit `del /q *.gcda *.gcno *.gcov` löschen; keine rekursiven Löschbefehle und keine Quelldateien anfassen. `tack clean` entfernt die früheren, fälschlich im Root erzeugten Dateien nicht. Nach dem korrigierten Lauf darf `dir /b *.gcda *.gcno *.gcov` keine neuen Treffer ausgeben.

1. **Compiler und gcov tatsächlich prüfen** (beide Programme müssen zum selben GCC-Paket passen):

```bat
where gcc
where gcov
gcc --version
gcov --version
```

Notiere Windows-Version/Architektur, GCC-Distribution (z. B. TDM-GCC), beide Versionsausgaben, die verwendeten Pfade und den Commit/Quellstand. Wenn `gcov` nicht vorhanden ist oder die Version nicht zur GCC-Instrumentierung passt: **BLOCKED**; nichts annehmen.

2. **`tack.ini` prüfen**, Linux-ID behalten und `id_windows` mit den **wirklichen** Windows-Versionen eintragen. Für den nachgewiesenen TDM-GCC-64/gcov-Stand `(tdm64-1) 10.3.0` ist dieser Wert im Lieferarchiv bereits vorbereitet:

```ini
[coverage]
id = linux-gcc-14.2.0-gcov-14.2.0-coverage-v1
id_windows = windows-gcc-10.3.0-gcov-10.3.0-coverage-v1
```

Diese ID gilt **nur** für die belegte Windows-TDM-GCC-10.3.0-Referenz und ist nicht automatisch auf andere Rechner übertragbar. Die `cflags`/`ldflags` bleiben im bereits vorhandenen `[target "app".coverage]`, nicht im CMD-Skript. Verschiedene GCC-/gcov-Versionen, Compiler-Builds, Quellmengendefinitionen oder Instrumentierungsflags erfordern eine **eigene** dokumentierte Messprofilentscheidung.

3. **Aktualisierte tack-Executable mit dem bisherigen funktionierenden tack bauen**, anschließend konsequent die neue Executable verwenden:

```bat
tack --config .\tack.ini build release --target app --rebuild -j 1
set TACK_CC=gcc
.\build\app\release\bin\tack.exe --config .\tack.ini ratchet tests
.\build\app\release\bin\tack.exe --config .\tack.ini test release --ci --rebuild -j 32
.\build\app\release\bin\tack.exe --config .\tack.ini test coverage --ci --rebuild -j 1
```

Beim **ersten** Windows-Coverage-Lauf ohne HWM kann der letzte Befehl mit `BLOCKED no accepted HWM` und einer fehlgeschlagenen CI-Zusammenfassung enden, **obwohl** eine gültige Messung nach allen Tests in `coverage/current.txt` geschrieben wurde. Nur dieser spezifische Fall ist vor der Erstanahme erwartbar. Wenn Kompilierung, Tests, gcov, Quelleninventar oder Messdatei scheitern, ist das ein echtes **FAIL/BLOCKED** und muss behoben werden.

4. **Messung fachlich prüfen**:

```bat
type coverage\current.txt
.\build\app\release\bin\tack.exe --config .\tack.ini coverage check
```

Erforderlich sind `profile=windows-...` gleich der aktiven `id_windows`, `os=windows`, `compiler=gcc`, `covered <= total` mit `total > 0`, eine vollständige Produktionsquellmenge sowie das unveränderte Testinventar. Zähler `tests_total`, `tests_passed`, `tests_skipped` müssen zu den **wirklich** ausgeführten Tests passen. Ein `BLOCKED no accepted HWM` beim Check bestätigt nur die fehlende Baseline, nicht die Abnahme. Zur ersten Owner-Freigabe gehört das Prüfen der tatsächlichen Covered/Total-Zahlen und der Ausschlüsse; eine Zielzahl wird nicht vorgegeben.

5. **Erst nach Owner-Prüfung Windows-HWM annehmen und kontrollieren**:

```bat
.\build\app\release\bin\tack.exe --config .\tack.ini coverage accept --owner-approve
.\build\app\release\bin\tack.exe --config .\tack.ini coverage check
.\build\app\release\bin\tack.exe --config .\tack.ini test coverage --ci --rebuild -j 1
```

Die beiden letzten Aufrufe müssen `PASS` melden. Das angenommene, versionierte Artefakt heißt **`coverage/hwm-windows.txt`**; `coverage/current.txt` ist nur der lokale Messnachweis. **Die Linux-HWM wird nicht ersetzt und niemals abgesenkt.** Wenn das Messprofil geändert werden muss, nicht einfach eine inkompatible alte HWM überschreiben; zuerst dokumentierte, separat genehmigte Profil-/Baseline-Migration.

6. **Git-Freigabe vorbereiten:** `coverage/hwm-windows.txt` **und** `coverage/hwm-linux.txt`, `coverage/test-inventory.txt`, Quellcode, `tack.ini` und DE/EN-Dokumentation gehören zusammen in den Release-Commit. `coverage/current.txt`, `build/`, `.tack-cache/`, `*.gcda`, `*.gcno` und `*.gcov` gehören nicht hinein. Der aktualisierte C89-Quellstand muss außerdem die Linux-HWM auf der Linux-Referenzumgebung erneut bestehen; Änderungen am Produktionscode nach einer Abnahme erfordern erneut aktuelle Messungen. Erst nach beiden nachgewiesenen Gates Commit, Tag und Push.

### 6. Windows-Gates

Die Windows-RC0.2-Abnahme liegt vom Owner vor: GCC Release und TinyCC Debug mit jeweils 25 kompilierten Tests, 24 PASS, einem transparenten Plattform-SKIP und 0 FAIL. Danach wurde die instrumentierte Windows-Messung zunächst wegen falsch abgelegter `.gcno` bei `0/25` blockiert. Der Windows-RC0.4-Retest zeigte anschließend **zwei reale Testfehler** durch Unix-only-Pfadvergleiche in `coverage_ratchet_test` und brach erneut vor der Messung ab. RC0.5 korrigiert diese Pfadvergleiche; ein vollständiger Windows-Retest und die Owner-Abnahme der Windows-HWM sind ausstehend. Die kanonische Linux-HWM wird **nicht** auf Windows übertragen; bis zur echten Windows-Messung bleibt das Gate **BLOCKED**.

## English

### 1. Test ratchet (separate from coverage)

The versioned `coverage/test-inventory.txt` stores names and fingerprints for `_test.c` files. Since RC0.2, path separators are canonicalized to `/` and CRLF/LF source newlines are hashed as LF. Other content changes still block acceptance. Check using `tack --config .\tack.ini ratchet tests`. Updates require owner review of the diff and assertions, as fingerprints **do not prove semantic equivalence**. After approval use `tack --config .\tack.ini ratchet tests --owner-approve`.

### 2. Measurement profile (INI-first)

The `[target "app".coverage]` profile above runs GCC with `--coverage` and strict C89 flags; the `[coverage] id` must match actual gcc/gcov toolchain versions, platform and denominator definition. It is **not** auto-probed. For Windows, configure a verified `id_windows = windows-...` alongside the existing Linux `id`. On Windows this override wins; Linux keeps `id`. Do not overwrite the Linux reference to perform Windows measurement. Never put build flags in shell scripts. On Linux when TinyCC is absent, select an available compiler with `TACK_CC=gcc` for the test invocation; this selects a compiler, not build flags. The generic functional smoke test now fails rather than silently passing with a missing compiler.

### 3. Measure, check, accept

`tack --config .\tack.ini test coverage --ci --rebuild -j 1` runs instrumented tests and writes real production-code line coverage to `coverage/current.txt`; test headers, test sources and build files are excluded. For the owner-accepted Linux GCC/gcov reference profile, a versioned `coverage/hwm-linux.txt` now exists. A compatible fresh reference run can PASS. Other platforms or profiles with no accepted HWM remain `BLOCKED`, even when tests pass. `tack --config .\tack.ini coverage check` validates a current measurement against an approved HWM. The owner explicitly accepts a reviewed higher baseline with `tack --config .\tack.ini coverage accept --owner-approve`, then rechecks. Commit a raised HWM in the **same accepted revision** as its validated increase. Never weaken test assertions, reset HWM or compare incompatible profiles to produce PASS.

### 4. Accepted Linux reference

The owner explicitly approved the Linux reference HWM. The Windows-profile-ready production source was remeasured at **3,228/5,708 (56.55221%)** and versioned in `coverage/hwm-linux.txt`. The RC0.2 number is historical evidence only. Source/test/flag fingerprints are checked for staleness; lowering the accepted HWM is forbidden.

### 5. Accepting the first Windows HWM (CMD, before pushing)

**Windows gcov artifact fix:** Earlier 0.8.4 candidates compiled and linked instrumented test sources in a single GCC invocation. TDM-GCC 10.3.0 could leave `.gcno`/`.gcda` in the repository root while the collector scanned `build/tests/coverage/bin/`, yielding `0 of 25` notes. The corrected runner first compiles each test to `build/tests/coverage/obj/<test>.o` and then links its executable under `build/tests/coverage/bin/`. Notes and runtime data live beside those objects; intermediate gcov reports stay under `build/coverage-gcov/`. Missing notes fail early, and each new instrumented run invalidates `coverage/current.txt` before compiling to prevent false acceptance after interrupted measurements. Accepted HWM files remain unchanged.

**RC0.5 follow-up for `coverage_ratchet_test: 2 failures`:** After fixing root artifact placement, RC0.4 still compared two artifact paths against Unix-only expectations (`build/tests/coverage/obj/sample_test.o` and `.gcno`). Windows `path_join()` inserts a native `\\` separator, so both assertions failed **before** gcov collection. RC0.5 requires platform-native paths and their canonical `/` forms; all four assertions remain active. Missing `coverage/current.txt` after this test failure does **not** by itself implicate the gcov collector. Rebuild RC0.5, require `coverage_ratchet_test: ok` in `test release`, then run `test coverage`. Windows HWM remains **BLOCKED** until a genuine complete measurement.

**One-time cleanup of old root artifacts:** Inspect `dir /b *.gcda *.gcno *.gcov` while in `C:\Projects\tack`; after confirming these are obsolete generated files, remove **only root-level artifacts** with `del /q *.gcda *.gcno *.gcov`. Do not delete recursively. `tack clean` cannot reach files misplaced into the root by earlier versions. The corrected build should not create new root-level gcov files.

1. Verify `where gcc`, `where gcov`, `gcc --version` and `gcov --version`. Record the actual compiler/distribution, Windows version/architecture and checked-out source revision. Block acceptance on missing or mismatched tools.
2. Keep the Linux `id` in `[coverage]`. For the verified TDM-GCC-64 gcc/gcov `(tdm64-1) 10.3.0` installation, the shipped `id_windows = windows-gcc-10.3.0-gcov-10.3.0-coverage-v1` is already set. Verify or change it for a different toolchain. Never copy the Linux HWM or use a made-up zero baseline. Keep GCC coverage flags in `[target "app".coverage]`.
3. Build the updated `tack.exe` with `tack --config .\tack.ini build release --target app --rebuild -j 1`. Then run the new executable, with `set TACK_CC=gcc`, in this order:

```bat
.\build\app\release\bin\tack.exe --config .\tack.ini ratchet tests
.\build\app\release\bin\tack.exe --config .\tack.ini test release --ci --rebuild -j 32
.\build\app\release\bin\tack.exe --config .\tack.ini test coverage --ci --rebuild -j 1
type coverage\current.txt
.\build\app\release\bin\tack.exe --config .\tack.ini coverage check
```

4. Before the first accepted Windows HWM, the instrumented test command may end with `BLOCKED no accepted HWM` after writing a valid measurement. Treat that as a **pending gate**, not PASS. Any failed test, missing/invalid `coverage/current.txt`, gcov error or invalid profile is a real failure. Check `profile`, `os`, `compiler`, `covered/total`, all production-source scope and the test counts against the actual evidence.
5. Only after explicit owner review accept the measured Windows baseline and rerun the gate:

```bat
.\build\app\release\bin\tack.exe --config .\tack.ini coverage accept --owner-approve
.\build\app\release\bin\tack.exe --config .\tack.ini coverage check
.\build\app\release\bin\tack.exe --config .\tack.ini test coverage --ci --rebuild -j 1
```

6. Commit `coverage/hwm-windows.txt` in the same accepted revision as the matching source, test inventory and configuration, **without** committing `coverage/current.txt` or build/gcov files. Preserve `coverage/hwm-linux.txt` and revalidate it after any source change. Only tag/push once both platform gates have evidence. Changing measurement profiles requires separate documented migration, not a reset of the HWM.

### 6. Windows verification

The owner re-ran Windows RC0.2: GCC release and TinyCC debug each compiled 25 tests, with 24 PASS, one explicit platform SKIP and zero failures. A later instrumented Windows run was **BLOCKED** at `0/25` misplaced `.gcno` notes. The RC0.4 Windows rerun then failed with **two real test assertions** from Unix-only expected paths in `coverage_ratchet_test`, before measurement. RC0.5 corrects these assertions; a fresh Windows validation and explicit owner HWM acceptance are still pending. Linux coverage must never be copied into the Windows HWM; the Windows gate remains **BLOCKED**.
