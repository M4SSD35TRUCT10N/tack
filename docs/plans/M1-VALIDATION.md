> **M1 RC0.4, neuer Implementierungskandidat 18.11.2026:** Nach Owner-Abnahme von RC0.3 (Windows HWM **3708/5996**, Linux HWM **3604/5790**, beide PASS; Windows-CLI-Opt-in **2/0/0/2 PASS**) wurde bei kritischem S15 ein tatsächlicher Win32-Enumerationsrandfall entdeckt. `rm_rf_depth` unterschied einen fehlgeschlagenen `FindFirstFileA`-Aufruf bislang nicht von einem leeren Verzeichnis. RC0.4 verweigert bei unerwarteten Fehlercodes jetzt die Löschung und ergänzt deterministische `FindFirstFileA`-/`FindNextFileA`-Fehlerinjektion im bestehenden M1-Test (Scanner, `rm_rf_contents`, `rm_rf_depth`, Sentinel). **Linux GCC/Clang C89 und M1-Einzeltest PASS; Windows GCC/TinyCC mit RC0.4 NOT RUN.** Da Test- und Produktionsquelle geändert sind, sind das RC0.3-Testinventar und beide HWM-Dateien absichtlich nicht neu angenommen. **Owner-Ratchet-Review und neue beidseitige Coverage-Messung erforderlich.** S15 erst nach Windows-Nachtest PASS, M1 noch offen. TOCTOU bleibt dokumentiertes Restrisiko. **Keine Git-Schreiboperation.** Siehe `docs/plans/M1-RC0.4-S15-EVIDENCE.md`. Alle nachfolgenden RC0.3-Zustandsangaben sind historische Evidenz.

# tack v0.8.6 M1 RC0.3 – Owner-HWM-Freigaben und Windows-CLI-E2E (18.11.2026)

**Aktuell und verbindlich:** Windows **3.708/5.996 (61,84123 %)** und Linux **3.604/5.790 (62,24525 %)** sind vom Owner ausdrücklich freigegebene Coverage-HWMs. Auf Linux wurde die Annahme am 18.11.2026 in der unprivilegierten Referenzumgebung mit `coverage accept --owner-approve` ausgeführt, anschließend `coverage check` **PASS HWM retained: 3604/5790**; die Datei `coverage/hwm-linux.txt` ist exakt bytegleich zur akzeptierten Messung. Windows hatte bereits `coverage check` **PASS HWM retained: 3708/5996**. Testinventar 27 Quellen, beide Plattformen GCC Release **26 PASS / 1 SKIP**, Windows TinyCC Debug **26 PASS / 1 SKIP**. Kein Testausfall.

**Windows CLI S09:** Der Owner hat vier echte `list`-Aufrufe mit isolierten INI-Dateien ausgeführt: Safe-Default **Exit 2**, explizites `--unsafe-paths` **Exit 0**, `allow_unsafe_paths = yes` **Exit 0**, Rückkehr zu Safe-Default **Exit 2**. S03 (Windows Traversal-CLI) und S09 sind damit **PASS**; Primärnachweis: [M1-WINDOWS-CLI-EVIDENCE.txt](M1-WINDOWS-CLI-EVIDENCE.txt). Linux S09 war bereits in der CLI-E2E geprüft.

**Offener kritischer Nachweis:** **S15 ist unter Windows nur TEILWEISE** abgenommen. `ERROR_SHARING_VIOLATION` und Fail-fast sind belegt, aber ein erzwungener, reproduzierbarer `FindNextFileA`-/Verzeichnisaufzählungsfehler ist noch nicht nachgewiesen. M1 darf daher noch **nicht** als vollständig abgeschlossen oder mit 100 % kritischer Abdeckung bezeichnet werden. TOCTOU bei konkurrierendem Linktausch bleibt als dokumentiertes, außerhalb der M1-Race-Free-Garantie liegendes Restrisiko offen. Kein Git-Commit, Push, Merge oder Tag durchgeführt.

