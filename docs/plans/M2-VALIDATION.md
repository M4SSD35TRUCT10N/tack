# M2 VALIDATION – tack v0.8.6 (Windows PASS; Linux GCC/Clang PASS; Linux HWM locally accepted; rerun pending)

**Baseline:** `39daf5047038dd88cfd7ca5521ceb97d951a929c`, feature branch `feature/v0.8.6-m2-process-execution`.
**Gate statuses:** PASS, FAIL, SKIP, NOT RUN, BLOCKED only. No historical evidence is reinterpreted as current.

## Deutsch

### RC0.8 Debian Trixie/WSL2: Owner-Linux-HWM lokal akzeptiert, check PASS (20.11.2026)

**Neue Owner-Evidenz nach Linux-gcov-Erstmessung:** `./build/host/tack --config ./tack.ini coverage accept --owner-approve` wurde ausdrücklich durch den Owner ausgeführt und meldete `accepted measured high-water mark: coverage/hwm-linux.txt`. Anschließendes `coverage check`: **PASS HWM retained: 3919/6010**. Geprüfter lokaler Diff `coverage/hwm-linux.txt`: `covered=3604 -> 3919`, `total=5790 -> 6010`, `source_state=6cf1bb03 -> 6ac94196`, `tests_state=142b4c94 -> a93396cc`, `tests_total=27 -> 29`, `tests_passed=26 -> 28`; `profile=linux-gcc-14.2.0-gcov-14.2.0-coverage-v1`, `sources_hash=39ea236e`, `source_count=1`, `flags_state=36eb9e95`, `tests_skipped=1` unverändert. Die **lokale** Coverage-Baseline liegt jetzt bei **3919/6010 = 65.20799 %**.

**Keine vorschnelle Release-Bestätigung:** Der zuletzt vor Owner-Accept gezeigte vollständige `test coverage --ci --rebuild -j 4`-Lauf hatte `status=failed` ausschließlich wegen der noch offenen HWM. Ein **vollständiger Nachlauf nach HWM-Akzeptanz** mit `TACK_SUMMARY status=ok` wurde noch nicht geliefert. Der GitHub-Feature-Branch enthält zum Dokumentationszeitpunkt in `coverage/hwm-linux.txt` **noch 3604/5790** (Git-Blob `d14f274cd2ec3fab0189f12bf8362396c02a9174`); der akzeptierte lokale HWM-Diff muss **durch den Owner committet und gepusht werden**, bevor die Linux-HWM auch im Remote-Branch als versioniert gilt. Keine direkte/automatische HWM-Schreiboperation des Agenten.

**Zeitstempel-Untersuchung:** Owner meldet `date -u` 19:06:22 UTC; `src/tack.c` mtime 15:34:11 +0100, `tests/process_execution_test.c` mtime 16:50:27 +0100, `build/tests/coverage/obj/process_execution_test.gcno` mtime 19:04:26 +0100. Nach diesen Dateisystemwerten ist die `.gcno` **jünger als beide Quellen**. Trotzdem meldete `gcov` beim zuvor erfolgten Lauf `source file is newer than notes file`. Die drei `stat`-Werte sind kein Beweis für ein inkonsistentes Coverage-Ergebnis und erklären die gcov-Warnung nicht; genaue Ursache und Auswirkung weiterhin zu klären, keine Unterdrückung. Die übrigen Linux GCC/Clang Release-Suites waren bereits 28/0/1 PASS; Linux ARM64 bleibt separat offen.

**Status:** Linux-GCC-/Clang-Release PASS; Linux-Coverage-HWM **lokal PASS, Remote-Versionierung und voller Nachlauf offen**; `-j1`/`-j4`-Performancevergleich offen; **M2 noch BLOCKED und PR #13 Draft**. GitHub Actions bleibt bewusst deaktiviert.

### RC0.8 Debian Trixie/WSL2 Linux GCC/gcov – 20.11.2026, gemessen; HWM-Freigabe offen

**Owner-Log nach zwei bestandenen Linux-x64-Release-Suites:** `TACK_CC=gcc ./build/host/tack --config ./tack.ini test coverage --ci --rebuild -j 4` kompiliert 29 Tests, 28 PASS, 0 FAIL, 1 SKIP (`path_join_test` Windows-only), 0 NOT RUN. `process_execution_test: ok` und `parallel_test_build_test: ok`. **Tatsächliche produktive Linux-gcov-Zeilenabdeckung:** `3919/6010 = 65.20799 %`, `source_count=1`, `sources_hash=39ea236e`, `source_state=6ac94196`, `tests_state=a93396cc`, `flags_state=36eb9e95`, Profil `linux-gcc-14.2.0-gcov-14.2.0-coverage-v1`. `coverage/current.txt` liegt lokal vor. Testgesamtlauf `duration_ms=43888`. `TACK_SUMMARY status=failed` resultiert **ausschließlich aus dem Coverage-HWM-Gate**: `BLOCKED improved coverage requires HWM increase in same accepted revision`; nachfolgendes `coverage check` identisch BLOCKED. Es liegt **kein** Testassertion-FAIL vor.

**Noch unveränderte, akzeptierte Linux-HWM:** `3604/5790 = 62.24525 %`, Git-Blob `d14f274cd2ec3fab0189f12bf8362396c02a9174`. Neue Messung: **+315 ausgeführte Produktionszeilen**, **+220 ausführbare Produktionszeilen**, **+2.96274 Prozentpunkte**. Die Verbesserung ist **nicht angenommen**; `coverage/hwm-linux.txt` bleibt unverändert, bis der Owner nach Review ausdrücklich `coverage accept --owner-approve` ausführt, den exakten HWM-Diff kontrolliert und versioniert. Keine automatische HWM-Modifikation.

**GitHub-Inventar inzwischen verifiziert:** `coverage/test-inventory.txt` Blob `40245f1940326245746391f2f0467482d5e844f3` enthält genau `tests/process_execution_test.c\ta68ce17a`. In GCC-, Clang-Release- und GCC-Coverage-Lauf `test-ratchet: PASS`; der vorher in diesem Dokument geschilderte nicht gepushte Inventar-Fingerprint ist historischer Zwischenstand und inzwischen überholt. Exakter lokaler Checkout-HEAD zum Coverage-Lauf im Log nicht separat angegeben.

**Messhinweis:** gcov meldet auch unter Linux bei zahlreichen Quellen `source file is newer than notes file` (zugehörige `.gcno` trotz `--rebuild`). Derselbe Diagnosetyp war unter Windows sichtbar. Ohne Untersuchung der Build-/Quelldateizeitstempel kein stilles Wegunterdrücken oder Behaupten einer ungültigen Messung. Diese Warnungen für die finale M2-Abnahme nachvollziehbar prüfen.

**Stand:** Linux GCC Release PASS, Linux Clang Release PASS, Linux GCC/gcov-Testassertions PASS; Linux-Coverage-HWM-Gate BLOCKED bis ausdrücklicher Owner-Annahme, keine Linux-Coverage-Gesamtfreigabe. Windows HWM 4036/6233 historisch PASS. Vollständiger gleichartiger `-j1`/`-j4`-Performancevergleich weiterhin NOT RUN; Linux ARM64 nicht durch WSL2/x64 abgedeckt. M2 noch BLOCKED, PR #13 Draft, GitHub Actions bewusst deaktiviert.

### RC0.8 Debian Trixie/WSL2 Owner-Gates: GCC und Clang Release PASS (20.11.2026)

**Neue tatsächliche Owner-Laufprotokolle nach Linux-Const-Korrektur `1b43b87`:** `TACK_CC=gcc ./build/host/tack --config ./tack.ini test release --ci --rebuild -j 4` meldete `TACK_SUMMARY version=1 mode=test status=ok profile=release total=29 compiled=29 passed=28 failed=0 skipped=1 not_run=0 duration_ms=36980`. Der gesonderte **echte** Clang-Aufruf `TACK_CC=clang ...` meldete denselben PASS-/SKIP-Stand bei `duration_ms=34184`. `process_execution_test: ok` und `parallel_test_build_test: ok` unter beiden Compilern; `path_join_test` aus Plattformgrund auf POSIX SKIP. Erwartete negative Compiler-/Spawn-/Signal-/Wait-Diagnosen sind von den Tests abgefangen.

