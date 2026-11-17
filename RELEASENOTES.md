# tack Release Notes / Changelog (DE/EN)

**Backlinks:** [README](README.md) • [FAQ](FAQ.md) • [Roadmap](ROADMAP.md) • [Docs](docs/README.md)

---

## Deutsch
### v0.8.5 finaler lokaler Release-Kandidat (HWM freigegeben, Veröffentlichung ausstehend)
- Windows RC0.7 vollständig bestätigt: GCC Release 25 PASS/1 SKIP, TinyCC Debug 24 PASS/2 SKIP, Ratchet/Docs PASS, echter clang-format-E2E mit Leerzeichen und Exitcodes 0/2/0/0/1/0 PASS.
- Linux GCC/Clang Strict C89, Release 25 PASS/1 SKIP, Ratchet/Docs/Formatter E2E PASS. Owner hat Windows 3577/5906 (60,56553 %) und Linux 3505/5710 (61,38354 %) zur HWM-Annahme freigegeben; Linux CLI-Annahme und Check PASS, Windows Post-Adoption-Check noch offen.
- Historische RC0.4–RC0.6-Prüfstände nach `docs/releases/archive/0.8.5/` verschoben. `docs/releases/0.8.5.md` ist der kanonische Bericht. Kein GitHub-Push/Merge/Tag ohne separate Freigabe.


### v0.8.5 RC0.7 (Release-Kandidat, NICHT FREIGEGEBEN)
- Korrekturen seit v0.8.4: Formatter-INI-Parser und externe Windows-Formatter-Argumentübergabe einschließlich `CreateProcessA` und abschließender NUL-Terminierung. `clang-format` bleibt optional.
- Owner-Nachweis aus RC0.6: Windows GCC Release 25 PASS/1 SKIP, TinyCC Debug 24 PASS/2 SKIP, isolierter Formatter-E2E mit Leerzeichen und unveränderten Originaldateien bei `--check` PASS.
- RC0.7 setzt die Versionsnummer auf 0.8.5; dokumentierte Fähigkeiten und Einschränkungen DE/EN abgeglichen. **Nicht veröffentlicht**; RC0.7-Linux-GCC/Clang, Release, Docs, Ratchet und Formatter-E2E PASS, Linux gcov 3505/5710 mit HWM BLOCKED. Windows-RC0.7-Gates und kontrollierte HWM-Annahme stehen aus.

### v0.8.5-dev RC0.6 (Formatter-Token-Terminierung, NICHT FREIGEGEBEN)
- Owner RC0.5 Windows GCC/TCC: `fmt_ini_parser_test` FAIL bei tatsächlichem Kindprozess mit abschließendem Backslash; TCC zudem eingebettetes Quote. Je 13 PASS, 1 FAIL, 12 NOT RUN; kein gültiger Windows-Coverage-Datensatz.
- Die Root-Cause ist die fehlende NUL-Terminierung in `fmt_replace_all_alloc` nach einem Platzhalter am Tokenende. Formatter-only, keine zusätzlichen Features oder Abhängigkeiten; sechs zusätzliche plattformübergreifende Regressionen, bisherige Windows-Child-Tests erhalten.
- Linux GCC/Clang C89 PASS, GCC Release 25 PASS/1 SKIP, Formatter E2E PASS, gcov 3505/5710 (61,38354 %) gemessen und HWM BLOCKED. Windows RC0.6 nicht getestet, HWMs unverändert. Status und CMD-Abnahme siehe `docs/releases/archive/0.8.5/0.8.5-dev-RC0.6-validation.md`.

### v0.8.5-dev RC0.5 (Windows-Formatter-Prozessstart, NICHT FREIGEGEBEN)
- Owner-RC0.4: Windows GCC Release **26 kompiliert, 25 PASS, 1 SKIP**; TinyCC Debug **26 kompiliert, 24 PASS, 2 SKIP**; Test-Ratchet und `tack doc --strict` PASS. GCC/gcov: **3459/5859 (59,03738 %)**, `coverage check` korrekt **BLOCKED**, nicht FAIL der funktionalen Tests.
- Tatsächlicher Windows-Formatter-E2E bleibt RC0.4-**FAIL**: LIST 0; CHECK_BEFORE 1 (statt 2); ORIGINAL_UNCHANGED 0; FORMAT 1 (statt 0); CHANGED 0 (statt 1); CHECK_AFTER 1 (statt 0). Dateinamen werden nicht mehr gespalten, enthalten aber beschädigte Suffixe. `clang-format 23.1.5` installiert. Das RC0.4-Quoting vor `_spawnvp` reicht nicht.
- Enger C89-Fixkandidat: **nur Formatter** nutzt unter Windows `CreateProcessA` mit selbst aufgebauter, begrenzter, NUL-terminierter Win32-Kommandozeile und echtem Wait/Exitcode; allgemeiner tack-Build-Runner bleibt unverändert. Keine Shell, keine neue Tool-Abhängigkeit. `fmt_ini_parser_test` erweitert um **drei echte Windows-Kindprozesse** mit Leerzeichen im Executable-Pfad und Leerzeichen/Quotes/abschließendem Backslash im Argument; bestehende fünf Quote-Assertions bleiben erhalten.
- Linux GCC und Clang Strict C89 BUILD PASS; GCC-Release: **26 kompiliert, 25 PASS, 1 SKIP**; Linux Formatter E2E in Ordner mit Leerzeichen PASS; Coverage frisch **3470/5709 (60,78122 %)**, HWM-Gate weiterhin **BLOCKED** bis Owner-Freigabe. Beide angenommenen HWM-Dateien unverändert.
- Windows RC0.5-Tests **NICHT AUSGEFÜHRT**. Git-SSH-Probe `ssh -T git@github.com` inzwischen PASS, `git fetch` noch nicht erfolgreich wiederholt. Programmversion weiter **0.8.4**; kein Git-Push, Merge oder Tag. Siehe `docs/releases/archive/0.8.5/0.8.5-dev-RC0.5-validation.md`. (Release Notes)

### v0.8.5-dev RC0.4 (Windows-Formatter-E2E-Fixkandidat, NICHT FREIGEGEBEN)
- Der reale RC0.3-Windows-Test deckte die Aufspaltung von Leerzeichen enthaltenden Argumentpfaden beim Formatterprozess auf. Nur dessen Windows-CRT-Argumentquoting wurde korrigiert, mit neuen C89-Regressionen.
- Linux GCC Release: 25 PASS, 1 SKIP; Linux externe Formatter-E2E: 0/2/0/0; neue Coverage 3470/5709, HWM-Übernahme BLOCKED. Beide angenommene HWMs unverändert.
- Windows RC0.4 E2E, TinyCC und GCC/gcov, Test-Ratchet-Owner-Review, Versionssprung und GitHub-Freigabe bleiben offen.

### v0.8.5-dev RC0.3 (Windows-HWM und externe Linux-Formatter-Prüfung, NICHT FREIGEGEBEN)
- Windows: TDM-GCC Release/TinyCC Debug 25 PASS, 1 SKIP; gemessene und angenommene HWM 3434/5827 (58,93256 %), erneuter Coverage-Gate PASS.
- Externer Formatter unter Linux mit Dateiname inklusive Leerzeichen end-to-end verifiziert; Windows externes Formatter-E2E noch offen.
- Linux 3470/5708 (60,79187 %) gemessen, Owner-HWM-Annahme noch ausstehend. Keine C89-Quell-/Teständerung und kein 0.8.5-Release.


