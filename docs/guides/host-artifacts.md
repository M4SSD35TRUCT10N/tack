# Host-Artefakte unter build/ / Bootstrap and CI output layout

**19.11.2026 | Ziel für tack v0.8.6 M2, CI-YAML geändert, echte Runner-Abnahme offen.**

## Windows CMD
Aus dem Repository-Root, für die aktuellen M2-Tests:

```bat
cd /d C:\Projects\tack
if not exist build\host mkdir build\host
gcc -std=c89 -pedantic -Werror -o build\host\tack.exe src\tack.c
build\host\tack.exe --config .\tack.ini ratchet tests
build\host\tack.exe --config .\tack.ini test release --ci --rebuild -j 1
```

**Nur nach Prüfung, dass die Root-Datei tatsächlich der alte, lokal gebaute Bootstrap ist**, das alte Binary entfernen. Dieses Kommando löscht **keine** Datei unter `build/`:

```bat
dir /a tack.exe
del /p tack.exe
git status --short --branch
```

Kein pauschales `git clean -fdx` oder `rd /s /q build`: Dadurch könnten Abnahme-Logs und bislang benötigte lokale Artefakte verloren gehen. `.gitignore` mit `*.exe` **versteckt** die Datei nur vor Git, es bereinigt sie nicht.

## Linux / POSIX
```sh
mkdir -p build/host
gcc -std=c89 -pedantic -Werror -o build/host/tack src/tack.c
./build/host/tack --config ./tack.ini ratchet tests
```
Bestehende ältere Root-Binaries nur nach `ls -l ./tack` und eigener Prüfung entfernen.

## CI
Self-hosted GitHub Actions kompiliert künftig unter `build/host/`, schreibt Berichte unter `build/ci/` und sammelt Uploads unter `build/artifacts/`. Der CI-Dateipfad-Commit belegt allein **keinen** erfolgreichen Workflow-Lauf. Falls alte `out/`- oder Root-Binaries von früher existieren, getrennt prüfen und manuell löschen. Die vollständige Clang-Matrix ist ein eigenes verpflichtendes v0.8.6-Freigabegate und in diesem Workflow noch nicht automatisiert.

## English
Bootstrap `tack` to `build/host/tack` or `build/host/tack.exe` and keep CI reports/uploads under `build/ci` and `build/artifacts`. Never create new host executables in repository root. `*.exe` in gitignore does **not** remove old local files. Delete prior root binaries only after manual inspection; never run broad destructive cleanup as a substitute.
