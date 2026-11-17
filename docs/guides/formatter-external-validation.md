# tack fmt: externer Integrationstest / External formatter integration test

## Deutsch

**Stand:** 0.8.5-dev RC0.3, kein veröffentlichtes 0.8.5-Release. Der Reparaturumfang ist der `[fmt]`-INI-Parser; die folgenden Tests prüfen zusätzlich den existierenden Ausführungspfad. Externe Formatter sind optional und gehören **nicht** zum tack-Buildsystem.

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

### Windows-Gate bleibt offen

Die Owner-Tests unter TDM-GCC 10.3.0 prüfen `fmt_ini_parser_test.c` und die CLI-Regelauswahl; ein **externer** Windows-Formatterprozess wurde bislang nicht nachgewiesen. Als optionale manuelle Prüfung kann ein vorhandenes `clang-format` auf eine **separate Testdatei** angesetzt werden; vorher mit `where clang-format` prüfen. Projekt-Produktivdateien niemals für die Erstprüfung formatieren. Ohne entsprechende Evidenz bleibt dieses Gate `NOT RUN`.

**Grenzen:** `check_cmd`, `diff_cmd`, `env.*` werden eingelesen, aber nicht als eigenständige Kommandos/Umgebungsdefinitionen ausgeführt. `fail_fast=no` gewährleistet noch keine Fehlerfortsetzung. Die Ausführung des separaten OS-Prozesses beweist keine parallele Testausführung.

## English

**Status:** 0.8.5-dev RC0.3, not an 0.8.5 release. The product fix changes `[fmt]` INI parsing; this procedure also exercises the existing external execution path. Formatters are optional tools, not additional build dependencies.

### Verified Linux experiment (2026-11-17)

A tack binary built fresh from RC0.2 with GCC 14.2.0 was run in an **isolated** fixture root. `src/with spaces.c` initially contained `alpha` plus a newline. No production source was modified. The formatter for this experiment was `sed`; this does **not** make sed a mandatory tack dependency.

The fixture's `fmt.ini` uses `mode = file`, `include = src/**/*.c`, `cmd = sed -i s/alpha/beta/g @FILE@`, and `requires = sed`, as shown above. Running `fmt --list --strict` selected one file. The first `fmt --check --strict` returned **2** without changing the original. `fmt --strict` changed `alpha` to `beta`, returning **0**. The second `fmt --check --strict` returned **0**. This checks temp-copy behavior, in-place execution, process invocation, and a filename containing spaces.

### Windows validation still pending

The owner has proved the Windows GCC release/debug tests and the parser regression, but not an external Windows formatter invocation. An installed `clang-format` may be used on **throwaway source files only** after checking `where clang-format`. Do not claim Windows external E2E PASS without the observed process exit codes and original-file checks.

**Limits:** `check_cmd`, `diff_cmd` and `env.*` are parsed but not executed/applied as additional actions. `fail_fast=no` does not guarantee continued execution after failure. The formatter experiment does not prove parallel test scheduling.
