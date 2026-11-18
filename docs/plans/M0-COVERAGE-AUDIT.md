# tack v0.8.6 — M0 Coverage-Baseline und Requirement-Audit

**Stand:** 2026-11-17 | **Status:** REVIEW / NOT ACCEPTED | **Scope:** M0, keine produktive Implementierung.

## 1. Prüfbasis und Abgrenzung

- Repository: `M4SSD35TRUCT10N/tack`.
- Stabiler Release `v0.8.5`: annotierter Tag `4aca141070d582afd6f5a67659723bb771abf965`, Ziel-Commit `a5c630b2bab4f8545feb591d1e3c01bcee656f11`.
- Aktuelles `main`: `6bb19dc2a9aae576b1792a4315c6388644a9f6c7`; gegenüber Tag genau ein weiterer Commit, ausschließlich `README.md` (eine Zeile ersetzt).
- Remote-Branch `dev/v0.8.6`: am Prüftag **nicht vorhanden**. Branch-Erstellung ist nicht Bestandteil dieser Lieferung.
- v0.8.5 GitHub Release: bereits veröffentlicht am 2026-11-17 19:34:08 UTC. Angaben 'nicht veröffentlicht' in älteren Projektakten sind historisch und sollen durch datierten Nachtrag, nicht durch Änderung der Beleggeschichte, berichtigt werden.
- Gelesen: `AGENTS.md`, `CLAUDE.md`, `docs/agents/TASK-TEMPLATE.md`, `docs/agents/STATE.md`, `docs/plans/0.8.6-coverage.md`, `docs/guides/coverage-ratchet.md`, `docs/specs/0010`, `0011`, `0012`, `docs/releases/0.8.5.md`, `tack.ini`, `src/tack.c`, `coverage/*`, die Tests zu Pfadsicherheit, Pfadkombination, Coverage-Ratchet, Formatter, Smoke, Cache/Relink und Reporting; Repository-Baum.
- Vollständiges, mit seiner ursprünglichen SHA256SUMS.txt erfolgreich geprüftes Owner-Quellarchiv ist lokal vorhanden. Keine `.git`-Historie oder gcov-Rohdaten verfügbar. Eine erneute Ausführung der Plattform-Gates sowie eine definitive Liste ungedeckter Zeilen/Funktionen war daher **NOT RUN**. Bisherige Testzahlen stammen ausschließlich aus vorhandener Owner-Evidenz.
- Die beigefügte Notiz `CI-Architektur für tack.txt` beschreibt einen historischen `0.8.1-dev`-Vorschlag für Doc UX und ist keine v0.8.5-Coverage-Evidenz.


### Lokale Quellarchiv-Provenienz und reproduzierbare Abweichungen

- Wiederhergestellte Quelle: Google Drive `tack_v0.8.5_final_hwm_approved_source_complete.zip` (340557 Bytes, SHA-256 `b51fde0eb6e05150ef5a87157cf96d8b6b9ff287c9ddeddbc6ad617e602e8aa9`). 89 ursprüngliche Dateien inklusive `SHA256SUMS.txt`; **88/88** ursprüngliche Manifesteinträge bestehen `sha256sum -c`. `unzip -t` bestätigt Archivintegrität.
- Mit GitHub-Tag abgeglichen: `src/tack.c` (Git-Blob `82224740657cbf7327aaa122d83124e9f6d02dcf`), `tack.ini`, `AGENTS.md`, beide HWMs, Testinventar, Plan und `STATE.md` sind vor M0-Änderungen blob-identisch. Der Google-Drive-Quell-Snapshot ist nicht in allen Hilfsdateien identisch zum Tag: `.fossil-settings/ignore-glob` ist im Owner-ZIP 550, auf GitHub 488 Bytes; `update_tools.bat` liegt im Owner-ZIP zusätzlich. Dies wird **bewusst übernommen**, nicht stillschweigend mit dem GitHub-Tag gleichgesetzt.
- GitHub-`main` ersetzt genau die erste README-Überschrift (entfernt „finaler lokaler Release-Kandidat“); diese freigegebene Dokumentationsänderung wird als einziges README-Delta in M0 übernommen. Der `main`-Commit wurde nicht verändert.
- Versionskennzeichen und beide Coverage-Profile bleiben auf der stabilen v0.8.5-Referenz. Kein Programmcode, keine Tests, keine Ratchets geändert. `SHA256SUMS.txt` wird nur für dieses vollständige neue M0-Quell-ZIP aufgrund der geänderten Dokumentationsdateien frisch und vollständig neu berechnet; die Historie von v0.8.5 bleibt unangetastet.

