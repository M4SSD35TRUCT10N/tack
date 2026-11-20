# M2 RC0.8 – bounded `test -j N`, gcov-Identitäten und Regressionstests

**Datum:** 19. November 2026
**Produktversion:** tack v0.8.6, ausschließlich Milestone M2
**Status:** Windows RC0.8 vier Testprofil-Läufe bestanden; GCC/gcov-Messung vorhanden, **HWM-Freigabe BLOCKED**; Linux GCC/Clang/gcov NOT RUN (Owner 20.11.2026)
**Ausgangsbasis:** Feature-Branch `feature/v0.8.6-m2-process-execution`, seit dem ungetesteten RC0.7b auf `308f0b9305e0ff1a7f62c774c3822f30464bdd0f`
**Release:** PR #13 bleibt Draft, kein Merge/Tag/Release ohne Owner.

## Problem

1. `tack test -j N` hat bislang `jobs` im Argumentparser akzeptiert, aber im Runner nicht verwendet. Jeder der 28 Test-Compiler-/Linkprozesse wurde synchron abgewartet.
2. Windows-GCC/gcov auf RC0.7 führte alle Testassertions erfolgreich aus, brach aber bei `production source not instrumented: src\tack.c` ab, da der Scan native `\` nutzt und `CovSet` die normalisierte `src/tack.c`-Identität. RC0.7b enthält bereits eine ungetestete Quell-ID-Normalisierung.
3. Der neue Coverage-Regressionstest aus RC0.7b enthielt irrtümlich im C-Stringliterale einen verdoppelten Backslash, also zur Laufzeit zwei Separatoren; die erwartete positive Identitätsprüfung wäre fehlgeschlagen.
4. Die getrennten Windows-/Linux-GCC/gcov-HWMs bleiben **3845/6075** und **3604/5790**; keine neue Messung auf dem aktuellen RC0.8-Stand.

## Enthaltener RC0.8-Quellkandidat

- `src/tack.c`: `test` übergibt `jobs` an `build_and_run_tests`. Für `N>1` führt ein bounded, shell-freier Prozess-Pool alle notwendigen Test-Compile-/Link-Aufträge vor den Test-Runtimes aus; die Runtime-Tests bleiben strikt sequenziell. Für `-j 1` läuft der bisherige Pfad weiter.
- Jeder Prozess hat ein eigenes exe-Ziel; für GCC-Coverage erst `.o` und `.gcno` pro Test in `build/tests/coverage/obj`, anschließend eigene Linkprozesse in `build/tests/coverage/bin`. Die `.gcda`-Laufzeitdaten werden erst später beim seriellen Start der Testprogramme erzeugt; danach erst `gcov`. Abgleich der gescannten Quellnamen erfolgt über kanonische Identitäten und bewahrt das fail-closed Verhalten für fehlende Instrumentierung.
- Bei Fehler: keine weiteren neuen Starts nach Entdeckung, alle offenen Proc-Handles/Childs abwarten (drain), fehlgeschlagenen Test markieren, übrige als not_run führen, keinen HWM-Zustand schreiben. Kollisionen gleicher Test-Basenamen frühzeitig abweisen.
- `tests/coverage_ratchet_test.c`: echte Windows-Einfachseparator-Positivprobe sowie explizite Negativprobe für doppelte Separatoren und fehlende Quellen.
- `tests/functional_smoke_test.c`: direkter eingebetteter C89-Aufruf erhält den expliziten `jobs=1`-Parameter.
- **Neu:** `tests/parallel_test_build_test.c` erstellt vier C89-Minimaltests unter `build/`, vergleicht seriell und `-j 4`, prüft mindestens zwei gleichzeitig gestartete, nicht abgewartete Compilerprozesse, inkrementelle Wiederholung, erwarteten Compiler-Fail/Drain und bei vorhandenem GCC getrennte Coverage-Objekt-/Link-Artefakte.
- Kein neues Buildsystem/Runtime-Library/Script, keine Warnungsunterdrückung, keine Änderung an akzeptierten HWM-/Inventardateien.

## Sicherheits-/Portabilitätsgrenzen

- `N` bleibt über `TACK_MAX_COMPILER_JOBS=256` begrenzt.
- Kein `system()`-/Shell-Spawn: `proc_spawn_nowait` und `proc_wait` bleiben die einzigen Prozessprimitive.
- Exe-/Objekt-/Notes-Pfade nutzen die vorhandene geprüfte `path_join`-Logik und liegen unter `build/`; Duplicate Basenames werden abgelehnt.
- Test-Runtimes bleiben bewusst seriell, solange Tests gemeinsame Verzeichnisse oder Dateien nutzen.
- Dokumentierte Path-Identity-Strategie ist noch kein `ba_core`-Abhängigkeitsimport. ADR 0006 bleibt PROPOSED.

## Owner-Abnahme / NOT RUN

Auf Windows im CMD nach Quell-Diff/Owner-Prüfung:

```bat
cd /d C:\Projects\tack
git fetch origin
git pull --ff-only origin feature/v0.8.6-m2-process-execution
git status --short --branch
git diff --check 308f0b9..HEAD
git diff 308f0b9..HEAD -- src/tack.c tests/coverage_ratchet_test.c tests/functional_smoke_test.c tests/parallel_test_build_test.c
if not exist build\host mkdir build\host
gcc -std=c89 -pedantic -Werror -o build\host\tack.exe src\tack.c
build\host\tack.exe --config .\tack.ini ratchet tests
```

Der Ratchet-Aufruf muss wegen der beiden modifizierten Testquellen und der neuen `parallel_test_build_test.c` zunächst **FAIL** liefern. Nur nach eigener Review-Freigabe:

```bat
build\host\tack.exe --config .\tack.ini ratchet tests --owner-approve

