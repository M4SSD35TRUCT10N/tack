# M2 VALIDATION – tack v0.8.6 (RC0.4, pending gates)

**Baseline:** `39daf5047038dd88cfd7ca5521ceb97d951a929c`, feature branch `feature/v0.8.6-m2-process-execution`.
**Gate statuses:** PASS, FAIL, SKIP, NOT RUN, BLOCKED only. No historical evidence is reinterpreted as current.

## Deutsch

### Windows RC0.4: Owner-Gates bestanden, HWM-Annahme offen (18.11.2026)

**Geprüfter Implementierungscommit:** `382471c950d126697391977fe9962bd0eb2d57e9` auf `feature/v0.8.6-m2-process-execution`. Die lokale `coverage/test-inventory.txt` wurde vom Owner für genau diese Testquellenfassung mit `tack --config .\tack.ini ratchet tests --owner-approve` angenommen; sie bleibt lokal modifiziert und ist noch **nicht** im GitHub-Branch versioniert. Im anschließenden Runner meldete die Test-Ratchet `PASS versioned source inventory`.

| Gate | Tatsächlicher Status | Nachweis |
| --- | --- | --- |
| Windows TinyCC debug | **PASS** | `TACK_SUMMARY status=ok total=28 compiled=28 passed=27 failed=0 skipped=1 not_run=0` |
| Windows TDM-GCC release | **PASS** | `TACK_SUMMARY status=ok total=28 compiled=28 passed=27 failed=0 skipped=1 not_run=0` |
| Windows TDM-GCC coverage, Testdurchlauf | **PASS** | `27 PASS / 0 FAIL / 1 SKIP`; `process_execution_test: ok` |
| Windows Coverage-HWM-Gate | **BLOCKED** | Neues Verhältnis übertrifft alte HWM, Annahme im selben angenommenen Revisionsstand erforderlich |
| Vollständiger Linux-Referenzlauf | **NOT RUN** | Nicht Bestandteil des Windows-Owner-Logs |

Die erwarteten diagnostischen `FAIL`-/`BLOCKED`-Meldungen im isolierten `coverage_ratchet_test` sind absichtliche Negativproben. Der Test selbst meldete `coverage_ratchet_test: ok`. Der bisherige fehlerhafte allgemeine Win32-Argumentpfad ist durch den echten Kindprozessvergleich behoben; die zusätzlichen Win32-Create/Wait/Exit/Close-Injektionstests liefen erfolgreich.

**Messung:** `coverage/current.txt`: Profil `windows-gcc-10.3.0-gcov-10.3.0-coverage-v1`, `covered=3844`, `total=6075`, **63.27572 %**, `source_count=1`, `sources_hash=39ea236e`, `source_state=811c9dc5`, `tests_state=ee1bba53`, `flags_state=36eb9e95`, `tests_total=28`, `tests_passed=27`, `tests_skipped=1`. Altes HWM `3724/6000 = 62.06667 %`; Differenz **+120 ausgeführte Zeilen, +75 ausführbare Zeilen, +1.20905 Prozentpunkte**. Kein Nenner-Trick: Beide Profile sind anhand des gemeinsamen Quellen-Scopes und der Flag-State-Identität vergleichbar.

**Annahmeprozess:** Der Owner muss den tatsächlich gemessenen Fortschritt ausdrücklich mit `tack --config .\tack.ini coverage accept --owner-approve` annehmen. Danach müssen das lokal akzeptierte `coverage/test-inventory.txt` und das neue `coverage/hwm-windows.txt` als gemeinsam geprüfte Artefakte in derselben akzeptierten Revision versioniert werden. Erst anschließend kann `tack --config .\tack.ini coverage check` erfolgreich sein und der Windows-Coverage-Gate als PASS gewertet werden. Die Linux-HWM `3604/5790` bleibt unverändert. **Kein automatisches HWM-Accept, kein Merge und kein M2-Abschluss.**

Die gcov-Hinweise `Quelldatei ist neuer als Hinweis-Datei` wurden gemeldet, verhinderten aber weder die Testausführung noch die gültige Coverage-Zusammenführung. Vor einer endgültigen Abnahme sind Build-Timestamps bei Bedarf separat zu prüfen.
### Windows-Owner-Evidenz: RC0.3 und RC0.4-Korrektur