**Source scope:** `src/tack.c` und `tests/path_filesystem_safety_test.c` weiterhin bytegleich zum geprüften RC0.3. Nur die freigegebene Linux-HWM, die neuen CLI-Nachweise und Statusdokumentation sind gegenüber dem vorigen Acceptance Candidate geändert. Keine Coverage-Regressionsausnahme, kein HWM-Absenken und keine neuen Build-Abhängigkeiten.

## Historische Abnahmeakte vor der Linux-HWM-Freigabe und dem Windows-CLI-Nachweis

Der nachfolgende Bericht hält die damaligen Teststände und offenen Freigaben als **historische Evidenz** fest. Bei abweichenden Statusaussagen gilt das aktuelle Update oben; die ursprünglichen Testprotokolle sind nicht rückwirkend als PASS umgeschrieben.

---

# tack v0.8.6 M1 RC0.3 — Konsolidierte Abnahme / Consolidated acceptance

**Stand:** 18.11.2026. **Windows:** Owner-abgenommen (einschließlich HWM). **Linux:** Release und vollständige Coverage-Messung ohne Root-Rechte vorhanden; **höhere Linux-HWM noch BLOCKED, nicht ownerangenommen**. **Gesamtmilestone:** noch nicht vollständig freigegeben; kein Git-Commit/Push/Merge/Tag und keine Versionsanhebung. Diese aktuelle Zusammenfassung hat Vorrang vor den zeitlich älteren RC0.1–RC0.3-Protokollen weiter unten.

## Deutsch: reproduzierbare Ergebnisse

| Gate | Windows / Owner | Linux / unabhängige Prüfung |
| --- | --- | --- |
| Quellstand | RC0.3, `src/tack.c` unverändert seit RC0.1, Testfixture mit C89-`memcpy` | dieselben `src/tack.c`- und `path_filesystem_safety_test.c`-Bytes wie RC0.3 (SHA-256 verifiziert) |
| Compiler | TDM-GCC 10.3.0 Release, TinyCC 0.9.27 Debug | GCC 14.2.0 Release und gcov 14.2.0; striktes C89 GCC/Clang |
| GCC-Release-Tests | **PASS**: 27 kompiliert, 26 PASS, 1 SKIP, 0 FAIL, Exit 0 | **PASS**: 27 kompiliert, 26 PASS, 1 SKIP, 0 FAIL |
| TinyCC Debug | **PASS**: 27 kompiliert, 26 PASS, 1 SKIP, 0 FAIL | nicht anwendbar |
| Testinventar-Ratchet | **PASS**: Owner nahm den RC0.3-Testfingerprint ausdrücklich mit `ratchet tests --owner-approve` an | **PASS**: isoliertes Testinventar mit 27 Quellen, ohne neue HWM-Freigabe |
| Symlink- und Negativtests | **PASS**: GCC/TinyCC, Root- und Zwischenkomponenten, gesperrte Datei, Sentinel erhalten; keine Symlink-Fixture-SKIPs | **PASS**: POSIX-Symlink-, Cleanup-, Permissions- und Bounds-Test sowie CLI-E2E aus M1 |
| Reale NTFS-Junction beim Scan | **PASS**: `--scan-root-link build\\m1_junction_link` Exit **2**, externer Sentinel erhalten | nicht anwendbar |
| Reale NTFS-Junction bei `clean`/`clobber` | **PASS**: isolierter `%TEMP%`-Baum; beide Exit **0**; Junction entfernt, `build` bei `clobber` gelöscht; externer Sentinel jeweils erhalten | nicht anwendbar |
| Coverage-Messung GCC/gcov | **3.708/5.996 = 61,84123 %**, 27 Tests, 26 PASS, 1 SKIP | **3.604/5.790 = 62,24525 %**, 27 Tests, 26 PASS, 1 SKIP |
| Coverage-HWM-Gate | **PASS**, Owner nahm Windows-HWM **3.708/5.996** an | **BLOCKED**, akzeptierte Linux-HWM **3.505/5.710** unverändert; `coverage check` Exit 2 |
| Git-Diff-/Build-Verzeichnis | `git diff --check` PASS (nur LF/CRLF-Warnungen), keine gcov-Dateien im Projektroot | im isolierten Prüfverzeichnis keine gcov-Dateien im Projektroot |

