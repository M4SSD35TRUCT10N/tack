\
# GitHub self-hosted runner installation on Linux ARM64

> **DE/EN guide**
>
> DE: Praktischer Leitfaden für einen **Linux-ARM64-Runner** auf Raspberry Pi oder ähnlichen Systemen.
>
> EN: Practical guide for a **Linux ARM64 runner** on a Raspberry Pi or similar system.

---

# Deutsch

## Ziel

Dieser Leitfaden richtet einen **Linux-ARM64-Runner** für `tack`-Workflows ein, passend zum aktuellen Workflow-Starter:

- `self-hosted`
- `linux`
- `ARM64`
- `pi4` (Custom-Label)

## Grundsatz

Nimm **immer die von GitHub im UI angebotenen Installationsbefehle** für genau dein Repository oder deine Organisation. GitHub erzeugt dabei die passende Download-URL und ein **zeitlich begrenztes Registrierungs-Token**.

## Einrichtung in GitHub

1. Repository oder Organisation öffnen.
2. **Settings** → **Actions** → **Runners**.
3. **New self-hosted runner** anklicken.
4. **Linux** und **ARM64** wählen.
5. Die im UI angezeigten Befehle in einer Shell auf dem Pi ausführen.

## Praktischer Ablauf auf Linux ARM64

### 1. Runner-Verzeichnis anlegen

Beispiel:

```sh
mkdir -p ~/actions-runner
cd ~/actions-runner
```

### 2. GitHub-Befehle aus dem UI ausführen

GitHub zeigt dir dafür die passenden Schritte an:

- Download des ARM64-Runner-Pakets
- Entpacken
- `config.sh` mit URL und Registrierungs-Token
- Start des Runners

### 3. Direkt als Dienst einrichten

Nach erfolgreicher Konfiguration erzeugt GitHub auf Linux ein `svc.sh`-Skript. Auf `systemd`-Systemen kannst du es als Dienst installieren.

Typischer Ablauf:

```sh
sudo ./svc.sh install
sudo ./svc.sh start
```

Status prüfen:

```sh
sudo ./svc.sh status
```

Stoppen:

```sh
sudo ./svc.sh stop
```

## Label-Empfehlung für dein Setup

Zusätzlich zu den Default-Labels solltest du dem Pi das Custom-Label geben:

```text
pi4
```

Dann passt er zum Workflow:

```yaml
runs-on: [self-hosted, linux, ARM64, pi4]
```

## Schnellprüfung

Im GitHub-UI unter **Settings → Actions → Runners** sollte der Runner sichtbar sein.

Wenn die Runner-Anwendung verbunden ist, zeigt die Konsole typischerweise:

```text
√ Connected to GitHub
... Listening for Jobs
```

## Empfehlung für den Pi

Für einen Raspberry Pi ist das hier sinnvoll:

- eigener Runner-Benutzer oder zumindest eigenes Runner-Verzeichnis
- separater Checkout für Actions-Jobs
- genug freier Speicher für `build/`, Artefakte und Cache
- stabile Uhrzeit und Netzwerkverbindung

## Wenn du Labels nachträglich setzen willst

Custom-Labels kannst du im GitHub-UI am Runner setzen oder bereits bei der Erstkonfiguration an `config.sh` übergeben.

Beispielidee:

```sh
./config.sh --url <REPOSITORY_URL> --token <REGISTRATION_TOKEN> --labels pi4
```

## Deinstallation / Neuaufsetzen

Wenn du den Runner neu aufsetzen willst:

1. Runner in GitHub entfernen.
2. Dienst stoppen:
   ```sh
   sudo ./svc.sh stop
   ```
3. Dienst deinstallieren:
   ```sh
   sudo ./svc.sh uninstall
   ```
4. Runner-Verzeichnis bereinigen oder neu anlegen
5. Einrichtung über **New self-hosted runner** erneut durchführen

---

# English

## Goal

This guide sets up a **Linux ARM64 runner** for `tack` workflows, matching the current workflow starter:

- `self-hosted`
- `linux`
- `ARM64`
- `pi4` (custom label)

## Principle

Always use the **installation commands shown by GitHub in the UI** for the specific repository or organization. GitHub provides the matching download URL and a **time-limited registration token**.

## Setup in GitHub

1. Open the repository or organization.
2. Go to **Settings** → **Actions** → **Runners**.
3. Click **New self-hosted runner**.
4. Choose **Linux** and **ARM64**.
5. Run the commands shown in the UI in a shell on the Pi.

## Practical flow on Linux ARM64

### 1. Create the runner directory

Example:

```sh
mkdir -p ~/actions-runner
cd ~/actions-runner
```

### 2. Run the GitHub commands from the UI

GitHub will show the exact steps:

- download the ARM64 runner package
- extract it
- run `config.sh` with the URL and registration token
- start the runner

### 3. Install it as a service immediately

After a successful configuration, GitHub creates an `svc.sh` script on Linux. On `systemd` systems you can install it as a service.

Typical flow:

```sh
sudo ./svc.sh install
sudo ./svc.sh start
```

Check status:

```sh
sudo ./svc.sh status
```

Stop it:

```sh
sudo ./svc.sh stop
```

## Label recommendation for your setup

In addition to the default labels, assign the custom label:

```text
pi4
```

That makes the machine match the workflow:

```yaml
runs-on: [self-hosted, linux, ARM64, pi4]
```

## Quick check

Under **Settings → Actions → Runners** the runner should be visible.

When the runner application is connected, the console typically shows:

```text
√ Connected to GitHub
... Listening for Jobs
```

## Recommendation for the Pi

For a Raspberry Pi, this setup is sensible:

- use a dedicated runner user or at least a dedicated runner directory
- use a separate checkout for Actions jobs
- keep enough free storage for `build/`, artifacts, and cache
- keep system time and networking stable

## If you want to set labels afterwards

You can assign custom labels in the GitHub UI or pass them to `config.sh` during initial setup.

Example idea:

```sh
./config.sh --url <REPOSITORY_URL> --token <REGISTRATION_TOKEN> --labels pi4
```

## Rebuild / reinstall

If you want to rebuild the runner cleanly:

1. remove the runner in GitHub
2. stop the service:
   ```sh
   sudo ./svc.sh stop
   ```
3. uninstall the service:
   ```sh
   sudo ./svc.sh uninstall
   ```
4. clean or recreate the runner directory
5. repeat the setup via **New self-hosted runner**