> `v0.8.4` wurde als eigenständiges CLI-Werkzeug zur Veröffentlichung freigegeben. Die Integration in weitere Fremdprojekte (z. B. ft2-clone oder nuklear) ist weiterhin nicht als getestet zu betrachten. Tag und GitHub-Release sind erst nach dem Push nachzuweisen.

### v0.8.5-dev RC0.2 (Formatter-Parserkorrektur, NICHT FREIGEGEBEN)
- Die README erklärt Compilerprogramme und Compiler-Policy je Targetprofil (seit v0.8.2-dev) jetzt im Hauptteil.
- `docs/guides/tack-ini.md` und `docs/reference/tack-ini.md` dokumentieren praktische Konfigurationen, Parser-Schlüssel und Einschränkungen.
- Die 0.8.4-Fehlverschachtelung von `[fmt]`/`[fmt "NAME"]` unter `[sbom]` ist in C89 korrigiert; der neue `fmt_ini_parser_test.c` prüft Parserwerte, `fmt --list --strict`, Leerzeichen und strikte Negativfälle. Linux GCC Release: 26 kompiliert, 25 PASS, 1 SKIP. Gemessener Linux-Coverage-Kandidat 3470/5708 (60,79187 %) ist besser als die bisherige HWM, benötigt aber noch Owner-Abnahme. Windows und externe Formatter-Ausführung waren zum Zeitpunkt von RC0.2 NOT RUN; `TACK_VERSION` bleibt 0.8.4 bis zum eigentlichen Release. Der separate 0.8.6-Plan bleibt unverändert.

### v0.8.4 (vom Owner zur Veröffentlichung freigegeben)
- **RC0.6 Windows-HWM:** Vom Owner am 17.11.2026 mit TDM-GCC/gcov 10.3.0 tatsächlich gemessen: 3192/5827 ausführbare Produktionszeilen (54,77947 %). Die Messung ist als `coverage/hwm-windows.txt` festgeschrieben. Linux 3228/5708 unverändert. Bestätigenden `coverage check` und vollständigen instrumentierten Windows-Lauf gegen diese HWM vor GitHub-Push erneut ausführen.
- RC0.5 korrigiert zwei Windows-falsche Pfaderwartungen im `coverage_ratchet_test`, ohne Testfälle zu entfernen: native Windows-/POSIX-Pfade und kanonische `build/`-Ablage werden geprüft, mit konkreter Pfaddiagnose. Linux GCC Release und Linux GCC/gcov bestehen; die Windows-HWM-Messung erfolgte anschließend im RC0.5-Owner-Lauf (siehe RC0.6 oben).
- Windows-Coverage-Nachkorrektur vor Veröffentlichung: Der GCC-Testlauf kompiliert instrumentierte `.o` zuerst unter `build/tests/coverage/obj/` und linkt dann die Programme. Fehlende `.gcno` werden sofort erkannt; die zuvor im Root abgelegten `.gcno`/`.gcda` gehören nicht mehr zum Ablauf. Eine angefangene oder fehlgeschlagene neue Messung verwirft `coverage/current.txt`, nicht aber angenommene HWMs.
- Repariert den strengen GCC-Windows-Build von `incremental_link_test.c`, ohne POSIX-Testlogik zu entfernen.
- Unterscheidet ausgeführte Tests von echten Plattform-SKIPs in CI-Summary, TAP und JUnit.
- Ergänzt `tack ratchet tests` mit versioniertem, prüfbarem Testquelleninventar und expliziter Owner-Annahme.
- Ergänzt `test coverage` und `coverage check/accept` mit GCC/gcov-Messung, Produktionsquellenfilter und versionierter High-Water-Mark pro Plattform.
- Die tatsächliche Linux-GCC/gcov-Referenz-Coverage wurde nach Owner-Entscheidung als `coverage/hwm-linux.txt` angenommen; die Messung des finalen Quellstands ist zu dokumentieren und `coverage check` muss PASS liefern.
- Die Windows-Abnahme von RC0.2 ergab sowohl mit GCC/Release als auch TinyCC/Debug 24 PASS, 1 SKIP, 0 FAIL bei 25 Testquellen. Windows-Coverage wurde anschließend mit `id_windows` im RC0.5-Lauf gemessen und in RC0.6 festgeschrieben; ein bestätigender Windows-PASS vor dem GitHub-Push bleibt erforderlich.

### v0.8.3-dev (Entwicklungskandidat, nicht freigegeben)
- Repariert veraltete `build_and_run_tests()`- und `cmd_doctor()`-Aufrufe in den Regressionstests.
- Ergänzt die POSIX-`fileno()`-Deklaration für strenge ISO-C89-Tests unter GCC.
- Synchronisiert DE/EN-Dokumentation, `tack.ini` und `tack init`/`tack new` mit den Profilcompiler-Funktionen.
- Beweist Linux/GCC Debug und Release mit den ausgeführten Tests; Windows/TinyCC/Coverage bleiben gesonderte Gates.
- BA-STD-C89-Ratchet bleibt ohne gemessene Coverage-High-Water-Baseline offen.

### v0.8.2-dev (Entwicklungskandidat, nicht freigegeben)
- Ergänzt `compiler` und `compiler_policy` je Targetprofil mit Vorrang für `TACK_CC`.
- `tack test` übernimmt Compiler und zusätzliche `cflags` des gewählten Zielprofils; `tack doctor release --target NAME` zeigt die Auswahl.
- Compilerwechsel im selben Profil erfordert derzeit `--rebuild`.

### v0.8.1-dev
- Abschluss: `v0.8.1-dev` ist als Doc-UX-/Search-/Polishing-Schritt abgeschlossen; Programmstand, Doku, INI, Templates und SPEC-Status sind für diesen Stand synchronisiert.
- Prüfung: vollständige lokale Testsuite grün (`TACK_SUMMARY ... status=ok ... total=22 ... failed=0`) sowie C89/Pedantic/Werror-Builds mit GCC und Clang erfolgreich.
- `tack doc`: Optionales JS-Suchfeld in die Kopfzeile verlegt statt in die linke Navigation.
- `tack doc`: Generiertes Suchskript korrigiert; die Seitensuche durchsucht jetzt wieder den vollständigen statischen Dokumentindex.
- `tack doc` rendert jetzt eine kleine, strukturierte Markdown-Ansicht statt eines reinen Plain-Text-Wrappers: Überschriften, Absätze, Listen, fenced code blocks, Inline-Code, Markdown-Links und einfache Auto-Links verbessern die Lesbarkeit, ohne tack in eine vollständige Markdown-Engine zu verwandeln.
- `tack doc` baut jetzt eine seitenbezogene ToC aus erkannten Überschriften auf, markiert die aktuelle Seite in der linken Navigation und bringt ein überarbeitetes Doc-CSS für bessere Typografie, Zeilenumbruch, Abstände und Dark-Mode-Verhalten mit.
- `tack doc` behandelt Unterverzeichnisse unter `docs/` jetzt als eigene Gruppen in Navigation und Index; `docs/README.md` bleibt unter `Docs`, während Verzeichnisse wie `docs/adr/` und `docs/specs/` als kürzere Gruppen wie `ADR` und `SPEC` dargestellt werden.
- Behebt relative CSS-Links in `tack doc` für verschachtelte Seiten unter `docs/adr/` und `docs/specs/`, damit die generierten HTML-Seiten wieder die mitgelieferten Styles und den automatischen Hell/Dunkel-Modus übernehmen.
- Hebt die interne Tool-Version auf `v0.8.1-dev` an, damit `tack version`, `tack init`, BOM/SBOM, DOC und weitere versionierte Ausgaben wieder denselben Stand wie die aktuelle Doku-Linie zeigen.
- `tack doc` kann jetzt optional eine kleine JS-Suche als Progressive Enhancement erzeugen. Standardmäßig bleibt die Ausgabe no-JS/offline-freundlich; aktiviert wird die Suche über `[doc] allow_js_search = yes` in `tack.ini`.
- `tack doc` schreibt Markdown-Links zwischen Root-Dokumenten sowie zwischen `docs/README.md`, `docs/adr/`, `docs/specs/` und `docs/guides/` jetzt als korrekte relative HTML-Ziele um, sodass Inline-Links im generierten Inhalt wieder genauso verlässlich funktionieren wie die linke Navigation.