### Coverage-Fingerprints und Delta

- **Windows:** `profile=windows-gcc-10.3.0-gcov-10.3.0-coverage-v1`, `sources_hash=39ea236e`, `source_count=1`, `flags_state=36eb9e95`, `source_state=811c9dc5`, `tests_state=f4993779`; vom Owner akzeptierte HWM identisch zur aktuellen Messung. Gegenüber 3.577/5.906: **+131 ausgeführte / +90 ausführbare Zeilen**, +1,27570 Prozentpunkte.
- **Linux:** `profile=linux-gcc-14.2.0-gcov-14.2.0-coverage-v1`, `sources_hash=39ea236e`, `source_count=1`, `flags_state=36eb9e95`, `source_state=6cf1bb03`, `tests_state=142b4c94`. Gegenüber akzeptierter HWM 3.505/5.710: **+99 ausgeführte / +80 ausführbare Zeilen**, +0,86171 Prozentpunkte. Neue Messung unter `coverage/current.txt` im isolierten Prüflauf; **nicht** nach `coverage/hwm-linux.txt` übertragen.
- **Linux privilege check:** An earlier *root* run produced 3.598/5.790 (62,14162 %) but explicitly **SKIP**ped the POSIX EACCES fixture. The final independent run under unprivileged user `oai` executed that permission-denied path and produced **3.604/5.790 (62,24525 %)**. Both runs used the same source/test bytes and the same GCC/gcov profile; the latter is the current proposal. Neither was accepted as a Linux HWM.
- **Messprotokoll:** [M1-LINUX-EVIDENCE.txt](M1-LINUX-EVIDENCE.txt) enthält die genaue unprivilegierte Messmetadaten und Prüfsummen der unveränderten C89-Quellen.
- Unterschiede in `source_state` und `tests_state` auf Linux sind durch den M1-Quell-/Teststand zu erwarten und dürfen nicht verschwiegen werden. Gleiche Quelleinbeziehung (`sources_hash`, `source_count`), Compiler-/Profilkennung und Flags sind nachgewiesen. Eine HWM-Annahme bleibt eine gesonderte Owner-Entscheidung.
- Die scheinbaren `FAIL`-/`BLOCKED`-Zeilen **innerhalb** `coverage_ratchet_test` sind erwartete Negativassertions (`coverage_ratchet_test: ok`). Der **abschließende** Linux-Coverage-Summary-Status `failed`/Exit 2 ist dagegen die **echte HWM-Sperre** trotz `26 PASS / 1 SKIP`.

### Anforderungen, Einschränkungen und Freigabegrenzen

