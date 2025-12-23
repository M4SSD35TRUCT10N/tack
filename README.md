# tack — Tiny ANSI-C Kit

`tack` ist ein **Build- und Projekt-Werkzeug in einer einzigen C-Datei** (C89/ANSI-C), gedacht für Projekte, die **ohne Make/CMake/Ninja** auskommen sollen und trotzdem:

- **mehrere Targets** (App + Tools + Demos) sauber bauen,
- **Shared Code** (Core) wiederverwenden,
- **konfigurierbar** bleiben, ohne dass man gleich ein „Build-Ökosystem“ mitschleppt,
- und sich hervorragend mit **tcc** (Tiny C Compiler) fahren lassen.

> Leitidee: **Build-Logik ist Code.** Wenn dein Projekt C ist, darf auch die Build-Pipeline C sein.

---

## Warum tack (und warum nicht Make/CMake/Ninja)?

### Warum sich historisch trotzdem Make/CMake/Ninja etc. durchgesetzt haben
- **Make** war die erste, pragmatische Lösung für Abhängigkeitsmanagement (Zeitstempel → neu bauen).
- **CMake** ist ein Meta-System, um „Makefiles für alles“ zu erzeugen (inkl. IDE-Projekten).
- **Ninja** ist ein schneller Executor für große Build-Graphen (typisch: von CMake/meson generiert).
- **jam/b2** adressiert „regelbasierten Build-Graph“ anders als Make.
- **mk (Plan 9/9front)** ist sehr elegant, aber nicht überall vorhanden.

### Warum tack trotzdem sinnvoll ist
- Du willst **keinen Build-Stack** (Generator → Executor → Toolchain).
- Du willst **eine sehr schmale Pipeline**: `tcc -run tack.c ...`
- Du willst Build-Logik **debuggen wie C-Code** (statt Build-Skriptsprache).
- Du willst **Portabilität** (C89), inklusive minimalistischer Systeme.

**tack** ersetzt nicht alle Features großer Systeme, aber es deckt den typischen Bereich ab:
- kleine bis mittlere C-Projekte,
- Tools + App + Tests,
- klare Konventionen,
- schnelles Iterieren.

---

## Features (v0.4.3)

- **Single-File Build Driver** (C89)
- **Keine externen Build-Tools** (Make/CMake/Ninja entfallen)
- **Recursive Scanning**: `src/**/*.c`, `tools/<name>/**/*.c`, `tests/**/*_test.c`
- **Target Discovery**: `app` + `tool:<name>` (aus `tools/`)
- **`tack list`**: Targets anzeigen
- **Robuste Prozessausführung** (kein `system()` für Build-Kommandos)
- **Parallel Compile** via `-j N`
- **Depfiles** (`-MD -MF`) für incremental builds
- **Strict Mode**: `--strict` aktiviert `-Wunsupported` (Default unterdrückt, damit Windows-Header nicht „fatal warnen“)
- **Echte Target-Konfiguration** (Includes/Defines/Libs pro Target)
- **Shared Core Code**: `src/core/` wird 1× pro Profil gebaut und in Targets gelinkt
- **tackfile.c (optional)**:
  - Overrides auslagern,
  - Targets hinzufügen/ändern,
  - Targets deaktivieren/entfernen,
  - Auto Tool Discovery abschaltbar (komplett deklarativ).

---

## Installation / Einbindung

### Minimal
Lege `tack.c` in die Root deines Repos.

Empfohlen:
- `tack.c` im Repo versionieren
- optional: `tackfile.c` (nur Projekt-Konfiguration)

### Aufruf unter Windows (tcc)
```bat
tcc -run tack.c help
tcc -run tack.c init
tcc -run tack.c build debug -v -j 8
tcc -run tack.c run debug -- --hello "Berlin"

