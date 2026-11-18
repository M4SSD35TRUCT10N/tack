# SPEC 0013 — tack v0.8.6 M1: Path & Filesystem Safety

**Status (current):** Windows RC0.3 Owner-accepted; Linux RC0.3 HWM acceptance and selected requirement gates outstanding. Historical RC0.1 scope below. **Scope:** M1 only, ISO C89, no new dependency; Owner-approved Windows HWM update recorded separately. This spec refines the M0 plan and S01–S16 requirement candidate; it is not a retrospective owner approval.
> **Abnahmehinweis 18.11.2026:** Windows GCC/TinyCC und echte NTFS-Junction-Scan-/Cleanup-Gates PASS; Windows-HWM 3708/5996 vom Owner angenommen. Linux-Coverage 3604/5790 gemessen, HWM-Owner-Accept noch ausstehend. Siehe `docs/plans/M1-VALIDATION.md`; TOCTOU bleibt ausgeschlossen von einer race-free Garantie. Historische RC0.1-Formulierungen zu unveränderten HWMs/Testinventar beziehen sich nur auf RC0.1.


## Deutsch

### Vertrag für Zielpfade

- Ohne explizites `--unsafe-paths` bzw. `allow_unsafe_paths = yes` müssen `id`/`bin` einfache Tokens und `src` relative, portable Pfade sein. `src` weist absolute POSIX-Pfade, Windows-Laufwerksangaben, UNC-Pfade, gemischte `..`-Traversal-Segmente, leere/`.`-Segmente, Steuerzeichen, `:`, `?`, `*`, `|`, `<`, `>`, `"`, abschließende Punkt-/Leerzeichen und die DOS-Gerätenamen `CON`, `PRN`, `AUX`, `NUL`, `COM1`–`COM9`, `LPT1`–`LPT9` (auch mit Erweiterung) ab. Die Ablehnung gilt auf allen Plattformen, damit ein akzeptiertes Repository portabel bleibt. Sichere Namen wie `console.c`, `com10`, `.hidden` und `a..b` bleiben zulässig.
- `--unsafe-paths` oder `allow_unsafe_paths = yes` setzen **nur** die lexikalische Zielpfadprüfung außer Kraft. Weder Scan-/Lösch-Symlink-Schutz noch Fail-fast-Grenzen werden abgeschaltet. Die bestehende INI-/CLI-Priorität wird nicht verändert.
- `path_join` muss bei `cap`-Grenzen vor der Summenbildung prüfen; exakter Puffer einschließlich NUL funktioniert, Überschreitung beendet den Prozess mit Code 2. `path_join_alloc` prüft `size_t`-Überlauf, NULL-Argumente werden wie Leerstrings behandelt. Historisches Verhalten eines abschließenden Separators bei leerem rechten Teil bleibt erhalten.

### Vertrag für Scanner und Bereinigung

- Rekursive Source-Scanner folgen weder Datei-Symlinks noch Verzeichnis-Symlinks/Junctions. Auch ein verlinktes **Startverzeichnis** und ein Pfad mit verlinkter **Zwischenkomponente** werden mit Exit 2 zurückgewiesen. Nicht vorhandene optionale Scan-Verzeichnisse sind weiterhin erlaubt; echte Scan-/Berechtigungsfehler sind fail-fast.
- `rm_rf_depth` arbeitet mit no-follow-Existenzprüfung, so dass auch **hängende Symlinks** entfernt werden; Windows-Verzeichnis-Reparse-Points werden per `RemoveDirectoryA` statt per `DeleteFileA` gelöscht. Bereinigung traversiert keine Linkziele. Ein verlinktes Startverzeichnis bei `clean` muss fehlschlagen, `clobber` kann den Link selbst entfernen. `.`/`..`/Root-Pfade und Eltern-Traversal als Löschziel werden abgewiesen.
- Win32-Verzeichnisaufzählung beim `rm_rf_contents_depth` darf Zugriffs- oder Aufzählungsfehler nicht als leeres Verzeichnis melden. Gleiches gilt für POSIX-Scans; Scanner- und Löschrekursion bleiben durch die bestehenden 64-Ebenen-Grenzen beschränkt.
- Bei Windows-Reparse-Points werden Attribute der Linkziele nicht vorsorglich geändert. Verweigerte Zugriffe/gesperrte Dateien müssen zum tatsächlichen Fehler führen. Negative Tests nutzen isolierte `build/m1_fs_*`-Fixtures und eine separate Sentinel-Datei.