## 2. Angenommene Referenzen (unverändert)

| Profil | Covered | Total | Exakter Anteil | 90%-Schwelle beim jetzigen Nenner | Zusätzliche ausgeführte Zeilen |
| --- | ---: | ---: | ---: | ---: | ---: |
| Windows, TDM-GCC/gcov 10.3.0 | 3577 | 5906 | 60,56553 % | 5316 | 1739 |
| Linux, GCC/gcov 14.2.0 | 3505 | 5710 | 61,38354 % | 5139 | 1634 |

Referenzen: `coverage/hwm-windows.txt` (`profile=windows-gcc-10.3.0-gcov-10.3.0-coverage-v1`), `coverage/hwm-linux.txt` (`profile=linux-gcc-14.2.0-gcov-14.2.0-coverage-v1`). Beide: `sources_hash=39ea236e`, `source_count=1`, `flags_state=36eb9e95`. Unterschiedliche `source_state` und `tests_state` sind Teil der jeweiligen getrennten OS-Nachweise und dürfen nicht vermischt werden. Keine HWM-Anhebung durch M0.

**Nennerdefinition:** ausführbare, instrumentierte Produktionszeilen unter `src/` und `include/`, jedoch nicht `include/tests/`, keine Test- und Buildartefakte. Die v0.8.5-Referenz hat eine Produktionsquelldatei (`src/tack.c`); der gesamte Quellbaum muss auch in Zukunft auf nicht gemessene neue Produktionsdateien kontrolliert werden. Profile, Flags, Compiler-Identitäten, Plattformen, Source- und Test-Fingerprints sind zu protokollieren.

**Historische Gates:** Windows GCC Release 26 Tests: 25 PASS / 1 SKIP; Windows TinyCC Debug: 24 PASS / 2 SKIP; Linux GCC Release: 25 PASS / 1 SKIP. Linux-HWM nach Annahme: `coverage check` PASS. Windows erneuter `coverage check` und `test coverage` nach HWM-Übernahme ist in der GitHub-Projektdokumentation noch als offen vermerkt; das jüngere Owner-CMD-Protokoll `Eingefügter Text(20261117-185021).txt` weist beides jedoch mit **PASS / Exit 0** nach: 3577/5906, 26 kompiliert, 25 PASS, 1 SKIP. Es handelt sich um historische Owner-Evidenz, nicht um einen in diesem M0 erneut ausgeführten Lauf (M0-Neumessung: NOT RUN).

## 3. Geprüfter Implementierungszuschnitt

`src/tack.c` umfasst im Tagstand ca. 11.299 Zeilen und enthält die Bereiche Pfad- und Zielvalidierung (`is_safe_simple_token`, `is_safe_relative_path`, `validate_target_path_value`), rekursives Scannen/Entfernen (`scan_dir_recursive_suffix_skip_depth`, `rm_rf_depth`, `rm_rf_contents_depth`), Prozesse (`proc_spawn_nowait`, `proc_wait`, `run_argv_wait`, Windows Formatter-`CreateProcessA`), INI (`split_list_tokens`, `ini_load_file`), Cache/Depfile/Relink (`cache_restore`, `cache_meta_valid`, `linkmeta_needs_relink_explain`), Coverage (`cov_read_gcov`, `cov_validate_source_scope`, `cov_compare`, `cov_fresh`, `cov_test_inventory`), Reporting (TAP/JUnit/Events) und BOM/SBOM/Doc/FMT.

Bestehende Pfadtests belegen statisch nur eine relativ kleine Testfläche: `path_safety_test.c` enthält 5 Assertions zu `id`, `bin`, `src`, gültigen Pfaden und bewusstem Unsafe-Opt-in; `path_join_test.c` enthält drei Windows-Fälle, unter Linux SKIP. Das ist **kein** Beleg für 100% Sicherheitsanforderungsdeckung. Aus den Quelltexten lässt sich keine gemessene noch fehlende Zeilenabdeckung ableiten.

`cov_compare` (v0.8.5, um Zeile 5688) vergleicht korrekt über Kreuzmultiplizierung und prüft Profil, OS, Compiler, Quellnamen-Hash, Quellanzahl und Flagzustand. Eine Änderung am ausführbaren Nenner innerhalb derselben Quellnamensmenge wird damit nicht als solche automatisch untersagt. Das ist kein nachgewiesener Defekt, aber ein zwingender manueller **Denominator-Review-Punkt**. Jede dokumentierte Verbesserung muss echte neue Assertions, eine unverfälschte Quellenmenge und eine reproduzierbare Messung nachweisen. `cov_fresh` prüft separat Source-/Test-/Flag-Fingerprints der aktuellen Messung; dieses Gate bleibt unangetastet.

