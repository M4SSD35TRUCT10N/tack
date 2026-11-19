# ADR 0006 – Kanonische Pfadidentitäten, native OS-Pfade und parallele Jobs

**Status: PROPOSED (keine Gruppenratifikation) | 19.11.2026**
**tack:** Architektur-/Testvertrag innerhalb v0.8.6 M2.
**ba_core:** Kandidat für wiederverwendbare C89-Primitiven, Umsetzung in eigenem Projekt separat zu genehmigen.

## Deutsch
**Motivation:** Die Windows-Coveragefehler RC0.7/RC0.7b entstanden durch Vermischung der nativen Windows-Schreibweise `src\tack.c` mit dem portablen Quellenbezeichner `src/tack.c`. Parallelisierung erfordert zusätzlich eindeutige, bounded Artefaktpfade und kontrollierte Prozesslebenszyklen.

**Drei getrennte Datentypen (konzeptuell, keine neue Typabhängigkeit):**
1. **OS-Pfad:** validierter, unverfälschter Pfad für `CreateProcessA`/`CreateFile` oder POSIX-Dateioperationen. Bewahrt relevante Windows-UNC-/Extended-Path-Konventionen. Fehler und nicht unterstützte Pfadformen werden explizit abgewiesen, nie still gekürzt.
2. **Kanonische Identität:** nur repository-interner relativer Pfad mit `/` für Coverage, Manifeste, Ratchets, Cache, Test-IDs, Report-Verweise; sichere, bounded Normalisierung, explizite Traversal-/Root-/Namensregeln.
3. **Darstellung:** kontextabhängig lesbarer Dateiname/Diagnose. Niemals als ungeprüfter Dateisystempfad verwenden.

**Prinzipien:** Semantische, keine unkontrollierte globale Textsubstitution. A und B niemals implizit vergleichen; an der Schnittstelle mit explizit getesteter Konversion und dokumentiertem Scope arbeiten. Leerzeichen, Backslash, slash, Windows-Gerätenamen, absolute/relative und lange Pfade, Symlink-/Reparse-Risiken in beiden OS-Matrizen testen. Fehlende oder nicht instrumentierte Produktionsquellen bleiben hart FAIL.

**Parallelität:** Bounded native Prozesspools `-j N` mit getrennten Compile-, Link- und Test-Artefakten; Determinismus der Testergebnisse und Reportreihenfolge; keine Kindprozesse/Handles leaken; nach Fehler alle laufenden Geschwister reapen; kein implizites Shell-Quoting. `gcov` nur nach vollständigem Testlauf. Test-Runtime bleibt serienmäßig sequenziell, bis Fixtures für echte Parallelausführung isoliert sind.

**Wiederverwendung:** `ba_core` darf diese Regeln künftig in strengem ISO C89 kapseln (Pfadidentität, bounded Operationen, Compiler-/OS-Schnittstellen, sichere Prozess-Pools), jedoch **nicht automatisch** in die monolithische `tack`-Bootstrap-Datei importieren. Eine Bibliotheksabhängigkeit, Quell-Spiegelung oder Single-Header-Übernahme benötigt je eine separate ADR, eindeutige Versionierung und plattformübergreifende Tests.

**Status:** Das ist der vorgeschlagene Vertrag, kein Bericht, dass ba_core bereits diese APIs anbietet oder dass `tack test -j N` schon parallel wäre. Erforderliche Tests und Freigaben unter `docs/plans/0.8.6-execution-contract.md`.

## English
Keep **native filesystem paths**, **canonical repository source identities** and **display paths** separate. No blind global separator replacement. Validate and bound all conversions, preserving real OS semantics and hard failure for absent instrumentation. Provide deterministic, bounded parallel compile/link with safe process cleanup and serial shared-fixture runtime tests. This is a candidate reusable contract for `ba_core`, not a new automatic dependency or an implemented parallel test runner. Owner architecture acceptance and actual Win/Linux tests remain required.