### Grenzen und Nicht-Ziele

- Prüfungen sind nicht race-free. Gegen einen konkurrierenden Angreifer, der während Traversal/Entfernen Links austauscht (TOCTOU), ist eine handle-/dirfd-basierte Architektur erforderlich. Das liegt außerhalb M1 und muss als offenes Sicherheitsrisiko dokumentiert bleiben; keine absolute Symlink-Isolationsgarantie behaupten.
- Windows-Symbolic-Link-Tests brauchen ggf. Developer Mode oder Privilegien; ein fehlender Fixture-Aufbau wird explizit gemeldet und **nicht** als ausgeführter Nachweis gezählt. Echte Junction-/Sharing-Violation-Gates erfordern Owner-Windows-Abnahme.
- Keine Änderung an `coverage/hwm-{windows,linux}.txt`, `coverage/test-inventory.txt`, `tack.ini`, Versionsnummer oder ABI. Änderungen an Testquellen erfordern gesonderte Ratchet-Owner-Freigabe, bevor das reguläre Test-/Coverage-Gate laufen darf.

## English

### Safe target paths

Without a deliberate `--unsafe-paths` or `allow_unsafe_paths = yes`, `id`/`bin` must be simple portable tokens and `src` must be repository-relative. Reject POSIX absolutes, drive and UNC prefixes, mixed-separator traversal, empty/dot segments, control characters, Windows-invalid characters, trailing dot/space, and DOS device basenames including extensions. Portability checks apply on all hosts. Unsafe opt-in bypasses **only lexical target validation**, never symlink/recursion/cleanup protections. Bounded joins fail before arithmetic overflow or truncation; preserving existing empty-right-component separator behavior is intentional.

### Scans and removal

Do not follow directory junctions/symlinks or symlinked source files. Reject a linked starting directory or intermediary path component. Preserve non-existent optional scan roots, but fail closed on actual enumeration/permission errors. Cleanup must see dangling links with lstat/reparse metadata, remove directory junction links via Windows RemoveDirectory, refuse filesystem roots and parent traversal, and never follow links during recursive deletion. Windows enumeration errors and locked files must not be reported as successful removal.

### Limitations and acceptance

Preflight lstat/GetFileAttributes checks do **not** prevent concurrent link-swap/TOCTOU attacks. A handle-based design is future work. Windows-only junction, lock and C89/TinyCC gates remain unverified until Owner logs. Both approved coverage HWMs and the old 26-file test inventory are unchanged in RC0.1; the added test therefore intentionally blocks the standard ratchet until explicit owner acceptance. No production release/version promotion and no Git operations are authorized by this spec.

## RC0.4-S15-Ergänzung (18.11.2026, Owner-Review ausstehend)

Bei `rm_rf_depth` muss Windows `FindFirstFileA` mit anderen Fehlern als `ERROR_FILE_NOT_FOUND` oder `ERROR_NO_MORE_FILES` zwingend mit Fehlerstatus quittieren. Insbesondere darf bei `ERROR_ACCESS_DENIED` kein Fallthrough zur Entfernung eines möglicherweise leeren Verzeichnisses erfolgen. Der bestehende M1-Sicherheitstest injiziert solche Fehler in `FindFirstFileA` und `FindNextFileA` ausschließlich in seiner Testübersetzungseinheit. Der Nachweis gilt erst nach erfolgreichem Owner-Windows-GCC-/TinyCC-Test; keine implizite Sicherheitsfreigabe.