RC0.3-Owner-Run am 18.11.2026, SHA `f74c7992e7f2e9a4e02c828a05ba2fa717479d31`: Windows TinyCC Debug, GCC Release und GCC Coverage kompilierten den neuen M2-C89-Test und endeten jeweils mit `TACK_SUMMARY total=28 compiled=24 passed=22 failed=1 skipped=1 not_run=4`. Einzige echte FAIL-Assertion: `child sees exact argv and literal metacharacters` im generischen CRT-`_spawnvp`-Pfad. Die gleichartige echte Formatter-`CreateProcessA`-Probe bestand. Erwartete Fehlerausgaben der negativen API-/Argument-/Compiler-Jobs-Proben sind kein eigenständiger Test-Fail.

**RC0.4, noch NOT RUN:** Gemeinsamer Windows-Commandline-Encoder mit CRT-kompatibler Kodierung für generische `CreateProcessA`-Child-Prozesse und den Formatter. Generischer Prozessstarter jetzt native Win32 Create/Wait/Exit-APIs mit expliziter Handle-Ownership. Im Test bleiben die Kindprozess-Argument-Assertions unverändert; hinzu kommen Diagnose von empfangenem versus erwartetem Argument, der echte generische Start eines Executables mit Leerzeichen und native Create/Wait/GetExitCode/Close-Fault-Tests. ADR 0003 beschreibt Architektur und Grenzen.

**Ratchet:** RC0.3-Testinventar wurde vom Owner lokal mit `--owner-approve` aktualisiert und meldet `tests/process_execution_test.c c2b48bd9`. Die geänderte Datei liegt noch uncommittet im Owner-Working-Tree und wurde nicht in das Repository gepusht. Mit der neuen RC0.4-Testquelle muss der Fingerprint erneut geprüft und vom Owner angenommen werden. Keine stille Aufnahme, keine Abschwächung.

**Coverage:** RC0.3 `coverage/current.txt` nicht erzeugt; `coverage check` NOT RUN. Windows-HWM 3724/6000 und Linux-HWM 3604/5790 unverändert. RC0.4 Windows/Linux und Instrumentierung NOT RUN; M2 nicht abgenommen.

### Stand der Änderungen

### Windows-Owner-Run: RC0.2, 18.11.2026 (vor RC0.3-Korrektur)

**Geprüfter SHA:** `2cdb3897ff7596de87b03766c96c725b162831f3`. Windows Git-Checkout, TinyCC-/GCC-Doctor und das lokale Update der Test-Ratchet sind belegt.

| Kommando/Profil | Ergebnis | TACK_SUMMARY |
| --- | --- | --- |
| `ratchet tests` vor Annahme | FAIL | neues `process_execution_test.c` noch nicht inventarisiert |
| `ratchet tests --owner-approve` | PASS (lokal) | `coverage/test-inventory.txt` bleibt lokal modifiziert |
| `test debug --ci --rebuild -j 1`, TinyCC | **FAIL** | `total=28 compiled=24 passed=22 failed=1 skipped=1 not_run=4` |
| `test release --ci --rebuild -j 1`, TDM-GCC | **FAIL** | `total=28 compiled=24 passed=22 failed=1 skipped=1 not_run=4` |
| `test coverage --ci --rebuild -j 1`, GCC | **FAIL** | `total=28 compiled=24 passed=22 failed=1 skipped=1 not_run=4` |
| `coverage check` | NOT RUN | `coverage/current.txt` existiert noch nicht |

**Ursache:** Nur der neue M2-Test kompiliert nicht. TinyCC meldet verwerfene `const`-Qualifier, GCC zusätzlich unter `-Wpedantic -Werror` nicht konstante C89-Aggregat-Initialisierer mit dem zur Laufzeit bekannten `self`-Pfad. Alle davor ausgeführten vorhandenen Tests sind erfolgreich. Die einzelnen `coverage_ratchet_test`-Ausgaben `FAIL`/`BLOCKED` sind beabsichtigte Negativtests und führen laut Suite zu `coverage_ratchet_test: ok`.

