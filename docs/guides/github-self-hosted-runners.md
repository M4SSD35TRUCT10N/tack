# tack — GitHub self-hosted runner starter

---

> **DE/EN Guide**
>
> DE: Diese Anleitung richtet einen kleinen, pragmatischen Startpunkt für `tack` mit zwei self-hosted GitHub-Runnern ein: Windows x64 und Linux ARM64.
>
> EN: This guide sets up a small, pragmatic starting point for `tack` with two self-hosted GitHub runners: Windows x64 and Linux ARM64.

## Deutsch

## Zielbild

- **Windows x64** auf dem Entwicklungs-Laptop
  - Standard-Labels: `self-hosted`, `windows`, `x64`
  - zusätzliches Custom-Label: `dev-laptop`
- **Linux ARM64** auf dem Raspberry Pi 4
  - Standard-Labels: `self-hosted`, `linux`, `ARM64`
  - zusätzliches Custom-Label: `pi4`

Die mitgelieferte Workflow-Datei `.github/workflows/tack-self-hosted-ci.yml` nutzt genau diese Label-Kombinationen.

## Empfohlene Reihenfolge

1. **Runner-Software aktualisieren**
   - Stelle sicher, dass beide self-hosted Runner aktuell sind.
   - Für die hier verwendeten offiziellen Actions-Versionen muss der Runner neu genug für Node 24 sein.

2. **Repository-Runner hinzufügen**
   - GitHub-Repo öffnen
   - **Settings** → **Actions** → **Runners**
   - **New self-hosted runner**
   - passendes Betriebssystem und passende Architektur wählen

3. **Custom-Labels setzen**
   - Windows-Rechner: `dev-laptop`
   - Raspberry Pi: `pi4`

4. **Runner als Dienst betreiben**
   - Windows: als Dienst konfigurieren
   - Linux: als Dienst/Daemon konfigurieren

5. **Compiler bereitstellen**
   - Beide Runner brauchen einen funktionierenden `gcc` im `PATH`, weil der Start-Workflow `src/tack.c` zunächst mit `gcc -std=c89 -pedantic -Werror` baut.

6. **Workflow einspielen**
   - Datei nach `.github/workflows/tack-self-hosted-ci.yml` kopieren
   - committen und pushen
   - im GitHub-Repo unter **Actions** starten oder per Push auslösen

## Was der Workflow macht

Pro Runner-Job:

1. Repository auschecken
2. `tack`-Hostbinary aus `src/tack.c` bauen
3. `tack test` mit CI-Ausgaben fahren
   - `--ci`
   - `--events-jsonl`
   - `--report-tap`
   - `--report-junit`
4. `tack bom`, `tack sbom` und `tack doc` ausführen
5. Berichte und generierte Dateien als Workflow-Artefakte hochladen

## Sicherheitsleitplanke

Der Start-Workflow ist absichtlich auf `push` und `workflow_dispatch` begrenzt.

`pull_request` ist in der Datei nur als Kommentar vorbereitet. Das sollte erst aktiviert werden, wenn das Repository und das Beitragsmodell für self-hosted Runner wirklich vertrauenswürdig sind.

## Anpassungspunkte

- **Branch-Namen**: `main`/`master` nach Bedarf anpassen
- **Compiler**: Falls du auf einem Runner bewusst `tcc` verwenden willst, kannst du `TACK_CC` pro Job ändern
- **Labels**: `pi4` und `dev-laptop` sind nur Empfehlungen; die Workflow-Datei muss zu den tatsächlich vergebenen Labels passen
- **Artefakte**: Der Workflow lädt aktuell TAP, JUnit XML, Events-JSONL, BOM, SBOM und DOC hoch

---

## English

## Target layout

- **Windows x64** on the development laptop
  - default labels: `self-hosted`, `windows`, `x64`
  - extra custom label: `dev-laptop`
- **Linux ARM64** on the Raspberry Pi 4
  - default labels: `self-hosted`, `linux`, `ARM64`
  - extra custom label: `pi4`

The supplied workflow file `.github/workflows/tack-self-hosted-ci.yml` uses exactly these label combinations.

## Recommended order

1. **Update the runner application**
   - Make sure both self-hosted runners are up to date.
   - For the official action versions used here, the runner must be new enough for Node 24.

2. **Add repository runners**
   - open the GitHub repository
   - go to **Settings** → **Actions** → **Runners**
   - choose **New self-hosted runner**
   - select the matching operating system and architecture

3. **Assign custom labels**
   - Windows machine: `dev-laptop`
   - Raspberry Pi: `pi4`

4. **Run the runner as a service**
   - Windows: configure it as a service
   - Linux: configure it as a service/daemon

5. **Provide the compiler**
   - Both runners need a working `gcc` in `PATH`, because the starter workflow first builds `src/tack.c` with `gcc -std=c89 -pedantic -Werror`.

6. **Add the workflow**
   - copy the file to `.github/workflows/tack-self-hosted-ci.yml`
   - commit and push it
   - start it from the **Actions** tab or trigger it via `push`

## What the workflow does

Per runner job it:

1. checks out the repository
2. builds the `tack` host binary from `src/tack.c`
3. runs `tack test` with CI outputs
   - `--ci`
   - `--events-jsonl`
   - `--report-tap`
   - `--report-junit`
4. runs `tack bom`, `tack sbom`, and `tack doc`
5. uploads reports and generated files as workflow artifacts

## Safety guardrail

The starter workflow is intentionally limited to `push` and `workflow_dispatch`.

`pull_request` is only left in the file as a commented template. Enable it only when the repository and contribution model are truly trusted for self-hosted runners.

## Adjustment points

- **Branch names**: adjust `main`/`master` as needed
- **Compiler**: if you intentionally want `tcc` on a runner, change `TACK_CC` per job
- **Labels**: `pi4` and `dev-laptop` are recommendations only; the workflow file must match the labels you actually assign
- **Artifacts**: the workflow currently uploads TAP, JUnit XML, events JSONL, BOM, SBOM, and DOC
