# M2 RC0.8 – manuelle Plattformvalidierung ohne GitHub Actions

**Datum:** 20. November 2026  
**Projekt:** tack v0.8.6, ausschließlich Milestone M2 (Prozessausführung)  
**Feature-Branch:** `feature/v0.8.6-m2-process-execution`  
**PR:** #13, Draft bis zur ausdrücklichen Owner-Gesamtabnahme  
**CI-Entscheidung:** GitHub Actions ist bewusst deaktiviert; praeco ist noch nicht bereit. Keine Abhängigkeit von praeco, keine neue CI-Implementierung und keine Reaktivierung eines GitHub-Workflows in M2.

## Aktueller Zusatz: Debian-Trixie-Fehler und Folgeprüfung (20.11.2026)

**Nachtrag zu den obigen historischen Fehlläufen:** Zwei weitere echte Linux-Owner-Läufe nach dem C89-Fix sind **GCC Release PASS** und **Clang Release PASS**, je `29 compiled / 28 passed / 0 failed / 1 skipped / 0 not_run`. GCC `duration_ms=36980`, Clang `duration_ms=34184`; `process_execution_test` und `parallel_test_build_test` jeweils PASS. Lokales `ratchet tests` PASS; der genaue lokale Git-Status und die Remote-Versionierung des seit `1b43b87` geänderten Testinventars sind weiterhin zu prüfen. **Linux GCC/gcov NOT RUN, keine neue Linux-Coverage behauptet.** Details siehe `M2-VALIDATION.md`.

**Beobachtung:** Die ersten beiden Debian-Trixie-/WSL2-GCC-Release-Versuche schlugen **vor der Testausführung** beim Übersetzen von `tests/process_execution_test.c` fehl (29 Tests, compiled=28, failed=1, passed=0, not_run=28). Die ursprüngliche Linux-only-Deklaration `char *noexec[]` ist zum `m2_spawn(const char * const *, Proc *)`-Aufruf nicht qualifikatorkorrekt. **Fix-Commit `1b43b87`** ändert ausschließlich diese Deklaration in `const char * const noexec[]`. Die beiden vollständigen GCC-Fehlläufe sind nicht nachträglich PASS. Der vermeintliche Clang-Lauf mit `set TACK_CC=clang` ist unter Bash **kein Clang-Nachweis**.

**Vor weiteren Tests:** auf Linux `git pull --ff-only` ausführen und mit `git diff 1b43b87^ 1b43b87 -- tests/process_execution_test.c` exakt die eine Änderung prüfen. Die Testinventarprüfung muss wegen der geänderten Testquelle zunächst bewusst fehlschlagen. **Erst nach Owner-Review und ausdrücklicher Genehmigung** auf diesem Linux-Checkout `./build/host/tack --config ./tack.ini ratchet tests --owner-approve` ausführen, `git diff -- coverage/test-inventory.txt` inspizieren und den akzeptierten Inventarzustand anschließend auf dem Feature-Branch versionieren. Keine automatische Inventaränderung durch Agenten. Erst dann GCC Release, Clang Release und GCC/gcov/HWM erneut ausführen. Clang **als** `TACK_CC=clang ./build/host/tack ...` starten, nicht `set TACK_CC=clang`. Windows-Gates nach Annahme des neuen Inventars erneut prüfen, bevor formale M2-Abnahme behauptet wird. Keine HWM-Absenkung und keine Änderung an GitHub Actions.

## 1. Belegte Windows-Evidenz (Owner, 20.11.2026)

Die Messungen sind durchgeführt und nicht als Planwerte zu verstehen:

| Windows, `-j 4` | Ergebnis |
| --- | --- |
| GCC Release | PASS: 29 kompiliert, 28 PASS, 0 FAIL, 1 SKIP |
| TinyCC Debug | PASS: 29 kompiliert, 27 PASS, 0 FAIL, 2 SKIP |
| Clang/MSVC Release | PASS: 29 kompiliert, 28 PASS, 0 FAIL, 1 SKIP |
| GCC Coverage mit `--rebuild` | PASS: 29 kompiliert, 28 PASS, 0 FAIL, 1 SKIP, `status=ok` |
| Windows Coverage-Ratchet | PASS: `4036/6233` = 64,75213 % |
| 29er-Testinventar | PASS, Owner-angenommen und versioniert |
| Windows-HWM | PASS, Owner-angenommen; Blob `b776991ce11aa5d5c2b0f53930148640b9f4864b`; Commit `998e753` |

Der reproduzierte Coverage-Lauf meldete `tack: coverage: PASS HWM retained: 4036/6233`. Die Meldungen aus absichtlichen Negativtests sind keine Suite-Fehler. Die Windows-`gcov`-Meldung „Quelldatei ist neuer als Hinweis-Datei“ bleibt **beobachtet und zu untersuchen**; eine fehlerhafte Messung ist damit nicht bewiesen. `source_state=2e423671`, `tests_state=d3433506`, `flags_state=36eb9e95`.

**Lokaler, nicht repositoryseitig bereinigbarer Punkt:** `?? "tatus --short --branch"` ist eine unversionierte Datei auf dem Windows-Checkout. Inhalt vor einem gezielten Entfernen inspizieren. Kein pauschales `git clean`.

## 2. Verbindlicher manueller Linux-Prüflauf