## 4. Requirement-Test-Matrix: Methodik

Die zugehörige Datei `M0-REQUIREMENTS.tsv` enthält einen ersten, **noch nicht ownerfreigegebenen** Anforderungenkatalog mit IDs, Kritikalität, zugehörigen Produktionsfunktionen, vorhandenen Testankern und Lücken. Ein Testanker beweist keine ausreichende Assertion. Status `STATIC` bedeutet nur nachweisbare Quelltextzuordnung, `GAP` bedeutet fehlender oder unzureichender nachgewiesener Szenario-Nachweis. Der Kandidatenkatalog umfasst **67 testbare Requirement-IDs**, darunter **57 zunächst als kritisch klassifizierte** IDs; 38 Quell-/Testanker sind als `STATIC`, 29 als `GAP` markiert. Diese Einstufung ist noch nicht ownerfreigegeben. Ausführung und zielgerichtetes Assertion-Review stehen für alle Zeilen aus. Deshalb wird **kein aktueller Prozentsatz** der Anforderungsabdeckung behauptet.

Abnahmedefinition: Eine Anforderung gilt als abgedeckt, wenn überprüfbare, bei Vertragsbruch fehlschlagende positive/negative/Grenz-/Fehlerfall-Assertions für die erforderlichen Plattformen vorhanden sind und tatsächlich ausgeführt wurden. `PASS` erfordert einen reproduzierbaren Lauf, `SKIP` zählt nur in sachlich und plattformspezifisch begründeten Ausnahmefällen, `NOT RUN` ist nicht abgedeckt. Aggregation: `abgedeckte prüfbare IDs / alle freigegebenen prüfbaren IDs >= 90 %`; für alle als kritisch klassifizierten Sicherheits- und Integritäts-IDs **100 %**. Unklare, duplizierte oder nicht prüfbare Einträge müssen im Owner-Review ausdrücklich begründet und dürfen nicht zur Prozentoptimierung entfernt werden.

## 5. Milestone-Plan für 0.8.6

| Milestone | Inhalt | Messbare Abnahme und Schwerpunkt |
| --- | --- | --- |
| M0 | Baseline, Requirement-Audit, planbare M1-Grenze | Referenzen und Befunde nachvollziehbar; Matrix als Review-Kandidat; keine HWM- oder Codeänderung |
| M1 | Pfad- und Dateisystemsicherheit | Negative/Boundary/CLI-E2E-Tests für Traversal, absolute/UNC/Drive-Pfade, Separatoren, Längen, Symlinks/Junctions und Cleanup; kritische S-IDs vollständig mit echten Assertions unter erforderlichen OS |
| M2 | Prozessausführung | `_spawnvp`, `fork/execvp`, `CreateProcessA`, Exitcodes, Quoting, Kindprozess-Fehler, fehlende Executables, keine Shell-Injektion |
| M3 | INI/CLI-Parsergrenzen | Überlänge, Quotes, Listen-Limits, fehlerhafte Sektionen/Werte, `--strict`, Flags/Compiler-Priorität und Inkrementalitätsregeln |
| M4 | Cache/Depfile/Relink | Metadaten-Manipulation, atomare/fehlgeschlagene Updates, Header-Änderung, Cache-Kollision, kaputte Dateien, veraltete Link-Ergebnisse |
| M5 | Coverage-Gate/Testinventar | gcov-Parsergrenzen, echte Produktionsquellenmenge, Stale/Profil-Fail-closed, Ratio/Denominator-Nachweis, Ratchet-Fingerprints |
| M6 | Reporting | TAP/JUnit/JSONL gültige und ungültige Ausgabe, Escaping, Fehlerabbruch, Exitcode, Schreibfehler, deterministische Statuswerte |
| M7 | BOM/SBOM/Doc/FMT und Artefaktintegrität | Vollständigkeit, Ausgabevalidierung, deterministic/fail-closed, Quell-ZIP- und SHA-256-Manifest-Prüfung; README-Drift gezielt beheben |
| M8 | Release-Qualifizierung | getrennte >=90% gcov-Messungen, >=90% Anforderungsdeckung, 100% kritische IDs, GCC/TinyCC- und Strict-C89-Gates, Review aller Prozent-/Nenneränderungen |

