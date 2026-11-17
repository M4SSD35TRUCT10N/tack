# tack fmt: externer Integrationstest / External formatter integration test

## Deutsch

**Aktueller Stand v0.8.5, lokaler finaler Kandidat:** Windows RC0.7 GCC Release (25 PASS/1 SKIP), TinyCC Debug (24 PASS/2 SKIP) und echtes `clang-format 23.1.5`-E2E mit Leerzeichen (0/2/0/0/1/0) wurden vom Owner erfolgreich ausgeführt. Linux GCC/Clang und Formatter E2E PASS. Beide HWM-Erhöhungen sind vom Owner gebilligt; Linux 3505/5710 angenommen und geprüft, Windows 3577/5906 nach Übernahme auf Windows noch einmal zu prüfen. Keine GitHub-Publikation. [Abschlussbericht](../releases/0.8.5.md).

**Historischer Ausgangsstand:** 0.8.5-dev RC0.4, kein veröffentlichtes 0.8.5-Release. RC0.3 korrigierte den `[fmt]`-INI-Parser; RC0.4 korrigiert ausschließlich die nachgewiesene Windows-Argumentübergabe. Externe Formatter sind optional und gehören **nicht** zum tack-Buildsystem.

### RC0.6: aktuelle Korrektur (17.11.2026)

Unter Windows schlugen die echten RC0.5-Kindprozess-Regressionen bei
abschließendem Backslash (GCC und TCC) und eingebettetem Quote (TCC)
fehl. Die Ursache ist ein nicht geschriebenes NUL-Byte nach einer
Platzhalter-Ersetzung am Tokenende in `fmt_replace_all_alloc()`. RC0.6
terminiert diesen Fall explizit; sechs portable Testfälle ergänzen die
bestehenden echten Windows-Kindprozess-Prüfungen. Der `CreateProcessA`-
Prozessstarter ist unverändert. Linux-E2E: 0 / 2 / 0 / 0 / 1 / 0
(List, Check, Unchanged, Format, Changed, Check). Windows RC0.6 NOT RUN.
Siehe [RC0.6-Abnahme](../releases/archive/0.8.5/0.8.5-dev-RC0.6-validation.md).

### Verifizierter Linux-Versuch (17.11.2026)

Verwendet wurde ein frisch aus RC0.2 gebautes `tack`-Binary mit GCC 14.2.0 und ein **isolierter** Arbeitsordner mit `src/with spaces.c` (`alpha` + Zeilenumbruch). Keine Produktivdatei wurde dafür verändert. Die externe Formatter-Implementierung dieses Versuchs ist `sed`; sie wird nicht als allgemeine Projektabhängigkeit eingeführt.

Testkonfiguration `fmt.ini`:

```ini
[fmt]
enabled = yes
fail_fast = yes
defaults = off
rules = probe

[fmt "probe"]
tool = sed
mode = file
include = src/**/*.c
cmd = sed -i s/alpha/beta/g @FILE@
requires = sed
```

Bei Ausführung im isolierten Test-Root (Befehle einzeln; `<TACK>` bezeichnet den absoluten Pfad zum zuvor gebauten tack):

```sh
<TACK> --config ./fmt.ini fmt --list --strict -- 'src/with spaces.c'
<TACK> --config ./fmt.ini fmt --check --strict -- 'src/with spaces.c'
<TACK> --config ./fmt.ini fmt --strict -- 'src/with spaces.c'
<TACK> --config ./fmt.ini fmt --check --strict -- 'src/with spaces.c'
```

**Beobachtung:** Listing eine Datei, erster Check Exitcode **2** und Originalinhalt weiterhin `alpha`, eigentliche Ausführung Exitcode **0** und Originalinhalt `beta`, anschließender Check Exitcode **0**. Der Check arbeitet mit temporärer Kopie; die Originaldatei bleibt unverändert. Der Test benutzt einen Dateinamen mit Leerzeichen und damit das einzelne, shell-frei übergebene `@FILE@`-Argument. `fmt --check` mit Exitcode 2 unterscheidet für sich allein nicht zwischen einer Formatabweichung und bestimmten Konfigurationsfehlern; die getrennten Positiv- und Negativprüfungen sind notwendig.