**Status: NOT RUN.** Die folgenden Befehle sind eine Abnahmeanleitung, kein Nachweis, dass sie bereits ausgeführt wurden. Auf einem Linux-Rechner mit GCC, Clang und `gcov` im Checkout des Feature-Branches ausführen. Die ausführbare Datei bleibt unter `build/`. Shell-Befehle sind nur Aufrufanweisungen, keine neue Projekt-Skriptabhängigkeit.

```sh
git fetch origin
git switch feature/v0.8.6-m2-process-execution
git pull --ff-only origin feature/v0.8.6-m2-process-execution
git status --short --branch
git rev-parse HEAD
git diff --check
git diff --exit-code 712ba87bbb71230fbc3578cfc76a7d0a776125c7 HEAD -- src/ include/ tests/ tack.ini

mkdir -p build/host
gcc -std=c89 -pedantic-errors -Wextra -Werror -o build/host/tack src/tack.c
./build/host/tack --config ./tack.ini ratchet tests

TACK_CC=gcc ./build/host/tack --config ./tack.ini test release --ci --rebuild -j 4
TACK_CC=clang ./build/host/tack --config ./tack.ini test release --ci --rebuild -j 4
TACK_CC=gcc ./build/host/tack --config ./tack.ini test coverage --ci --rebuild -j 4
cat coverage/current.txt
./build/host/tack --config ./tack.ini coverage check

git status --short --branch
```

**Erwartete Gate-Semantik:** GCC/Clang: `TACK_SUMMARY status=ok`, `failed=0`, `not_run=0` (begründete Plattform-SKIPs sind erlaubt). Coverage: vollständige Messung realer Produktionszeilen, nicht nur bestandene Testassertions; akzeptierte Linux-HWM als Untergrenze, kein stilles Absenken. Vorherige Linux-HWM `3604/5790` ist ein historischer Vergleich, **kein** neuer RC0.8-PASS.

Wenn eine **gemessene Verbesserung** das Coverage-Gate als `BLOCKED` meldet, ist das erwartetes Ratchet-Verhalten: erst `coverage/current.txt`, Änderungen und Scope prüfen, dann **nur durch den Owner** getrennt `coverage accept --owner-approve` und `coverage check` ausführen. Ohne diese Freigabe bleibt das Gate BLOCKED. Bei Regression FAIL, niemals die HWM senken, um den Lauf grün zu machen. Keine automatische Ratchet-Annahme durch Agenten.

Falls auf Linux Abweichungen entstehen: konkrete Ausgabe, Compiler-/gcov-Version, `git rev-parse HEAD`, `coverage/current.txt`, betroffenen Test und reproduzierbaren Befehl protokollieren; keine erfundenen PASS-Werte.

## 3. Parallelität und Messdisziplin

Für eine **aussagekräftige Performanceaussage** sind zusätzlich vollständige GCC-Release-Läufe mit `--rebuild -j 1` und `--rebuild -j 4` auf **demselben Rechner und identischer Revision** durchzuführen und ihre `TACK_SUMMARY duration_ms` zu vergleichen:

```sh
TACK_CC=gcc ./build/host/tack --config ./tack.ini test release --ci --rebuild -j 1
TACK_CC=gcc ./build/host/tack --config ./tack.ini test release --ci --rebuild -j 4
```

Der vorhandene `parallel_test_build_test` belegt funktionale Parallelität, **keinen** quantifizierten Gesamtlauf-Speedup. Compiler-/Link-Prozesse dürfen parallel arbeiten; Test-Runtimes bleiben wegen gemeinsamer Fixtures sequenziell. Bei Fehlstart, Compilerfehler oder Child-Abbruch müssen Fail-fast, Prozess-Drain, ehrliche `not_run`-Zahlen und korrekte Reporter-Ausgaben erhalten bleiben.

## 4. Abnahmematrix und Dokumentation

| Gate | Stand |
| --- | --- |
| Windows GCC, TinyCC, Clang | PASS |
| Windows GCC/gcov, HWM, Testinventar | PASS |
| Linux GCC Release | NOT RUN |
| Linux Clang Release | NOT RUN |
| Linux GCC/gcov und Linux-HWM | NOT RUN |
| Voller `-j 1`/`-j 4`-Zeitvergleich | NOT RUN |
| GitHub Actions | Bewusst deaktiviert, **kein** technischer Produkt-Gate |
| praeco | Nicht erforderlich für M2 |
| PR #13 Merge/Release | BLOCKED bis Owner-Abnahme |

Nach echten Linux-Läufen die Befunde in `docs/plans/M2-VALIDATION.md`, `docs/plans/M2-REQUIREMENTS.tsv` (insbesondere P10–P15, P17–P21, P31–P35) und `docs/agents/STATE.md` **mit Quellrevision und Belegen** ergänzen. Historische Angaben nicht still überschreiben. M2 bleibt bis dahin **BLOCKED**. Keine neuen Funktionen, kein BA-STD-C89-Gesamtumbau, kein M3–M8-Scope, keine Änderung des Testinventars oder der HWMs ohne separate Owner-Freigabe. Das v0.8.6-Releaseziel von mindestens 90 % realer Produktions-Line-Coverage auf **beiden** Betriebssystemen bleibt unabhängig von der M2-Funktionsabnahme bestehen.
