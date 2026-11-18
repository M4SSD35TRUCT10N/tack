# M2 VALIDATION – tack v0.8.6 (RC0.4, pending gates)

**Baseline:** `39daf5047038dd88cfd7ca5521ceb97d951a929c`, feature branch `feature/v0.8.6-m2-process-execution`.
**Gate statuses:** PASS, FAIL, SKIP, NOT RUN, BLOCKED only. No historical evidence is reinterpreted as current.

## Deutsch

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
| Windows TDM-GCC/gcov 10.3.0 | 3724/6000 | NOT RUN | unbekannt | unbekannt | unbekannt |
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
| Windows TinyCC debug | FAIL RC0.3 / NOT RUN RC0.4 | Allgemeiner `_spawnvp`-Argv-Test, Formatter-Äquivalent PASS |
| Windows TDM-GCC release | FAIL RC0.3 / NOT RUN RC0.4 | Gleicher Argv-Fehler |
| Windows real formatter/argv | PASS RC0.3 / NOT RUN RC0.4 | Reale `CreateProcessA`-Child-Argumente stimmten überein |
| Test-Ratchet in dieser Revision | BLOCKED RC0.4 | RC0.3 lokal Owner-angenommen, RC0.4-Fingerprint neu prüfpflichtig |
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