### v0.8.0-dev
- Abschluss: `v0.8.0-dev` ist als kleiner, erfolgreich geprüfter CI-Grundlagenblock abgeschlossen.
- Eröffnet die **0.8-Serie** als neue Entwicklungslinie für Grundsatzthemen.
- Führt `docs/README.md`, `docs/adr/0001-open-v0.8-series.md`, `docs/specs/0001-bilingual-documentation-policy.md` und `docs/specs/0002-ini-first-toolchain-policy-foundation.md` ein.
- Hebt die interne Tool-Version damals auf `v0.8.0-dev` an, damit `tack version`, `tack init`, BOM/SBOM und weitere versionierte Ausgaben nicht mehr hinter der 0.8-Dokumentation zurückbleiben.
- Ergänzt README/FAQ/ROADMAP/RELEASENOTES um Verweise auf `docs/`.
- Präzisiert mit `docs/specs/0003-compiler-selection-and-policy-split.md`, dass Compilerwahl, Built-ins und zusätzliche Projektpolitik getrennt betrachtet werden.
- Landet den ersten kleinen 0.8-Produktions-Code-Schritt: Auswahl-Priorität ist jetzt `TACK_CC` → `[project] compiler` → Built-in-Default `tcc`.
- Führt optional `[project] compiler_policy = auto|tcc|gcc|clang|generic` ein, damit compilerbewusste Built-in-Logik explizit fixiert werden kann, ohne tack zu einer generischen Toolchain-DSL umzubauen.
- `tack doctor` meldet jetzt zusätzlich **Compilerquelle**, **wirksame Compiler-Policy** und die **Herkunft dieser Policy**.
- Führt `--ci` als kleinen, plattformneutralen CI-Modus für `build`, `run` und `test` ein.
- Gibt in diesem Modus eine stabile `TACK_SUMMARY`-Zeile aus und schaltet stdout/stderr auf unbuffered Ausgabe.
- Ergänzt die Root-Dokumente und `docs/README.md` um direkte Verweise auf `docs/specs/0004-ci-mode-and-summary-foundation.md`.
- Führt `--events-jsonl FILE` für `build`, `run` und `test` als kleine versionierte Maschinenebene (`tack.events.v1`) ein.
- Führt `--report-tap FILE` für `tack test` als kleinen TAP-v13-Bericht ein.
- Führt `--report-junit FILE` für `tack test` als konservativen JUnit-XML-Bericht ein.
- Ergänzt die Root-Dokumente und `docs/README.md` um direkte Verweise auf `docs/specs/0005-events-jsonl-foundation.md`, `docs/specs/0006-tap-report-foundation.md` und `docs/specs/0007-junit-xml-foundation.md`.
- Die Hilfeausgabe wurde bereinigt und bündelt Compilerwahl/Policy jetzt an einer Stelle; die mitgelieferte `tack.ini`, die von `tack init` erzeugte Start-Konfiguration, README, FAQ und ROADMAP wurden entsprechend nachgezogen.
- Neuer Regressionstest: `tests/compiler_selection_policy_test.c`.

### v0.7.25
- P0-Fix für Incremental-Linking: tack erzwingt den Link-Schritt jetzt, wenn in diesem Lauf ein Objekt neu kompiliert oder aus dem Cache restauriert wurde. Zusätzlich wird pro Binary eine kleine `.linkmeta`-Datei geschrieben und geprüft, damit stille stale Binaries auch über mehrere Läufe hinweg erkannt werden.
- Neue Regression `tests/incremental_link_test.c`: reproduziert die Header-only-Änderung mit absichtlich gleichgezogenen Output-Timestamps und stellt sicher, dass das Binary danach wirklich den neuen Wert ausführt.
- Striktes C89/C90 nachgezogen: die eingebetteten Init-Templates werden jetzt als C90-freundliche String-Listen geschrieben; `tests/strict_c89_compile_test.c` prüft einen `-std=c89 -pedantic -Werror`-Compile von `src/tack.c`.
- UX/Diagnose: `tack doctor` meldet jetzt zusätzlich, ob der aktuell gewählte Compiler im `PATH` gefunden wird; die verbose DOC-Ausgabe schreibt Einzelseitenpfade wieder sauber mit Zeilenumbrüchen.

### v0.7.24
- `tack init` erzeugt `src/main.c` jetzt versionsneutral mit `puts("Hello from tack!")` statt mit einer hart verdrahteten Altversion; künftige Releases müssen das Scaffold damit nicht mehr pro Version anfassen.
- `TACK_INIT_DEFAULT_TACK_INI` referenziert `TACK_VERSION` jetzt direkt statt eine Versionsnummer zu duplizieren; die generierte `tack.ini` bleibt dadurch enger an der echten Tool-Version.
- `tests/init_scaffold_version_test.c` prüft jetzt sowohl die per `tack init` generierte `tack.ini` als auch das Standard-`src/main.c` auf Versionsdrift.
- Aufräumarbeit im Quelltext: der interne Altkommentar `tackfile.c auto-config (v0.6.0)` wurde in eine versionsneutrale Form gebracht, um unnötige Drift-Signale in Re-Reviews zu vermeiden.

### v0.7.23
- Clang/LLVM-Buildpfad repariert: tcc-spezifische Warn-Flags (`-Wunsupported` / `-Wno-unsupported`) werden nur noch für tcc/TinyCC gesetzt; `TACK_CC=clang` bricht damit nicht mehr an unbekannten Warn-Optionen unter `-Werror`.
- Sichere Target-Pfade als Default eingeführt: `id` und `bin` werden als einfache, pfadsichere Tokens validiert; `src` bleibt repo-relativ ohne `..`-Traversal. Für kontrollierte Sonderfälle gibt es ein explizites Opt-in via `--unsafe-paths` oder `[project] allow_unsafe_paths = yes`.
- CycloneDX-/SPDX-Metadaten fachlich nachgezogen: CycloneDX schreibt jetzt `serialNumber` und `metadata.timestamp`; SPDX schreibt ein echtes UTC-`creationInfo.created` und einen eindeutigeren `documentNamespace`. `SOURCE_DATE_EPOCH` wird für reproduzierbare Timestamps respektiert.
- Versions-/Scaffold-Hygiene repariert: `TACK_VERSION`, die von `tack init` erzeugte `tack.ini` und die mitgelieferte Beispiel-`tack.ini` zeigen wieder denselben Versionsstand.
- Neue Regressionstests: `path_safety_test.c`, `sbom_metadata_test.c`, `init_scaffold_version_test.c`; `compiler_profile_flags_test.c` prüft jetzt zusätzlich das compilerbewusste Warn-Flag-Gating.

