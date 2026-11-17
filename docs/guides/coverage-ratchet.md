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
```

Die Profil-ID ist **kein** automatischer Versionsnachweis. Der Owner gleicht sie mit dem tatsächlich verwendeten `gcc --version` und `gcov --version`, der Plattform und der Messdefinition ab. Für Windows muss eine eigene `windows-...`-Profil-ID verwendet werden. Compilerflags dürfen ausschließlich in `tack.ini` stehen. Unter Linux ohne installiertes TinyCC kann für **Compilerwahl, nicht für Compilerflags**, `TACK_CC=gcc` beim Testlauf gesetzt werden; sonst schlägt der generische `functional_smoke_test` zu Recht wegen fehlendem Compiler fehl.

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

Der Owner hat die HWM für das Linux-Referenzprofil ausdrücklich freigegeben. Die auf der finalen Produktionsquelle neu erstellte Messung **3.171/5.651 (56,11396 %)** ist in `coverage/hwm-linux.txt` versioniert. Maßgeblich sind die **tatsächlich** gespeicherten `covered`/`total`-Werte; die Messung vom RC0.2 ist nur historische Evidenz. Der Ratchet prüft Quell-/Test-/Flag-Fingerprints und blockiert geänderte oder inkompatible Messungen. Der einmal angenommene HWM-Wert darf nicht abgesenkt werden.

### 5. Windows-Gates

Die Windows-RC0.2-Abnahme liegt vom Owner vor: GCC Release und TinyCC Debug mit jeweils 25 kompilierten Tests, 24 PASS, einem transparenten Plattform-SKIP und 0 FAIL. Die kanonische Linux-HWM wird **nicht** auf Windows übertragen. Windows-Instrumentierung braucht eine eigene überprüfte `windows-...`-ID, einen frischen Messlauf und Owner-Abnahme vor `coverage/hwm-windows.txt`. Bis dahin ist Windows-Coverage **NOT RUN**, nicht PASS.

## English

### 1. Test ratchet (separate from coverage)

The versioned `coverage/test-inventory.txt` stores names and fingerprints for `_test.c` files. Since RC0.2, path separators are canonicalized to `/` and CRLF/LF source newlines are hashed as LF. Other content changes still block acceptance. Check using `tack --config .\tack.ini ratchet tests`. Updates require owner review of the diff and assertions, as fingerprints **do not prove semantic equivalence**. After approval use `tack --config .\tack.ini ratchet tests --owner-approve`.

### 2. Measurement profile (INI-first)

The `[target "app".coverage]` profile above runs GCC with `--coverage` and strict C89 flags; the `[coverage] id` must match actual gcc/gcov toolchain versions, platform and denominator definition. It is **not** auto-probed. For Windows, use a separate `windows-...` ID. Never put build flags in shell scripts. On Linux when TinyCC is absent, select an available compiler with `TACK_CC=gcc` for the test invocation; this selects a compiler, not build flags. The generic functional smoke test now fails rather than silently passing with a missing compiler.

### 3. Measure, check, accept

`tack --config .\tack.ini test coverage --ci --rebuild -j 1` runs instrumented tests and writes real production-code line coverage to `coverage/current.txt`; test headers, test sources and build files are excluded. For the owner-accepted Linux GCC/gcov reference profile, a versioned `coverage/hwm-linux.txt` now exists. A compatible fresh reference run can PASS. Other platforms or profiles with no accepted HWM remain `BLOCKED`, even when tests pass. `tack --config .\tack.ini coverage check` validates a current measurement against an approved HWM. The owner explicitly accepts a reviewed higher baseline with `tack --config .\tack.ini coverage accept --owner-approve`, then rechecks. Commit a raised HWM in the **same accepted revision** as its validated increase. Never weaken test assertions, reset HWM or compare incompatible profiles to produce PASS.

### 4. Accepted Linux reference

The owner explicitly approved the Linux reference HWM. The final production source was remeasured at **3,171/5,651 (56.11396%)** and versioned in `coverage/hwm-linux.txt`. The RC0.2 number is historical evidence only. Source/test/flag fingerprints are checked for staleness; lowering the accepted HWM is forbidden.

### 5. Windows verification

The owner re-ran Windows RC0.2: GCC release and TinyCC debug each compiled 25 tests, with 24 PASS, one explicit platform SKIP and zero failures. The Linux reference HWM cannot be reused for Windows. Windows instrumentation requires its own verified `windows-...` profile ID, fresh measurement and owner acceptance before `coverage/hwm-windows.txt` exists; Windows coverage is **NOT RUN**.
