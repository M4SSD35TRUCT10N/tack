\
# GitHub self-hosted runner installation on Windows x64

> **DE/EN guide**
>
> DE: Praktischer Leitfaden für einen **repository-nahen self-hosted Runner** auf einem Windows-x64-Rechner.
>
> EN: Practical guide for a **repository-scoped self-hosted runner** on a Windows x64 machine.

---

# Deutsch

## Ziel

Dieser Leitfaden richtet einen **Windows-x64-Runner** für `tack`-Workflows ein, passend zum aktuellen Workflow-Starter:

- `self-hosted`
- `windows`
- `x64`
- `dev-laptop` (Custom-Label)

## Grundsatz

Nimm **immer die von GitHub im UI angebotenen Installationsbefehle** für genau dein Repository oder deine Organisation. GitHub erzeugt dabei die passende Download-URL und ein **zeitlich begrenztes Registrierungs-Token**.

## Empfohlener Zielpfad

GitHub empfiehlt unter Windows den Runner unter:

```text
C:\actions-runner
```

zu installieren, damit Windows-Systemkonten auf das Verzeichnis zugreifen können.

## Einrichtung in GitHub

1. Repository öffnen.
2. **Settings** → **Actions** → **Runners**.
3. **New self-hosted runner** anklicken.
4. **Windows** und **x64** wählen.
5. Die im UI angezeigten Befehle in einer **administrativen PowerShell** oder Eingabeaufforderung ausführen.

## Praktischer Ablauf auf Windows

### 1. Verzeichnis anlegen

```powershell
mkdir C:\actions-runner
cd C:\actions-runner
```

### 2. GitHub-Befehle aus dem UI ausführen

GitHub zeigt dir dafür die passenden Schritte an:

- Download des Runner-Pakets
- Entpacken
- `config.cmd` mit URL und Registrierungs-Token
- Start des Runners

### 3. Dienstbetrieb sofort mitnehmen

Wenn `config.cmd` fragt, ob der Runner als **Windows service** installiert werden soll, antworte mit **yes**.

Wichtig: Wenn der Runner bereits ohne Dienstbetrieb konfiguriert wurde, muss er laut GitHub **aus GitHub entfernt und neu konfiguriert** werden, damit du die Dienst-Option sauber setzen kannst.

## Label-Empfehlung für dein Setup

Zusätzlich zu den Default-Labels solltest du dem Windows-Rechner das Custom-Label geben:

```text
dev-laptop
```

Dann passt er zum Workflow:

```yaml
runs-on: [self-hosted, windows, x64, dev-laptop]
```

## Dienstbetrieb

Nach erfolgreicher Konfiguration kannst du den Runner über die Windows-Diensteverwaltung oder per PowerShell verwalten.

Typische Praxis:

- Starttyp auf automatisch lassen
- Dienst nach der Erstkonfiguration einmal sauber starten
- im GitHub-UI prüfen, ob der Runner als **Idle** bzw. online erscheint

## Schnellprüfung

Im GitHub-UI unter **Settings → Actions → Runners** sollte der Runner sichtbar sein.

Wenn die Runner-Anwendung verbunden ist, zeigt die Konsole typischerweise:

```text
√ Connected to GitHub
... Listening for Jobs
```

## Empfehlung für deinen Rechner

Für einen Entwickler-Laptop ist das hier sinnvoll:

- eigener Runner-Ordner außerhalb des Entwicklungs-Checkouts
- **nicht** im aktiven Arbeits-Checkout bauen
- ein separater Runner-Clone oder Worktree für die Actions-Jobs

## Deinstallation / Neuaufsetzen

Wenn du den Runner neu aufsetzen willst:

1. Runner in GitHub entfernen.
2. lokalen Runner-Dienst stoppen
3. Runner-Verzeichnis bereinigen oder neu anlegen
4. Einrichtung über **New self-hosted runner** erneut durchführen

---

# English

## Goal

This guide sets up a **Windows x64 runner** for `tack` workflows, matching the current workflow starter:

- `self-hosted`
- `windows`
- `x64`
- `dev-laptop` (custom label)

## Principle

Always use the **installation commands shown by GitHub in the UI** for the specific repository or organization. GitHub provides the matching download URL and a **time-limited registration token**.

## Recommended target path

GitHub recommends installing the runner under:

```text
C:\actions-runner
```

so that Windows system accounts can access the runner directory.

## Setup in GitHub

1. Open the repository.
2. Go to **Settings** → **Actions** → **Runners**.
3. Click **New self-hosted runner**.
4. Choose **Windows** and **x64**.
5. Run the commands shown in the UI in an **elevated PowerShell** or command prompt.

## Practical flow on Windows

### 1. Create the directory

```powershell
mkdir C:\actions-runner
cd C:\actions-runner
```

### 2. Run the GitHub commands from the UI

GitHub will show the exact steps:

- download the runner package
- extract it
- run `config.cmd` with the URL and registration token
- start the runner

### 3. Enable service mode immediately

When `config.cmd` asks whether the runner should be installed as a **Windows service**, answer **yes**.

Important: if the runner was already configured without service mode, GitHub says you must **remove it from GitHub and re-configure it** to enable the service option cleanly.

## Label recommendation for your setup

In addition to the default labels, assign the custom label:

```text
dev-laptop
```

That makes the machine match the workflow:

```yaml
runs-on: [self-hosted, windows, x64, dev-laptop]
```

## Service operation

After configuration, you can manage the runner through Windows Services or PowerShell.

Typical practice:

- keep the service start type automatic
- start the service cleanly once after the first configuration
- verify in the GitHub UI that the runner appears as **Idle** or online

## Quick check

Under **Settings → Actions → Runners** the runner should be visible.

When the runner application is connected, the console typically shows:

```text
√ Connected to GitHub
... Listening for Jobs
```

## Recommendation for your machine

For a developer laptop, this setup is sensible:

- keep the runner in its own directory outside your development checkout
- do **not** build in the active working checkout
- use a separate runner clone or worktree for Actions jobs

## Rebuild / reinstall

If you want to rebuild the runner cleanly:

1. remove the runner in GitHub
2. stop the local runner service
3. clean or recreate the runner directory
4. repeat the setup via **New self-hosted runner**
