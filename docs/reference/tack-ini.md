# tack.ini: Schlüsselreferenz / Configuration key reference

> **Implementierungsstand:** `src/tack.c` aus tack 0.8.4 (dokumentiert für 0.8.5-dev). Diese Liste stammt aus dem realen INI-Parser `ini_load_file` und der Formatter-Ausführung. **Der Parserfehler aus 0.8.4 ist in 0.8.5-dev RC0.2 korrigiert und durch einen C89-Regressionstest abgedeckt; die tatsächliche Windows-Abnahme steht noch aus.** Nicht alle eingelesenen Schlüssel haben Ausführungswirkung.
>
> **Implementation scope:** `src/tack.c` from tack 0.8.4, documented during 0.8.5-dev. This is derived from `ini_load_file` and formatter execution. **The misplaced `[sbom]` brace found in 0.8.4 is fixed in 0.8.5-dev RC0.2 and covered by a C89 regression test. Windows validation is pending; not every accepted key has runtime semantics.**

---

# Deutsch

## Syntax und Geltungsbereich

- Sektionen `[project]`, `[target "NAME"]`, `[target "NAME".debug]`, `[target "NAME".release]`, `[target "NAME".coverage]`, `[doc]`, `[bom]`, `[sbom]`, `[fmt]`, `[fmt "NAME"]`, `[coverage]`.
- Leerzeilen und mit `;` oder `#` beginnende **ganze Kommentarzeilen** werden ignoriert. Ein Inline-Kommentar hinter einem Wert ist **nicht zuverlässig unterstützt**: `key = value ; note` kann als Teil des Werts interpretiert werden. Für portable Beispiele Kommentare auf separate Zeilen setzen.
- Sektionennamen und Schlüssel werden ohne Beachtung der Groß-/Kleinschreibung verglichen. Target- und Regelnamen sind konkrete Bezeichner.
- Boolean-Werte: `yes/no`, `true/false`, `on/off`, `1/0`. Ungültige Booleans werden in den meisten Sektionen nicht aktiv übernommen; deshalb Eingaben sorgfältig prüfen.
- Listen mit `;` trennen; Whitespace und quotierte Tokens sind zusätzlich möglich. Leere Listen bedeuten **nicht in jedem Layer** ein ausdrückliches Löschen der vorherigen Liste.
- `--config PATH` vor dem Kommando; Projekt-INI vor optionalem `tackfile.c`-Layer vor Built-ins. `TACK_CC` hat Vorrang bei der **Compilerwahl**, aber nicht automatisch bei der Compiler-Policy.

## `[project]`

| Schlüssel | Typ / Werte | Semantik |
|---|---|---|
| `default_target` | Targetname | CLI-Default, eingebaut `app` |
| `disable_auto_tools` | Boolean | Tool-Autodiscovery abschalten; Standard `no` |
| `allow_unsafe_paths` | Boolean | Sicherheitsprüfungen für Targetpfade bewusst lockern; Standard `no` |
| `compiler` | Compilerprogramm | Für alle Profile, soweit nicht überschrieben; eingebaut `tcc` |
| `compiler_policy` | `auto`, `tcc`, `gcc`, `clang`, `generic` | Eingebaute compilerspezifische Flags steuern; Standard `auto` |

`[project]` legt keinen universellen Linker oder Coverage-Prozentsatz fest. Die Compilerwahl ist **Programmwahl** und die Compiler-Policy eine getrennte Klassifikation. `doctor [debug|release] --target NAME` erläutert beide.

## `[target "NAME"]` (Basis)

| Schlüssel | Typ | Semantik |
|---|---|---|
| `src` | relativer Ordner | Quellbaum dieses Targets; `src/app` kann bei App-Autodiscovery bevorzugt werden |
| `bin` | sicherer Name | Name der ausführbaren Datei |
| `id` | sicherer Name | Verzeichnis-ID unter `build/` |
| `enabled` | Boolean | Target deaktivieren, wenn `no` |
| `remove` | Boolean | Geerbtes/gefundenes Target aus Definitionen entfernen, wenn `yes` |
| `core` | Boolean | `src/core` beim Linken verwenden |
| `includes` | Liste | Extra-Include-Pfade; `-I` wird ergänzt |
| `defines` | Liste | Extra-Makros; `-D` wird ergänzt |
| `cflags` | Liste | Extra-Compilerflags |
| `ldflags` | Liste | Extra-Linkerflags |
| `libs` | Liste | Extra-Link-Bibliotheken |