**Milestone-Grenze:** Ein Auftrag enthält genau einen Milestone. Eine behauptete neue Coverage-HWM ist bis zur expliziten Owner-Abnahme `BLOCKED` und darf weder durch Testabschwächung noch durch Denominator-Manipulation als PASS behandelt werden. Fortschritt wird als absolute Covered/Total-Zahlen, Differenz zum vorherigen Profil und neue/freigegebene Anforderungen dokumentiert; keine verbindliche Prozentsteigerung pro Einzelmilestone wird vorab erfunden.

## 6. M1, konkreter abnahmefähiger Folgeauftrag

**Version/Branch:** `v0.8.6`, vorgesehen `dev/v0.8.6` (vor Start noch vom Owner anzulegen oder explizit freizugeben). **Scope:** ausschließlich Pfadprüfung und Dateisystemsicherheit. **Keine neuen abhängigen Buildtools**, kein Umbau generischer Prozesse, INI-/Formatter-Implementierung, kein Coverage-HWM-Accept.

1. Ausgangsref und Arbeitsbaum festhalten; `README.md`-Delta zwischen Tag und `main` bewusst entscheiden. Historische HWM und Testinventar bytegleich sichern.
2. Vor jeglicher Änderung auf Linux und Windows je einen vollständigen GCC/gcov-Lauf mit gcov-Zeilenzuordnung sichern (getrennte Logs/Artefakte). `gcc --version` / `gcov --version`, Flags, SHA der geprüften Quellen und die echte Quellenliste dokumentieren.
3. Bestehende `path_safety_test.c` und `path_join_test.c` prüfen; neue Testszenarien in separater, thematisch begrenzter Testquelle oder vorhandenen Tests anlegen, ohne Assertions zu entfernen: gültige relative Pfade, `.`/`..`-Segmente, `..` in Dateinamen, `//`, `\\`, absolute POSIX-Pfade, Drive-Präfixe, UNC, leere Werte, NUL-terminierte Maximalwerte, eine Einheit über der Grenze, sehr tiefe Verzeichnisse, symbolische Verknüpfungen/Junctions, Cleanup-Grenzen und explizites Opt-in.
4. **Sicherheitsinvariante:** Ohne ausdrückliches Unsafe-Opt-in kann eine durch `id/bin/src` deklarierte Pfadoperation das Projektverzeichnis nicht durch Traversal verlassen. `clean`/`clobber` und Directory-Scan dürfen Verknüpfungen in fremde Verzeichnisse nicht rekursiv verfolgen; fremde Sentinel-Dateien bleiben unberührt. Falls testbar notwendige Beträge fehlen, zuerst an den Tests aufzeigen, nicht Verhalten stillschweigend ändern.
5. Bevorzugt nur zusätzliche ISO-C89-Tests. Produktionscodeänderungen ausschließlich zur Behebung einer reproduzierbaren Anforderungsverletzung und nach erforderlicher Scope-/Sicherheitsfreigabe; der M1-Folgetask spezifiziert sie gesondert.
6. `ratchet tests` nach Erweiterung zunächst als erwartetes `BLOCKED` dokumentieren; Test-Diff und Fingerprint-Liste zur Owner-Abnahme vorlegen. Niemals `--owner-approve` eigenmächtig ausführen.
7. Alle Test-Gates für Linux GCC Release/Debug, Windows TDM-GCC Release und TinyCC Debug, strikt C89, echte CLI-E2E und gcov-Messung als PASS/FAIL/NOT RUN mit Logs protokollieren. Verbesserte Coverage-HWM ohne Freigabe = **BLOCKED**, nicht FAIL und nicht PASS.
8. Owner erhält vollständiges Quell-ZIP, SHA-256-Dateimanifest, unverfälschten Vergleich zu Ausgangsref, Conventional-Commit-Vorschlag und den Nachweis sämtlicher S-Requirements. Kein Commit/Push/Merge/Tag ohne separate explizite Zustimmung.

**Abnahmekriterium M1:** Alle S-IDs der ownerfreigegebenen M1-Matrix haben einen expliziten Testfall und nachweisbar bestandenes zuständiges Plattform-Gate; sämtliche bestehenden Assertions und Testquellen sind beibehalten; keine neu ungeschützten Pfade, keine stillen Trunkierungen; bestehende HWMs unverändert. Falls ein kritischer Test tatsächlich fehlschlägt, M1 = FAIL/BLOCKED bis gesonderter Fix und erneuter Lauf.

## 7. SHA-256SUMS und Release-Integrität