- Die 16 kritischen Kandidaten **S01–S16** werden in `M1-REQUIREMENTS.tsv` zu den tatsächlichen Assertions und E2E-Nachweisen zurückverfolgt. Ein bestandener Unit-Test beweist keine vollständige Fehler-Injektion für alle OS-API-Pfade. Windows-CLI-Opt-in-E2E und erzwungene Windows-Verzeichnisaufzählungsfehler sind aus den vorliegenden Owner-Protokollen **nicht vollständig belegt** und sind im Trace entsprechend als **TEILWEISE** markiert; sie werden nicht zu PASS umetikettiert. Eine formale 100%-Freigabe sämtlicher kritischer Requirements steht daher noch aus.
- Die Implementierung prüft Pfadkomponenten **vor** Scan/Löschung; konkurrierende Link-Swaps (**TOCTOU**) sind ohne handle-/dirfd-basierte Traversierung nicht ausgeschlossen. Das ist ein dokumentiertes Sicherheitsrestrisiko und liegt außerhalb des beauftragten M1-Scopes.
- Das akzeptierte Windows-`coverage/hwm-windows.txt` gehört gemeinsam mit `coverage/test-inventory.txt`, Quellcode, Spezifikation und Abnahmeakte in **denselben später freigegebenen Commit**. Die Linux-HWM darf nicht stillschweigend angehoben oder herabgesetzt werden. `coverage/current.txt`, Build-Dateien und gcov-Artefakte sind nicht zu versionieren.
- `update_tools.bat` liegt unverändert im Quell-ZIP; sie ist durch `*.bat` in `.gitignore` weiterhin untracked. Kein automatisches `git add -f`.
- Stand der Branches gemäß Owner Windows `git status`: `feature/v0.8.6-m1-path-safety` mit lokalen, noch nicht committeten Änderungen. **Keine Git-Schreiboperation durch den Assistenten.** `TACK_VERSION` bleibt bis M8 bei 0.8.5.

### Nächste Owner-Gates

1. Linux-Messung 3.604/5.790 und ihre Fingerprints prüfen; **nur bei ausdrücklicher Freigabe** auf der Linux-Referenz `tack --config ./tack.ini coverage accept --owner-approve`, danach `coverage check` und neue instrumentierte Tests mit PASS. Das aus Linux angenommene `coverage/hwm-linux.txt` im selben späteren Git-Commit mitführen.
2. Noch nicht lückenlos nachgewiesene kritische Windows-Requirements (siehe Trace) explizit prüfen oder den nachgewiesenen Umfang mit Owner-Entscheid dokumentieren. Keine stillschweigende Verschiebung von Sicherheitskriterien.
3. Erst nach diesen Gates M1 als abgeschlossen markieren; vollständiges Quellpaket/SHA-256 prüfen und Git-Commit/Push auf den Feature-Branch nur mit ausdrücklicher Owner-Freigabe. `dev/v0.8.6` erst danach per Review integrieren; kein Merge/Tag ohne eigene Freigabe.

## English: consolidated status

Windows RC0.3 is **Owner-accepted**: GCC release and TinyCC debug each compiled 27 tests with 26 PASS, 1 SKIP and zero failures; real NTFS junction scan returned exit 2 with the external sentinel intact, and isolated `clean`/`clobber` each returned exit 0 while removing the junction without touching its external target. The Windows coverage HWM was explicitly raised and rechecked to **3708/5996 (61.84123%)**. The final Linux GCC/gcov 14.2.0 run compiled 27 tests, with 26 PASS, 1 SKIP, zero test failures, and measured **3604/5790 (62.24525%)**. Its final CI status is **BLOCKED** solely because the improved Linux HWM has not been owner-approved. The old Linux HWM **3505/5710** is preserved. Remaining requirement-trace gaps and the explicit TOCTOU limitation preclude a full M1 security sign-off. No Git writes or Linux HWM acceptance have been performed.

---

## Historische RC-Berichte / Historical RC records (superseded by current summary)

# tack v0.8.6 M1 RC0.3 — Win32 GCC/TinyCC callback portability

**Status:** Reparaturkandidat; Owner-Windows-Gates erneut ausstehend (18.11.2026). Die RC0.2- und RC0.1-Berichte bleiben unten als historische Evidenz erhalten.

## Deutsch: RC0.3-Delta

