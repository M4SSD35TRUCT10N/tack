# Aktueller Projektstand / Current agent handoff

## Deutsch
### Finale HWM-Annahme v0.8.5 (17.11.2026) — Windows-Nachtest / GitHub-Publikation offen

- Owner Windows RC0.7: GCC Release 25 PASS/1 SKIP; TinyCC Debug 24 PASS/2 SKIP; echte `clang-format 23.1.5`-E2E-Exitcodes 0/2/0/0/1/0 PASS; `git fetch` Exit 0. `tack --version` = 0.8.5.
- Windows HWM **3577/5906** (60,56553 %) wurde vom Owner zur Annahme freigegeben und aus dessen vollständiger RC0.7-Messung exakt in den lokalen Release-Kandidaten übertragen. Ein frischer `coverage check` und `test coverage` unter Windows mit dieser neuen Baseline sind **noch nicht nachgewiesen**.
- Linux HWM **3505/5710** (61,38354 %) wurde durch den Owner freigegeben und mit `coverage accept --owner-approve` auf dem isolierten RC0.7-Linux-Checkout übernommen; anschließender `coverage check` PASS. Quellcode/Tests/INI gegenüber RC0.7 unverändert.
- Kanonischer Abschlussbericht: [0.8.5](../releases/0.8.5.md). Historische 0.8.5-RC0.4–RC0.6-Dokumente unter `docs/releases/archive/0.8.5/`. `docs/releases/0.8.5-rc0.7-validation.md` bleibt als unmittelbar geprüfte Evidenz. **Kein Push/Merge/Tag** ohne separate Freigabe.


### RC0.7: Versionsanhebung und Dokumentationskonsolidierung (17.11.2026) — RELEASE-KANDIDAT, NICHT FREIGEGEBEN

- Owner-RC0.6 Windows: GCC Build/Ratchet/Release PASS (26/25 PASS/1 SKIP); TinyCC Debug PASS (26/24 PASS/2 SKIP); Formatter-E2E mit `clang-format 23.1.5`, Leerzeichen und Exitcodes 0/2/0/0/1/0 PASS; `git fetch` Exit 0.
- Owner-RC0.6 GCC/gcov 3577/5906 (60,56553 %), Linux RC0.6 3505/5710 (61,38354 %). Beide verbesserten Werte sind **nicht** als HWM angenommen. Windows `coverage check` BLOCKED (Exit 2) erwartungsgemäß.
- RC0.7 ändert nur `TACK_VERSION` auf 0.8.5, das mitgelieferte `tack.ini`-Versionslabel und DE/EN-Release-Dokumentation. Keine neuen Features, kein Test-/HWM-Absenken, kein Git-Eingriff. RC0.7 Linux GCC/Clang Strict C89, tack Release/Version 0.8.5, 25 PASS/1 SKIP, Docs/Ratchet/E2E PASS; gcc/gcov 3505/5710 gemessen, HWM BLOCKED. **Windows RC0.7** und beide separaten HWM-Freigaben sowie GitHub-Release erst nach ausdrücklichem Owner-Go. Siehe [RC0.7](../releases/0.8.5-rc0.7-validation.md).

### RC0.6: Formatter-Platzhalter-Terminierung (17.11.2026) — FIXKANDIDAT

