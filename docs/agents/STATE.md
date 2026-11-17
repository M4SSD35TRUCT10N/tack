# Aktueller Projektstand / Current agent handoff

## Deutsch

- **tack v0.8.4:** vom Owner zur GitHub-Veröffentlichung freigegeben; `TACK_VERSION` und Projekt-`tack.ini` sind auf `0.8.4` synchronisiert. Veröffentlichungs-Tag und Push stehen noch aus.
- **Windows RC0.2:** Owner-Protokolle: GCC Release und TinyCC Debug jeweils 25 kompiliert, 24 PASS, 1 SKIP, 0 FAIL; `ratchet tests` PASS.
- **Linux Releasequelle:** GCC Debug und striktes GCC Release mit jeweils 25 kompiliert, 24 PASS, 1 Windows-only-SKIP, 0 FAIL.
- **Coverage:** Linux GCC 14.2.0 / gcov 14.2.0, Referenzprofil aus `tack.ini`: frisch gemessen und vom Owner als `coverage/hwm-linux.txt` angenommen; `coverage check` muss PASS sein. `coverage/current.txt` ist eine nicht versionierte Messdatei.
- **Grenzen:** Windows-Coverage `NOT RUN`, keine Windows-HWM. Nicht als komplette Windows-Coverage-Abnahme darstellen. Gruppenweiter BA-STD-C89-Entwurf bleibt nicht ratifiziert.
- **Historie:** RC0.2 behob Windows-Pfade/Zeilenenden im Testinventar. Kein Test zur Freigabe abgeschwächt. Nur finaler `docs/releases/0.8.4.md` ist der aktuelle Releasebericht; alte dev-Stände 0.8.2/0.8.3 bleiben historische Dokumente.

## English

- **tack v0.8.4:** owner-approved for GitHub publication; `TACK_VERSION` and `tack.ini` agree on `0.8.4`. Push and release tag not yet performed.
- **Windows RC0.2:** owner logs prove GCC release and TinyCC debug each compiled 25, with 24 PASS, one SKIP and zero failures; `ratchet tests` PASS.
- **Linux release source:** GCC debug and strict GCC release each compiled 25, with 24 PASS, one Windows-only SKIP and zero failures.
- **Coverage:** fresh Linux GCC 14.2.0 / gcov 14.2.0 reference measurement, matching `tack.ini`, owner-accepted and versioned as `coverage/hwm-linux.txt`; `coverage check` must PASS. `coverage/current.txt` is an unversioned generated measurement.
- **Limits:** Windows coverage remains `NOT RUN` and has no Windows HWM. Do not claim cross-platform coverage acceptance. Group-wide BA-STD-C89 draft is not ratified.
- **History:** RC0.2 fixed Windows path/newline inventory portability without weakening tests. `docs/releases/0.8.4.md` is the canonical final release record; older 0.8.2/0.8.3 dev documents remain historical.
