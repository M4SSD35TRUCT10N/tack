# Veröffentlichung von tack v0.8.4 / Publishing tack v0.8.4

**Stand:** Releasequellen zur Veröffentlichung freigegeben; Git-Commit, Tag und GitHub-Release noch nicht angelegt.

## Deutsch

### Voraussetzungen und Nachweise

- `src/tack.c` meldet `tack 0.8.4`; `tack.ini` trägt denselben Stand. Der vom Owner bestätigte Windows-RC0.2-Testlauf ergab für GCC/Release und TinyCC/Debug jeweils 25 kompiliert, 24 PASS, einen Plattform-SKIP und null Fehler.
- Das Linux-Referenzprofil mit GCC 14.2.0 und gcov 14.2.0 hat **3.171/5.651 = 56,11396 %** produktive Line-Coverage gemessen und als `coverage/hwm-linux.txt` versioniert. `tack coverage check` muss auf derselben vergleichbaren Umgebung PASS melden.
- `coverage/current.txt`, Build-Ausgaben, `.gcda`/`.gcno` und ausführbare Dateien gehören **nicht** in den Git-Commit. Das versionierte HWM und das Testinventar dagegen schon.
- Die Windows-Coverage ist `NOT RUN`. Ein Windows-HWM-Eintrag wird nicht aus der Linux-Messung kopiert. Der bestehende GitHub-Workflow auf ARM64/Windows ersetzt die maßgebliche Linux-x86_64-GCC/gcov-Referenzmessung nicht automatisch.

### Lokale Prüfung (Windows, aktuelle `tack.exe`)

```bat
tack --config .\tack.ini ratchet tests
tack --config .\tack.ini test release --ci --rebuild -j 32
tack --config .\tack.ini doctor release --target app
```

Für den TinyCC-Debug-Lauf `TACK_CC` vor dem Test entfernen und `tack --config .\tack.ini test debug --ci --rebuild -j 32` ausführen. In Windows-CMD: `set TACK_CC=`. Die C89-/Coverage-Referenzprüfung erfolgt unter Linux mit passender GCC/gcov-Version und Profil-ID. `coverage check` setzt dort eine frische Messung voraus; mit einer fehlenden Messung meldet es folgerichtig `NOT RUN/BLOCKED`.

### Git-Commit, Tag und GitHub-Release

1. Vollständigen Quellstand übernehmen, die dadurch ersetzte Datei `docs/releases/0.8.4-dev.md` im bestehenden Git-Checkout ausdrücklich löschen und Diff sowie `git status` prüfen. `coverage/hwm-linux.txt` und `coverage/test-inventory.txt` müssen versioniert werden. `coverage/current.txt` nicht committen.
2. Einen **zusammenhängenden Release-Commit** erstellen; die freigegebene HWM muss im selben Commit wie der Releasequellstand enthalten sein.
3. Nach dem Commit den annotierten Tag erstellen und beide Referenzen pushen:

```bat
git tag -a v0.8.4 -m "tack v0.8.4"
git push origin HEAD
git push origin v0.8.4
```

4. In GitHub unter *Releases* einen Release für den bereits gepushten Tag `v0.8.4` mit der Zusammenfassung aus [RELEASENOTES](../../RELEASENOTES.md) erstellen. CI-Ergebnisse auf GitHub prüfen. **Kein bereits erfolgter Push, CI-Lauf oder Release wird vorausgesetzt.**

## English

### Prerequisites and evidence

- `src/tack.c` reports `tack 0.8.4` and the shipped `tack.ini` matches. Owner-provided Windows RC0.2 runs for GCC/release and TinyCC/debug each compiled 25 tests with 24 PASS, one platform SKIP and zero failures.
- The Linux reference profile with GCC 14.2.0 and gcov 14.2.0 measured **3,171/5,651 = 56.11396%** production line coverage and has been versioned as `coverage/hwm-linux.txt`. `tack coverage check` must PASS on the same comparable reference environment.
- Do **not** commit `coverage/current.txt`, build artifacts, `.gcda`/`.gcno` or executables; do version the HWM and test inventory.
- Windows coverage remains `NOT RUN`. Do not copy Linux HWM values into a Windows baseline. The existing self-hosted GitHub ARM64/Windows workflow does not automatically reproduce the canonical Linux x86_64 GCC/gcov measurement.

### Local verification (Windows, current `tack.exe`)

```bat
tack --config .\tack.ini ratchet tests
tack --config .\tack.ini test release --ci --rebuild -j 32
tack --config .\tack.ini doctor release --target app
```

For TinyCC debug, clear `TACK_CC` first (`set TACK_CC=` in CMD) and run `tack --config .\tack.ini test debug --ci --rebuild -j 32`. Reference coverage validation requires matching Linux GCC/gcov versions and profile ID. `coverage check` requires a fresh local measurement on that machine; with no current measurement, `NOT RUN/BLOCKED` is expected.

### Git commit, tag and GitHub Release

1. Copy the complete source tree, explicitly delete the superseded `docs/releases/0.8.4-dev.md` from the existing Git checkout, then review the diff and `git status`. Both `coverage/hwm-linux.txt` and `coverage/test-inventory.txt` belong in Git, but `coverage/current.txt` does not.
2. Create **one coherent release commit** containing the accepted HWM together with the corresponding release source.
3. After committing, create an annotated tag and push the branch and tag:

```bat
git tag -a v0.8.4 -m "tack v0.8.4"
git push origin HEAD
git push origin v0.8.4
```

4. Create a GitHub Release from the existing pushed tag `v0.8.4`, using the summary in [RELEASENOTES](../../RELEASENOTES.md). Verify CI results on GitHub. **No completed push, CI run, or published GitHub Release is assumed here.**
