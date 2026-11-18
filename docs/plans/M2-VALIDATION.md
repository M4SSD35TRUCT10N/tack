# M2 VALIDATION – tack v0.8.6 (RC0.1, pending gates)

**Baseline:** `39daf5047038dd88cfd7ca5521ceb97d951a929c`, feature branch `feature/v0.8.6-m2-process-execution`.
**Gate statuses:** PASS, FAIL, SKIP, NOT RUN, BLOCKED only. No historical evidence is reinterpreted as current.

## Deutsch

### Stand der Änderungen

- M2-Quellcodekandidat für Eingabevalidierung, POSIX-Exec-Fehlerkanal, Wait/EINTR/Signal, Compiler-Sibling-Drain, Job-Grenze und Formatter-CWD.
- Neue C89-Testquelle `tests/process_execution_test.c` mit echten Child-Argumentproben, tatsächlichen Exitstatusfällen und einem langsamen Compiler-Geschwisterprozess.
- Bestehende 27 Testquellen, akzeptierte Ratchets und HWM-Dateien unverändert. Neuer Test bislang **nicht in Test-Ratchet angenommen**. Erster offizieller `tack test`-Lauf soll daher bis zur Freigabe BLOCKED bleiben.
- Offene Requirement-Tests: P16, P19, P23-P26, einige echte Windows-Child- und kontrollierte Jobpool-Varianten. Siehe Matrix.

### Referenzstände, unverändert

| Plattform | Alte HWM | Neue Messung | Delta ausgeführte Zeilen | Nennerdelta | Prozentpunkte |
| --- | ---: | --- | --- | --- | --- |
| Windows TDM-GCC/gcov 10.3.0 | 3724/6000 | NOT RUN | unbekannt | unbekannt | unbekannt |
| Linux GCC/gcov 14.2.0 | 3604/5790 | NOT RUN | unbekannt | unbekannt | unbekannt |

Quellenbasierte M0-/M1-Werte sind keine M2-Messung. Für M8 90 % und 100 % kritische Anforderungen bleiben Zielwerte, keine automatische M2-Abnahme.

### Gates

| Prüfung | Status | Nachweis |
| --- | --- | --- |
| Repository-Ref, Agentenvertrag, v0.5-Entwurf gelesen | PASS | GitHub/Drive read-only, 18.11.2026 |
| Vollständiger tatsächlicher Linux-tack-Testlauf | NOT RUN | Kein lokaler Repository-Checkout (Netzwerkzugriff gesperrt) |
| Windows TinyCC debug | NOT RUN | Owner-CMD erforderlich |
| Windows TDM-GCC release | NOT RUN | Owner-CMD erforderlich |
| Windows real formatter/argv | NOT RUN | Owner-CMD erforderlich |
| Test-Ratchet in dieser Revision | BLOCKED | Neuer `_test.c` ohne Owner-Freigabe |
| Windows/ Linux instrumentierte M2-Coverage | NOT RUN | Danach Owner-HWM-Review |
| M2 gesamthaft | BLOCKED | Plattform- und Requirement-Gates ausstehend |

### Windows-CMD-Abnahme (erst nach Test-Ratchet-Review)

```bat
cd /d C:\Projects\tack
git fetch origin
git switch --track origin/feature/v0.8.6-m2-process-execution
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

RC0.1 is a source/test candidate, not an accepted milestone. Neither reference-platform full suite nor instrumented M2 coverage has been run in this environment. Windows tests require real owner logs; the added test changes the locked inventory, so the standard runner intentionally blocks before an explicit owner ratchet acceptance. Never misreport the BLOCKED result as PASS, or modify the reference HWMs without review.