### v0.7.22
- Test-Fix für den bislang übergangenen Nebenbefund `cache dir missing` im `functional_smoke_test`.
- Der Cache-Test entfernt jetzt vor der Prüfung gezielt die target-spezifischen Build-Ausgaben unter `build/<target>/<profile>/{obj,dep,bin}`, damit ein echter Build-/Restore-Pfad entsteht und nicht bloß ein Up-to-date-Short-Circuit.
- Dadurch prüft der Smoke-Test wieder belastbar, dass `.tack-cache/` bei cache-fähigen Builds angelegt wird und der Warm-Cache-Pfad nach entfernten Build-Ausgaben weiter funktioniert.
- Der `--no-cache`-Pfad wird mit derselben Ausgangslage gegengeprüft; Produktionscode bleibt unverändert.

### v0.7.21
- Abschlussprüfung der DE/EN-Doku: README, FAQ, ROADMAP, Release Notes und `templates/README.md` wurden inhaltlich angeglichen statt nur markerbasiert geprüft.
- README EN enthält jetzt die zuvor nur auf Deutsch dokumentierten Konfigurationsdetails (`tack.ini`, `[doc]`/`[bom]`, Flag-Semantik, Beispiel-INI, clean/clobber-Abgrenzung).
- FAQ EN wurde in dieselbe Themenreihenfolge wie FAQ DE gebracht; `templates/README.md` enthält nun auch im englischen Teil dieselben Layout-/Template-Hinweise.

### v0.7.20
- CSS-Feinschliff für die mitgelieferten DOC/BOM-Templates: `templates/tack_doc.css` enthält jetzt Grundunterstützung für `prefers-contrast: more` und `forced-colors: active`.
- Lesbarkeit/Orientierung in kontraststarken Modi verbessert: Links werden unterstrichen, Rahmen etwas deutlicher gezeichnet, der aktuelle Nav-Eintrag fällt klarer auf, und der Tastaturfokus ist sichtbarer.
- Dokumentation in README/FAQ/templates/README ergänzt; in diesem Schritt wurde bewusst kein Produktions-C-Code geändert.

### v0.7.19
- Windows-Long-Path-Guidance in README/FAQ deutlich erweitert: konkrete Praxisempfehlungen für flache Workspace-Pfade, Aktivierung von `LongPathsEnabled` bzw. Gruppenrichtlinie und Hinweis auf erforderlichen Neustart/Reboot.
- Doku stellt jetzt ausdrücklich klar, dass Long-Paths unter Windows nur helfen, wenn das jeweilige Programm selbst **long-path-aware** ist; externe Tool-Limits bleiben also relevant.
- Keine Produktionscode-Änderung in diesem Schritt; es handelt sich bewusst um einen kleinen Doku-/Erfahrungs-Commit.

### v0.7.18
- Test-Hardening für GCC/Clang mit `-Werror`: eingebettete `#include "../src/tack.c"`-Tests nutzen jetzt einen kleinen Wrapper, der `-Wunused-function` bewusst nur im Testkontext unterdrückt.
- `functional_smoke_test` prüft den Cache-Pfad ohne forcierten Rebuild; damit entspricht die Testerwartung wieder der realen Cache-Semantik (`force` umgeht den Compile-Cache absichtlich).
- `path_join_test.c` definiert testlokale Hilfsfunktionen jetzt nur noch dort, wo sie tatsächlich gebraucht werden.
- Produktionscode und Single-File-Ansatz bleiben unverändert; der Fix bleibt absichtlich testlokal.

### v0.7.17
- `tack sbom` fachlich präzisiert: Die Doku beschreibt die Ausgabe jetzt ausdrücklich als **Build-Input-SBOM**. CycloneDX/SPDX bleiben Exportformate, aber ohne Resolver/Package-Manager gibt es bewusst keine erfundenen Fremdkomponenten-Versionen.
- Neuer CLI-Batchmodus: `tack sbom --all-targets` schreibt je aktiviertem Target eine eigene JSON-Datei (`build/sbom.<target>.json`, `build/sbom.<target>.cdx.json`, `build/sbom.<target>.spdx.json`).
- `[sbom] output` bleibt absichtlich Single-Target-spezifisch; so bleiben Einzel-Export und Batch-Export semantisch sauber getrennt.
- Neuer Regressionstest `tests/sbom_all_targets_test.c` prüft den Mehrziel-Export inklusive Dateinamen und Target-Zuordnung.

### v0.7.16
- Cache-Validierung nach `tack clean` deterministischer gemacht: Das Depfile selbst wird für Cache-Restores jetzt über **Dateigröße + Content-Hash** statt über eine harte `mtime`-Gleichheit geprüft.
- Normale Dependencies bleiben weiterhin über `mtime` + Größe + FNV-1a validiert; nur der Depfile-Fingerprint wurde gezielt entschärft.
- Damit verschwinden timing-abhängige Cache-Misses nach `clean`, ohne Änderungen am Dependency-Graphen zu übersehen.
- Neuer Regressionstest `tests/cache_restore_after_clean_test.c` simuliert den `clean`/Restore-Fall und prüft zugleich, dass echte Depfile-Inhaltsänderungen den Cache weiterhin verwerfen.

### v0.7.15
- Compilerbewusste Debug-Profil-Flags: tack setzt `-g` und `-DDEBUG=1` allgemein, `-bt20` aber nur noch für tcc/TinyCC.
- `TACK_CC=gcc` bzw. `TACK_CC=clang` funktionieren damit im Debug-Profil portabler; der im Paper reproduzierte `-bt20`-Fehler wird vermieden.
- Doku präzisiert die effektiven Basis-Profil-Flags in README/FAQ.
- Neuer Regressionstest `tests/compiler_profile_flags_test.c` prüft BOM/Flag-Verhalten für tcc vs. gcc.

### v0.7.14
- `tack new <name>`: erstellt ein Verzeichnis `<name>/` und führt darin automatisch `tack init` aus (Projekt-Scaffold in einem Schritt).

### v0.7.13
- `tack clobber`: räumt jetzt immer zusätzlich `.tack-cache/` ab (Big Hammer: Build + Cache weg).
- `tack clean --cache`: löscht zusätzlich `.tack-cache/` (Cache-Reset ohne manuelle Dateisystem-Operationen).

### v0.7.12
- `tack doc`: keine festen Dateinamen mehr. Stattdessen werden alle `*.md` im Projekt-Root als HTML in `build/doc/` gerendert (z.B. `README.md`, `CHANGELOG.md`, `RELEASENOTES.md`, … – je nachdem, was im Repo existiert).
  - Navigation/Index werden aus dieser dynamischen Root-Liste erzeugt (kein Dead-Link-Nav mehr).
- `tack init`: legt zusätzlich (nicht destruktiv) `templates/` inkl. Standard-CSS/Template sowie eine Start-`tack.ini` an.
- Convenience: Wenn keine CSS-Datei konfiguriert ist, nutzt `tack doc`/`tack bom` automatisch `templates/tack_doc.css`, falls vorhanden.

### v0.7.11
- `tack doc`: berücksichtigt optional einen `docs/`-Ordner (alle `docs/**/*.md`) und erzeugt dafür HTML-Seiten unter `build/doc/docs/`.
- `build/doc/index.html` zeigt bei vorhandenen `docs/**/*.md` zusätzlich eine „Docs“-Sektion (Anchor `#docs`) und die Seiten enthalten einen passenden Nav-Link.

### v0.7.10
- Feature: `tack fmt` – Orchestrator für externe Formatter (Policy in `tack.ini` via `[fmt]` / `[fmt "NAME"]`).
  - `tack fmt` formatiert in-place.
  - `tack fmt --check` ändert nichts und liefert Exit-Code 2, wenn Dateien neu formatiert werden müssten.
  - `--rule/--target/--list/--no-defaults/--diff` (Diff best effort via `git diff --no-index` oder `diff -u`).