- **RC0.2 GCC-Fehler:** TDM-GCC 10.3 unter `-Werror=cast-function-type` verweigert die Umwandlung `FARPROC` -> `FsCreateSymbolicLinkFn` in der neuen Windows-Testfixture. `test release` und `test coverage` scheiterten beide am **Compile** des 22. Tests: 27 total, 21 kompiliert, 19 PASS, 1 FAIL, 1 SKIP, 6 NOT RUN. Die Coverage-Pipeline entfernte die ungültige `coverage/current.txt` korrekt; **keine neue gemessene RC0.2-Coverage**. Die akzeptierte Windows-HWM blieb 3577/5906.
- **TinyCC RC0.2:** Owner bestätigte direkten neuen Test und die offizielle Debug-Suite jeweils PASS; Gesamtergebnis 27 kompiliert, 26 PASS, 1 SKIP, 0 FAIL. Die vier Symlink-Fixtures und Windows-Dateisperren-Prüfung liefen ohne Fixture-SKIPs. Testinventar mit 27 Quellen wurde vom Owner für den RC0.2-Test erneut freigegeben.
- **RC0.3-Fix:** Nur `tests/path_filesystem_safety_test.c` geändert. `GetProcAddress` bleibt; der unzulässige Funktionszeiger-Cast wird durch `sizeof`-Prüfung plus ISO-C89-`memcpy` der Funktionszeiger-Objektrepräsentation ersetzt. Beide sind Win32-Funktionszeiger; die Größenprüfung verhindert unbemerkte abweichende ABI-Annahmen. Kein `src/tack.c`-Verhaltenswechsel, keine neuen Bibliotheken und kein Absenken von Compilerwarnungen. Der Test-Quellfingerprint verändert sich **erneut** und muss nach Owner-Prüfung im Ratchet angenommen werden.
- **Lokal unabhängig geprüft:** Linux GCC/Clang strikt C89 (aktualisierter M1-Test), direkte Linux-Ausführung, zusätzlich isolierte Compilerprobe für die unter Windows aktive `memcpy`-Funktionszeiger-Kopie. Diese Gegenprobe ersetzt **keinen** echten Windows-GCC-/TinyCC-Lauf.
- **Noch offen:** Windows GCC Release/Coverage und TinyCC Debug mit RC0.3; exakter NTFS-Junction-Nachweis; Linux-Coverage; Owner-HWM-Abnahme nach frischer vergleichbarer Messung; TOCTOU als bekanntes Restrisiko. Keine Git- oder HWM-Operation durchgeführt.

## English: RC0.3

RC0.2 resolved TinyCC's missing `CreateSymbolicLinkA` declaration, but introduced an incompatible function-pointer cast rejected by Windows GCC under `-Werror=cast-function-type`. Both RC0.2 GCC gates failed before coverage measurement, and `coverage/current.txt` was correctly absent. RC0.3 changes only the Windows test helper to use a size-checked C89 `memcpy` between Win32 function-pointer object representations, retaining dynamic API lookup and all assertions. The test inventory requires renewed explicit owner review because its checksum changed. Historical Windows RC0.2 TinyCC debug succeeded (26 PASS, 1 SKIP); RC0.3 Windows gates, independent Linux coverage and real NTFS junction fixtures are NOT RUN. No HWM, production code or release status change.

---

# tack v0.8.6 M1 RC0.2 — TinyCC-Testfixture-Fix / Test fixture portability

**Status:** Kandidat, Windows-TinyCC-Retest und erneute Owner-Ratchet-Annahme ausstehend (2026-11-18). Die nachfolgende RC0.1-Akte bleibt als historische Evidenz erhalten und beschreibt ältere Prüfstände.

## Deutsch: RC0.2-Delta und Owner-Protokoll