**RC0.3-Korrektur:** String-Literale bleiben `const char *`; die `argv[0]`-Elemente werden nach der statischen C89-Initialisierung zugewiesen. Ein abgegrenzter Pointer-Vektor-Adapter übergibt Argumente ohne Änderungen der Strings an die bestehenden `char **`-Prozess-APIs, wobei `errno` beim Freigeben erhalten bleibt. Assertions und Fehlerproben werden nicht abgeschwächt. **RC0.3 Windows/ Linux: NOT RUN.** Weil die Testquelle geändert wurde, ist die neue Inventar-Fingerprint-Freigabe erneut vom Owner zu prüfen. Die lokale RC0.2-Inventardatei ist weder in GitHub übernommen noch verloren zu geben.


- M2-Quellcodekandidat für Eingabevalidierung, POSIX-Exec-Fehlerkanal, Wait/EINTR/Signal, Compiler-Sibling-Drain, Job-Grenze und Formatter-CWD.
- Neue C89-Testquelle `tests/process_execution_test.c` mit echten Child-Argumentproben, tatsächlichen Exitstatusfällen und einem langsamen Compiler-Geschwisterprozess.
- Bestehende 27 Testquellen und akzeptierte HWM-Dateien im Repository unverändert. Das neue Inventar wurde für RC0.2 lokal vom Owner akzeptiert, aber noch nicht gepusht. RC0.3 benötigt wegen geänderter Testquelle erneut eine explizite Owner-Ratchet-Prüfung.
- Testcode wurde für P16, P19, P23-P26 ergänzt (Windows API-Fault-Injektionen, tatsächlicher Child mit Leerzeichen im Executable-Pfad, 16000-Byte-Argument, umgekehrte -j-Reihenfolge). Ihre Gates bleiben NOT RUN. Ausstehend sind u.a. zusätzliche Spawn-Fehler während aktiver Geschwister und tatsächliche Plattformabnahme.

### GitHub-/Manifeststatus

Draft-PR **#13** zielt auf `dev/v0.8.6`; kein Merge oder Tag. Die bestehende `SHA256SUMS.txt` stammt vom integrierten M1-Stand und ist **kein aktuelles M2-RC-Manifest**, da `src/tack.c`, Test und Dokumentation geändert wurden. Ein vollständiges neues SHA-256-Release-Manifest ist vor M8-Release-Freigabe zu erzeugen und zu verifizieren. Kein Quell-ZIP im aktuellen GitHub-only-Workflow.

### Referenzstände, unverändert

| Plattform | Alte HWM | Neue Messung | Delta ausgeführte Zeilen | Nennerdelta | Prozentpunkte |
| --- | ---: | --- | --- | --- | --- |
| Windows TDM-GCC/gcov 10.3.0 | 3724/6000 | **3844/6075, HWM BLOCKED** | +120 | +75 | +1.20905 |
| Linux GCC/gcov 14.2.0 | 3604/5790 | NOT RUN | unbekannt | unbekannt | unbekannt |

Quellenbasierte M0-/M1-Werte sind keine M2-Messung. Für M8 90 % und 100 % kritische Anforderungen bleiben Zielwerte, keine automatische M2-Abnahme.


### Isolierter POSIX-Algorithmuscheck (keine tack-Gesamtabnahme)

Die identische M2-Fork/Exec/Wait-Algorithmusfassung wurde als eigenständige C89-Prüfdatei unter Linux ausgeführt (nicht als `tack test`):

- Debian GCC 14.2.0, `-std=c89 -pedantic-errors -Wall -Wextra -Werror`: **PASS**.
- Clang 17.0.0, dieselben strengen C89-Flags: **PASS**.
- Jede Ausführung: `ENOENT` aus fehlgeschlagenem `execvp` unterscheidbar von echtem Child-Exit 127, SIGTERM als 128+SIGTERM: **PASS**.

Dieser Check belegt nur den isolierten Algorithmus und ist nicht gleichzusetzen mit kompiliertem `src/tack.c`, dem neuen Testinventar oder dem vollständigen Linux-Referenz-Coverageprofil. Alle integrierten M2-Gates bleiben **NOT RUN**.

### Gates