**Nicht unterstützt in der Basis-Sektion:** `compiler`, `compiler_policy`. Solche Felder würden hier stillschweigend ignoriert. Sie gehören in `[project]` oder `[target "NAME".PROFILE]`.

## `[target "NAME".debug|release|coverage]` (Profil)

| Schlüssel | Typ | Semantik |
|---|---|---|
| `compiler` | Programm | Compiler-Override nur für dieses Target/Profil |
| `compiler_policy` | Enum wie `[project]` | Policy-Override nur für Target/Profil |
| `core` | Boolean | Core-Linking für dieses Profil überschreiben |
| `includes` | Liste | Extra-Includes für dieses Profil |
| `defines` | Liste | Extra-Defines für dieses Profil |
| `cflags` | Liste | Extra-Compilerflags für dieses Profil |
| `ldflags` | Liste | Extra-Linkerflags für dieses Profil |
| `libs` | Liste | Extra-Bibliotheken für dieses Profil |

Im Profil **nicht** unterstützt: `src`, `id`, `bin`, `enabled`, `remove`. Der `coverage`-Profilname ist für instrumentierte Läufe reserviert; `--coverage`/`gcov` sind nicht automatisch in gewöhnlichen Builds aktiv. `cflags` und `ldflags` müssen dafür passend konfiguriert werden.

## `[doc]`, `[bom]`, `[sbom]`, `[coverage]`

| Sektion | Schlüssel | Bedeutung / Grenzen |
|---|---|---|
| `[doc]` | `template`, `css` | Optionaler HTML-Template-/CSS-Pfad; gesetzte fehlende Datei = Fehler |
| `[doc]` | `allow_js_search` | Boolean; optionale JS-Suche, Standard `no` |
| `[bom]` | `template`, `css` | Gleiches Konzept für BOM; bei fehlendem BOM-Override kann DOC-Konfiguration greifen |
| `[sbom]` | `format` | `tack`, `tack-sbom-1`, `cyclonedx`, `cyclonedx-1.4`, `spdx`, `spdx-2.3` |
| `[sbom]` | `spec_version` | Formatabhängige Version; kein Dependency-Resolver |
| `[sbom]` | `output` | Nur **Single-Target**-Ausgabe; `--all-targets` hat eigene Dateien |
| `[coverage]` | `id` | Vergleichs-ID für nicht-Windows-/Standardprofil, z. B. Linux GCC/gcov |
| `[coverage]` | `id_windows` | Separate Windows-Vergleichs-ID für tatsächlich verwendete GCC/gcov-Version |

Coverage-HWM-Dateien (`coverage/hwm-linux.txt`, `coverage/hwm-windows.txt`) sind **keine INI-Felder**. `coverage/current.txt` wird generiert und nicht versioniert. Eine Owner-Freigabe erfordert zuvor eine gültige vollständige Messung. Die Ratchet-Kommandos stehen im [Coverage-Leitfaden](../guides/coverage-ratchet.md).

## `[fmt]` (globale Formatter-Policy, Parser repariert in 0.8.5-dev RC0.2)

| Schlüssel | Typ / Werte | Wirksames Verhalten (vorbehaltlich Windows-Abnahme) |
|---|---|---|
| `enabled` | Boolean | `no` deaktiviert `tack fmt`; Default `yes` |
| `defaults` | `auto` / `off`, `no`, `false` | Automatische Regeln bei vorhandenen Tools/Dateien; Default `auto` |
| `rules` | Namenliste | Reihenfolge der gewählten Regeln |
| `exclude` | Glob-Liste | Global ausgeschlossene Formatierdateien |
| `fail_fast` | Boolean | **Eingelesen**, aber die aktuelle Runtime bricht bei einem Regelfehler ohnehin ab; `no` ist keine garantierte Fortsetzung |