Beide Logs melden `tack: test-ratchet: PASS versioned source inventory` im **lokalen WSL2-Checkout**. **GitHub-Revision und lokaler Git-Status dieser Läufe wurden nicht mitgeliefert.** Die GitHub-Datei `coverage/test-inventory.txt` enthält bei dieser Dokumentation weiterhin den alten Eintrag `tests/process_execution_test.c\t9ba06b3c` (Blob `3a800818...`), obwohl `tests/process_execution_test.c` durch Commit `1b43b87` geändert wurde. Damit ist die **Owner-Akzeptanz und Versionierung des aktualisierten Inventars auf dem Remote-Branch noch nicht belegt**. Den lokalen Inventar-Diff prüfen und nach Owner-Freigabe committen/pushen; Ratchet niemals automatisch annehmen oder umgehen.

**Weiter offen:** Linux GCC/gcov Coverage-Lauf + Linux-HWM-Check; Linux-Coverage-Quote und Status **NOT RUN**. Vergleich der Gesamtlaufzeiten `-j1` gegenüber `-j4` auf demselben Compiler **NOT RUN**. Windows-Gates sind historische PASS auf älterem Testinventar und müssen gegen die neue Testquellrevision bestätigt werden, falls dies nach Freigaberegel erforderlich ist. GitHub Actions bleibt deaktiviert; M2 **BLOCKED**, PR #13 **Draft**. Es wurden keinerlei neuen Produktionsfunktionen implementiert.

### RC0.8 Linux GCC Build-FAIL auf Debian Trixie/WSL2; isolierter C89-Fix 1b43b87 (20.11.2026)

**Owner-Probe vor dem Fix:** Zweimal `TACK_CC=gcc ... test release --ci --rebuild -j 4` unter Debian Trixie/WSL2. Beide Läufe **FAIL** bei `tests/process_execution_test.c:514` (`char *noexec[]` verliert `const`) und `:520` (unvereinbarer Argumentzeigertyp gegenüber `m2_spawn(const char * const *, Proc *)`). Zusammenfassung jeweils `total=29 compiled=28 passed=0 failed=1 skipped=0 not_run=28 status=failed`. **Clang-Release NOT RUN:** `set TACK_CC=clang` ist keine Bash-Variablenzuweisung; der dritte Lauf zeigt erneut GCC-Diagnostik und ist kein Clang-Nachweis.

**Minimalkorrektur:** GitHub-Commit `1b43b870226e68c2957c86ea9c410849f00a3132` ändert **nur** die Linux-Deklaration in `tests/process_execution_test.c` zu `const char * const noexec[] = {"build/m2-process/noexec", 0};`; alle Assertions unverändert, keine Änderung am Produktivcode. Die konkrete Typkonversion wurde separat mit `cc -std=c89 -pedantic-errors -Wextra -Werror` bestätigt, vollständige Linux- und Windows-Gates **auf dem neuen Testquellstand NOT RUN**. Wegen geänderter Testquelle ist die bisherige 29er-Testinventar-Ratchet jetzt **BLOCKED bis zur ausdrücklichen Owner-Review/-Annahme**. Weder Testinventar noch Windows-/Linux-HWM automatisch geändert. Historische Windows-PASS-Ergebnisse stammen von der vorigen Testquellrevision und bleiben historische Evidenz. Linux GCC/Clang/gcov auf dem Fix-Commit offen, PR #13 Draft und M2 BLOCKED. Befehle und erwartete Semantik in `docs/plans/M2-RC08-MANUAL-VALIDATION.md`.

### M2 RC0.8 – Windows-HWM akzeptiert; manuelle Linux-Validierung noch offen (20.11.2026)

**Aktueller Nachtrag, ältere RC-Einträge bleiben historische Evidenz:** Der Owner hat Windows-GCC/gcov nach ausdrücklicher Annahme der HWM **4036/6233 (64,75213 %)** mit `test coverage --ci --rebuild -j 4` erneut ausgeführt: `TACK_SUMMARY status=ok`, 29 kompiliert, 28 PASS, 0 FAIL, 1 SKIP; anschließendes `coverage check`: **PASS HWM retained 4036/6233**. Windows-Source-/Test-Identitäten `source_state=2e423671`, `tests_state=d3433506`; Windows-HWM Git-Blob `b776991ce11aa5d5c2b0f53930148640b9f4864b`, Owner-Commit `998e753`. Windows GCC Release, TinyCC Debug und Clang/MSVC Release bleiben PASS, Testinventar ebenfalls PASS. **Linux GCC/Clang/gcov RC0.8 NOT RUN; Linux-HWM 3604/5790 unverändert; voller -j1/-j4-Laufzeitvergleich NOT RUN.** GitHub Actions wurde vom Owner bis zur Fertigstellung von praeco bewusst deaktiviert und ist kein M2-Produktgate; Linux-Prüfungen erfolgen manuell, ohne sie als bereits ausgeführt darzustellen. Der Windows-gcov-Zeitstempelhinweis wird beobachtet und getrennt bewertet. Ein lokal unversioniertes `"tatus --short --branch"` nur nach Inhaltsprüfung entfernen. **M2-Gesamtfreigabe BLOCKED; PR #13 Draft, kein Merge/Release.** Ausführungsprotokoll und Befehle: `docs/plans/M2-RC08-MANUAL-VALIDATION.md`.

### RC0.8 Owner-Abnahme Windows, 20.11.2026, Source-Commit `712ba87bbb71230fbc3578cfc76a7d0a776125c7`

**Quellrevision und Bootstrap:** `git rev-parse HEAD=712ba87bbb71230fbc3578cfc76a7d0a776125c7`, `git diff --check 308f0b9..HEAD` PASS, Git-Checkout vor Ratchet sauber. `gcc -std=c89 -pedantic-errors -Wextra -Werror -o build\host\tack.exe src\tack.c` PASS, Host-Executable ausschließlich unter `build/host/`. Owner prüfte Quell-/Test-/Workflow-Diff und führte ausdrücklich `build\host\tack.exe --config .\tack.ini ratchet tests --owner-approve` aus; danach `ratchet tests` **PASS**. Der Git-Blob für genau diese lokal genehmigte 29er-Inventardatei ist **`3a800818492bc8ce07762a79b13f36c20073a31c`**, identisch mit dem Owner-Diff-Präfix `3a80081`: `coverage_ratchet_test.c e79b42df→8ac01234`, `functional_smoke_test.c be6f01b0→a6eb8827`, neuer `parallel_test_build_test.c 11ccc21b`. Andere 26 Einträge unverändert. Dieses Owner-Inventar ist versioniert, **nicht** still automatisch angenommen.

| Windows `-j 4` | Gesamtstatus | compiled | passed | failed | skipped | not_run | Dauer |
| --- | --- | ---: | ---: | ---: | ---: | ---: | --- |
| GCC Release | **PASS** | 29 | 28 | 0 | 1 | 0 | 68438 ms |
| TinyCC Debug | **PASS** | 29 | 27 | 0 | 2 | 0 | 39438 ms |
| Clang/MSVC Release | **PASS** | 29 | 28 | 0 | 1 | 0 | 61625 ms |
| GCC/gcov-Testassertions | **PASS** | 29 | 28 | 0 | 1 | 0 | 88562 ms |
| GCC/gcov Gesamt-/HWM-Gate | **BLOCKED** (verbesserte HWM noch nicht Owner-angenommen) | 29 | 28 | 0 | 1 | 0 | 88562 ms |