| Prüfung | Status | Nachweis |
| --- | --- | --- |
| Repository-Ref, Agentenvertrag, v0.5-Entwurf gelesen | PASS | GitHub/Drive read-only, 18.11.2026 |
| Vollständiger tatsächlicher Linux-tack-Testlauf | NOT RUN | Kein lokaler Repository-Checkout (Netzwerkzugriff gesperrt) |
| Windows TinyCC debug | **PASS RC0.4** | 27 PASS, 0 FAIL, 1 SKIP; real Win32 arg/handle probes PASS |
| Windows TDM-GCC release | **PASS RC0.4** | 27 PASS, 0 FAIL, 1 SKIP |
| Windows real formatter/argv | **PASS RC0.4** | Allgemeiner Win32-Prozessstart und Formatter-Child-Argv vollständig PASS |
| Test-Ratchet in dieser Revision | **PASS lokal / Versionierung offen** | Owner hat RC0.4-Fingerprint angenommen; Datei lokal uncommittet |
| Windows / Linux instrumentierte M2-Coverage | NOT RUN | Windows RC0.2 Compiler-FAIL, keine gültige aktuelle Messung; Linux noch offen |
| M2 gesamthaft | BLOCKED | Plattform- und Requirement-Gates ausstehend |

### Windows-CMD-Abnahme (erst nach Test-Ratchet-Review)

```bat
cd /d C:\Projects\tack
git fetch origin
git switch feature/v0.8.6-m2-process-execution
git pull --ff-only origin feature/v0.8.6-m2-process-execution
git status --short --branch
git log -1 --format=full

set "TACK_CC="
tack --config .\tack.ini doctor debug --target app
tack --config .\tack.ini ratchet tests
REM Erwartet BLOCKED bis zur ausdruecklichen Ratchet-Owner-Freigabe.
REM Erst nach Owner-Freigabe: tack --config .\tack.ini ratchet tests --owner-approve
tack --config .\tack.ini test debug --ci --rebuild -j 1

set "TACK_CC=gcc"
tack --config .\tack.ini doctor release --target app
tack --config .\tack.ini test release --ci --rebuild -j 1
tack --config .\tack.ini test coverage --ci --rebuild -j 1
type coverage\current.txt
type coverage\hwm-windows.txt
tack --config .\tack.ini coverage check
git diff --check
git status --short --branch
```

**Keine stillschweigende HWM-Annahme.** Owner-Protokolle an `M2-VALIDATION.md` anhängen; ausdrücklich angenommene höhere HWM muss in derselben angenommenen Revision versioniert sein. Compiler-/gcov-Version, Source-Nenner und Branch-Coverage getrennt ausweisen.

## English

RC0.2 Windows owner logs prove that the newly added process test failed compilation in TinyCC debug, GCC release and GCC coverage, while prior legacy tests passed. The owner locally accepted the RC0.2 test inventory. The RC0.3 change addresses the C89 initializer and const-qualifier failures without weakening assertions. RC0.3 integrated tests remain NOT RUN, and its new fingerprint requires fresh owner review. Never misreport the BLOCKED result as PASS, or modify the reference HWMs without review.

### English: RC0.3 Owner evidence and RC0.4 remediation

Owner Windows RC0.3 passed compilation under TinyCC and GCC, but exactly one real generic `_spawnvp` child-argument assertion failed in each profile (22 PASS, 1 FAIL, 1 SKIP, 4 NOT RUN); the equivalent formatter `CreateProcessA` child probe passed. RC0.4 fixes the Windows general launcher using the shared formatter encoder and native process/wait/handle APIs. It keeps the argv requirement and strengthens native error-path probes. RC0.4 platform tests and coverage remain NOT RUN. The locally accepted RC0.3 test-inventory fingerprint must be reviewed again after RC0.4 test-source changes.

### English: RC0.4 Windows gates (18 November 2026)

On commit `382471c950d126697391977fe9962bd0eb2d57e9`, Owner Windows TinyCC debug, GCC release and GCC coverage test suites each executed 28 compiled tests with **27 passed, zero failed, one deliberately skipped and none unrun**. The previously failing generic Windows child argument probe now passes. GCC/gcov measured **3844/6075 (63.27572%)** against the accepted 3724/6000 (62.06667%) HWM. This is a valid improvement (+120 covered lines, +75 executable lines, +1.20905 percentage points), but the coverage command remains **BLOCKED pending explicit Owner HWM acceptance and versioning**. The RC0.4 test inventory was accepted locally, not committed. Linux reference tests remain NOT RUN; no milestone closure is claimed.