## `[fmt "NAME"]` (Regel, Parser repariert in 0.8.5-dev RC0.2)

| Schlüssel | Typ / Werte | Wirksames Verhalten (vorbehaltlich Windows-Abnahme) |
|---|---|---|
| `tool` | Programmname | Bezeichnung des Formatters |
| `mode` | `file` / `batch` | Einzeldatei- oder Response-File-Modus |
| `include` | Glob-Liste | Pflicht für wirksame Dateiauswahl |
| `exclude` | Glob-Liste | Zusätzliche Ausschlüsse |
| `cmd` | Befehlstemplate | `@FILE@` in `file`, `@RSP@` in `batch` verpflichtend |
| `requires` | Programmliste | Werkzeuge im PATH prüfen |
| `workdir` | Verzeichnispfad | Arbeitsverzeichnis für die Ausführung |
| `check_cmd` | Zeichenfolge | **Eingelesen, derzeit vom Ausführungspfad nicht benutzt** |
| `diff_cmd` | Zeichenfolge | **Eingelesen, derzeit vom Ausführungspfad nicht benutzt** |
| `env.NAME` | Zeichenfolge | **Eingelesen, derzeit nicht an Kindprozesse übergeben** |

In 0.8.5-dev RC0.2 werden explizite Formatter-INI-Regeln korrekt eingelesen, und `fmt --list --strict` ist im C89-Regressionstest nachgewiesen. Die **interne** `fmt --check`-Implementierung arbeitet mit temporären Kopien und Vergleichen; ein externes Formatter-End-to-End-Gate und die Windows-Abnahme stehen noch aus. `--diff` ist best effort. `fmt --strict` prüft u. a. unbekannte Formatter-Schlüssel und Platzhalter; außerhalb des Formatterbereichs werden unbekannte INI-Schlüssel nicht generell streng zurückgewiesen. Daraus **keine** vollständige Schema-Validierung aller Sektionen ableiten.

## Klare Nicht-Funktionen

- Kein `[target ...] compiler` ohne Profil; kein allgemeines `[build]`, `[test]`, `[ci]` oder `[cache]`-INI-Schema im aktuellen Parser.
- Kein automatisches Herabsetzen der HWM; kein Ersatz für `--owner-approve`.
- Kein Package-Resolver, kein beliebiges Make-/CMake-Skript und keine automatisch ausgeführten `check_cmd`-/`diff_cmd`-Formatterbefehle.

---

# English

## Syntax and scope

Supported sections: `[project]`, `[target "NAME"]`, `[target "NAME".debug]`, `[target "NAME".release]`, `[target "NAME".coverage]`, `[doc]`, `[bom]`, `[sbom]`, `[fmt]`, `[fmt "NAME"]`, and `[coverage]`.

Whole lines beginning with `;` or `#` are comments; **inline comments are not a guaranteed feature**. Section/key matching is case-insensitive. Booleans: `yes/no`, `true/false`, `on/off`, `1/0`. Lists should use semicolons; whitespace and quoted tokens are additionally supported. Empty list assignments do not necessarily clear inherited lists. Explicit `--config` takes precedence over the generated `tackfile.c` INI layer and built-ins. `TACK_CC` overrides compiler program selection.

## `[project]`

| Key | Values | Effect |
|---|---|---|
| `default_target` | name | Default CLI target; built-in `app` |
| `disable_auto_tools` | boolean | Disable automatic tool discovery; default `no` |
| `allow_unsafe_paths` | boolean | Opt out of safe target path defaults; default `no` |
| `compiler` | executable | Default compiler program; built-in `tcc` |
| `compiler_policy` | `auto|tcc|gcc|clang|generic` | Select compiler-specific built-ins, separately from program choice |

## `[target "NAME"]`