`SHA256SUMS.txt` enthält für `./README.md` derzeit den Eintrag `606cde2aca39a101940a80a1156fb686505af3e4cda4f87432091c317510609e`. Da sich `main` seit dem `v0.8.5`-Tag nur in README geändert hat, ist das Manifest für `main` mindestens aktualisierungsbedürftig. **Keine stille Regeneration an v0.8.5.** Für den final eingefrorenen v0.8.6-Quellstand und bereits für dieses vollständig neu gepackte M0-Quellarchiv (einschließlich README/STATE/Plan) werden alle Manifest-Einträge vor dem finalen ZIP vollständig neu berechnet, Dateien gegen SHA-256 geprüft und der Archivinhalt gegen den Manifestbestand abgeglichen. Selbstreferenz des Manifests eindeutig regeln; zusätzlich die ZIP-Datei extern mit SHA-256 auszeichnen.

## 8. Conventional-Commit-Vorschlag (nicht ausgeführt)

```text
docs(tack): audit v0.8.6 M0 coverage and requirements baseline

- Reconcile accepted Windows and Linux coverage HWMs against v0.8.5
- Document traceable requirement candidates and M1 security acceptance
- Update bilingual coverage plan and agent handoff without C89 edits
- Regenerate SHA-256 source manifest for the complete M0 archive

Signed-off-by: Enrico Lefass <enrico.lefass@mailbox.org>
```

**Hinweis:** Dies ist ausschließlich ein Entwurf. Weder Commit noch Tag, Push, Merge, Ratchet-Annahme oder HWM-Erhöhung wurde ausgeführt.

## 9. Gate-Status dieser M0-Prüfung

| Prüfung | Status | Beleg / Einschränkung |
| --- | --- | --- |
| Git-Release-/Branch-Metadaten | PASS | GitHub API gelesen; Entwicklungsbranch fehlt |
| HWM-Textdateien | PASS | Beide unverändert und unter getrennten Profilen vorliegend |
| Originales Owner-Quellarchiv SHA-256-Manifest | PASS | 88/88 ursprüngliche Dateieinträge stimmen; SHA256SUMS.txt wird nicht selbst geführt |
| README-Delta zu `main` | PASS | Nur erste Zeile übernommen; sonst keine Änderung an README |
| 0.8.6-Plan-Drift erkannt | PASS | v0.8.4-Zahlen / 25 Tests in aktuellem Plan |
| Testinventar-Zählung | PASS | 26 registrierte `_test.c`-Dateien |
| Quellfunktions-/Testzuordnung (statisch) | PASS | geprüfte Quelltexte und Testanker |
| Windows GCC Release **neu** | NOT RUN | kein Windows-Runner/kein vollständiges Checkout |
| Windows TinyCC Debug **neu** | NOT RUN | kein Windows-Runner |
| Windows Coverage-Post-HWM-Nachtest (historisch) | PASS | Jüngeres Owner-CMD-Protokoll: `coverage check` Exit 0, erneuter instrumentierter Lauf Exit 0, 3577/5906 |
| Windows Coverage-Neumessung während M0 | NOT RUN | kein Windows-Runner/keine frischen gcov-Rohdaten in dieser Sitzung |
| Linux GCC/gcov **neu** | NOT RUN | kein lokal ausführbares Quellcheckout und keine Rohdaten |
| Requirement-Matrix freigegeben / Prozent berechnet | NOT RUN | Owner-Review und Assertions/E2E nötig |
| Kritische Sicherheitsrequirements voll nachgewiesen | NOT RUN | M1–M7 |
| HWM-Anhebung | NOT RUN | ausdrücklich nicht autorisiert |
| Neuer Git-Commit/Push/Tag | NOT RUN | ausdrücklich nicht autorisiert |
| Vollständiges Quell-ZIP erstellt | PASS | restauriertes 0.8.5-Owner-Archiv; M0-Dokumente integriert; SHA-256-Manifest und Archiv geprüft |

**M0-Abnahmeempfehlung:** vollständiges M0-Quell-ZIP mit aktualisiertem Plan und vollständiger `docs/agents/STATE.md`, neuer Matrix, rechnerisch gültigem SHA-256-Dateimanifest und Archivintegritätsprüfung liegt zur Owner-Abnahme vor. Die fehlende Remote-Entwicklungsbranch, noch fehlende neue Referenzläufe und nicht freigegebene Requirement-Matrix sind transparent dokumentiert; sie dürfen nicht als PASS für M1 oder 90%-Release gewertet werden.

*It's done when it's done.*