- **Ursache:** TinyCC 0.9.27 Win64 meldet beim direkten Aufruf von `CreateSymbolicLinkA` in `tests/path_filesystem_safety_test.c` eine implizite Funktionsdeklaration, wodurch `test debug --ci --rebuild` fehlschlägt. Kein nachgewiesener Fehler in `src/tack.c`.
- **Abhilfe:** Windows-Link-Fixtures ermitteln den dokumentierten Kernel32-Export zur Laufzeit via `GetModuleHandleA`/`GetProcAddress`; Aufruf über ISO-C89-konformen Funktionszeigertyp mit Windows-Aufrufkonvention. Flags `3/1` (Verzeichnis) und `2/0` (Datei), alle bestehenden Assertions und SKIP-Nachweise bleiben. Kein Shell-Aufruf, keine neue Bibliothek.
- **Owner Windows RC0.1:** GCC Release 27 kompiliert, 26 PASS, 1 SKIP, 0 FAIL. GCC Coverage führte alle 27 Tests aus (26 PASS, 1 SKIP), **3704/5996 (61,77452 %)** gemessen; Abschluss **BLOCKED** wegen verbesserter Coverage über akzeptierter Windows-HWM **3577/5906**. Der Nenner ist um 90 ausführbare Produktionszeilen, der Zähler um 127 angewachsen; `sources_hash=39ea236e`, `source_count=1`, `flags_state=36eb9e95` und Profil sind unverändert. Linux-HWM **3505/5710** unberührt.
- **Owner-Test-Ratchet:** RC0.1 mit 27 Testquellen ausdrücklich akzeptiert; freigegebene Inventarzeile `tests/path_filesystem_safety_test.c	66fbfcad`. RC0.2 ändert die Testquelldatei und muss daher **erneut** vom Owner geprüft/akzeptiert werden. Das alte akzeptierte Inventar ist bewusst im Paket enthalten, statt heimlich zu aktualisieren.
- **Owner TinyCC Debug RC0.1:** 27 vorgesehen; 21 kompiliert, 19 PASS, 1 FAIL, 1 SKIP, 6 NOT RUN, Fehler am fehlenden `CreateSymbolicLinkA`-Prototyp. Das ist **nicht PASS**. RC0.2 Windows-TinyCC: **NOT RUN**, Owner-Retest erforderlich.
- **Lokale RC0.2-Prüfung:** Linux GCC/Clang striktes ISO C89 und direkter M1-Test PASS; Windows TinyCC, vollständige offizielle Windows/Linux-Gates, aktuelle Linux-Coverage sowie echter NTFS-Junction-Test **NOT RUN**. Die bereits vom Owner bereinigte Leerzeichenzeile 945 in `src/tack.c` ist im Paket korrigiert.
- **Offen:** Owner-Diff-Review → `ratchet tests --owner-approve` → TinyCC Debug → GCC Release/Coverage → nach Vergleich gesonderte Coverage-HWM-Freigabe. Kein Commit, Push, Merge oder Tag; keine HWM-Datei geändert. TOCTOU zwischen Pfadprüfung und Dateioperation bleibt bekannt.

## English: RC0.2 delta and validation status

TinyCC 0.9.27 Windows headers omit the `CreateSymbolicLinkA` declaration; RC0.2 resolves this test-only Win32 API dynamically instead of relying on the prototype or static import. The Windows GCC RC0.1 release and coverage suite reached 26 PASS/1 SKIP of 27 tests, with improved coverage 3704/5996 and HWM acceptance correctly blocked. The prior 27-test inventory was owner-approved; the changed RC0.2 test requires fresh owner review and ratchet approval. Linux GCC/Clang strict C89 and the direct M1 test passed for RC0.2; Windows TinyCC, full cross-platform gates, updated Linux coverage and NTFS junction tests are NOT RUN. Accepted HWMs remain unchanged; no Git or release operations performed.

---

# tack v0.8.6 M1 RC0.1 — Sicherheitsabnahme / Validation record

**Status:** Implementation candidate / OWNER REVIEW REQUIRED. Ausgeführte Prüfungen beziehen sich auf die RC0.1-Quellen mit dem neuen Test, **nicht** auf einen ownerangenommenen Test-Ratchet-Stand. Datum 2026-11-18.

## Deutsch

### Nachvollziehbare Ausgangslage