| Key | Effect |
|---|---|
| `src` | Repository-relative source directory |
| `bin` | Executable base name; safe token by default |
| `id` | Build directory identifier; safe token by default |
| `enabled` | Disable target with `no` |
| `remove` | Remove target with `yes` |
| `core` | Link shared `src/core` code |
| `includes` | Extra include directories (`-I` applied by tack) |
| `defines` | Extra macros (`-D` applied by tack) |
| `cflags` | Extra compilation flags |
| `ldflags` | Extra linker flags |
| `libs` | Extra libraries |

The unprofiled target section does **not** implement `compiler` or `compiler_policy`.

## `[target "NAME".debug|release|coverage]`

| Key | Effect |
|---|---|
| `compiler` | Profile-specific compiler program |
| `compiler_policy` | Profile-specific built-in compiler policy |
| `core` | Profile-specific core link setting |
| `includes` | Profile-specific extra includes |
| `defines` | Profile-specific extra defines |
| `cflags` | Profile-specific extra compiler flags |
| `ldflags` | Profile-specific extra linker flags |
| `libs` | Profile-specific extra libraries |

Profile sections do **not** accept `src`, `bin`, `id`, `enabled`, or `remove` as effective keys. Coverage instrumentation requires explicit GCC flags; naming a profile `.coverage` alone does not add instrumentation to arbitrary builds.

## Documentation, BOM, SBOM and coverage

| Section | Keys | Effect |
|---|---|---|
| `[doc]` | `template`, `css`, `allow_js_search` | HTML presentation; optional JS search; missing explicitly configured assets fail |
| `[bom]` | `template`, `css` | BOM presentation with DOC fallback if not overridden |
| `[sbom]` | `format`, `spec_version`, `output` | Build-input SBOM formatting and single-target destination |
| `[coverage]` | `id`, `id_windows` | Separate comparable GCC/gcov measurement identities per platform |

Supported SBOM formats: `tack`, `tack-sbom-1`, `cyclonedx`, `cyclonedx-1.4`, `spdx`, `spdx-2.3`. `output` is **not** the batch output destination for `--all-targets`. The HWM files are versioned measured artifacts, not INI keys; `coverage/current.txt` is generated. See the [coverage guide](../guides/coverage-ratchet.md).

## `[fmt]` (parser fixed in 0.8.5-dev RC0.2)

| Key | Intended effect **after** parser repair |
|---|---|
| `enabled` | Boolean; `no` disables fmt; default enabled |
| `defaults` | `auto` / `off|no|false`; default auto-discovery |
| `rules` | Ordered rule names |
| `exclude` | Global exclusion patterns |
| `fail_fast` | Parsed, but `no` does not currently guarantee continuing after a rule error |

## `[fmt "NAME"]` (parser fixed in 0.8.5-dev RC0.2)

| Key | Intended effect **after** parser repair |
|---|---|
| `tool` | Formatter identifier |
| `mode` | `file` or `batch` |
| `include` | File inclusion patterns |
| `exclude` | Per-rule exclusion patterns |
| `cmd` | Command template: `@FILE@` (file) or `@RSP@` (batch) |
| `requires` | Executables that must be found |
| `workdir` | Execution working directory |
| `check_cmd` | **Parsed, not currently used by execution** |
| `diff_cmd` | **Parsed, not currently used by execution** |
| `env.NAME` | **Parsed, not currently forwarded to child processes** |

In 0.8.5-dev RC0.2 the formatter rule keys are parsed, and a C89 regression test verifies `fmt --list --strict` without executing an external formatter. The **internal** `fmt --check` path uses temporary copies; an external formatter end-to-end test and Windows acceptance remain pending. `fmt --diff` is best effort. `fmt --strict` validates configured rules and unknown formatter keys, but should not be mistaken for universal strict schema validation of all INI sections.

## Not implemented as INI contracts

There is no profile-independent `[target "NAME"] compiler`, generic `[build]` / `[test]` / `[ci]` / `[cache]` schema, automatic HWM decrease, package resolution, or active `check_cmd` / `diff_cmd` execution. Use the [practical walkthrough](../guides/tack-ini.md) for working combinations of the supported keys.
