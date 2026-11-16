# Security / Sicherheit

DE: Alle Pfade und Konfigurationswerte begrenzen und prüfen. Unsichere Pfade sind keine normale Betriebsart. Fehlende Compiler oder ungültige Profile sind Fehler, kein Anlass für stillen Fallback. Nicht autorisierte neue Runtime- oder Buildabhängigkeiten verboten. Test-Fixtures und Coverage-High-Water-Werte dürfen nicht zur Herstellung grüner Ergebnisse abgeschwächt werden.

EN: Validate and bound all paths and configuration values. Unsafe paths are not a normal operating mode. Missing compilers and invalid profiles must fail closed. No unauthorized dependencies, test weakening, or lowered coverage marks.
