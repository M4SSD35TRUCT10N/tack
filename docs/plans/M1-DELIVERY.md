> **M1 RC0.4, neuer Implementierungskandidat 18.11.2026:** Nach Owner-Abnahme von RC0.3 (Windows HWM **3708/5996**, Linux HWM **3604/5790**, beide PASS; Windows-CLI-Opt-in **2/0/0/2 PASS**) wurde bei kritischem S15 ein tatsächlicher Win32-Enumerationsrandfall entdeckt. `rm_rf_depth` unterschied einen fehlgeschlagenen `FindFirstFileA`-Aufruf bislang nicht von einem leeren Verzeichnis. RC0.4 verweigert bei unerwarteten Fehlercodes jetzt die Löschung und ergänzt deterministische `FindFirstFileA`-/`FindNextFileA`-Fehlerinjektion im bestehenden M1-Test (Scanner, `rm_rf_contents`, `rm_rf_depth`, Sentinel). **Linux GCC/Clang C89 und M1-Einzeltest PASS; Windows GCC/TinyCC mit RC0.4 NOT RUN.** Da Test- und Produktionsquelle geändert sind, sind das RC0.3-Testinventar und beide HWM-Dateien absichtlich nicht neu angenommen. **Owner-Ratchet-Review und neue beidseitige Coverage-Messung erforderlich.** S15 erst nach Windows-Nachtest PASS, M1 noch offen. TOCTOU bleibt dokumentiertes Restrisiko. **Keine Git-Schreiboperation.** Siehe `docs/plans/M1-RC0.4-S15-EVIDENCE.md`. Alle nachfolgenden RC0.3-Zustandsangaben sind historische Evidenz.

# tack M1 RC0.3 – Owner-HWM-Freigaben und Integrationsstatus (18.11.2026)

**Verbindlicher aktueller Stand:** Windows-HWM **3708/5996** und Linux-HWM **3604/5790** sind mit ausdrücklicher Owner-Freigabe angenommen und jeweils mit `coverage check` **PASS** geprüft. Linux-Annahme erfolgte in der unveränderten unprivilegierten GCC/gcov-14.2.0-Referenzmessumgebung. Windows CLI S09: Safe/CLI-Opt-in/INI-Opt-in/Restore = **2/0/0/2**, PASS. Die separate [Windows-CLI-Evidenz](M1-WINDOWS-CLI-EVIDENCE.txt) ist beigelegt.

**M1 weiterhin nicht final:** Windows **S15** benötigt noch eine deterministische Prüfung eines echten Enumerationsfehlers. Die Windows-Sperrdatei-, Scanner-, Symlink- und Junction-Nachweise sind PASS. Ein Race-Free-/TOCTOU-Versprechen wird ausdrücklich nicht abgegeben. Beide akzeptierten HWM-Dateien und das freigegebene 27er-Testinventar gehören später zusammen in den Owner-freigegebenen M1-Commit. Bis dahin keine Git-Schreiboperation.

**Windows-Übernahme:** Vom vollständigen ZIP nur die M1-Dokumentation und `coverage/hwm-linux.txt` übernehmen. Bereits Owner-abgenommene Windows-HWM und Testinventar nicht überschreiben. Ein Windows-`coverage check` validiert die Windows-HWM, nicht die Linux-HWM. Vor der Integration Quell- und Testdateien auf Bytegleichheit vergleichen und das Archivmanifest prüfen.

## Historische Lieferung vor diesen zusätzlichen Nachweisen

Der folgende Lieferbericht ist die frühere Momentaufnahme. Sein Linux-HWM-`BLOCKED` und sein S09-`NOT RUN` sind durch obige Freigaben/Nachweise überholt.

---

# tack M1 RC0.3 — Liefer- und Integrationshinweise / Delivery and integration

**Stand:** 18.11.2026. Dieses vollständige Quellpaket konsolidiert RC0.3-Code, die vom Owner freigegebene Windows-HWM und Testinventar sowie aktuelle DE/EN-Abnahmedokumentation. **Kein Git-Commit oder Push**; Linux-HWM bleibt nach unabhängiger Messung bewusst unverändert.

## Deutsch

- **Produktionscode und Testquelle:** Bytegleich zu `tack_v0.8.6_M1_RC0.3_source_complete.zip`. Kein weiterer M1-Scope, keine neue Abhängigkeit, kein neuer Buildmechanismus.
- **Owner Windows:** GCC Release und TinyCC Debug je 26 PASS/1 SKIP, reale Junction-Scanner- und `clean`/`clobber`-Tests bestanden; `coverage/hwm-windows.txt` wurde mit ausdrücklichem Owner-`coverage accept --owner-approve` auf **3708/5996** (61,84123 %) angehoben und danach `coverage check` PASS.
- **Linux:** GCC/gcov 14.2.0, unprivilegierter Coverage-Lauf **3604/5790** (62,24525 %) bei 26 PASS/1 SKIP. `coverage/current.txt` ist ein *nicht versioniertes* Messartefakt im isolierten Linux-Prüfverzeichnis; `coverage/hwm-linux.txt` bleibt auf **3505/5710** (61,38354 %). `coverage check` meldet erwartungsgemäß **BLOCKED**, nicht PASS; eine neue HWM bedarf Owner-Freigabe.
- **Offene Sicherheit:** Windows-CLI-Opt-in-E2E und erzwungener Windows-Enumerationsfehler nicht vollständig nachgewiesen; konkurrierende Link-Swaps (TOCTOU) nicht race-free geschützt. S01–S16-Trace in `M1-REQUIREMENTS.tsv` bleibt verbindliche Grundlage für den Abschluss.
- **Archiv:** `SHA256SUMS.txt` deckt jede verpackte Datei außer sich selbst ab; `update_tools.bat` ist bytegleich zur Owner-Korrektur und bleibt Git-ignored. Buildartefakte, `.tack-cache`, gcov-Daten und `coverage/current.txt` sind nicht enthalten.

### Arbeitsablauf auf Windows (`cmd.exe`)

```bat
cd /d C:\Projects\tack
git status --short --branch
rem Quellarchiv vor Änderungen mit certutil -hashfile prüfen
rem Nur die dokumentierten M1-Dateien aus dem ZIP übernehmen; keine unbedachten Überschreibungen
set "TACK_CC=gcc"
tack --config .\tack.ini ratchet tests
tack --config .\tack.ini coverage check
tack --config .\tack.ini test release --ci --rebuild -j 1
git diff --check
git status --short --branch
```

**Nicht jetzt ausführen:** `coverage accept --owner-approve` für Linux auf Windows, `git add -f update_tools.bat`, `git commit`, `git push` oder Merge. Vor Git-Freigabe müssen die offene Requirement-Evidenz und Linux-HWM-Annahme abgeschlossen bzw. ausdrücklich vom Owner entschieden sein. Das SHA256SUMS-Manifest repräsentiert die unveränderten Archivbytes, nicht einen inzwischen bearbeiteten lokalen Checkout.

## English

This is a source-complete RC0.3 acceptance consolidation, not a new production-code candidate or a completed M1 release. Windows GCC/TinyCC gates and owner-accepted 3708/5996 HWM are recorded; independent unprivileged Linux GCC/gcov coverage is 3604/5790, **BLOCKED** pending explicit owner approval. Retain the Linux HWM 3505/5710 until that review. The requirement-trace gaps and explicit TOCTOU limitation remain open. No Git writes or ratchet acceptance have been performed by the assistant.
