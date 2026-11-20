# ADR 0003 – Nativer Win32-Prozessstart in M2 / Native Win32 process launch

**Datum / Date:** 2026-11-18
**Status:** Vorläufige Implementierungsentscheidung RC0.4, Owner-Plattformabnahme ausstehend / Provisional RC0.4 decision, platform validation pending
**Scope:** tack v0.8.6 M2; keine M3–M8-Arbeiten.

## Deutsch

### Ausgangslage und Evidenz

Der M2-Vertrag fordert eine shell-freie, bytegenaue Kindprozess-Argumentübergabe. Windows RC0.3 (SHA `f74c7992e7f2e9a4e02c828a05ba2fa717479d31`) kompilierte unter TinyCC Debug und GCC Release/Coverage. Alle drei Läufe scheiterten bei genau einer Assertion: `child sees exact argv and literal metacharacters` über den allgemeinen `_spawnvp`-Pfad. Die äquivalente echte Formatter-Child-Probe über `CreateProcessA` bestand. Die genauen abweichenden Bytes wurden im RC0.3-Log nicht ausgegeben; RC0.4 ergänzt die Diagnostik. Die Assertion darf nicht abgeschwächt werden.

### Änderung in RC0.4

- Der generische Windows-Starter verwendet `CreateProcessA` statt `_spawnvp`, der Wait-Pfad `WaitForSingleObject` und `GetExitCodeProcess` statt `_cwait`.
- Die existierende, getestete CRT-kompatible Kodierung `fmt_windows_quote_cmd_arg` wird über einen gemeinsamen begrenzten Commandline-Builder sowohl vom Formatter als auch vom allgemeinen Starter verwendet. Keine Shell.
- Erfolgreicher Spawn schließt den Thread-Handle sofort und speichert den Prozess-Handle. Wait- und Exitstatus-Fehler schließen den Prozess-Handle; nach jedem Wait wird die Referenz gelöscht. Fehler werden diagnostiziert.
- Neue Tests prüfen dieselben Argumentvektoren über beide Pfade, den generischen Start mit Leerzeichen im Executable-Pfad sowie Fehler-Injektionen für CreateProcessA, WaitForSingleObject, GetExitCodeProcess und CloseHandle. Der nicht mehr relevante `_cwait`-Fehlerfall wird durch zusätzliche native Handle-Lebensdauertests ersetzt.
- POSIX-Architektur und alle anderen Milestones bleiben unverändert. Keine neue Build-, Compiler- oder Runtime-Abhängigkeit.

### Bewusste Grenzen

Der Code verwendet weiterhin die Windows-ANSI-API `CreateProcessA` und ist damit weder Unicode-/Long-Path-Migration noch Garantie für beliebige nicht-CRT-Argumentparser. Grenzen: 16000 Byte pro kodiertem Eingabeargument, maximal 32766 Byte Windows-Kommandozeile und bisherige PATH-/Executable-Suchregeln. RC0.4 ist **NOT RUN** auf Windows und Linux und bleibt bis zur Owner-Abnahme, Test-Ratchet-Freigabe und gemessenen Coverage ein Implementierungskandidat.

## English

M2 requires shell-free exact child-received arguments. Windows RC0.3 owner runs reproduced a single generic CRT `_spawnvp` argv failure across TinyCC debug, GCC release, and GCC coverage. The corresponding real `CreateProcessA` formatter probe passed. RC0.4 reuses the existing CRT-compatible formatter argument encoder for a common commandline builder, replacing the generic Windows CRT launcher/wait with `CreateProcessA`, `WaitForSingleObject`, and `GetExitCodeProcess`. Thread and process HANDLE ownership and cleanup are explicit. Stronger native Win32 fault tests replace the obsolete `_cwait` test. POSIX and build-system boundaries remain unchanged. ANSI, search and commandline limits remain. RC0.4 awaits Windows/Linux tests, ratchet acceptance, coverage and Owner review.