### v0.7.9
- Bugfix: Rebuild-Entscheidung nutzt nicht mehr die reine Reihenfolge `depfile mtime > object mtime`; stattdessen werden gespeicherte Abhängigkeits-Metadaten (`mtime`/Größe/FNV-1a) geprüft, inklusive Depfile-Fingerprint zur Erkennung geänderter Dependency-Graphen. Dadurch keine False-Positives nach Cache-Restores, bei denen Objekt und Depfile nacheinander geschrieben werden.
- Depfile-Scan erfasst jetzt auch `#include <...>` und löst diese gegen die effektiven Include-Pfade auf; Header-Änderungen über `-I` triggern damit korrekt Recompiles.
- Dokumentation auf neuen Stand gebracht (README/FAQ/ROADMAP/RELEASENOTES) und Version auf `v0.7.9` angehoben.

### v0.7.8
- INI: Profil-spezifische Target-Overrides via `[target "NAME".debug]` / `[target "NAME".release]` für `core`, `includes`, `defines`, `cflags`, `ldflags`, `libs`.

### v0.7.7
- Bugfix-Release: C89-Compile-Fixes nach fehlerhaftem Merge (SBOM-Pfad/Variablen/Braces bereinigt).
- `tack sbom`: CycloneDX/SPDX-Ausgabe vollständig implementiert; `[sbom]` unterstützt `format`, `spec_version`, `output` inkl. formatabhängiger Defaults.

### v0.7.6
- Dokumentations-Sync: README/FAQ/ROADMAP/Release Notes auf den aktuellen Implementationsstand (Features/Optionen) abgeglichen.
- `tack sbom`: Mehrformat-Ausgabe (Default `tack`), Format/Output steuerbar via `[sbom]` in `tack.ini` (CycloneDX/SPDX-Optionen dokumentiert, noch nicht implementiert).

### v0.7.5
- `tack sbom`: deterministischer SBOM-Export als JSON (`build/sbom.json`, Format `tack-sbom-1`).

### v0.7.4
- Bugfix-Release: Compile-Cache validiert Abhängigkeiten jetzt über `mtime` + Dateigröße + Content-Hash (32-bit FNV-1a), um false positives auf Dateisystemen mit grober Timestamp-Auflösung zu vermeiden.
- Cache-Entries werden atomarer geschrieben (Tempfiles + `rename()`), um partielle Einträge zu vermeiden.
- Striktere C89/ANSI-C-Kompatibilität (kein `snprintf`/`vsnprintf`; stabiler 32-bit Cache-Key).

### v0.7.3
- Optionaler Compile-Cache (`.tack-cache/`) für schnellere Incremental Builds, abschaltbar via `--no-cache`.

### v0.7.2
- `--why` / `--explain`: kurze Diagnoseausgaben, *warum* tack einen Compile- oder Link‑Schritt ausführt (z.B. "output missing", "source newer", "dependency newer", "forced (--rebuild)").

### v0.7.1
- `tack bom`: erzeugt ein Build‑Manifest (BOM) als `build/bom.md` und `build/bom.html`.
- `tack doc`: erzeugt offline HTML‑Doku in `build/doc/` (Wrapper um Markdown) und verlinkt die BOM.
- Optional: HTML-Templates + CSS für DOC/BOM via `tack.ini` (`[doc]`/`[bom]`: `template`, `css`).
- HTML-Ausgabe: stabile Template-Ankerpunkte (Marker + IDs) für CSS-Hooks und optionales Post-Processing.
- Fixes/Clarifications (v0.7.1 Patch):
  - Fail-fast: fehlende `template`/`css` Dateien oder fehlendes `{{TACK_CONTENT_HTML}}` → Exitcode 2.
  - `--no-code-config` greift jetzt tatsächlich.
  - `doc`/`bom` Profil-Parsing korrigiert (keine falschen Argument-Offsets mehr bei globalen Optionen).
  - INI: Layering über mehrere Quellen funktioniert; `[target ...]` wird korrekt geparst.
  - Template-Rendering: Token-Längen korrigiert (keine stray `}` Artefakte); Marker-Vertrag präzisiert (genau einmal im Output).

### v0.7.0
- `tack init` erzeugt optional eine `.gitignore` sowie `.fossil-settings/ignore-glob`.
- **Non-destructive:** Existierende Dateien werden **nicht überschrieben**; ein klar markierter tack‑Block wird nur angehängt, wenn er fehlt.
- Zweck: Standardmäßig keine Build‑Artefakte, Cache‑Ordner oder Generator‑Outputs im VCS einchecken.

### v0.6.x (Hardening-Serie, Überblick)
- „Fail-fast“ als Default, mit besseren Fehlermeldungen und robusteren Checks.
- Konsistentere, sicherere String-/Path-Operationen (Bounds-/Truncation‑Checks).
- Konfiguration via `tack.ini` (declarative) plus optionaler Code‑Pfad (`tackfile.c` → generierte INI) mit Verifikations‑Makros.
- Schalter zum Abschalten von Code‑Konfig und Auto‑Tool‑Discovery (Team/CI‑Sicherheit).

---

## English
### v0.8.5 final local release candidate (HWMs approved, publication pending)
- Owner Windows RC0.7: GCC release 25 PASS/1 SKIP, TinyCC debug 24 PASS/2 SKIP, ratchet/docs PASS, real clang-format E2E with spaces and 0/2/0/0/1/0 exit codes PASS.
- Linux GCC/Clang strict C89, release 25 PASS/1 SKIP, ratchet/docs/formatter E2E PASS. Owner approved independent Windows 3577/5906 (60.56553%) and Linux 3505/5710 (61.38354%) HWMs. Linux CLI acceptance/check PASS; Windows post-adoption rerun pending.
- Historical RC0.4–RC0.6 records archived under `docs/releases/archive/0.8.5/`. Canonical report is `docs/releases/0.8.5.md`. No GitHub push/merge/tag without separate approval.


### v0.8.5 RC0.7 (release candidate, NOT RELEASED)
- Fixes since v0.8.4: formatter INI parser and Windows external formatter argument passing, including `CreateProcessA` and final NUL termination. `clang-format` remains optional.
- Owner's RC0.6 evidence: Windows GCC release 25 PASS/1 SKIP, TinyCC debug 24 PASS/2 SKIP; isolated formatter E2E with spaced paths and original files unchanged by `--check` PASS.
- RC0.7 promotes the version to 0.8.5 and aligns actual functionality and limitations in DE/EN. **Not published**; RC0.7 Linux GCC/Clang, release, docs, ratchet and formatter E2E PASS, Linux gcov 3505/5710 with HWM BLOCKED. Windows RC0.7 gates and controlled HWM approvals remain outstanding.

### v0.8.5-dev RC0.6 (formatter token NUL termination, NOT RELEASED)
- Real RC0.5 Windows GCC/TCC child-process tests failed on a trailing backslash; TinyCC also failed on an embedded quote. Both suites stopped with 13 PASS/1 FAIL/12 NOT RUN; gcov had no valid current measurement.
- `fmt_replace_all_alloc` omitted the terminating NUL when the replacement occurred at the end of a token. RC0.6 fixes only that path, preserving the RC0.5 `CreateProcessA` formatter runner. Added six portable regressions; existing assertions unchanged.
- Linux strict C89, GCC Release 25 PASS/1 SKIP, docs, and isolated formatter E2E pass. Linux coverage 3505/5710 (61.38354%) is measured but HWM BLOCKED. Separate accepted HWMs unchanged; Windows RC0.6 NOT RUN. See `docs/releases/archive/0.8.5/0.8.5-dev-RC0.6-validation.md`.

