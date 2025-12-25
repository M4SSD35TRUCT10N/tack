# tack FAQ

**Backlinks:** [README](README.md) • [Roadmap](ROADMAP.md)

---

## Deutsch

### Ist tack „Cargo für C“?
**Vom Workflow her: ja (DX‑Ziel).**  
**Vom Umfang her: nein.** Cargo bringt Paketverwaltung/Registry/Lockfiles mit. tack fokussiert auf **build/run/test** und **Target‑Konfiguration**.

### Unterstützt tack Paket-/Abhängigkeitsmanagement?
Aktuell: **nein**.

tack kann externe Libraries einbinden über:
- `libs` / `ldflags` in `tack.ini`
- Systempakete (z.B. SDL2 via Paketmanager)
- Vendoring (Library‑Code direkt im Repo)
- optional ein “Bridge‑Ansatz” über `pkg-config` (liefert CFLAGS/LIBS), wenn du das in `tackfile.c` automatisieren willst.

### Warum C89/ANSI‑C?
Portabilität und geringe Abhängigkeiten. C89 ist alt, aber auf sehr vielen Systemen verfügbar (tcc, alte Toolchains, exotische Targets).

### „tack.ini“ oder „tackfile.c“?
**Standard: `tack.ini`.**  
Nimm `tackfile.c`, wenn du Logik brauchst, z.B.:
- Plattform‑Weichen (Windows vs. Linux)
- Feature‑Matrix (Debug‑Flags, optionale Libs)
- automatische Ableitung von Includes/Defines

### Wie funktioniert tackfile.c in v0.6.0 genau?
Wenn `tackfile.c` im Projekt‑Root liegt (und du nicht `--no-config` nutzt), dann:

1) tack baut einen kleinen Generator unter `build/_tackfile/`  
2) dieser Generator erzeugt `build/_tackfile/tackfile.generated.ini`  
3) tack lädt die generated INI als **Konfig‑Layer** (unter `tack.ini`, über Built‑ins)

**Fail‑fast:** Wenn `tackfile.c` existiert, aber das Generieren fehlschlägt, bricht tack ab. Das macht Pipelines deterministisch.

### Was ist der Unterschied zwischen clean und clobber?
- **clean:** löscht Inhalte unter `build/`, lässt `build/` selbst stehen  
- **clobber:** löscht `build/` komplett

### Welche Projekte passen besonders gut zu tack?
- kleine bis mittlere C‑Repos
- “single repo, simple targets”
- Tools/Utilities + gemeinsame `src/core/`
- Projekte, die bewusst keine Generator‑Toolchain wollen

### Wo sind die Grenzen?
- keine Paketverwaltung (noch)
- keine IDE‑Projektgenerierung
- komplexe Plattform‑Abhängigkeiten bedeuten weiterhin: Libs/Flags kennen und sauber dokumentieren

---

## English

### Is tack “Cargo for C”?
**Workflow-wise: yes (DX goal).**  
**Scope-wise: no.** Cargo includes package management/registry/lockfiles. tack focuses on **build/run/test** and **target configuration**.

### Does tack do dependency/package management?
Currently: **no**.

You can still use external libraries via:
- `libs` / `ldflags` in `tack.ini`
- system packages (e.g. SDL2 from your OS package manager)
- vendoring (keep sources in the repo)
- optionally `pkg-config` as a bridge (CFLAGS/LIBS), automated via `tackfile.c` if you want

### Why C89/ANSI-C?
Portability and minimal dependencies. C89 runs on many compilers and niche systems.

### Should I use tack.ini or tackfile.c?
**Default: `tack.ini`.**  
Use `tackfile.c` when you need logic, e.g.:
- platform switches (Windows vs Linux)
- feature matrix (debug flags, optional libs)
- automatically deriving includes/defines

### How does tackfile.c work in v0.6.0?
If `tackfile.c` exists in the project root (and you don’t use `--no-config`):

1) tack builds a small generator under `build/_tackfile/`  
2) the generator writes `build/_tackfile/tackfile.generated.ini`  
3) tack loads that generated INI as a **config layer** (below `tack.ini`, above built-ins)

**Fail-fast:** if `tackfile.c` exists but generation fails, tack exits non-zero (deterministic pipelines).

### clean vs clobber?
- **clean:** delete contents under `build/`, keep the directory  
- **clobber:** delete `build/` entirely

### What projects are a good fit?
- small to medium C repos
- simple target graphs
- tools/utilities + a shared `src/core/`
- projects that want to avoid generator toolchains

### What are current limitations?
- no package manager (yet)
- no IDE project generator
- complex platform dependencies still require knowing/recording correct flags/libs

---

**Backlinks:** [README](README.md) • [Roadmap](ROADMAP.md)
