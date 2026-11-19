# ADR 0005 – Native Windows-Pfade in der Produktions-Coverage-Frischeprüfung

**Datum / Date:** 2026-11-19
**Status:** M2 RC0.7 Korrekturkandidat; Plattformtests und Owner-Ratchets NOT RUN
**Scope:** tack v0.8.6 M2, P32 mandatory coverage/source-inventory integrity. No M3–M8 features.

## Deutsch

### Befund und Ursache

Nach echten Owner-Läufen RC0.6a auf Windows (TinyCC Debug, GCC Release, Clang/MSVC Release, GCC Coverage: jeweils 28 compiled / 27 PASS / 0 FAIL / 1 SKIP) wurde eine frische gcov-Quote **3845/6075** gemessen und `coverage check PASS` bestätigt. Zugleich zeigen die Windows-Coverage-Records RC0.5 und RC0.6a `source_state=811c9dc5` trotz geänderter `src/tack.c`. `0x811c9dc5` ist exakt der FNV-1a-Initialwert. Codepfad: `cov_workspace_state(0)` scannt unter Windows mit `scan_dir_recursive_suffix`/`path_join_alloc` native Backslash-Pfade (`src\...`, `include\...`); `cov_is_production` akzeptierte ausschließlich Prefixe `src/` und `include/`. Es wurden folglich keine produktiven Dateihashes in den State eingerechnet. Der separate `tests_state` war davon nicht betroffen. `cov_fresh` vergleicht den aktuellen State mit erneut berechnetem Workspace-State, was auf Windows bislang den leeren Startwert verglich.

Diese Schwäche betrifft die **Source-Byte-Freshness-Prüfung**, nicht den Nachweis, dass `gcov` die in `coverage/current.txt` genannte Produktionszeilenquote tatsächlich gemessen hat. Der Vergleich `cov_compare` prüft HWM gegen aktuelle Quote, Plattform/Compiler, Quellenmenge und Flags; `source_state`/`tests_state` werden dort absichtlich nicht gegen historische HWM-State-Werte geprüft, damit unveränderte Quoten bei legitimen Quelländerungen PASS bleiben. `cov_fresh` muss dagegen die aktuelle Messung gegenüber dem aktuellen Workspace absichern. **RC0.6a P32 bleibt BLOCKED**, auch wenn die gemessene Quote PASS ist.

### Entscheidung für RC0.7

Die vorhandene `cov_is_production`-Klassifizierung akzeptiert ab RC0.7 unmittelbar nach dem Stamm `src` oder `include` **beide** zulässigen Verzeichnistrenner (`/` und `\`). `include/tests` bleibt für beide Schreibweisen ausdrücklich ausgeschlossen. Klassifizierung von `srcmalicious` oder `include-not-api` ist negativ. Dateisystempfade werden **nicht** manipuliert; die Änderung ist ausschließlich eine logische Pfadklassifizierung im bestehenden C89-Einzeldateiprogramm.

`tests/coverage_ratchet_test.c` ergänzt auf beiden Plattformen Positiv-/Negativassertions mit Slash- und Backslash-Pfaden und eine tatsächlich auf dem Checkout laufende Probe, dass `cov_workspace_state(0)` erfolgreich und ungleich dem initialen FNV-Wert ist. Sämtliche bisherigen Assertions bleiben aktiv. Keine zusätzlichen Compilerflags, keine neuen Abhängigkeiten, kein neues Buildsystem und keine Testabschaltung.

### Freigabegates und Baselines

Neue Source-/Testbytes invalidieren Fingerprints und benötigen die explizite Annahme des neuen Testinventars. Neue Windows-TinyCC/GCC/Clang-Läufe plus echte GCC/gcov-Messung sind **NOT RUN**. Wegen der reparierten Produktionspfadklassifizierung muss `source_state` auf Windows künftig von `811c9dc5` abweichen. Die akzeptierte Windows-HWM **3845/6075** und Linux-HWM **3604/5790** bleiben unberührt; wenn sich die reale Coverage-Quote erhöht, muss der Owner eine vergleichbare neue HWM ausdrücklich annehmen, wenn sie sinkt muss das Gate FAIL bleiben. Die ursprünglichen Produktions-Coveragequoten dürfen niemals stillschweigend 'repariert' oder neu interpretiert werden. Linux GCC/Clang/gcov weiterhin **NOT RUN**; M2 nicht abgeschlossen, Draft PR #13 offen.

## English

Windows scanners return native backslash-separated paths while `cov_is_production` previously accepted only `src/` and `include/`. As a result, the Windows `source_state` was the unchanged FNV-1a offset basis `0x811c9dc5` and production-byte freshness was ineffective. The measured gcov ratio itself was real. RC0.7 narrowly accepts either slash after `src`/`include`, excluding `include/tests` with either separator, and adds mixed-path checks and an actual nonempty production-state regression. Strict C89, all tests, -Werror and both accepted HWMs remain unchanged. No Windows or Linux RC0.7 gate has run; renewed Owner source-inventory approval and real gcov measurement are required before M2 acceptance.