- Windows Owner-RC0.5: GCC Release 13 PASS/1 FAIL/12 NOT RUN bei `spawn child with trailing backslash`; TinyCC zusätzlich `spawn child with embedded quote`; Coverage vor Messung abgebrochen, `coverage/current.txt` fehlt. Ratchet/Docs PASS. RC0.5 Windows Formatter-E2E wurde nicht wiederholt.
- Fehlendes `\0` in `fmt_replace_all_alloc` bei `@FILE@` am Tokenende nachgewiesen. Nur diese Terminierung im C89-Formatter-Pfad behoben. Sechs zusätzliche plattformübergreifende Testfälle; drei echte Windows-Kindprozess-Regressionen aus RC0.5 unverändert.
- Linux GCC/Clang C89, GCC Release (25 PASS/1 SKIP), Docs und isoliertes Formatter-E2E PASS; gcov 3505/5710 (61,38354 %) gemessen; HWM BLOCKED. Beide HWM-Dateien unverändert; Testinventar-Fingerprint nur als Review-Kandidat. Windows RC0.6 NOT RUN.
- Version 0.8.4, keine HWM-Annahme, kein Commit/Push/Merge/Tag. Nächstes Gate: echter Windows-Retest RC0.6, dann HWM-Owner-Review. Siehe `docs/releases/archive/0.8.5/0.8.5-dev-RC0.6-validation.md`.

### RC0.5: Windows Formatter CreateProcessA (17.11.2026) — Fixkandidat, keine Freigabe

- Owner-RC0.4-Log: GCC Release 26/25 PASS/1 SKIP, TinyCC Debug 26/24 PASS/2 SKIP; Ratchet und Docs PASS. Windows gcc/gcov 3459/5859 (59,03738 %) gemessen; Coverage-HWM korrekt BLOCKED.
- Externer Windows `clang-format 23.1.5` weiterhin FAIL: `--list` 0; `--check` 1 (erwartet 2); Format 1 (erwartet 0), beschädigte Pfad-Suffixe. Die RC0.4-Quoting-Korrektur vor `_spawnvp` reicht nicht. Original unverändert.
- RC0.5 ersetzt **nur den Formatter-Kindprozessstart** unter Windows durch `CreateProcessA` plus begrenzte Kommandozeile, Warten und Exitcode. Kein Shell-Aufruf, keine zusätzlichen Abhängigkeiten, generischer Build-Runner unverändert. `fmt_ini_parser_test` prüft jetzt auch drei echte Windows-Kindprozesse mit Leerzeichen im Exe- und Argumentpfad, Quotes und trailing Backslash.
- Linux strict GCC/Clang PASS; GCC Release 26/25 PASS/1 SKIP; Linux external formatter PASS; Coverage **3470/5709 (60,78122 %)** erneut gemessen, HWM **BLOCKED**. Angenommene HWMs unverändert. Windows RC0.5 NOT RUN; Owner-Abnahme erforderlich.
- SSH `ssh -T git@github.com` inzwischen authentifiziert, Git-Fetch noch nicht nachgewiesen; weder Version 0.8.5 noch Commit, Push, Merge, Tag freigegeben.

### RC0.4: Windows-Formatter-Argumente mit Leerzeichen (17.11.2026)

- Owner-RC0.3-E2E: `clang-format` installiert; `--list` PASS; `--check`
  und Write mit echten Leerzeichen im Verzeichnis und Dateinamen FAIL
  (je Exit 1), die Originaldatei blieb bytegleich. Nachgewiesener
  Windows-Release-Blocker, nicht als PASS umgedeutet.
- RC0.4-Kandidat: Quoting ausschließlich von Windows-Kindargumenten in
  `fmt_exec_cmd()` vor `_spawnvp`, mit Tests für Leerzeichen, Quotes und
  abschließende Backslashes. Keine Änderung des globalen Build-Runners.
- Neu gemessen auf Linux: GCC Release und Formatter-E2E; Windows-Retest,
  TinyCC, Windows-Coverage und beide HWM-Gates stehen weiterhin aus.
  Testinventaränderung bedarf Owner-Diff-Review.
- SSH-`git fetch` schlug wegen `Permission denied (publickey)` fehl;
  lokales `.git` blieb laut Owner `dev/v0.8.5` clean. Kein Git-Push,
  Merge, Tag, Versionssprung oder HWM-Annahme vorgenommen.


### RC0.3: Windows-HWM und externes Formatter-E2E auf Linux (17.11.2026)