### RC0.4: Windows-Spaces-Befund und Retest (17.11.2026)

**Tatsächlicher Owner-Befund auf RC0.3:** `clang-format.exe` unter
`C:\Program Files\LLVM\bin\` vorhanden; `fmt --list --strict` findet
`src/with spaces.c` (Exit 0). `fmt --check` und `fmt --strict` scheitern
jeweils mit Exit 1: der aufgerufene Formatter meldet gespaltene
Dateipfad-Fragmente (`...\\fmt-e2e`, `v085\\...`, `spaces.c`). Die
Originaldatei blieb unverändert. Der RC0.3-Windows-E2E-Test ist damit
**FAIL, nicht PASS**. Der gleichzeitig fehlgeschlagene Git-SSH-Fetch ist
ein getrennter Authentifizierungsfehler und kein Formatter-Befund.

**Ursache:** In `fmt_exec_cmd` wird die INI-Kommandozeile zwar in
Argumente tokenisiert und `@FILE@` als einzelner Wert expandiert. Die
Windows-CRT `_spawnvp`-Schnittstelle zerlegt jedoch ungequotete
Argumentwerte mit Leerzeichen. RC0.4 quotiert und escaped ausschließlich
die an den externen Formatter übergebenen Windows-Kindargumente, nach
den Windows-CRT-Regeln (einschließlich Quotes und abschließender
Backslashes). Der Build-Prozessstarter bleibt unverändert; es werden
weder Shell-Kommandos noch Abhängigkeiten eingeführt. In
`tests/fmt_ini_parser_test.c` gibt es passende Windows-Regressionen.

**Owner-Retest erforderlich, RC0.4-Windows-E2E noch NOT RUN:** In einem
getrennten Root mit Leerzeichen (`build\fmt-e2e v085`) `--list` (0),
`--check` vor Formatieren (2, Original bytegleich), `fmt --strict` (0,
Datei geändert), `--check` danach (0) prüfen. Für alle Kommandos eine
neu gebaute RC0.4-Executable einsetzen. `clang-format --version` im
Protokoll festhalten. Geänderte Programmlogik erfordert erneut GCC-
Release, TinyCC-Debug, GCC/gcov-Coverage und getrennte HWM-Prüfung
auf Windows und Linux. Bis zu echten Windows-Logs: kein Release-Gate-PASS.

### Historischer RC0.4-Stand: Windows-Gate war offen

RC0.3 schlug beim echten externen Windows-Formatterprozess fehl; RC0.4 wurde auf Windows noch nicht getestet. `clang-format` ist beim Owner installiert. Für den Retest ausschließlich **separate Testdateien** einsetzen und den erfolgreichen Prozessaufruf samt Exitcodes nachweisen. Projekt-Produktivdateien niemals für die Erstprüfung formatieren. Das RC0.4-Gate bleibt `NOT RUN`, während der RC0.3-Befund `FAIL` ist.

**Grenzen:** `check_cmd`, `diff_cmd`, `env.*` werden eingelesen, aber nicht als eigenständige Kommandos/Umgebungsdefinitionen ausgeführt. `fail_fast=no` gewährleistet noch keine Fehlerfortsetzung. Die Ausführung des separaten OS-Prozesses beweist keine parallele Testausführung.

## English

### RC0.6: missing token terminator after placeholder (2026-11-17)

The owner's RC0.5 Windows GCC/TCC tests failed their real subprocess
checks on paths ending in a backslash (and an embedded double quote for
TinyCC). The root cause is a missing NUL terminator in
`fmt_replace_all_alloc()` when the final token element is a placeholder.
RC0.6 explicitly terminates this result; the `CreateProcessA` runner is
unchanged. Six portable regression checks were added. The isolated Linux
external formatter E2E passes, but Windows RC0.6 remains NOT RUN. See
[RC0.6 validation](../releases/archive/0.8.5/0.8.5-dev-RC0.6-validation.md).

**Current v0.8.5 final local candidate:** Owner Windows RC0.7 GCC release (25 PASS/1 SKIP), TinyCC debug (24 PASS/2 SKIP) and real `clang-format 23.1.5` E2E with spaces (0/2/0/0/1/0) all passed. Linux GCC/Clang and external E2E passed. Both increased HWMs are owner-approved; Linux 3505/5710 was accepted and checked, while Windows 3577/5906 still requires a post-adoption rerun on Windows. No GitHub publication. [Final record](../releases/0.8.5.md).

**Status:** 0.8.5-dev RC0.4, not an 0.8.5 release. RC0.3 fixed `[fmt]` INI parsing; RC0.4 fixes Windows argument quoting in the formatter subprocess path. Formatters are optional tools, not additional build dependencies.

### Verified Linux experiment (2026-11-17)

A tack binary built fresh from RC0.2 with GCC 14.2.0 was run in an **isolated** fixture root. `src/with spaces.c` initially contained `alpha` plus a newline. No production source was modified. The formatter for this experiment was `sed`; this does **not** make sed a mandatory tack dependency.

The fixture's `fmt.ini` uses `mode = file`, `include = src/**/*.c`, `cmd = sed -i s/alpha/beta/g @FILE@`, and `requires = sed`, as shown above. Running `fmt --list --strict` selected one file. The first `fmt --check --strict` returned **2** without changing the original. `fmt --strict` changed `alpha` to `beta`, returning **0**. The second `fmt --check --strict` returned **0**. This checks temp-copy behavior, in-place execution, process invocation, and a filename containing spaces.

### Historical RC0.4 status: Windows validation was pending

The owner has proved the Windows GCC release/debug tests and the parser regression, but not an external Windows formatter invocation. An installed `clang-format` may be used on **throwaway source files only** after checking `where clang-format`. Do not claim Windows external E2E PASS without the observed process exit codes and original-file checks.

### RC0.4 Windows path-space blocker

The owner ran RC0.3 on Windows and showed that `fmt --list` returned 0,
but `fmt --check` and `fmt --strict` returned 1 and passed fragmented
space-containing paths to the external formatter. The input file was
unchanged. This is a confirmed RC0.3 E2E **FAIL**. RC0.4 quotes Windows
CRT `_spawnvp` child arguments within the formatter runner only, with
regression checks for spaces, embedded quotes, and trailing backslashes.
The general build subprocess runner is untouched. A fresh Windows RC0.4
E2E run and full platform test/coverage gates are still **NOT RUN**;
neither HWM approval nor release permission is implied.

**Limits:** `check_cmd`, `diff_cmd` and `env.*` are parsed but not executed/applied as additional actions. `fail_fast=no` does not guarantee continued execution after failure. The formatter experiment does not prove parallel test scheduling.


### RC0.5 owner verification (Deutsch)

Auf RC0.4 zeigte der echte Owner-Windows-Test trotz vorheriger Quotierung per `_spawnvp` beschädigte Dateipfad-Suffixe. Die Werte blieben LIST 0, CHECK_BEFORE 1, UNCHANGED 0, FORMAT 1, CHANGED 0, CHECK_AFTER 1. Der Fixkandidat RC0.5 verwendet unter Windows ausschließlich für externe Formatter `CreateProcessA` mit begrenzter, vollständig terminierter Kommandozeile; der Build-Runner bleibt unverändert. Ein zusätzlicher tatsächlicher Kindprozess-Test prüft Argumentgrenzen unter Windows; ein reiner String-Quoting-Test gilt nicht mehr als hinreichender Nachweis. Der separate `clang-format`-End-to-End-Lauf des Owners ist weiterhin verbindlich. Keine automatische HWM-Annahme und kein Release.

### RC0.5 verification (English)

The owner's RC0.4 Windows run still failed, now with corrupted suffixes on file paths despite CRT `_spawnvp` argument quoting. RC0.5 launches external formatter children via bounded `CreateProcessA` command line only on Windows; the generic build runner remains unchanged. A live spawned-child probe validates argument boundaries, but does not replace the owner's real `clang-format` E2E. Windows RC0.5 validation, both HWM approvals and Git release are still pending.
