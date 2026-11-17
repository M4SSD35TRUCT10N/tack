# Veröffentlichung von tack v0.8.4 / Publishing tack v0.8.4

**Stand:** Releasequellen zur Veröffentlichung freigegeben; Git-Commit, Tag und GitHub-Release noch nicht angelegt.

## Deutsch

### Voraussetzungen und Nachweise

- `src/tack.c` meldet `tack 0.8.4`; `tack.ini` trägt denselben Stand. Der vom Owner bestätigte Windows-RC0.2-Testlauf ergab für GCC/Release und TinyCC/Debug jeweils 25 kompiliert, 24 PASS, einen Plattform-SKIP und null Fehler.
- Das Linux-Referenzprofil mit GCC 14.2.0 und gcov 14.2.0 hat **3.228/5.708 = 56,55221 %** produktive Line-Coverage gemessen und als `coverage/hwm-linux.txt` versioniert. `tack coverage check` muss auf derselben vergleichbaren Umgebung PASS melden.
- `coverage/current.txt`, Build-Ausgaben, `.gcda`/`.gcno` und ausführbare Dateien gehören **nicht** in den Git-Commit. Das versionierte HWM und das Testinventar dagegen schon.
- **Vor GitHub-Push:** Windows-Coverage ist zunächst `NOT RUN`. Sie muss mit der zusätzlichen `id_windows` in `[coverage]` und dem lokalen GCC/gcov **tatsächlich gemessen**, nach Review akzeptiert und als `coverage/hwm-windows.txt` gemeinsam mit der Linux-HWM versioniert werden. Vollständiger Ablauf: [Coverage-Ratchet-Leitfaden](coverage-ratchet.md), Abschnitt 5. Der bestehende GitHub-Workflow auf ARM64/Windows ersetzt die maßgebliche Linux-x86_64-GCC/gcov-Referenzmessung nicht automatisch.
- **RC0.5-Hinweis:** Der Owner-RC0.4-Testlauf auf Windows schlug vor der Messung mit `coverage_ratchet_test: 2 failures` fehl. Dies waren zwei unportable Unix-Pfaderwartungen im Test; RC0.5 prüft stattdessen plattformnative **und** kanonische Pfade. Der neue Windows-Testlauf und die HWM-Annahme stehen weiterhin aus. Vorher kein Tag oder Push.

### Lokale Prüfung (Windows, aktuelle `tack.exe`)

```bat
tack --config .\tack.ini ratchet tests
tack --config .\tack.ini test release --ci --rebuild -j 32
tack --config .\tack.ini doctor release --target app
```

Für den TinyCC-Debug-Lauf `TACK_CC` vor dem Test entfernen und `tack --config .\tack.ini test debug --ci --rebuild -j 32` ausführen. In Windows-CMD: `set TACK_CC=`. Die Linux-C89-/Coverage-Referenzprüfung muss nach dem Quellcode-Nachtrag ebenfalls **erneut** gemessen und mit der angenommenen Linux-HWM abgeglichen werden. Für Windows ist die eigenständige GCC/gcov-HWM nach dem [Coverage-Leitfaden](coverage-ratchet.md) verpflichtend, bevor Tag und Push erfolgen. `coverage check` setzt dort eine frische Messung voraus; mit einer fehlenden Messung meldet es folgerichtig `NOT RUN/BLOCKED`.

### Git-Commit, Tag und GitHub-Release

1. Vollständigen Quellstand übernehmen, die dadurch ersetzte Datei `docs/releases/0.8.4-dev.md` und den alten doppelten Testheader `tests/tack_test_embed.h` im bestehenden Git-Checkout ausdrücklich löschen und Diff sowie `git status` prüfen. Die kanonische Header-Datei bleibt `include/tests/tack_test_embed.h`. Nicht übernommene ältere Commit-Vorlagen aus `docs/releases/` nicht versehentlich committen. `coverage/hwm-linux.txt`, die nach Messung akzeptierte `coverage/hwm-windows.txt` und `coverage/test-inventory.txt` müssen versioniert werden. `coverage/current.txt` nicht committen.
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
- The Linux reference profile with GCC 14.2.0 and gcov 14.2.0 measured **3,228/5,708 = 56.55221%** production line coverage and has been versioned as `coverage/hwm-linux.txt`. `tack coverage check` must PASS on the same comparable reference environment.
- Do **not** commit `coverage/current.txt`, build artifacts, `.gcda`/`.gcno` or executables; do version the HWM and test inventory.
- **Before publishing:** Windows coverage is initially `NOT RUN`; measure it with the platform-specific `id_windows`, explicitly approve the valid result, commit `coverage/hwm-windows.txt` alongside the Linux baseline, then verify both gates. See [coverage acceptance guide](coverage-ratchet.md), section 5. Never copy Linux counts into a Windows baseline. The existing self-hosted GitHub ARM64/Windows workflow does not automatically reproduce the canonical Linux x86_64 GCC/gcov measurement.
- **RC0.5 note:** The owner-provided RC0.4 Windows run failed with `coverage_ratchet_test: 2 failures` before gcov collection. Two Unix-only expected paths in the test were corrected in RC0.5; native and canonical paths are both checked. Fresh Windows test and HWM acceptance remain pending; do not tag or push beforehand.

### Local verification (Windows, current `tack.exe`)

```bat
tack --config .\tack.ini ratchet tests
tack --config .\tack.ini test release --ci --rebuild -j 32
tack --config .\tack.ini doctor release --target app
```

For TinyCC debug, clear `TACK_CC` first (`set TACK_CC=` in CMD) and run `tack --config .\tack.ini test debug --ci --rebuild -j 32`. Revalidate Linux reference coverage after the production code change and follow the [Windows HWM steps](coverage-ratchet.md) before tagging or pushing. `coverage check` requires a fresh local measurement on that machine; with no current measurement, `NOT RUN/BLOCKED` is expected.

### Git commit, tag and GitHub Release

1. Copy the complete source tree, explicitly delete the superseded `docs/releases/0.8.4-dev.md` and duplicate old test header `tests/tack_test_embed.h` from the existing Git checkout; the canonical header is `include/tests/tack_test_embed.h`. Do not accidentally commit superseded commit-message templates. Review the diff and `git status`. Both platform HWM files (`coverage/hwm-linux.txt` and accepted `coverage/hwm-windows.txt`) and `coverage/test-inventory.txt` belong in Git, but `coverage/current.txt` does not.
2. Create **one coherent release commit** containing the accepted HWM together with the corresponding release source.
3. After committing, create an annotated tag and push the branch and tag:

```bat
git tag -a v0.8.4 -m "tack v0.8.4"
git push origin HEAD
git push origin v0.8.4
```

4. Create a GitHub Release from the existing pushed tag `v0.8.4`, using the summary in [RELEASENOTES](../../RELEASENOTES.md). Verify CI results on GitHub. **No completed push, CI run, or published GitHub Release is assumed here.**