### v0.8.5-dev RC0.5 (Windows formatter process launch, NOT RELEASED)
- Owner RC0.4 Windows: GCC release **26 compiled, 25 PASS, 1 SKIP**; TinyCC debug **26 compiled, 24 PASS, 2 SKIP**; inventory ratchet and docs PASS. GCC/gcov **3459/5859 (59.03738%)**, coverage HWM check correctly **BLOCKED** (not a functional-test failure).
- Real Windows formatter still **FAIL** on RC0.4: LIST 0; first CHECK 1 (expected 2); original unchanged 0; FORMAT 1 (expected 0); no file change; final CHECK 1 (expected 0). The file paths carry corrupted suffixes rather than split fragments. `clang-format 23.1.5` installed. CRT `_spawnvp` quoting alone was insufficient.
- Narrow C89 candidate: Windows external **formatter only** now launches via `CreateProcessA`, with a fully bounded, NUL-terminated command line, wait and exit-code verification. General tack build process launcher untouched. No shell or new dependencies. Existing five Windows quoting assertions retained; three actual child-process probes added for space-containing executable/argument paths, trailing backslash and embedded quote.
- Linux GCC and Clang strict C89 builds PASS, GCC release **26 compiled, 25 PASS, 1 SKIP**, external formatter E2E PASS, Linux coverage **3470/5709 (60.78122%)** measured, higher HWM **BLOCKED** until owner acceptance. Accepted Windows/Linux HWMs byte-identical to RC0.4.
- Windows RC0.5 **NOT RUN**, SSH authentication probe successful but Git fetch not reverified. Binary version **0.8.4**, no push/merge/tag. See `docs/releases/archive/0.8.5/0.8.5-dev-RC0.5-validation.md`. (Release Notes)

### v0.8.5-dev RC0.4 (Windows formatter E2E fix candidate, NOT RELEASED)
- Actual Windows RC0.3 formatter processes split file/directory arguments containing spaces. RC0.4 fixes only Windows CRT child argument quoting in the formatter path, with C89 regressions.
- Linux GCC release: 25 PASS, one SKIP; real Linux formatter E2E exit codes 0/2/0/0; coverage 3470/5709, HWM acceptance BLOCKED. Both existing accepted HWM files unchanged.
- Windows RC0.4 E2E, TinyCC and GCC/gcov, owner review of test inventory, version promotion and GitHub release acceptance remain pending.


### v0.8.5-dev RC0.3 (Windows HWM and external Linux formatter check, NOT RELEASED)
- Windows TDM-GCC release/TinyCC debug: 25 PASS, one SKIP; measured and accepted HWM 3434/5827 (58.93256%), fresh coverage gate PASS.
- Real external formatter end-to-end verified on Linux, including a filename with spaces; Windows external formatter E2E pending.
- Linux 3470/5708 (60.79187%) measured, owner HWM acceptance pending. No changes to C89 production/tests and no 0.8.5 release.


> `v0.8.4` has been approved for publication as a standalone CLI. Integration with additional third-party projects (e.g. ft2-clone or nuklear) has not been verified. Git tag and GitHub Release are still pending the push.

### v0.8.5-dev RC0.2 (formatter parser correction, NOT RELEASED)
- README prominently explains the per-target/profile compiler program and policy introduced in v0.8.2-dev.
- Bilingual INI walkthrough and key reference document validated configuration patterns and parsed-only or unsupported fields.
- Fixes the 0.8.4 `[fmt]`/`[fmt "NAME"]` nesting under `[sbom]`; strict C89 regression covers parsed values, `fmt --list --strict`, paths with spaces and negative validation. Linux GCC release: 26 compiled, 25 PASS, one SKIP. Measured Linux coverage candidate 3470/5708 (60.79187%) improves on the accepted HWM but still needs owner acceptance. Windows and external formatter execution NOT RUN. Runtime version remains 0.8.4 until the release gate; the separate v0.8.6 plan remains unchanged.

### v0.8.4 (owner-approved for publication)
- **RC0.6 Windows HWM:** Owner measured 3192/5827 executable production lines (54.77947%) under TDM-GCC/gcov 10.3.0 on 2026-11-17. Versioned unchanged in `coverage/hwm-windows.txt`; Linux remains 3228/5708. Re-run Windows `coverage check` and the full instrumented suite against this baseline before pushing to GitHub.
- RC0.5 fixes two incorrect Windows path expectations in `coverage_ratchet_test` without removing assertions: native Windows/POSIX paths and canonical `build/` locations are checked with specific path diagnostics. Linux GCC release and GCC/gcov pass; Windows HWM measurement subsequently completed during the RC0.5 owner run (see RC0.6 above).
- Pre-publication Windows gcov correction: instrumented test translation units compile to explicit objects beneath `build/tests/coverage/obj/` before linking. Reject missing `.gcno` immediately; prevent root-level notes/data and invalidate stale `coverage/current.txt` on a new run without touching accepted HWMs.
- Fixes strict GCC-on-Windows compilation of `incremental_link_test.c` without removing its POSIX testing behavior.
- Separates actually executed tests from platform SKIPs in CI summaries, TAP and JUnit reports.
- Adds `tack ratchet tests` with a versioned test-source inventory and explicit owner acceptance.
- Adds `test coverage` and `coverage check/accept` using GCC/gcov production-line measurement and per-platform high-water baselines.
- The Linux GCC/gcov reference coverage measurement for the final source has been owner-accepted as `coverage/hwm-linux.txt`; the release measurement must be recorded and `coverage check` must PASS.
- Owner-provided Windows RC0.2 runs passed for GCC/release and TinyCC/debug (25 compiled, 24 PASS, one SKIP, zero failures each). The later RC0.5 Windows coverage measurement is now versioned in RC0.6; a fresh Windows PASS against it is still pending.

### v0.8.3-dev (development candidate, not released)
- Fixes outdated `build_and_run_tests()` and `cmd_doctor()` regression test calls.
- Adds the POSIX `fileno()` declaration for strict ISO C89 GCC tests.
- Aligns DE/EN documentation, `tack.ini`, and `tack init`/`tack new` with profile compiler features.
- Records executed Linux/GCC debug and release tests; Windows/TinyCC/coverage remain separate gates.
- BA-STD-C89 ratchet acceptance remains open without a measured coverage high-water baseline.

### v0.8.2-dev (development candidate, not released)
- Adds per-target/profile `compiler` and `compiler_policy` with `TACK_CC` precedence.
- `tack test` inherits compiler and extra `cflags` for the chosen profile; `tack doctor release --target NAME` reports the selection.
- Compiler switches within one profile currently require `--rebuild`.

### v0.8.1-dev
- Closure: `v0.8.1-dev` is complete as the doc UX/search/polish step; implementation state, documentation, INI, templates, and SPEC status are synchronized for this state.
- Verification: full local test suite passed (`TACK_SUMMARY ... status=ok ... total=22 ... failed=0`) and GCC/Clang C89/pedantic/Werror builds succeeded.
- `tack doc`: moved the optional JS search box into the header instead of the left navigation.
- `tack doc`: fixed the generated search script; document search now works again against the full static document index.
- `tack doc` now renders a small structured Markdown view instead of a plain-text-only wrapper: headings, paragraphs, lists, fenced code blocks, inline code, Markdown links, and simple auto-links improve readability without turning tack into a full Markdown engine.
- `tack doc` now builds a page-local ToC from detected headings, marks the current page in the left navigation, and ships a refreshed doc CSS for better typography, wrapping, spacing, and dark-mode behaviour.
- `tack doc` now treats subdirectories below `docs/` as their own groups in navigation and index; `docs/README.md` stays under `Docs`, while directories such as `docs/adr/` and `docs/specs/` are rendered as shorter groups like `ADR` and `SPEC`.
- `tack doc` now rewrites Markdown links between root documents and between `docs/README.md`, `docs/adr/`, `docs/specs/`, and `docs/guides/` to the correct relative HTML targets, so inline links inside generated content are as reliable again as the left navigation.
- Fixes relative CSS links in `tack doc` for nested pages under `docs/adr/` and `docs/specs/`, so generated HTML pages once again pick up the shipped styles and the automatic light/dark mode.
- Bumps the internal tool version to `v0.8.1-dev`, so `tack version`, `tack init`, BOM/SBOM, DOC, and other versioned outputs are back in sync with the current documentation line.