**Gesicherte Coverage-Messung:** `coverage/current.txt` wurde erfolgreich erzeugt: `profile=windows-gcc-10.3.0-gcov-10.3.0-coverage-v1`, `covered=4036`, `total=6233`, **64.75213 %**, `source_count=1`, `sources_hash=39ea236e`, `source_state=2e423671` (**nicht** der alte leere FNV-Wert `811c9dc5`), `tests_state=d3433506`, `flags_state=36eb9e95`, `tests_total=29`, `tests_passed=28`, `tests_skipped=1`. Älteste akzeptierte Windows-HWM weiterhin **3845/6075 = 63.29218 %**: **+191 ausgeführte und +158 ausführbare Zeilen, +1.45995 Prozentpunkte**. `coverage check` meldet ausdrücklich **BLOCKED improved coverage requires HWM increase in same accepted revision**, also keine vollständige Release-Coverage-Freigabe. Die akzeptierte Linux-HWM bleibt **3604/5790**; echte Linux GCC/Clang/gcov RC0.8-Gates **NOT RUN**. Keine HWM-Datei geändert oder angenommen.

**M2-Pool:** `parallel_test_build_test: ok` unter GCC Release, GCC Coverage, TinyCC Debug und Clang Release. Echte `-j 4`-Compiler-/Link-Jobs, synthetische `-j 1`/`-j 4`-Vergleichsprüfung, inkrementeller Build, Failfast/Drain und GCC-Coverage-Zweistufen-Link mit unabsichtlich verhindertem falschem Erfolg bestehen; die absichtlich ungültige C89-Quelle und die Meldung 'compiler produced no output' gehören zu den Negativtests. Test-Runtimes bleiben sequenziell. **Es liegt noch kein gemessener vollständiger `-j 1`-gegen-`-j 4`-Laufzeitvergleich vor**, daher kein belegter Speedup. Die `gcov`-Warnung 'Quelldatei ist neuer als Hinweis-Datei' ist auf diesem neu gebauten Quellstand sichtbar; die Messung selbst und Quell-ID sind jedoch vollständig. Frühere Probleme mit `src\tack.c` versus `src/tack.c` sind im Windows-Sammlerpfad damit funktional behoben.

**Root-Status:** Am Ende des Owner-Logs erscheint neben der legitimen lokal geänderten `coverage/test-inventory.txt` die unversionierte Datei **`tatus --short --branch`**. Ihre Entstehung/Inhalte sind nicht geklärt; nichts automatisch löschen oder mit `git clean` entfernen. Das Artefakt zuerst mit `dir /a` und `type` inspizieren, dann nur bewusst ggf. `del /p` ausführen. Der akzeptierte Inventar-Diff darf nach fast-forward Pull nicht wieder in denselben Checkout eingespielt werden.

**Offen:** (1) gesonderte Owner-Annahme der neuen **Windows-HWM 4036/6233** mit `coverage accept --owner-approve` und reproduzierbarem `coverage check`, (2) Git-Versionierung des exakt angenommenen Windows-HWM-Blobs, (3) echte Linux GCC/Clang/gcov-Matrix auf demselben Source-Commit, (4) repräsentativer Laufzeitvergleich `-j 1` vs. `-j 4`, (5) für v0.8.6 später >=90 % Produktionszeilenabdeckung auf beiden OS. M2 noch **BLOCKED**, PR #13 Draft, kein Merge/Tag/Release.

### M2 RC0.8 – gebündelter, noch ungeprüfter Parallel-Test-Build und Coverage-Identity-Fix (19.11.2026)

**Status: Kandidat, alle neuen Gates NOT RUN.** Ausgangspunkt der Owner-Lognachweise ist M2 RC0.7 `24500ef`: Windows TinyCC Debug, GCC Release, Clang/MSVC Release je **28 compiled / 27 PASS / 0 FAIL / 1 SKIP**. GCC Coverage-Testassertions ebenfalls PASS, aber **gcov-Collector FAIL** wegen Windows `src\tack.c` gegenüber normalisiertem `src/tack.c`; daher kein neues `coverage/current.txt`. RC0.7b `d9ee32b` hat den Quellvergleich korrigiert, wurde jedoch noch nicht getestet. Die letzte Owner-freigegebene und versionierte 28er-Testinventardatei hat Blob `61c0eb04778fb1f0f9908af48f1191fa71a50473`. Baselines Windows **3845/6075**, Linux **3604/5790** unverändert.

**RC0.8 fasst zusammengehörige M2-Änderungen in einer größeren Runde zusammen:** Korrigiertes C89-Testliteral für genau **einen** Windows-Backslash und negativer Doppelseparator-Test in `coverage_ratchet_test.c`. `tack test -j N` leitet `jobs` nun als Kandidat tatsächlich weiter und setzt für `N>1` einen begrenzten Prozess-Pool ein: alle neu zu bauenden Test-Executables werden mit eindeutigen Dateinamen aus den existierenden Base-IDs gestartet, bei `coverage` erst alle unabhängigen `.o/.gcno`, danach die entsprechenden Link-Prozesse. Jeder Compile-/Link-Prozess wird vor dem Test-Start reaped. **Die Test-Runtime bleibt strikt sequenziell**, damit gemeinsame `build/`-Fixtures sicher bleiben. Duplicate Basenames/Spawn-/Compile-/Link-Fehler müssen mit FAIL, vollständigem Prozess-Drain und korrekter not_run-/TAP-/JUnit-Policy enden. `-j 1` verbleibt beim bisherigen seriellen Weg.

**Neue Tests:** `tests/parallel_test_build_test.c` vergleicht `-j 1` und `-j 4` mit vier synthetischen C89-Tests, kontrolliert mehrere pending Kindcompiler, Inkrementalpfad, absichtlichen Compile-Fail (kein stilles Weiterlaufen), und die GCC-Coverage-Zweistufen-Buildpfade separat unter `build/`. `functional_smoke_test` passt seinen direkten Aufruf an die neue Funktionssignatur an. `coverage_ratchet_test` prüft native/einfache/zweifache Separatoren und fehlende Source-Scope-Namen. **Keine Tests abgeschwächt, keine neuen Abhängigkeiten oder CMake/Python, keine Änderung an HWMs.** Neue Testquellen/Fingerprints sind **noch nicht ownerangenommen** und dürfen vom Agenten nicht automatisch ins Inventar geschrieben werden.

**Vor Freigabe:** Owner prüft alle Source-/Test-Diffs und gibt `ratchet tests --owner-approve` ausdrücklich frei, sofern korrekt. Erst gezielte GCC-Coverage-Reparaturprobe, danach Windows TinyCC/GCC/Clang mit `-j 4`, Windows GCC/gcov mit `-j 4` und Prüfung `source_state != 811c9dc5` und gemessener HWM. Erneut `-j 1`-Referenzlauf und E2E-Pool-Tests samt Failfast/CI-Ausgabe. Anschließend echte Linux GCC/Clang/gcov-Läufe; Clang ist verpflichtendes Release-Gate. **Nichts davon ist auf RC0.8 bereits PASS**, M2 bleibt BLOCKED, PR #13 Draft, kein Merge/Tag/Release. 0.8.6-Release braucht zusätzlich >=90 % echten Produktionszeilen auf beiden OS.


### RC0.7 Windows Owner: Testassertions PASS, GCC-Coverage-Collector FAIL; RC0.7b Kandidat (NOT RUN)

Owner-Protokoll vom 19.11.2026 auf Commit `24500ef04267e9483eee29c60fd339a0115d0823`: Git sauber, Bootstrap `gcc -std=c89 -pedantic -Werror` PASS. `ratchet tests` erwarteter FAIL nach Teständerung; Owner führte `ratchet tests --owner-approve` aus; Suite bestätigt Inventar PASS. Der **exakte** freigegebene Inventar-Git-Blob ist `61c0eb04778fb1f0f9908af48f1191fa71a50473`, allein `tests/coverage_ratchet_test.c f15851ee -> e79b42df`, ohne Abschwächung einer Assertion; er wird versioniert.

Windows TinyCC Debug, GCC Release und Clang/MSVC Release jeweils **28 compiled / 27 PASS / 0 FAIL / 1 SKIP / 0 NOT RUN**. GCC Coverage-Testassertions ebenfalls 27 PASS, aber Gesamt-Gate **FAIL**: `production source not instrumented: src\tack.c`, `source scope missing`, `FAIL gcov collection incomplete`, `TACK_SUMMARY status=failed`. `coverage/current.txt` existiert nicht, `coverage check` meldet **NOT RUN**. **Keine neue Coverage-Quote wird behauptet**, bisher angenommene HWMs Windows **3845/6075** und Linux **3604/5790** bleiben unberührt.