set "TACK_CC=gcc"
build\host\tack.exe --config .\tack.ini test coverage --ci --rebuild -j 4
type coverage\current.txt
build\host\tack.exe --config .\tack.ini coverage check

set "TACK_CC=tcc"
build\host\tack.exe --config .\tack.ini test debug --ci --rebuild -j 4

set "TACK_CC=gcc"
build\host\tack.exe --config .\tack.ini test release --ci --rebuild -j 4

set "TACK_CC=clang"
build\host\tack.exe --config .\tack.ini test release --ci --rebuild -j 4

git diff -- coverage/test-inventory.txt coverage/hwm-windows.txt
git status --short --branch
```

**Erwartung, nicht Messnachweis:** 29 Testquellen, Windows üblicherweise 28 PASS/0 FAIL/1 SKIP; GCC/gcov muss eine gültige `coverage/current.txt` mit `source_state != 811c9dc5` schreiben und eine vergleichbare Quote messen. Ratchet-Freigabe bei erhöhtem realen HWM weiterhin ausschließlich durch Owner, niemals automatisch. Bei echter Quotenregression bleibt das Gate FAIL.

Danach auf **echtem Linux**: GCC Release, Clang Release, GCC/gcov Coverage und HWM-Check, jeweils mit `-j 1` und `-j 4` für repräsentative Äquivalenz- und Parallelitätsnachweise. Exakte Versionen von GCC/Clang/gcov, jeweilige `TACK_SUMMARY`, Runner-Limits, Profil-HWM und etwaige plattformspezifische SKIPs protokollieren.

**Keine Sicherheits-/Test- oder HWM-Abnahme durch ChatGPT:** Ohne tatsächliche Owner-Laufprotokolle bleiben alle RC0.8-Gates `NOT RUN`. Keine automatische Zusammenführung von Draft PR #13 und kein Release.

## English

RC0.8 is a **single, larger M2 candidate**, not a succession of small release claims. It fixes the untested RC0.7b Windows gcov source-ID test literal and provides native bounded parallel compile/link for `tack test -j N` while retaining serialized runtime tests and fail-closed process draining. A new strict C89 test exercises `-j1`/`-j4`, incremental builds, expected compile failure and isolated GCC coverage object/link artifacts. All source/test inventory ratchets and Windows/Linux GCC/gcov HWMs retain Owner control. Mandatory Windows TinyCC/GCC/Clang and actual Linux GCC/Clang/gcov release gates are **NOT RUN**; 90% production line coverage remains an unmet v0.8.6 goal.

## Owner-Nachlauf RC0.8, 20.11.2026: Windows test PASS / HWM BLOCKED

Owner führte `-j4` auf Source `712ba87` in allen vier Windows-Profilen aus: GCC Release 29 compiled/28 PASS/0 FAIL/1 SKIP (68.438 s); GCC Coverage-Testassertions 29/28/0/1 (88.562 s), aber Gesamtlauf status=failed **ausschließlich wegen der HWM-Annahmesperre**; TinyCC Debug 29/27/0/2 (39.438 s); Clang/MSVC Release 29/28/0/1 (61.625 s). `parallel_test_build_test: ok` viermal, einschließlich erzwungenem Compilerfehler und fehlendem Output. GCC/gcov hat den Coverage-Sammler und den Windows-/POSIX-Quell-ID-Vergleich tatsächlich wieder vollständig durchlaufen. Gemessen: `covered=4036`, `total=6233`, `source_count=1`, `source_state=2e423671` statt FNV-Initialwert, `tests_state=d3433506`. Windows-HWM 3845/6075 unverändert; `coverage check` BLOCKED bis separate Owner-Annahme. Owner hat das 29er-**Testinventar** dagegen bereits ausdrücklich angenommen, exakter Blob `3a800818492bc8ce07762a79b13f36c20073a31c`, und dieser wird versioniert. Linux-HWM 3604/5790 unverändert; Linux-/Clang-/gcov-Gesamtläufe NOT RUN. Auch voller `-j1`-Laufzeitvergleich ausstehend. Keine HWM-Änderung, kein Merge/Release. Unversionierte Datei `tatus --short --branch` im Log nur manuell nach Sichtprüfung entfernen.