### v0.8.0-dev
- Closure: `v0.8.0-dev` is complete as a small, successfully validated CI foundation block.
- Opens the **0.8 series** as a new development line for foundational work.
- Introduces `docs/README.md`, `docs/adr/0001-open-v0.8-series.md`, `docs/specs/0001-bilingual-documentation-policy.md`, and `docs/specs/0002-ini-first-toolchain-policy-foundation.md`.
- Bumped the internal tool version at that time to `v0.8.0-dev`, so `tack version`, `tack init`, BOM/SBOM, and other versioned outputs no longer lagged behind the 0.8 documentation.
- Added references to `docs/` from README/FAQ/ROADMAP/RELEASENOTES.
- Clarified via `docs/specs/0003-compiler-selection-and-policy-split.md` that compiler selection, built-ins, and additional project policy are separate concerns.
- Landed the first small 0.8 production-code step: selection priority is now `TACK_CC` → `[project] compiler` → built-in default `tcc`.
- Added optional `[project] compiler_policy = auto|tcc|gcc|clang|generic` so compiler-aware built-in logic can be pinned explicitly without turning tack into a generic toolchain DSL.
- `tack doctor` now also reports the **compiler source**, **active compiler policy**, and the **source of that policy**.
- Added `--ci` as a small, platform-neutral CI mode for `build`, `run`, and `test`.
- Emits a stable `TACK_SUMMARY` line in that mode and switches stdout/stderr to unbuffered output.
- Added direct links from the root docs and `docs/README.md` to `docs/specs/0004-ci-mode-and-summary-foundation.md`.
- Added `--events-jsonl FILE` for `build`, `run`, and `test` as a small versioned machine-facing layer (`tack.events.v1`).
- Added `--report-tap FILE` for `tack test` as a small TAP v13 report.
- Added `--report-junit FILE` for `tack test` as a conservative JUnit XML report.
- Added direct links from the root docs and `docs/README.md` to `docs/specs/0005-events-jsonl-foundation.md`, `docs/specs/0006-tap-report-foundation.md`, and `docs/specs/0007-junit-xml-foundation.md`.
- Cleaned up help output so compiler selection/policy is presented in one place; the shipped `tack.ini`, the `tack init` starter config, README, FAQ, and ROADMAP were updated accordingly.
- New regression test: `tests/compiler_selection_policy_test.c`.

### v0.7.25
- P0 fix for incremental linking: tack now forces the link step when an object was rebuilt or restored from cache in the current run. In addition, each binary now gets a small `.linkmeta` file so silent stale binaries are detected across later runs as well.
- New regression `tests/incremental_link_test.c`: reproduces the header-only change with intentionally flattened output timestamps and verifies that the resulting binary actually runs the new value.
- Strict C89/C90 tightened up: the embedded init templates are now emitted from C90-friendly string lists; `tests/strict_c89_compile_test.c` checks a `-std=c89 -pedantic -Werror` compile of `src/tack.c`.
- UX/diagnostics: `tack doctor` now also reports whether the currently selected compiler is actually found on `PATH`; verbose DOC output once again prints individual page paths with proper line breaks.

### v0.7.24
- `tack init` now writes `src/main.c` in a version-neutral form with `puts("Hello from tack!")` instead of a hard-coded legacy version string, so future releases no longer need to touch the scaffold per version.
- `TACK_INIT_DEFAULT_TACK_INI` now references `TACK_VERSION` directly instead of duplicating a version string, keeping the generated `tack.ini` closer to the real tool version.
- `tests/init_scaffold_version_test.c` now checks both the `tack.ini` generated by `tack init` and the default `src/main.c` scaffold for version drift.
- Source cleanup: the internal legacy comment `tackfile.c auto-config (v0.6.0)` was rewritten in a version-neutral form to avoid noisy drift signals in re-reviews.

### v0.7.23
- Repaired the Clang/LLVM build path: tcc-specific warning flags (`-Wunsupported` / `-Wno-unsupported`) are now emitted only for tcc/TinyCC, so `TACK_CC=clang` no longer fails on unknown warning options under `-Werror`.
- Introduced safe target paths by default: `id` and `bin` are validated as simple filesystem-safe tokens; `src` stays repo-relative without `..` traversal. Controlled special cases now require explicit opt-in via `--unsafe-paths` or `[project] allow_unsafe_paths = yes`.
- Tightened CycloneDX/SPDX metadata semantics: CycloneDX now writes `serialNumber` and `metadata.timestamp`; SPDX writes a real UTC `creationInfo.created` value and a more unique `documentNamespace`. `SOURCE_DATE_EPOCH` is respected for reproducible timestamps.
- Repaired version/scaffold hygiene: `TACK_VERSION`, the `tack.ini` generated by `tack init`, and the shipped sample `tack.ini` now show the same version again.
- New regression tests: `path_safety_test.c`, `sbom_metadata_test.c`, `init_scaffold_version_test.c`; `compiler_profile_flags_test.c` now also checks compiler-aware warning-flag gating.

### v0.7.22
- Test fix for the previously parked `cache dir missing` side finding in `functional_smoke_test`.
- Before asserting cache behavior, the smoke test now removes the target-specific build outputs under `build/<target>/<profile>/{obj,dep,bin}`, so it exercises a real build/restore path instead of an up-to-date short-circuit.
- This makes the smoke test reliably verify again that `.tack-cache/` is created for cache-capable builds and that the warm-cache path still works after build outputs were removed.
- The `--no-cache` path is checked from the same starting conditions; production code is unchanged.

### v0.7.21
- Final DE/EN documentation pass: README, FAQ, ROADMAP, Release Notes, and `templates/README.md` were aligned by content rather than by markers only.
- README EN now includes the configuration details that were previously documented only in German (`tack.ini`, `[doc]`/`[bom]`, flag semantics, example INI, clean/clobber distinction).
- FAQ EN now follows the same topic order as FAQ DE; `templates/README.md` now carries the same layout/template hints in the English section.

### v0.7.20
- Shipped CSS polish for the DOC/BOM templates: `templates/tack_doc.css` now includes baseline support for `prefers-contrast: more` and `forced-colors: active`.
- Improved readability/orientation in higher-contrast modes: links are underlined, borders become stronger, the current nav item is more obvious, and keyboard focus is easier to see.
- Documentation updated in README/FAQ/templates/README; this step intentionally does not change production C code.

### v0.7.19
- Expanded Windows long-path guidance in README/FAQ: concrete advice for shallow workspace roots, enabling `LongPathsEnabled` / Group Policy, and restarting the shell or rebooting afterwards.
- The docs now explicitly state that long paths only help when the affected program is itself **long-path aware**; external tool limits still matter.
- No production-code change in this step; this is intentionally a small documentation/operability commit.