- `main`, `dev/v0.8.6`, `feature/v0.8.6-m1-path-safety`: Owner hat alle drei auf M0-Commit `1b0d3a0` angelegt bzw. synchronisiert; Feature-Branch laut Protokoll sauber. **Kein** Agenten-Commit/-Push.
- Vor M1 hat der Owner Windows GCC/gcov TDM 10.3.0 erfolgreich erneut gemessen: `test release` 25 PASS/1 SKIP, `test coverage` 25 PASS/1 SKIP, **3577/5906 (60,56553 %)**, HWM-Check PASS, 0 gcno/gcda/gcov im Projektroot. `where tack` zeigte die installierte 0.8.5. Diese Werte sind v0.8.5-Ausgangsevidenz, **keine Windows-M1-Messung**.
- Unveränderte akzeptierte HWMs: Windows **3577/5906**, Linux **3505/5710**. Neue Anforderungen erhöhen nicht künstlich den Nenner; kein HWM-Accept und kein Test-Ratchet-Accept im RC0.1.

### Implementierte M1-Änderungen

- `src/tack.c`: Portable, strengere Targetpfade inklusive Windows-Gerätenamen/ADS-/Punkt-/Separatorkanten; overflow-sichere Joins; no-follow auf jeder Scan-Pfadkomponente und für Quell-Symlink-Dateien; fail-fast bei Zugriffs-/Aufzählungsfehler; Root-/Traversal-Abwehr und no-follow-Existenz bei rekursiver Löschung einschließlich hängender Symlinks; Windows-Junction-Löschung und korrektes Fehlersignal bei `FindFirstFileA`.
- `tests/path_filesystem_safety_test.c`: neues C89-Regressionsprogramm für S01–S16, inklusive separater Kinderprozesse für Exit-2-Fail-fast, sichere Positivfälle, explizites Opt-in, Symlink-/Sentinel-Fixtures, verweigerte POSIX-Rechte und optionalen Windows-Link-/Lock-Probes. Die bisherigen 26 `_test.c`-Dateien bleiben **bytegleich**.
- `docs/specs/0013-m1-path-filesystem-safety.md`, `docs/plans/M1-REQUIREMENTS.tsv`, diese Abnahmeakte und neuer `docs/agents/STATE.md`-Handoff.
- Die vom Owner korrigierte `update_tools.bat` ist im **Quell-ZIP** enthalten (Installation unter `C:\Users\enric\OneDrive\Dokumente\Tools`, Projekt unter `C:\Projects\tack`). Aufgrund `*.bat` in `.gitignore` ist sie ohne gesondertes `git add -f` **nicht** Teil eines normalen Git-Commits. Keine `.gitignore`-Scope-Erweiterung in M1.

### Ausgeführte Gates (Linux, nicht Windows)

| Gate | Status | Evidenz |
| --- | --- | --- |
| GCC 14.2.0 striktes ISO C89 / `-std=c89 -pedantic-errors -Wall -Wextra -Werror` | PASS | `src/tack.c` und **alle 27** `_test.c` einzeln kompiliert |
| Clang striktes ISO C89 | PASS | Produktion und neuer M1-Test kompiliert, M1-Test ausgeführt |
| M1 POSIX-Sicherheitstest | PASS | Positive/negative Pfade, Overflow-/Tiefe-Exit 2, externe Symlink-/Cleanup-Sentinels, Dangling-Link, Rechteverweigerung, Scan-Fehler |
| Individuelle Linux-Tests mit `TACK_CC=gcc` | PASS | **27 kompiliert, 26 PASS, 1 SKIP** (`path_join_test` ist Windows-only); keiner fehlgeschlagen |
| Echtes Linux-CLI-E2E (frisch gebautes tack) | PASS | `list` mit unsicherem src: Exit 2; CLI-/INI-Unsafe: je Exit 0; sicherer src: 0; Scan verlinktes src: 2; `clobber`: 0; externer Sentinel zweimal erhalten |
| Linux Release-`build` mit tack/GCC | PASS | `tack --config ./tack.ini build release --target app --rebuild -j 1`; Binary meldet absichtlich weiterhin `tack 0.8.5` |
| `tack ratchet tests` | **BLOCKED / erwartetes FAIL** | neues `_test.c` noch nicht ownerabgenommen; 26-Datei-Baseline absichtlich unverändert |
| Reguläres `tack test release --ci --rebuild` | BLOCKED | Ratchet verlangt zunächst Owner-Review; individueller GCC-Testlauf ist **kein** Ersatz für den offiziellen Gate-PASS |
| Reguläre Linux-Coverage, `coverage check`, aktuelle covered/total | NOT RUN | Ohne Ratchet-Annahme nicht vergleichbar/ausführbar; HWM bleibt 3505/5710 |
| Windows TDM-GCC Release/Debug, TinyCC Debug, Windows Symlink/Junction/Lock, Windows Coverage | NOT RUN | Owner muss RC0.1 auf Windows nachprüfen; alte v0.8.5-Erfolge nicht umetikettieren |
| Git-Merge/Tag/Push bzw. HWM-/Ratchet-Accept | NOT RUN | Owner-Gate |