- **Owner-Windows-Abnahme:** TDM-GCC Release und TinyCC Debug jeweils 26 kompiliert, 25 PASS, 1 SKIP, 0 FAIL. GCC/gcov maß **3434/5827 = 58,93256 %**, erhöhte Windows-HWM auf Nutzerbefehl angenommen und nach vollständiger Neu-Messung erneut **PASS HWM retained**. Profil/Fingerprints wurden wortgetreu aus `coverage/hwm-windows.txt` übernommen.
- **Linux:** Externer Formatter mit isolierter `sed`-Fixture auf echtem CLI-Pfad erfolgreich (`--list`, `--check` vor und nach tatsächlicher Formatierung; Dateiname mit Leerzeichen; temporärer Check verändert Original nicht). [Reproduzierbarer Ablauf](../guides/formatter-external-validation.md).
- **Linux-HWM:** vorher angenommen **3228/5708**; vollständige Messung aus RC0.2 **3470/5708 (60,79187 %)** ist ein höherer, noch nicht vom Owner angenommener Kandidat. Nicht stillschweigend erhöhen.
- **Offen:** Windows externe Formatter-Ausführung; Linux-HWM-Annahme und Re-Check, tatsächlicher Versionssprung auf 0.8.5 mit nötiger Neumessung und Release-Abnahme. Produktions-C und Testbestand sind gegenüber RC0.2 unverändert. Kein Tag/Push.

- **0.8.5-dev RC0.2 (17.11.2026):** Vorher unter `[sbom]` verschachtelte `[fmt]`- und `[fmt "NAME"]`-INI-Zweige sind wieder eigenständig erreichbar. Neuer `tests/fmt_ini_parser_test.c` prüft Regelwerte, CLI-Listing, Leerzeichen und strikte Negativfälle; Testinventar erweitert. Linux GCC Release **26 kompiliert, 25 PASS, 1 SKIP**; instrumentiert 3470/5708 (**60,79187 %**, verbesserter Kandidat, HWM noch nicht vom Owner angenommen). Der zuvor akzeptierte Linux-HWM-Wert **3228/5708 bleibt unverändert**; Windows-Compiler, Windows-Coverage und externe Formatter-Ausführung für RC0.2 sind NOT RUN. Für v0.8.5 noch Versionsanhebung und plattformübergreifende Abnahme nötig. BA-STD-C89 v0.3 DRAFT ergänzt Commit-Nachrichten, Portabilität, CLI-Tests und parallelisierbare Tests; keine Gruppenratifikation behauptet.

- **0.8.5-dev Dokumentationsmeilenstein (17.11.2026):** Quellstand aus freigegebenem 0.8.4/RC0.6, GitHub-`main` Blob-SHAs für `src/tack.c`, `tack.ini`, README und Windows-HWM abgeglichen. README DE/EN erklärt 0.8.2-Profilerweiterung jetzt im Hauptteil; neue `docs/guides/tack-ini.md`, `docs/reference/tack-ini.md`, Plan für 0.8.6. Keine Produktionscode-Änderung, kein Auto-Tag und keine erneute HWM-Abnahme in diesem dokumentarischen Meilenstein. **Neuer P1-Befund:** `[fmt]`/`[fmt "NAME"]`-Schlüssel-Handler stehen in `ini_load_file` innerhalb `if (sec == SEC_SBOM)`; Ausführung unerreichbar, konkreter `fmt --list --strict`-Test ergibt `missing cmd`. Getrennten C89-Fix mit Regressionstests und Windows-/Linux-Gates vor 0.8.5-Freigabe durchführen. Owner meldete inzwischen Windows-Coverage-Gate PASS (3192/5827) nach RC0.6.