### v0.7.18
- Test hardening for GCC/Clang with `-Werror`: embedded `#include "../src/tack.c"` tests now use a tiny wrapper that suppresses `-Wunused-function` only in the test context.
- `functional_smoke_test` now checks the cache path without a forced rebuild so the test expectation matches actual cache semantics (`force` intentionally bypasses the compile cache).
- `path_join_test.c` now defines test-local helpers only where they are actually used.
- Production code and the single-file approach remain unchanged; this fix intentionally stays local to the tests.

### v0.7.17
- `tack sbom` is now positioned more precisely in the docs as a **build-input SBOM**. CycloneDX/SPDX remain export formats, but without a resolver/package manager tack intentionally does not invent third-party component versions.
- New CLI batch mode: `tack sbom --all-targets` writes one JSON file per enabled target (`build/sbom.<target>.json`, `build/sbom.<target>.cdx.json`, `build/sbom.<target>.spdx.json`).
- `[sbom] output` intentionally remains single-target specific, keeping single-export and batch-export semantics clearly separated.
- New regression test `tests/sbom_all_targets_test.c` covers multi-target export, filenames, and target mapping.

### v0.7.16
- Made cache validation after `tack clean` deterministic: for cache restores, the depfile itself is now validated via **file size + content hash** instead of requiring strict `mtime` equality.
- Normal dependencies still use `mtime` + size + FNV-1a; only the depfile fingerprint rule was relaxed on purpose.
- This removes timing-dependent cache misses after `clean` without masking real dependency-graph changes.
- New regression test `tests/cache_restore_after_clean_test.c` simulates the `clean`/restore path and also verifies that real depfile content changes still invalidate the cache.

### v0.7.15
- Compiler-aware debug profile flags: tack always emits `-g` and `-DDEBUG=1`, but now only emits `-bt20` for tcc/TinyCC.
- This keeps `TACK_CC=gcc` and `TACK_CC=clang` portable in debug builds and avoids the `-bt20` failure reproduced in the paper.
- Documentation now states the effective built-in profile flags more precisely in README/FAQ.
- New regression test `tests/compiler_profile_flags_test.c` checks BOM/flag behavior for tcc vs. gcc.

### v0.7.14
- `tack new <name>`: creates a `<name>/` directory and automatically runs `tack init` inside it (one-step project scaffold).

### v0.7.13
- `tack clobber`: now always deletes `.tack-cache/` as well (big hammer: build + cache reset).
- `tack clean --cache`: also deletes `.tack-cache/` (cache reset without manual filesystem operations).

### v0.7.12
- `tack doc`: no more fixed root filenames. Instead, all `*.md` in the project root are rendered to HTML under `build/doc/` (e.g., `README.md`, `CHANGELOG.md`, `RELEASENOTES.md`, … — whatever exists in the repo).
  - Navigation/index is generated from that dynamic root list (no dead-link nav).
- `tack init`: additionally (non-destructively) provisions `templates/` with default CSS/template plus a starter `tack.ini`.
- Convenience: if no CSS is configured, `tack doc`/`tack bom` automatically uses `templates/tack_doc.css` when present.

### v0.7.11
- `tack doc`: optionally includes a `docs/` folder (all `docs/**/*.md`) and generates HTML pages under `build/doc/docs/`.
- `build/doc/index.html` adds a “Docs” section when `docs/**/*.md` exists (anchor `#docs`), and pages include a matching nav link.

### v0.7.10
- Feature: `tack fmt` – orchestrator for external formatters (policy in `tack.ini` via `[fmt]` / `[fmt "NAME"]`).
  - `tack fmt` formats in-place.
  - `tack fmt --check` never modifies files and returns exit code 2 if changes would be required.
  - `--rule/--target/--list/--no-defaults/--diff` (diff best effort via `git diff --no-index` or `diff -u`).

### v0.7.9
- Bugfix: rebuild decisions no longer rely on raw `depfile mtime > object mtime`; they now use recorded dependency metadata (`mtime`/size/FNV-1a), including a depfile fingerprint to detect dependency-graph changes, avoiding false stale detections after cache restore where object and depfile are written sequentially.
- Depfile scanning now also tracks `#include <...>` by resolving them against effective include search paths; header changes through `-I` now trigger recompilation correctly.
- Documentation refreshed (README/FAQ/ROADMAP/RELEASENOTES) and version bumped to `v0.7.9`.

### v0.7.8
- INI: profile-specific target overrides via `[target "NAME".debug]` / `[target "NAME".release]` for `core`, `includes`, `defines`, `cflags`, `ldflags`, `libs`.

### v0.7.7
- Bugfix release: C89 compile fixes after a faulty merge (SBOM path/variables/braces cleaned up).
- `tack sbom`: CycloneDX/SPDX output fully implemented; `[sbom]` supports `format`, `spec_version`, `output` with format-specific defaults.

### v0.7.6
- Documentation sync: aligned README/FAQ/ROADMAP/Release Notes with the current implementation (features/options).
- `tack sbom`: multi-format output (default `tack`), format/output controlled via `[sbom]` in `tack.ini` (CycloneDX/SPDX options documented, not implemented yet).

### v0.7.5
- `tack sbom`: deterministic SBOM export as JSON (`build/sbom.json`, format `tack-sbom-1`).

### v0.7.4
- Bugfix release: compile cache now validates dependencies via `mtime` + file size + content hash (32-bit FNV-1a) to avoid false positives on file systems with coarse timestamp resolution.
- Cache entries are written more atomically (temp files + `rename()`) to reduce partial entries.
- Stricter C89/ANSI-C compatibility (no `snprintf`/`vsnprintf`; stable 32-bit cache key).

### v0.7.3
- Optional compile cache (`.tack-cache/`) for faster incremental builds, disable with `--no-cache`.

### v0.7.2
- `--why` / `--explain`: short diagnostics that explain *why* tack runs a compile/link step (e.g. "output missing", "source newer", "dependency newer", "forced (--rebuild)").

### v0.7.1
- `tack bom`: writes a build manifest (BOM) as `build/bom.md` and `build/bom.html`.
- `tack doc`: writes offline HTML docs into `build/doc/` (Markdown wrappers) and links the BOM.
- Optional: HTML templates + CSS for DOC/BOM via `tack.ini` (`[doc]`/`[bom]`: `template`, `css`).
- HTML output: stable template anchor markers (markers + IDs) for CSS hooks and optional post-processing.
- Fixes/Clarifications (v0.7.1 patch):
  - Fail-fast: missing `template`/`css` files or missing `{{TACK_CONTENT_HTML}}` → exit code 2.
  - `--no-code-config` now actually takes effect.
  - `doc`/`bom` profile parsing fixed (no wrong argument offsets with global options).
  - INI: multi-layer loading works; `[target ...]` is parsed correctly.
  - Template rendering: token lengths fixed (no stray `}`); marker contract clarified (exactly once in output).


### v0.7.0
- `tack init` can provision a `.gitignore` and `.fossil-settings/ignore-glob`.
- **Non-destructive:** existing files are **not overwritten**; a clearly marked tack block is appended only if missing.
- Rationale: keep build artifacts, caches and generator outputs out of version control by default.

### v0.6.x (Hardening series, overview)
- Fail-fast defaults with improved diagnostics.
- More consistent, safer string/path handling (bounds & truncation checks).
- `tack.ini` for declarative configuration plus an optional code path (`tackfile.c` → generated INI) with verification macros.
- Switches to disable code config and auto tool discovery for stricter team/CI setups.

---

**Backlinks:** [README](README.md) • [FAQ](FAQ.md) • [Roadmap](ROADMAP.md)
