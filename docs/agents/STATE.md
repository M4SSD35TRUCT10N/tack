# Aktueller Projektstand / Current agent handoff

## Deutsch

- Ausgangspunkt: vom Owner bereitgestellte `tack.zip`-Quellen und danach v0.8.3-RC0.1-Korrekturen. Neuer Kandidat: `v0.8.4-dev`.
- Fehler: Windows mit `TACK_CC=gcc` brach bei POSIX-only-Symbolen in `incremental_link_test.c` unter `-Werror` ab. Guards korrigiert; erneute Windows-Abnahme **NICHT DURCHGEFÜHRT**.
- Tests: C89-Regression `coverage_ratchet_test.c` und getrennte Skip-Resultate (Exit 77); Linux/GCC Debug und Release haben 24 PASS und 1 Windows-only SKIP von 25 Testquellen.
- Ratchet: Testinventar `coverage/test-inventory.txt` aufgenommen; Änderungen nur nach semantischem Owner-Review und expliziter Annahme. Instrumentierte Linux-GCC/gcov-Messung vorhanden, aber `coverage/hwm-linux.txt` fehlt bis zur Owner-Abnahme.
- Coverage: `tack test coverage --ci --rebuild`, `tack coverage check`, `tack coverage accept --owner-approve`; bei nicht angenommener HWM bleibt CI-Coverage bewusst `BLOCKED`.
- Windows/TDM-GCC und TinyCC für 0.8.4: NICHT DURCHGEFÜHRT. Version: NICHT FREIGEGEBEN. Keine erfundene Null-Baseline.
- Nur `docs/releases/0.8.4-dev.md`; drei alte `COMMIT_MESSAGE_*.txt` werden nicht erneut übernommen.

## English

- Baseline: owner-supplied `tack.zip` followed by v0.8.3-RC0.1 fixes. New candidate: `v0.8.4-dev`.
- Fix: `TACK_CC=gcc` on Windows previously failed on POSIX-only symbols in `incremental_link_test.c` under `-Werror`. Guards corrected; Windows rerun **NOT RUN**.
- Tests: new C89 regression `coverage_ratchet_test.c` and explicit skip status (exit 77); Linux/GCC debug and release have 24 PASS and one Windows-only SKIP across 25 test sources.
- Ratchet: recorded `coverage/test-inventory.txt`; changes require semantic owner review and explicit approval. Linux GCC/gcov instrumented measurement exists, but no accepted `coverage/hwm-linux.txt` until owner approval.
- Coverage commands: `tack test coverage --ci --rebuild`, `tack coverage check`, `tack coverage accept --owner-approve`; missing accepted HWM deliberately keeps coverage CI `BLOCKED`.
- Windows/TDM-GCC and TinyCC v0.8.4: **NOT RUN**. Version **NOT RELEASED**. No invented zero baseline.
- Keep only `docs/releases/0.8.4-dev.md`; do not re-add the three former `COMMIT_MESSAGE_*.txt` files.