- **RC0.6 Windows-HWM-Versionierung (17.11.2026):** Owner-CMD-Protokoll von RC0.5 bestätigt `ratchet tests` PASS, GCC Release und GCC/gcov Coverage je 25 kompiliert, 24 PASS, 1 SKIP, 0 FAIL. `coverage/current.txt` enthielt **3192/5827 (54,77947 %)**, `profile=windows-gcc-10.3.0-gcov-10.3.0-coverage-v1` und alle Hash-/Testfelder. Auf ausdrücklichen Owner-Auftrag als `coverage/hwm-windows.txt` versioniert; getrennte Linux-HWM **3228/5708 (56,55221 %)** unverändert. Normatives HWM-Verhalten im DE/EN-Coverage-Leitfaden und FAQ dokumentiert. **Windows-`coverage check` nach Übernahme ins Checkout noch offen; vor PASS kein Tag/Push.**

- **RC0.5 Windows-Gate (17.11.2026):** Der aktuelle Owner-Snapshot `tack.zip` enthält RC0.4 ohne Commit. Windows/GCC Release und Coverage brachen in `coverage_ratchet_test` mit exakt zwei Fehlern ab, vor der gcov-Messung. Ursache: zwei Unix-only-Pfaderwartungen gegenüber Windows-`path_join`. RC0.5 verwendet Windows-/POSIX-Sollwerte sowie einen zweiten kanonischen `build/`-Pfadvergleich mit Pfaddiagnose. Produktionscode unverändert; Testinventar-Fingerprint des korrigierten Tests aktualisiert. Linux/GCC Release **24 PASS, 1 SKIP**, instrumentierte Linux-Messung **3228/5708 HWM PASS**, **25** gcno/gcda-Paare unter `build/`, keine im Root. Windows-Retest/HWM-Freigabe ausstehend; **kein Tag/Push**.
- **RC0.4, Windows-Coverage-Gate vor dem GitHub-Tag:** Der Owner bestätigte TDM-GCC/gcov `(tdm64-1) 10.3.0`, der instrumentierte Testlauf bestand die funktionalen Tests, brach aber bei 0/25 gefundenen `.gcno` ab und legte Artefakte im Root ab. Korrektur: GCC `-c` auf explizite Objekte unter `build/tests/coverage/obj/`, dann separater Link; `.gcno` dort verpflichtend, Collector entsprechend umgestellt, alte `coverage/current.txt` vor Neumessung invalidiert. **Erneuter Windows-Messlauf und HWM-Abnahme ausstehend, kein Push.**

- **tack v0.8.4:** vom Owner zur GitHub-Veröffentlichung freigegeben; `TACK_VERSION` und Projekt-`tack.ini` sind auf `0.8.4` synchronisiert. Veröffentlichungs-Tag und Push stehen noch aus.
- **Windows RC0.2 (historischer Stand, vor `id_windows`):** Owner-Protokolle: GCC Release und TinyCC Debug jeweils 25 kompiliert, 24 PASS, 1 SKIP, 0 FAIL; `ratchet tests` PASS.
- **Linux Releasequelle:** GCC Debug und striktes GCC Release mit jeweils 25 kompiliert, 24 PASS, 1 Windows-only-SKIP, 0 FAIL.
- **Coverage:** Linux GCC 14.2.0 / gcov 14.2.0, Referenzprofil aus `tack.ini`: nach OS-ID-Erweiterung erneut mit 3.228/5.708 (56,55221 %) gemessen und in `coverage/hwm-linux.txt` angehoben; `coverage check` muss PASS sein. `coverage/current.txt` ist eine nicht versionierte Messdatei.
- **Nächster Gate:** Windows-HWM wurde aus gültiger RC0.5-Owner-Messung versioniert; anschließenden `coverage check` und vollständigen `test coverage` auf Windows wiederholen und PASS protokollieren. Die Linux-HWM wurde nach der RC0.4-Quelländerung frisch geprüft und auf 3.228/5.708 angehoben (PASS). Windows-HWM wurde erst nach tatsächlicher Windows-Messung aufgenommen. Gruppenweiter BA-STD-C89-Entwurf bleibt nicht ratifiziert.
- **Historie:** RC0.2 behob Windows-Pfade/Zeilenenden im Testinventar. Kein Test zur Freigabe abgeschwächt. Nur finaler `docs/releases/0.8.4.md` ist der aktuelle Releasebericht; alte dev-Stände 0.8.2/0.8.3 bleiben historische Dokumente.