### Owner-Abnahme und bekannte Einschränkungen

1. Den neuen Sicherheitstest und die Produktionsänderung anhand von SPEC/Trace prüfen. **Erst danach** `tack --config .\tack.ini ratchet tests --owner-approve` lokal selbst ausführen; dieser Befehl verändert `coverage/test-inventory.txt`. Keine automatische Annahme im Quellpaket.
2. Nach Annahme GCC/TinyCC-Tests, echte Windows-Junction-/Symlink-Sentinel-Tests und getrennte OS-Coverage vollständig ausführen. Neue Coverage-Werte samt Nenner-/Quellvergleich dokumentieren; höhere HWM nicht eigenmächtig übernehmen. Die 90%-Ziele für v0.8.6 sind damit keinesfalls schon erreicht.
3. **TOCTOU bleibt offen:** Ein lokaler Angreifer mit Schreibzugriff auf den Baum könnte Komponenten während des Scannens/Löschens ersetzen. Die Vorprüfung ist keine race-free Garantie. Ein zukünftiges Design muss handle-basiert sein. Kein pauschales Sicherheitszertifikat für produktive Löschung.
4. SHA256SUMS im ZIP wurde auf die **verpackten** Bytes berechnet; wenn Owner später das Testinventar annimmt oder das Arbeitsverzeichnis verändert, ist das Manifest **vor dem Commit** erneut zu erzeugen. Alte HWM-Dateien/Tests nicht durch vermeintlich erfolgreiche Prozentwerte ersetzen.

### Commit-Vorschlag

`test(tack): harden M1 path and filesystem safety`

- Reject nonportable and traversal-prone target paths
- Protect recursive scans and cleanup from symlink and junction traversal
- Add C89 boundary, permission and sentinel regression tests
- Document M1 evidence and owner-controlled ratchet gate

`git commit -s` erzeugt den `Signed-off-by`-Trailer aus der lokalen Git-Konfiguration. Kein Commit im RC0.1 ausgeführt.

## English

M1 RC0.1 is an implementation candidate, not an accepted ratchet or Windows release. The Owner's preceding 2026-11-18 Windows v0.8.5 measurement was 3577/5906, 25 PASS/1 SKIP with no root gcov files; it is baseline evidence only. This candidate changes one ISO C89 production file, adds **one** test source (the existing 26 remain byte-identical), and documents 16 candidate critical M1 requirements. Linux GCC14.2 strict C89 compiled all 27 test sources, individual execution with TACK_CC=gcc yielded **26 PASS/1 SKIP**, and Clang strict C89/M1 security plus real isolated CLI acceptance passed. The standard test ratchet **intentionally fails pending owner review**, so official test/coverage and any new HWMs are **not accepted**. Windows GCC/TinyCC, Junction/Symlink/lock E2Es and fresh Windows coverage are **NOT RUN**. Race-free TOCTOU protection remains out of scope; the preflight-only design must not be advertised as race-proof. No commit, push, merge, tag or acceptance performed.
