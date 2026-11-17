# Test- und Coverage-Ratchet / Test and coverage ratchets

**Stand / status:** v0.8.4-dev, Entwicklungskandidat / development candidate, not released.

## Deutsch

### 1. Test-Ratchet (separat von Coverage)

Das versionierte `coverage/test-inventory.txt` erfasst Namen und Fingerprints aller `_test.c`-Dateien. Prüfen:

```cmd
tack --config .\tack.ini ratchet tests
```

Änderungen dürfen nur nach Diff- und Assertion-Review akzeptiert werden. Ein Fingerprint beweist **keine** fachliche Gleichwertigkeit. Explizit nach Owner-Entscheid:

```cmd
tack --config .\tack.ini ratchet tests --owner-approve
```

### 2. Messprofil (INI-first)

```ini
[target "app".coverage]
compiler = gcc
compiler_policy = gcc
cflags = -std=c89;-pedantic-errors;-Wextra;-Werror;--coverage;-O0
ldflags = --coverage

[coverage]
id = linux-gcc-14.2.0-gcov-14.2.0-coverage-v1
```

Die Profil-ID ist **kein** automatischer Versionsnachweis. Der Owner gleicht sie mit dem tatsächlich verwendeten `gcc --version` und `gcov --version`, der Plattform und der Messdefinition ab. Für Windows muss eine eigene `windows-...`-Profil-ID verwendet werden. Compilerflags dürfen ausschließlich in `tack.ini` stehen. Unter Linux ohne installiertes TinyCC kann für **Compilerwahl, nicht für Compilerflags**, `TACK_CC=gcc` beim Testlauf gesetzt werden; sonst schlägt der generische `functional_smoke_test` zu Recht wegen fehlendem Compiler fehl.

### 3. Messen, prüfen, annehmen

```cmd
tack --config .\tack.ini test coverage --ci --rebuild -j 1
tack --config .\tack.ini coverage check
```

Der erste Befehl misst Produktionszeilen (`src/` und `include/`, ohne `include/tests/`) mittels GCC/gcov und schreibt `coverage/current.txt`. Solange keine freigegebene plattformspezifische `coverage/hwm-*.txt` existiert, wird der Coverage-Lauf **nicht grün**: `BLOCKED` trotz erfolgreich ausgeführter Tests. Ein niedrigerer Wert als eine akzeptierte HWM ist FAIL; ein höherer bleibt bis zur Owner-Freigabe BLOCKED. `--rebuild` und frische Messdaten verhindern veraltete Instrumentierung.

Nach Prüfung des angezeigten `covered/total`, der Testresultate, Quell-/Toolchain-Filter und eines dokumentierten Ownersentscheids:

```cmd
tack --config .\tack.ini coverage accept --owner-approve
tack --config .\tack.ini coverage check
```

Die Anhebung ist Teil **desselben freigegebenen Commits** wie die zugehörige höhere Messung. Ein neues Messprofil braucht Vergleichsmessung und dokumentierte Migration; das Absenken der bisherigen HWM ist verboten. `coverage/current.txt` ist **kein** angenommenes HWM und bleibt als lokaler Messbericht von Commit-Artefakten getrennt.

### 4. Windows-Gates

Die vom Owner gemeldete alte Windows-TCC-Suite meldete 24 PASS, zählte aber tatsächlich Plattform-SKIPs als PASS. Das ist für v0.8.4 korrigiert, jedoch nicht erneut auf Windows geprüft. Die Owner-Windows-Abnahme muss GCC/TDM-GCC und TinyCC getrennt ausführen. Ein GCC-Coverageprofil auf Windows erfordert eine überprüfte `windows-...`-ID und ein eigenes `coverage/hwm-windows.txt`.

## English

### 1. Test ratchet (separate from coverage)

The versioned `coverage/test-inventory.txt` stores names and fingerprints for `_test.c` files. Check using `tack --config .\tack.ini ratchet tests`. Updates require owner review of the diff and assertions, as fingerprints **do not prove semantic equivalence**. After approval use `tack --config .\tack.ini ratchet tests --owner-approve`.

### 2. Measurement profile (INI-first)

The `[target "app".coverage]` profile above runs GCC with `--coverage` and strict C89 flags; the `[coverage] id` must match actual gcc/gcov toolchain versions, platform and denominator definition. It is **not** auto-probed. For Windows, use a separate `windows-...` ID. Never put build flags in shell scripts. On Linux when TinyCC is absent, select an available compiler with `TACK_CC=gcc` for the test invocation; this selects a compiler, not build flags. The generic functional smoke test now fails rather than silently passing with a missing compiler.

### 3. Measure, check, accept

`tack --config .\tack.ini test coverage --ci --rebuild -j 1` runs instrumented tests and writes real production-code line coverage to `coverage/current.txt`; test headers, test sources and build files are excluded. With no accepted platform-specific HWM the gate deliberately returns `BLOCKED` even if tests pass. `tack --config .\tack.ini coverage check` validates a current measurement against an approved HWM. The owner explicitly accepts a reviewed higher baseline with `tack --config .\tack.ini coverage accept --owner-approve`, then rechecks. Commit a raised HWM in the **same accepted revision** as its validated increase. Never weaken test assertions, reset HWM or compare incompatible profiles to produce PASS.

### 4. Windows verification

The earlier Windows TCC suite reported 24 PASS but counted internal platform skips as PASS. v0.8.4 separates them; this has **not yet been re-run on Windows**. Owner verification must run GCC/TDM-GCC and TinyCC separately; Windows instrumented coverage needs its own verified `windows-...` profile ID and `coverage/hwm-windows.txt`.
