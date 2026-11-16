# Sicherheit / Security

## Deutsch

Alle Projektpfade und Konfigurationen begrenzen und validieren. Unsichere Pfade sind kein Normalbetrieb. Fehlende Compiler oder ungültige Profile müssen fehlschlagen, statt heimlich einen anderen Compiler zu nutzen. Neue Laufzeit-/Buildabhängigkeiten benötigen Freigabe. Regressionstests und gemessene Coverage-High-Water-Werte dürfen niemals zum Erreichen eines grünen Status abgeschwächt werden.

## English

Bound and validate all project paths and configuration values. Unsafe paths are not the normal mode. Missing compilers or invalid profiles must fail rather than silently selecting another compiler. New runtime/build dependencies require approval. Regression tests and measured coverage high-water marks must never be weakened to obtain a green status.
