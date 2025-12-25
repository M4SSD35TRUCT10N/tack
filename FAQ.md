# tack FAQ / FQA (DE/EN) — v0.6.6

**Backlinks:** [README](README.md) • [Roadmap](ROADMAP.md)

---

## Deutsch (FAQ)

### Ist tack ein Ersatz für CMake/make?
tack ersetzt für viele Projekte das klassische Build-Skript (Makefile/CMakeLists), indem es eine feste, simple Konvention nutzt und die üblichen Tasks (`build/run/test/clean`) anbietet. Für extrem komplexe Toolchains ist make/cmake weiterhin besser geeignet.

### Welche Compiler funktionieren?
Standard ist **tcc**. Über `TACK_CC` kannst du z.B. `gcc` oder `clang` nutzen, solange sie „klassische“ C‑Kommandozeilen verstehen.  
**Wichtig:** `TACK_CC` ist der **Compiler**, nicht „Compiler + Flags“. Flags gehören in `tack.ini`.

### Warum lehnt tack `TACK_CC="clang -std=c89"` ab?
Weil tack den Compiler als argv[0] startet. Flags würden als Teil des Programnamens verstanden.  
Lösung: `TACK_CC=clang` und `cflags = -std=c89` in `tack.ini`.

### Was ist der Unterschied zwischen `--no-config` und `--no-code-config`?
- `--no-config`: ignoriert **INI + tackfile.c** (alles aus)
- `--no-code-config`: ignoriert **nur tackfile.c**, INI bleibt aktiv (CI/Team‑Modus)

### Wie deaktiviere ich Auto-Tool-Discovery?
- CLI: `--no-auto-tools`
- INI: `[project] disable_auto_tools = yes`
- tackfile.c (Makro): `#define TACKFILE_DISABLE_AUTO_TOOLS 1`

### Wie kann ich Targets deaktivieren oder entfernen?
In INI oder tackfile.c (als generierte INI):
```ini
[target "tool:foo"]
enabled = no
```
oder
```ini
[target "tool:foo"]
remove = yes
```
`enabled=no` lässt das Target existieren, aber „aus“. `remove=yes` entfernt es aus dem Graph.

### Wie aktiviere/deaktiviere ich `src/core/`?
- Pro Target in INI: `core = yes|no`
- Für einen Lauf per CLI: `--no-core`

### Was bedeutet „fail-fast“ in tack?
tack bricht bewusst ab, wenn:
- Pfade/Strings zu lang werden
- INI‑Zeilen abgeschnitten wären (zu lang)
- Rekursionstiefe überschritten wird (Scan/RM)
- Token-/Listen-Limits überschritten werden
- tackfile.c Generator fehlschlägt

Das ist Absicht: lieber **klarer Fehler** statt undefiniertes Verhalten.

### Windows: Was ist mit langen Pfaden?
tack baut Pfade dynamisch, nutzt aber trotzdem harte Limits (fail-fast). Wenn dein Windows Setup Long-Paths unterstützt, hilft das. Bei extrem langen Repo-Pfaden bekommst du eine klare Fehlermeldung.

---

## English (FAQ)

### Is tack a replacement for CMake/make?
For many projects, yes: tack replaces custom build scripts by using conventions and providing `build/run/test/clean`. For very complex toolchains, make/cmake may still be the better fit.

### Which compilers work?
Default is **tcc**. You can set `TACK_CC` to `gcc`/`clang` etc. as long as they behave like classic C compilers.  
Important: `TACK_CC` is the compiler program, not “compiler + flags”. Put flags into `tack.ini`.

### Why does tack reject `TACK_CC="clang -std=c89"`?
Because tack starts the compiler as argv[0]. Flags would be part of the program name.  
Fix: `TACK_CC=clang` and `cflags = -std=c89` in `tack.ini`.

### `--no-config` vs `--no-code-config`?
- `--no-config`: ignore **INI + tackfile.c**
- `--no-code-config`: ignore **only tackfile.c**, still load INI

### How do I disable auto tool discovery?
- CLI: `--no-auto-tools`
- INI: `[project] disable_auto_tools = yes`
- tackfile.c macro: `#define TACKFILE_DISABLE_AUTO_TOOLS 1`

### Disable/remove targets?
```ini
[target "tool:foo"]
enabled = no
```
or
```ini
[target "tool:foo"]
remove = yes
```

### Enable/disable `src/core/`?
- Per target: `core = yes|no`
- Per run: `--no-core`

### What does “fail-fast” mean?
tack intentionally aborts on:
- overly long paths/strings
- truncated INI lines
- recursion depth limits (scan/rm)
- token/list limits
- tackfile.c generator failures

---

Back: **[README](README.md)** · Next: **[ROADMAP](ROADMAP.md)**