## English
### Final v0.8.5 HWM adoption (2026-11-17) — Windows post-check and GitHub publication pending

- Owner Windows RC0.7: GCC release 25 PASS/1 SKIP; TinyCC debug 24 PASS/2 SKIP; real `clang-format 23.1.5` E2E 0/2/0/0/1/0 PASS; `git fetch` exit 0. `tack --version` = 0.8.5.
- Owner-approved Windows HWM **3577/5906** (60.56553%) copied exactly from the complete RC0.7 owner measurement. A fresh Windows `coverage check` and instrumented test rerun using the new baseline are **not yet observed**.
- Owner-approved Linux HWM **3505/5710** (61.38354%) accepted using `coverage accept --owner-approve` on isolated RC0.7 Linux checkout and checked PASS. Production source/tests/INI unchanged from RC0.7.
- Canonical final record: [0.8.5](../releases/0.8.5.md). Historical RC0.4–RC0.6 notes moved to `docs/releases/archive/0.8.5/`; RC0.7 validation remains as evidence. **No push/merge/tag** without separate explicit permission.


### RC0.7: version promotion and release docs (2026-11-17) — NOT RELEASED

- Owner's RC0.6 Windows evidence: GCC build/ratchet/release 25 PASS/1 SKIP, TinyCC debug 24 PASS/2 SKIP; real `clang-format 23.1.5` E2E with spaces and expected 0/2/0/0/1/0; `git fetch` exit 0.
- Windows RC0.6 gcov 3577/5906 (60.56553%) and Linux RC0.6 gcov 3505/5710 (61.38354%) are measured improvements, but **not accepted** HWMs. Windows coverage gate BLOCKED (exit 2) by design.
- RC0.7 changes only `TACK_VERSION` to 0.8.5, the shipped INI comment and bilingual release documentation. No new features, HWM reduction, weakened tests or Git mutations. Fresh Linux RC0.7 GCC/Clang C89, Release (25 PASS/1 SKIP), docs, ratchet and external E2E all PASS; gcov 3505/5710 measured, HWM BLOCKED. Windows RC0.7 and separate owner HWM approvals remain mandatory before publication. [RC0.7 record](../releases/0.8.5-rc0.7-validation.md).

### RC0.6: terminate trailing formatter placeholders (2026-11-17) — candidate

- Owner Windows RC0.5 GCC and TinyCC each failed one real child-process test (13 PASS, 1 FAIL, 12 NOT RUN). GCC: trailing backslash; TCC: trailing backslash plus embedded quote assertion. Coverage never produced `coverage/current.txt`; ratchet and docs passed.
- Root cause: `fmt_replace_all_alloc` did not NUL-terminate a placeholder replacement at the end of the token. Narrow C89 change, six new cross-platform regression checks. RC0.5 live Windows child-process tests and `CreateProcessA` runner unchanged.
- Linux strict GCC/Clang C89, GCC release (25 PASS/1 SKIP), docs and external formatter E2E passed. gcov measured 3505/5710 (61.38354%); HWM BLOCKED. Both accepted HWMs unchanged. Windows RC0.6 NOT RUN, source test fingerprint subject to owner review.
- Version still 0.8.4. No HWM acceptance, commit, push, merge or tag. Next gate: full real Windows RC0.6 suite and external `clang-format` E2E. See `docs/releases/archive/0.8.5/0.8.5-dev-RC0.6-validation.md`.

### RC0.5: Windows formatter CreateProcessA (2026-11-17) — Candidate, not approved