**Ursache im Code bestätigt:** Das neue `cov_is_production()` erkennt native `src\tack.c`, aber `cov_validate_source_scope()` vergleicht diesen Scan-Namen unverändert mit der von `cov_read_gcov` normalisierten Quelle `src/tack.c`. Bei diesem Bytevergleich fehlt die Instrumentierung nur scheinbar. **RC0.7b Quellkorrektur (NOT RUN):** `cov_source_is_instrumented` gleicht ausschließlich eine kanonische ID über `cov_relative_source` ab und weist fehlende, überlange oder `../`-Quellangaben weiterhin zurück. Native Dateisystempfade und die eigentliche Messung bleiben unberührt; der Test prüft beide Separatoren und abwesende Quellen. Kein HWM-Accept, keine Testabschaltung. P32 weiterhin BLOCKED bis Windows-GCC-Coverage tatsächlich neu gemessen.

**Separater Performance-Befund in M2, noch ohne Implementierung:** `test ... -j N` akzeptiert `jobs` im Argumentparser, aber ruft `build_and_run_tests(target_name, p, verbose, force..., strict, &stats)` **ohne jobs** auf. Innerhalb des Runners folgen Compiler-/Link-Aufrufe und Tests strikt seriell via `run_argv_wait`. `-j 4/8` beschleunigt den Testlauf daher heute nicht; `--rebuild` erzwingt wiederholtes Kompilieren. Geplanter isolierter M2-Folgeschritt: bounded parallele Test-Compile/Link-Jobs mit eindeutigem Ausgabeziel, deterministischer Fail-fast-/Drain-/Reporter-Semantik; Testausführung wegen gemeinsamer Fixtures zunächst sequenziell, Coverage-gcov nach Abschluss. Keine Ausweitung der RC0.7b-Fixquelle um ungeprüftes Thread-/Job-Scheduling. Linux GCC/Clang/gcov unverändert NOT RUN, PR #13 Draft, M2 offen.

### RC0.7 – Windows-Source-Fingerprint-Integrität: gezielter Korrekturkandidat, NOT RUN

Nach vollständigem RC0.6a-Windows-Owner-Nachweis (TinyCC Debug, GCC Release, Clang/MSVC Release, GCC Coverage jeweils **28 kompiliert / 27 PASS / 0 FAIL / 1 SKIP**, Windows gcov **3845/6075 PASS**) ist P32 aufgrund eines zuvor unerkannten Windows-Coverage-Freshness-Problems weiterhin **BLOCKED**. Die in RC0.5 und RC0.6a wiederholte `source_state=811c9dc5` entspricht exakt dem FNV-1a-Startwert: `cov_workspace_state(0)` nutzt unter Windows scannerseitig Backslash-Pfade, `cov_is_production` filtert nur Slash-Pfade. Die tatsächlich gemessene rc0.6a-Coveragequote ist davon unabhängig, aber die zusätzliche Quellfrischeprüfung war wirkungslos. `tests_state=be2d16b1` aus RC0.6a ist ein korrekt frischer Test-State; bei unveränderter Coveragequote ist ein neuer HWM-Accept nicht erforderlich.