- Owner RC0.4 evidence: Windows GCC release 26/25 PASS/1 SKIP, TinyCC debug 26/24 PASS/2 SKIP; ratchet and docs PASS. GCC/gcov 3459/5859 (59.03738%) measured, HWM correctly BLOCKED.
- Real Windows `clang-format 23.1.5` E2E still FAILED: list 0, check 1 instead of 2, write 1 instead of 0; paths contained corrupted suffixes. Source original unchanged. RC0.4 `_spawnvp` argument quoting alone was insufficient.
- RC0.5 uses `CreateProcessA` only for external formatter child execution with bounded explicit command line and synchronous wait/exit-code checks; generic build runner unchanged, no shell/new dependencies. Added three live Windows child process probes to the existing parser test, retaining all prior assertions.
- Linux GCC/Clang C89 builds PASS, GCC release 26/25 PASS/1 SKIP, external formatter PASS, Linux coverage measured 3470/5709 (60.78122%), HWM BLOCKED; both accepted HWM files unchanged. Windows RC0.5 not run; owner verification required. SSH `ssh -T` succeeds but Git fetch still unverified. No version promotion or Git release actions.

### RC0.4: Windows formatter argument quoting (2026-11-17)

- Owner RC0.3 E2E: installed `clang-format`; `--list` PASS, actual
  `--check` and write FAIL due to split file/directory paths containing
  spaces; original source remains byte-identical.
- RC0.4 candidate changes formatter-only Windows CRT `_spawnvp` argument
  quoting and adds regression checks for spaces, quotes and trailing
  backslashes. General build process spawning remains unchanged.
- Linux GCC release retested; Windows formatter E2E, TinyCC and coverage
  require owner retest. Test inventory fingerprint is review-pending.
  Both coverage HWMs remain owner-controlled and unchanged.
- SSH `git fetch` failed independently with `Permission denied
  (publickey)`. No push, merge, tag or version promotion.


### RC0.3: Windows HWM and real external formatter execution on Linux (2026-11-17)

- **Owner Windows validation:** TDM-GCC release and TinyCC debug each compiled 26 tests, 25 PASS, one SKIP, zero failures. GCC/gcov measured **3434/5827 = 58.93256%**. The owner accepted this increased Windows HWM and confirmed **PASS HWM retained** in a fresh complete instrumented run. Profile and fingerprints are transcribed from the owner-provided baseline.
- **Linux:** Verified real external formatting in an isolated fixture using CLI `--list`, `--check` before/after in-place formatting, a filename with spaces, and unchanged original content during the check. See the [reproduction guide](../guides/formatter-external-validation.md).
- **Linux HWM:** Previously accepted baseline **3228/5708** remains unchanged. Fully measured improved candidate **3470/5708 (60.79187%)** still requires owner approval; do not silently raise it.
- **Pending:** Windows external formatter execution, Linux HWM approval/recheck, version promotion to 0.8.5 (requiring fresh measurement) and release approval. C89 production and tests are byte-identical to RC0.2. No Git tag/push.

- **0.8.5-dev RC0.2 (2026-11-17):** Restored independent `[fmt]`/`[fmt "NAME"]` parsing previously nested under `[sbom]`. New `tests/fmt_ini_parser_test.c` checks rule fields, CLI listing, spaces in paths and strict negative cases, with refreshed test inventory. Linux GCC release **26 compiled, 25 PASS, one SKIP**; measured instrumented coverage 3470/5708 (**60.79187%**), improved candidate **BLOCKED pending owner HWM acceptance**. Accepted Linux HWM **3228/5708 is untouched**; Windows compiler/coverage and external formatter execution NOT RUN. Version promotion and cross-platform acceptance remain pending. BA-STD-C89 v0.3 DRAFT adds commit messages, portability, CLI output and parallel-test targets; group-wide ratification not claimed.