**RC0.7 Kandidat nach ADR 0005:** `cov_is_production` akzeptiert `/` und `\` als Trenner hinter `src`/`include`, während `include/tests` bei beiden Schreibweisen ausgeschlossen bleibt. `tests/coverage_ratchet_test.c` sichert ausdrücklich beide Schreibweisen, Prefix-Abgrenzungen und `cov_workspace_state(0) != 0x811c9dc5`. Einzeldatei ISO C89, keine zusätzlichen Abhängigkeiten, unveränderte Prozesse und alle alten Assertions. **RC0.7 Gates, Inventory-Owner-Accept, Windows/Linux Coverage: NOT RUN**. GitHub-HWMs Windows **3845/6075**, Linux **3604/5790** und das bei RC0.6a Owner-angenommene/committete 28er-Inventar **7233bed7f6edfea524eb605d46d37d39eca4be60** bleiben unberührt, bis echte Messungen neue Freigaben rechtfertigen. Kein Merge/Tag/Release.

### RC0.6a – Windows TinyCC/GCC/GCC-Coverage PASS; Ratchet-Inventar Owner-genehmigt und versioniert (19.11.2026)

**Owner-Checkout:** `ce8ab07cfaf3e4a0794872ed8b25ebf16508d826`, nur Dokumentation nach Quellstand `1ba915f3ee48961dd5e2dbbab307ad74b1422f6b`. Windows-TinyCC Debug, GCC Release und GCC Coverage jeweils **`total=28 compiled=28 passed=27 failed=0 skipped=1 not_run=0 status=ok`**. Die separate Windows-Clang/MSVC-Release-Suite auf identischem Quellstand war bereits PASS mit denselben Assertions. `process_execution_test: ok` in allen Profilen; absichtliche Negativdiagnosen und non-POSIX-SKIP sind erwartete Testfälle.

**Echte frische Windows-GCC/gcov-Messung:** `coverage/current.txt`: `profile=windows-gcc-10.3.0-gcov-10.3.0-coverage-v1`, `covered=3845`, `total=6075`, `source_count=1`, `sources_hash=39ea236e`, `source_state=811c9dc5`, `tests_state=be2d16b1`, `flags_state=36eb9e95`, `tests_total=28`, `tests_passed=27`, `tests_skipped=1`. `coverage check` **PASS HWM retained: 3845/6075**. Es gibt **keine Coverage-Steigerung oder -Regression**, daher ist nach `cov_compare` keine HWM-Erhöhung nötig. Die aktuelle Messung wird durch `cov_fresh` gegen das Arbeitsverzeichnis geprüft; `tests_state` gehört ausdrücklich nicht zu den mit der alten HWM gleichzusetzenden Vergleichsfeldern. Die angenommene Windows-HWM `3845/6075` und Linux-HWM `3604/5790` bleiben exakt unverändert.

**Testinventar:** Owner führte bereits ausdrücklich `ratchet tests --owner-approve` aus und prüfte das Git-Diff; alle RC0.6a Suites meldeten anschließend Inventar PASS. Lokaler Git-Diff zeigt Blob-Präfix **`7233bed`** und neue Fingerprints für vier Testquellen: `cache_restore_after_clean_test.c c9cdcd93`, `docs_bilingual_contract_test.c 83fa0efc`, `path_filesystem_safety_test.c a5793c36`, `process_execution_test.c 9ba06b3c`. Der vierte Eintrag erscheint nicht vollständig im gepasteten Diff-Ausschnitt, aber nur der Record mit genau diesen vier Änderungen bildet **exakt** den vom Owner gemeldeten Git-Blob `7233bed7f6edfea524eb605d46d37d39eca4be60` ab. Das Inventar ist deshalb ohne freie Rekonstruktion oder stille Tests-Abschwächung auf GitHub versionierbar.

**Neuer kritischer Validierungsbefund (BLOCKED, keine Produktionskorrektur in dieser Revision):** Die reproduzierte Windows-`source_state=811c9dc5` entspricht dem unveränderten FNV-1a-Startwert `2166136261`. `cov_workspace_state(0)` filtert gescannte Pfade mit `cov_is_production`, das nur `src/` bzw. `include/` akzeptiert, während `scan_dir_recursive_suffix_skip_depth` und `path_join_alloc` unter Windows `src\...`/`include\...` liefern. Dadurch werden unter Windows keine Produktionsdateien in `source_state` eingerechnet. Der Codeprüfpfad `cov_fresh` vergleicht diesen wertlosen Source-State gegen denselben Wert; diese Quellfrischeprüfung ist somit **wirkungslos auf Windows**, obwohl die eigentliche gcov-Ratio aus echten Messdaten stammt. Das war bereits in der angenommenen RC0.5-HWM vorhanden und wurde jetzt durch Quellcodeabgleich entdeckt. **M2-P32 bleibt trotz PASS-Coverage-Ratio BLOCKED**, bis dieser vorhandene Fingerprint-Fehler mit eigener Regression und erneuter echter Coverage-Messung korrigiert ist. Keine Baseline-Absenkung, keine Ratchet-Umgehung, kein ungeprüfter Windows-Source-Fingerprint. Echte Linux GCC/Clang/gcov-Gates weiterhin **NOT RUN**. Draft PR #13, kein Merge/Release.

### RC0.6a – Windows Clang/MSVC Release vollständig PASS, Owner 19.11.2026

**Getesteter Quellstand:** `1ba915f3ee48961dd5e2dbbab307ad74b1422f6b` mit nachträglichem reinem Dokumentationscommit `62d7c640989ac186f2582c4e619f1e46d5be8c38`. Owner `git fetch/pull --ff-only` PASS, `git diff` geprüft, `gcc -std=c89 -pedantic -Werror -o tack.exe src\tack.c` **PASS**.

**Testinventar:** `ratchet tests` meldete ordnungsgemäß **FAIL inventory mismatch** für `tests/cache_restore_after_clean_test.c`; der Owner führte daraufhin ausdrücklich `ratchet tests --owner-approve` aus. Der nachfolgende Clang-Testlauf bestätigte **`PASS versioned source inventory`**. Die vom Owner lokal geänderten Fingerprints sind noch nicht auf GitHub versioniert. Vor einem Commit müssen die exakten `git diff -- coverage/test-inventory.txt` und `git status` geprüft werden; an der vorhandenen Windows-/Linux-HWM ist bis zu einer tatsächlichen neuen GCC-Messung nichts zu ändern.

**Echter Windows-Clang 23.1.5 (MSVC-Ziel):** `set "TACK_CC=clang"` und `tack --config .\tack.ini test release --ci --rebuild -j 1` ergaben **`TACK_SUMMARY version=1 mode=test status=ok profile=release total=28 compiled=28 passed=27 failed=0 skipped=1 not_run=0`**. Alle 28 Testquellen wurden neu übersetzt; `cache_restore_after_clean_test: ok`, `process_execution_test: ok` und `strict_c89_compile_test: ok`. Der eine SKIP (`incremental_link_test`) ist die erwartete non-POSIX-Ausnahme. Im Ratchet-Test auftretende `FAIL`/`BLOCKED` und bei Win32-Fault-Proben absichtlich ausgegebene Diagnosen sind erwartete Negativtests, nicht fehlgeschlagene Assertions.

**Status:** RC0.6a Windows Clang Release **PASS**, nicht nur Syntaxprüfung. RC0.6a TinyCC Debug, GCC Release, GCC Coverage inkl. HWM-Prüfung und POSIX-Linux-GCC/Clang/gcov bleiben **NOT RUN** auf diesem Quellstand. Die von RC0.5 Owner-angenommenen Windows-/Linux-HWMs **3845/6075** und **3604/5790** bleiben versioniert unverändert; geänderter Produktions-Source-State verlangt erneute aktuelle GCC-Coverage-Messung und Ratchet-Review. Kein M2-Gesamtabschluss, PR #13 Draft, kein Merge/Tag/Release.

### RC0.6a Windows-Clang-Owner-Nachweis 19.11.2026: C89-Compile-Probes PASS

Auf `1ba915f3ee48961dd5e2dbbab307ad74b1422f6b` hat der Owner den Git-Pull per Fast-forward ausgeführt und `git diff --check 2528f13..HEAD` ohne Befund geprüft. Die **tatsächlich ausgeführte Einzelübersetzung** `clang -std=c89 -pedantic-errors -Wextra -Werror -Iinclude -Isrc -c tests\cache_restore_after_clean_test.c -o build\cache-clang-msvc-probe.obj` war **PASS** (keine Fehlermeldung). Danach war die **vollständige Syntaxprüfung der 28 Testdateien im Verzeichnis `tests`** mittels `for %F in (tests\*_test.c) do @clang ... -fsyntax-only "%F" || echo CLANG_FAIL: %F` **PASS** (keine Compilerdiagnose, kein `CLANG_FAIL`). Zielcompiler ist nach wie vor **Windows Clang 23.1.5, `x86_64-pc-windows-msvc`** und damit **keine Linux-Evidenz**.

Diese Proben bestätigen den Fix des zuvor unter Windows Clang fehlgeschlagenen Cache-Tests und die Compile-/Syntaxverträglichkeit der Testquellen. **Nicht geprüft** wurden hier Linken, Runtime-Assertions, ganze `tack test`-Suite oder gcov. Diese bleiben **NOT RUN**. Die zuvor Owner-genehmigte RC0.5-HWM Windows **3845/6075**, Linux **3604/5790** und das versionierte 28er-Inventar bleiben unverändert, bis die neue test/source state nach tatsächlichen Gates explizit freigegeben wird. M2 bleibt offen, PR #13 Draft.

### RC0.6a – Clang-MSVC: `getcwd`-Fehler im ersten Test eingegrenzt, Fix NOT RUN

Owner-Probe auf Feature-HEAD `2528f131da1b472f35989d3aea20c5448e281eaa`: `git diff --check 65544f2..HEAD` ohne Fehler; `clang -std=c89 -pedantic-errors -Wextra -Werror -c src\tack.c -o build\tack-clang-msvc-probe.obj` **PASS**. Die gleiche direkte Strict-C89-Kompilierung des ersten Tests `tests/cache_restore_after_clean_test.c` **FAIL** bei `getcwd` in Zeile 47, Microsoft-UCRT-Attribut `_CRT_NONSTDC_DEPRECATE(_getcwd)` und `[-Werror,-Wdeprecated-declarations]`. Der zuvor angepasste `_CRT_SECURE_NO_WARNINGS`-Guard hat das eigenständige tack-Kompilierungsproblem beseitigt, aber nicht Microsofts separate POSIX-Namens-Deprecation (was beabsichtigt ist).

RC0.6a führt **nur im Cache-Test** eine C89-kompatible `read_cwd_local`-Funktion ein: `_getcwd` mit geprüftem `INT_MAX` unter Windows, `getcwd` unter POSIX. Kein globaler `_CRT_NONSTDC_NO_WARNINGS`-Schalter, keine Verringerung von `-Werror`, keine weiteren Compileroptionen oder Änderungen der Prozesslogik. Andere geprüfte Tests verwenden unter Windows bereits `_getcwd`.

**RC0.6a Tests/Gates noch NOT RUN.** Das Windows-Clang-Gesamtprofil hat weiterhin kein PASS; der vorherige RC0.6-Einzeltest ist FAIL. RC0.5 Windows GCC/TinyCC/Coverage bleibt historisch PASS. Die akzeptierten Windows-/Linux-Coverage-HWMs 3845/6075 und 3604/5790 sowie die versionierte Testinventar-Ratchet bleiben unverändert, bis neuer vollständiger Lauf und Owner-Freigaben vorliegen.

### RC0.6 Clang-MSVC/UCRT-Kompatibilitätskandidat, 19.11.2026 (NOT RUN)

**Ist-Zustand aus Owner-Log auf `65544f2fa56a978979c6ee4e3564432612d0f5e8`:** Windows Clang **23.1.5** meldet Ziel `x86_64-pc-windows-msvc` und **FAIL** durch standardkonforme C89-API-Aufrufe (`fopen`, `getenv`, `strerror`), die Microsofts UCRT als vendor-deprecated annotiert. `-Werror` stoppt beim ersten Test: `total=28 compiled=1 passed=0 failed=1 skipped=0 not_run=27`. Dieser Windows-Lauf ist **keine Linux-Referenzprüfung**. Im selben Owner-Log: Windows GCC Debug / Release / Coverage je **27 PASS, 0 FAIL, 1 SKIP**, Coverage `3845/6075`, `coverage check PASS`. Die Meldungen des `coverage_ratchet_test` sind erwartete Negativtests, keine Suitefehler.

**RC0.6-Korrektur gemäß ADR 0004:** `_CRT_SECURE_NO_WARNINGS` ist explizit auf `_WIN32 && _MSC_VER && __clang__` begrenzt und wird vor CRT-/Windows-Headern in Single-File-tack, Test-Embed-Header und drei Früh-Header-Tests gesetzt. C89, alle vorherigen Assertions, `-Werror`, -j-Grenzen und Prozessaufrufe bleiben unverändert. Die erste Clang-Fehlerliste ist auf 20 Diagnosen begrenzt; weitere Probleme können nach deren Behebung sichtbar werden.

**Freigabestatus:** RC0.6 Windows TinyCC Debug, GCC Release, Clang MSC Release und GCC Coverage sowie Linux GCC/Clang/gcov **NOT RUN**. Geänderte `src/tack.c` und drei Testdateien verlangen neue lokale Owner-Inventar-Ratchet und echte Messung. **Windows-HWM 3845/6075, Linux-HWM 3604/5790 und versioniertes Inventar bleiben unverändert; kein automatisches Ratchet-Accept.** P17/P19/P20 Windows RC0.5 bleiben historische PASS, aber nicht neu als RC0.6 bestanden ausgegeben. M2 weiterhin offen, PR #13 Draft.

### RC0.5 – Windows-HWM ausdrücklich durch Owner angenommen (19.11.2026)

Der Owner führte auf dem bereits erfolgreich getesteten RC0.5-Quellenstand `99ba12660ce3ff968706094fbd96b42f3d0979f4` nach Annahme des geänderten Testinventars ausdrücklich `tack --config .\tack.ini coverage accept --owner-approve` aus. Ausgabe: `accepted measured high-water mark: coverage/hwm-windows.txt`. Der unmittelbar anschließende `coverage check` meldete **`PASS HWM retained: 3845/6075`**. `ratchet tests` bestätigte ebenfalls PASS. `git diff --check` meldete ausschließlich die bekannten Windows-LF/CRLF-Konvertierungswarnungen, keine Whitespace-Fehler.

Die bestätigte aktuelle Messung lautet `3845/6075 (63.29218%)`, `source_state=811c9dc5`, `tests_state=bd1ac24d`, `flags_state=36eb9e95`, `sources_hash=39ea236e`, 28 Tests (27 PASS, 1 SKIP). Das ist eine um **1 Produktionszeile höhere HWM** gegenüber der bereits angenommenen RC0.4-HWM `3844/6075`. **Die HWM wurde nicht stillschweigend erhöht; die Owner-Ausführung von `coverage accept --owner-approve` ist belegt.**

Die gemeinsam versionierten RC0.5-Ratchet-Artefakte sind:

- `coverage/hwm-windows.txt`: Git-Blob `79d0806a53414fd5a093421e9df44228e0557bb4`, aus unverändertem alten HWM-Record plus den durch `coverage/current.txt` belegten Änderungen `covered=3845` und `tests_state=bd1ac24d`. Der vollständige neue HWM-Diff wurde in dieser Owner-Nachricht nicht nochmals ausgegeben; der gespeicherte Record folgt den nachgewiesenen Messfeldern.
- `coverage/test-inventory.txt`: Git-Blob `1dc6e302fd31fd241fdf3d77ad20e4341df2cc11`, 28 unverändert gezählte Quellen, `tests/process_execution_test.c 53d637eb`. Der Git-Blob-Präfix `1dc6e30` wurde bereits im Owner-Windows-Log ausgewiesen und stimmt exakt.

Beide Dateien werden in derselben Revision des Feature-Branches gespeichert. `src/tack.c` und Linux-HWM `3604/5790` bleiben unverändert. **P17/P19/P20 Windows PASS, Linux weiterhin NOT RUN, M2 als Ganzes noch nicht abgenommen.** PR #13 bleibt Draft; kein Merge oder Release.

### RC0.5 Windows Owner-Evidenz, 19.11.2026: P17/P19/P20 PASS, neue HWM noch BLOCKED

**Geprüfter Commit:** `99ba12660ce3ff968706094fbd96b42f3d0979f4`. Alle drei Suites verwenden die geänderte reine C89-Testquelle ohne Änderung an `src/tack.c`. Der Owner führte `git diff --check` ohne Befund aus, prüfte den Quell-Diff und nahm das neue Inventar explizit mit `tack --config .\tack.ini ratchet tests --owner-approve` lokal an. Folge-Suites melden `PASS versioned source inventory`. Alle 28 Testquellen wurden kompiliert; die neue Testquelle hat den Fingerprint `53d637eb` statt `0750f9e5`, lokaler Git-Diff-Blob-Prefix `1dc6e30`. Die Inventardatei bleibt zunächst nur lokal modifiziert.

| Windows-Gate | Owner-Nachweis | Bewertung |
| --- | --- | --- |
| TinyCC Debug | `TACK_SUMMARY status=ok total=28 compiled=28 passed=27 failed=0 skipped=1 not_run=0` | **PASS** |
| TDM-GCC Release | `TACK_SUMMARY status=ok total=28 compiled=28 passed=27 failed=0 skipped=1 not_run=0` | **PASS** |
| TDM-GCC Coverage-Testassertions | `process_execution_test: ok`, 27 bestanden, 0 fehlgeschlagen, 1 SKIP | **PASS** |
| P17 Native -j-Prozesszählung | Erweiterte Instrumentierung in `process_execution_test: ok` enthalten | **PASS Windows / Linux NOT RUN** |
| P19 Fast-before-slow Marker | Beide Quellreihenfolgen in `process_execution_test: ok` enthalten; OS-Exit-Zeitpunkt nicht behauptet | **PASS Windows / Linux NOT RUN** |
| P20 Spawn-Fehler mit lebendem Geschwister | Erwartete `CreateProcessA`- und `compiler spawn failed`-Diagnosen; Test meldet `ok` | **PASS Windows / Linux NOT RUN** |
| Testinventar RC0.5 | Owner-angenommen, lokal noch nicht versioniert | **PASS lokal / Commit offen** |
| Windows Coverage Gesamtlauf und `coverage check` | Neue Ratio über angenommener HWM | **BLOCKED, kein Test-Assertion-FAIL** |
| Linux GCC/Clang Referenz und Linux Coverage | Kein Linux-Lauf im Owner-Log | **NOT RUN** |

**Windows gcov:** `coverage/current.txt` enthält `profile=windows-gcc-10.3.0-gcov-10.3.0-coverage-v1`, `covered=3845`, `total=6075` (**63.29218 %**), `sources_hash=39ea236e`, `source_count=1`, `source_state=811c9dc5`, `tests_state=bd1ac24d`, `flags_state=36eb9e95`, `tests_total=28`, `tests_passed=27`, `tests_skipped=1`. Gegen die weiterhin akzeptierte Windows-HWM **3844/6075 (63.27572 %)** sind es **+1 ausgeführte Produktionszeile** bei gleichem Nenner, rund **+0.01646 Prozentpunkte**. Die Testquellenmessung 97.22 % von 432 Zeilen für `process_execution_test.c` ist separat und nicht die Produktionscode-HWM.

**Ausstehende Abnahme:** Der Owner muss bei Anerkennung dieser Messung separat `tack --config .\tack.ini coverage accept --owner-approve` und anschließend `tack --config .\tack.ini coverage check` ausführen. Erst danach werden die beiden genehmigten Ratchet-Dateien zusammen in derselben Revision versioniert. Linux-HWM 3604/5790 bleibt unverändert. Kein M2-Gesamtabschluss, kein Merge oder Release. Die erwarteten Negativdiagnosen aus `coverage_ratchet_test` und P20 sind keine fehlgeschlagenen Testassertions.

### RC0.5 – P17/P19/P20 erweiterte Parallelprozessproben (18.11.2026, Kandidat)

**Ausgangspunkt:** RC0.4 Produktionscode und seine vom Owner bestätigten Windows-Gates bleiben unverändert. Commit `ca9090dda0aa302d404784cb78a0716b5289a059` enthält das owner-genehmigte Windows-Testinventar und HWM **3844/6075**. RC0.5 erweitert ausschließlich `tests/process_execution_test.c`. **RC0.5 Windows und Linux NOT RUN**, keine Ratchet- oder HWM-Freigabe für diesen neuen Test-Fingerprint.

- **P17, echte Jobbegrenzung:** Die Elternseite instrumentiert die echten Betriebssystem-Spawn- und Wait-APIs (Win32 CreateProcessA/WaitForSingleObject, POSIX fork/waitpid) und zählt Spawnversuche, erfolgreiche Starts, abgeschlossene Waits, aktive/unreaped Prozesse, Spitzenwert und Grenzverletzungen. Vier Fake-Compiler-Kinder laufen mit `-j 2`; die Paare 0/1 und 2/3 warten auf gegenseitige Ready-Dateien, wodurch zwei tatsächlich gleichzeitig laufende Prozesse statt nur der Slotzahl nachgewiesen werden. Jede Ready-Wartebedingung ist bounded. Erwartung: 4 Starts/4 Waits, Peak exakt 2, nie mehr als 2 aktive Handles/Jobs, alle vier Output-Objekte vorhanden.
- **P19, Completion-Reihenfolge:** Zwei echte Compiler-Kinder (`m2_order_slow.c`, `m2_order_fast.c`) werden unter `-j 2` in beiden Startreihenfolgen geprüft. Der schnelle Kindprozess schreibt sein Objekt und publiziert erst dann `order-fast.done`; das langsame Kind darf sein Objekt erst nach dem beobachteten Marker und eigener Bestätigung schreiben. Das erzwingt eine überprüfbare Reihenfolge der abgeschlossenen *Compiler-Arbeit*, unabhängig von Scheduling-Timestamps, bei gleichzeitig erfolgreicher FIFO-Prozessverwaltung. Dies beweist nicht zwingend die Reihenfolge der endgültigen OS-Exit-Zustände; der Vertrag betrifft erhaltene Ergebnisse trotz Completion-Reordering.
- **P20, injizierter Spawn-Fehler bei lebendem Kind:** Der erste Fake-Compiler `m2_hold.c` publiziert `hold-ready` und wartet bounded auf `hold-release`. Die Parent-API-Injektion wartet bounded auf dessen Ready-Marker, bevor sie **den zweiten Start** mit ACCESS_DENIED/EAGAIN ablehnt; danach gibt sie den ersten Kindprozess frei. Erwartung: genau zwei Versuche, ein gestartetes und ein eingesammeltes Kind, null aktive Handles, `hold-complete` vorhanden, kein `.meta`-Erfolgseintrag für den abgebrochenen Job.

**Nicht verändert:** `src/tack.c`, `coverage/hwm-windows.txt` (3844/6075), `coverage/hwm-linux.txt` (3604/5790), bestehende 27 Regressionstests, Buildsystem und Abhängigkeiten. Kein neues M3–M8-Feature.

**Ausstehende Gates in dieser RC:** Windows TinyCC debug, GCC release, GCC coverage, neue Test-Ratchet-Ownerprüfung sowie Linux GCC/Clang-Referenzsuite und Linux gcov. Tests und Coverage dürfen erst nach neuer tatsächlicher Messung als PASS übernommen werden. Sind neue Windows-Zeilen stärker abgedeckt, ist eine neue HWM-Annahme durch den Owner erforderlich. No automatic ratchet acceptance, no merge.

### Windows Owner-Gate und HWM-Abnahme: endgültig PASS (18.11.2026)

Auf dem Feature-Branch `feature/v0.8.6-m2-process-execution` wurde der Source-/Test-Commit `382471c950d126697391977fe9962bd0eb2d57e9` ausgeführt (drei Windows-Profile jeweils **28 kompiliert, 27 PASS, 0 FAIL, 1 SKIP, 0 NOT RUN**). Der zwischenzeitliche reine Dokumentationscommit `ef748c9399c39eab64bbdd4e0323e99700daa0a6` änderte weder Produktions- noch Testquellen. Das Testinventar wurde vom Owner mit `ratchet tests --owner-approve` angenommen; der erneute `ratchet tests`-Lauf zeigte PASS.

Danach führte der Owner ausdrücklich `tack --config .\tack.ini coverage accept --owner-approve` aus. Das Kommando bestätigte `accepted measured high-water mark: coverage/hwm-windows.txt`. Der anschließende `tack --config .\tack.ini coverage check` meldete **PASS HWM retained: 3844/6075**. Die vorherige BLOCKED-Ausgabe im `coverage accept`-Kommando gehört zur vorgeschalteten, noch gegen den alten HWM prüfenden Negativentscheidung; die anschließende explizite Annahme und PASS-Prüfung sind maßgeblich.

**Exakt vom Owner freigegebene Artefakte:**

- `coverage/hwm-windows.txt`: `3844/6075`, `tests_state=ee1bba53`, 28 Tests (27 PASS, 1 SKIP), Git-Blob **`2f4b698566ae86a53f6b0383579e64a07bee7911`**. Entspricht dem lokalen `git diff`-Index `2f4b698`.
- `coverage/test-inventory.txt`: alle bisherigen 27 Hash-Zeilen erhalten, **`tests/process_execution_test.c\t0750f9e5`** als zusätzliche 28. Zeile, Git-Blob **`2f8dbecf0b2d56e4eb2fbacd0f9efc4720a6e7c0`**. Entspricht dem lokalen `git diff`-Index `2f8dbec`.
- Die beiden Files werden zusammen in einer Revision aufgenommen. Historische Windows-HWM 3724/6000 und Linux-HWM 3604/5790 dienen nur dem Vorher-Vergleich, es erfolgt keine Baseline-Senkung.

**Offen:** Vollständiger Linux-GCC-/Clang-Referenzlauf und Linux-Coverage, sowie Anforderungen P17/P19/P20 mit zusätzlichen instrumentierten Parallelprozess-Randfällen. Ein Windows HWM-PASS alleine ist noch keine gesamte M2-Abnahme. Keine Änderung an `main` oder `dev/v0.8.6`.


### Windows RC0.4: historische Owner-Tests vor HWM-Freigabe (18.11.2026)

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
| Test-Ratchet in dieser Revision | **PASS Windows RC0.4, owner-angenommen und versioniert** | Genaues Inventar-Blob `2f8dbecf0b2d56e4eb2fbacd0f9efc4720a6e7c0` |
| Windows / Linux instrumentierte M2-Coverage | NOT RUN | Windows RC0.2 Compiler-FAIL, keine gültige aktuelle Messung; Linux noch offen |
| M2 gesamthaft | BLOCKED | Linux und noch unvollständig belegte Parallelprozess-Randfälle P17/P19/P20 |

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

### English: owner-accepted Windows HWM and test inventory

The Owner explicitly ran `tack coverage accept --owner-approve` and then `tack coverage check`, which returned **PASS HWM retained: 3844/6075**. Both accepted ratchet artifacts were reconstructed without content changes and verified against the local Git diff prefixes: Windows HWM full blob `2f4b698566ae86a53f6b0383579e64a07bee7911` and 28-item test inventory full blob `2f8dbecf0b2d56e4eb2fbacd0f9efc4720a6e7c0`. The two files are versioned in the same commit. Windows tests passed; full Linux gates and residual parallel-process boundary requirements remain pending. M2 as a whole is not accepted.

### English – RC0.5 parallel subprocess proof candidates

RC0.5 modifies **only the C89 M2 process regression test**. It instruments native parent-side spawn/wait accounting for P17, adds a bounded two-child start barrier to prove real concurrency, checks fast-before-slow completion-marker ordering in both launch orders for P19, and injects the second spawn failure only once the first held child is confirmed running for P20. The held child is released and must be reaped without a success metadata record. The verified RC0.4 Windows HWM **3844/6075** and 28-test inventory remain untouched. All RC0.5 Windows/Linux gates and fresh ratchets are **NOT RUN / BLOCKED** pending Owner review. No complete milestone acceptance.

### English: RC0.5 Windows Owner results (19 November 2026)

At `99ba12660ce3ff968706094fbd96b42f3d0979f4`, Windows TinyCC debug, GCC release and GCC coverage suites compiled 28 tests and each reported 27 PASS, 0 FAIL, 1 SKIP. P17/P19/P20 assertions passed. The Owner accepted the revised inventory locally, with new process test fingerprint `53d637eb`; it is not committed yet. Current Windows production coverage is **3845/6075 (63.29218%)** against accepted **3844/6075 (63.27572%)**, a one-covered-line gain at unchanged denominator. HWM check remains BLOCKED until a separate Owner acceptance and joint versioning of inventory and HWM. Linux gates remain NOT RUN; M2 not complete.

### English RC0.5 Owner HWM acceptance, 19 November 2026

Owner explicitly executed `tack coverage accept --owner-approve`; it wrote `coverage/hwm-windows.txt`. The next `tack coverage check` returned **PASS HWM retained: 3845/6075**. The modified test inventory had already been Owner-approved and passed `ratchet tests`. The accepted Windows HWM is 3845/6075 (63.29218%), plus one covered production line against RC0.4. Both approved ratchets are versioned together (HWM blob `79d0806a53414fd5a093421e9df44228e0557bb4`; inventory blob `1dc6e302fd31fd241fdf3d77ad20e4341df2cc11`). No Linux gate is claimed and M2 is not accepted overall.

### English RC0.6 Clang MSVC CRT compatibility candidate

Windows Clang 23.1.5 targeted MSVC UCRT and failed at the first test due to Microsoft-specific deprecation attributes on standard C89 `fopen`, `getenv`, `strerror` under `-Werror` (1 compiled / 1 failed / 27 not run). This was not Linux validation. RC0.6 adds an explicitly documented narrow CRT opt-out before all relevant includes; C89, `-Werror`, process code and test assertions are unchanged. Windows GCC RC0.5 PASS remains historical evidence only. All RC0.6 and Linux gates NOT RUN; owner-gated inventory/HWM remain untouched until fresh measurement. See ADR 0004.

### English RC0.6a direct Windows Clang probe

At `2528f13` standalone `src/tack.c` **compiled successfully** with strict C89 Clang/MSVC. The embedded cache test **failed** on the UCRT-deprecated POSIX alias `getcwd`. RC0.6a uses the already-project-standard `_getcwd` Windows call and `getcwd` POSIX call with checked capacity; neither `-Werror` nor other CRT warnings are suppressed. Updated test and full profiles **NOT RUN**; ratchets remain unchanged and Owner-gated.

### English: RC0.6a Owner direct Clang compile probes, 19 November 2026

At `1ba915f3ee48961dd5e2dbbab307ad74b1422f6b`, the Owner verified `git diff --check` and successfully compiled `tests/cache_restore_after_clean_test.c` with Windows Clang 23.1.5 and strict C89 flags. `-fsyntax-only` for all 28 test sources completed without compiler diagnostics or any `CLANG_FAIL`. This **PASS** is limited to direct compile/syntax probes; linking, integrated test execution, gcov and Linux validation are still NOT RUN. Both coverage HWMs and the test inventory remain unchanged and require new Owner review/acceptance on any updated measurements.

### English: RC0.6a actual Windows Clang/MSVC runtime test PASS (Owner, 19 November 2026)

Owner fetched tested source `1ba915f` (plus documentation-only `62d7c64`) and built the `tack` host under strict C89 GCC. The inventory correctly rejected changed test sources until the Owner ran `ratchet tests --owner-approve`; following Clang suite reported inventory PASS. Windows Clang 23.1.5/MSVC `test release --ci --rebuild -j 1` returned **status=ok, 28 compiled, 27 passed, 0 failed, 1 skipped, 0 not run**, including process execution, cache restoration and strict C89 tests. The one non-POSIX skip and deliberate negative-test failure diagnostics are expected. The locally approved inventory was not yet committed; exact diff and new GCC coverage remain pending. RC0.6a TinyCC/GCC and true Linux gates are NOT RUN. Existing Windows 3845/6075 and Linux 3604/5790 coverage HWMs remain versioned and unchanged. PR stays Draft, milestone not closed.

### English – Owner RC0.6a Windows GCC/TinyCC coverage PASS and fingerprint integrity blocker

Windows TinyCC Debug, GCC Release and GCC Coverage all ran successfully at the same RC0.6a code revision, 28 compiled/27 PASS/0 FAIL/1 SKIP per suite. Clang/MSVC Release had already passed. Fresh GCC/gcov measured **3845/6075 (63.29218%)**; coverage check returned PASS at the unchanged accepted HWM. Local Owner-approved test inventory is blob `7233bed7f6edfea524eb605d46d37d39eca4be60`, with four updated source hashes, including process execution; inventory can be versioned without altering either HWM. **Critical:** Windows `source_state=811c9dc5` equals FNV-1a offset basis because `cov_is_production` only accepts slash-separated `src/`/`include/` but Windows scanner emits native backslashes. Thus production source freshness is NOT actually enforced by that field on Windows, despite measured ratio PASS. P32 is BLOCKED pending a narrowly tested path normalization fix and fresh measured ratchets. Integrated Linux GCC/Clang/gcov remains NOT RUN; no merge/release.

### English RC0.7 Windows source-state integrity candidate

RC0.6a Windows cross-compiler runtime suites and measured GCC/gcov 3845/6075 PASS remain valid historical evidence. However, Windows native backslash scanning meant `cov_is_production` admitted no production files and `source_state` remained the FNV offset basis. ADR 0005 documents the targeted RC0.7 classifier fix accepting both separators while preserving `include/tests` exclusion, with new strict C89 tests including nonempty actual workspace-state assertion. The RC0.7 suite, ratchet acceptance and fresh Windows/Linux coverage measurement are **NOT RUN**, both HWMs and previously owner-approved inventory remain unmodified. M2 remains BLOCKED, PR Draft.

### English: RC0.7 owner tests and RC0.7b comparison fix

Owner TinyCC Debug, GCC Release and Clang/MSVC Release at `24500ef` all PASS (28 compiled, 27 passed, 0 failed, 1 skipped), GCC Coverage test assertions PASS but **overall collection FAIL** because native Windows scanned `src\\tack.c` mismatches normalized gcov ID `src/tack.c`. No current measurement was produced. Owner-approved test inventory blob `61c0eb04778fb1f0f9908af48f1191fa71a50473` is versioned; both HWMs unchanged. RC0.7b only normalizes comparison identities and regression-tests missing/malformed source mappings, not actual filesystem paths; **NOT RUN**. Separately, `-j` is silently unused by `tack test` and needs a dedicated M2 parallel compile/link phase without concurrently running shared-fixture tests. Linux gates pending.

### English – M2 RC0.8 larger coherent batch candidate

RC0.8 combines the already untested RC0.7b Windows gcov canonical source identity correction with a fix to a double-backslash regression test literal, and introduces bounded, native-process parallel test compile/link for `test -j N`. It retains serial test execution due shared fixtures and defers gcov collection until all test children exit. The existing `-j 1` path remains serial. A new embedded C89 regression covers `-j 1/4`, an actual multiple-pending-compiler measure, incremental no-rebuild, compile-failure drain, and GCC two-stage gcov object/link jobs isolated under build/. Only historical RC0.7 Windows TinyCC/GCC/Clang runtime passes are available; RC0.7 coverage collection FAILED and **all RC0.8 gates including Linux are NOT RUN**. New test inventory owner approval is REQUIRED; both HWMs unchanged. No claim of >=90% attainment, no milestone closure.

### English: RC0.8 Owner Windows actual run (20 Nov 2026)

Commit `712ba87` passed strict GCC C89 bootstrap and `git diff --check`; Owner explicitly approved the 29-test inventory, exact Git blob `3a800818492bc8ce07762a79b13f36c20073a31c`. All Windows `-j4` suites passed their assertions: GCC Release 28/0/1, TinyCC Debug 27/0/2, Clang/MSVC Release 28/0/1, GCC/gcov tests 28/0/1, all 29 compiled. Actual Windows gcov `4036/6233 = 64.75213%`, `source_state=2e423671`, 29 tests, 1 source; collection PASS, while **overall HWM gate is BLOCKED** pending a separate explicit Owner `coverage accept --owner-approve`. Accepted prior Windows 3845/6075 and Linux 3604/5790 HWMs were not changed. Integrated Linux GCC/Clang/gcov and a full `-j1` vs. `-j4` speed comparison remain NOT RUN. Untracked local file `tatus --short --branch` must be inspected before deletion; no automated cleanup. PR remains Draft; M2 not accepted.