- **0.8.5-dev documentation milestone (2026-11-17):** Based on the accepted 0.8.4/RC0.6 source, verified key GitHub `main` blob SHAs for `src/tack.c`, `tack.ini`, README and Windows HWM. README DE/EN now explains 0.8.2 compiler profiles in the main body; added `docs/guides/tack-ini.md`, `docs/reference/tack-ini.md` and a separate 0.8.6 plan. Production C89 source unchanged; no auto-tag or HWM re-acceptance. **New P1 defect:** `[fmt]` / `[fmt "NAME"]` key handlers are nested within the `SEC_SBOM` branch, so explicit formatter rule INI keys are unreachable (`fmt --list --strict` reports `missing cmd`). Fix and revalidate cross-platform before v0.8.5 release. Owner subsequently proved Windows coverage gate PASS (3192/5827) against RC0.6.

- **RC0.6 versioned Windows HWM (2026-11-17):** Owner's RC0.5 Windows log proves test inventory PASS, GCC release and GCC/gcov coverage with 25 compiled, 24 PASS, one SKIP, zero failures. `coverage/current.txt` recorded **3192/5827 (54.77947%)**, Windows TDM-GCC/gcov 10.3.0 profile and all source/test/flag hashes. On explicit owner request, the record is versioned as `coverage/hwm-windows.txt`; separate Linux **3228/5708 (56.55221%)** HWM is unchanged. Gate behavior is documented in the DE/EN guide and FAQ. **Fresh Windows `coverage check` against the adopted file remains pending; do not tag/push before PASS.**

- **RC0.5 Windows gate (2026-11-17):** The owner supplied the uncommitted RC0.4 snapshot `tack.zip`. Windows GCC release and coverage aborted in `coverage_ratchet_test` with precisely two failures before gcov measurement. Two Unix-only expected paths conflicted with native Windows `path_join`. RC0.5 checks platform-native and canonical `build/` paths, reporting actual/expected values on failure. Production code is unchanged; the test inventory fingerprint is updated. Linux GCC release **24 PASS/1 SKIP**, instrumented Linux line coverage **3228/5708 HWM PASS**, all **25** gcno/gcda pairs under `build/` and none in the root. Windows retest and HWM approval pending; **no tag/push**.
- **RC0.4, Windows coverage gate before GitHub tag:** Owner verified TDM-GCC/gcov `(tdm64-1) 10.3.0`. Instrumented functional tests passed but the collector found 0/25 `.gcno` files and old GCC behavior polluted the root. Fix: compile with `-c` to objects under `build/tests/coverage/obj/`, then link separately; require notes there, collect from that directory, invalidate previous `coverage/current.txt` before a new measurement. **Fresh Windows measurement and owner HWM acceptance pending; do not push.**

- **tack v0.8.4:** owner-approved for GitHub publication; `TACK_VERSION` and `tack.ini` agree on `0.8.4`. Push and release tag not yet performed.
- **Windows RC0.2 (historical, before `id_windows`):** owner logs prove GCC release and TinyCC debug each compiled 25, with 24 PASS, one SKIP and zero failures; `ratchet tests` PASS.
- **Linux release source:** GCC debug and strict GCC release each compiled 25, with 24 PASS, one Windows-only SKIP and zero failures.
- **Coverage:** fresh Linux GCC 14.2.0 / gcov 14.2.0 reference measurement after platform ID changes (3,228/5,708, 56.55221%), matching `tack.ini`, owner-accepted and versioned as `coverage/hwm-linux.txt`; `coverage check` must PASS. `coverage/current.txt` is an unversioned generated measurement.
- **Next gate:** the owner-measured Windows HWM is now versioned; rerun `coverage check` and the complete `test coverage` on Windows and require PASS before push; the Linux HWM was remeasured at 3,228/5,708 after the RC0.4 source change and checked PASS. The Windows HWM was versioned only after actual measurement. Group-wide BA-STD-C89 draft is not ratified.
- **History:** RC0.2 fixed Windows path/newline inventory portability without weakening tests. `docs/releases/0.8.4.md` is the canonical final release record; older 0.8.2/0.8.3 dev documents remain historical.
