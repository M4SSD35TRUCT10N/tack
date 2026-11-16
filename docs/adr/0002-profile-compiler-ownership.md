# ADR 0002 — Compiler je Targetprofil / Compiler per target profile

**Status:** vorgeschlagen / proposed (0.8.2-dev)

## Deutsch

Die Compilerwahl pro Buildprofil gehört in die explizite INI-Sektion `[target "NAME".debug]` oder `[target "NAME".release]`. `[project] compiler` bleibt Fallback, `TACK_CC` hat Vorrang. Tests nutzen den Compiler und die zusätzlichen `cflags` des ausgewählten Zielprofils, damit ein GCC-Release nicht versehentlich mit TinyCC-Tests abgenommen wird. `tack` bleibt ein Buildtreiber.

Eine ausschließlich globale Compilerwahl wurde verworfen, weil dies für jeden Release-Gate manuelle Umgebungsänderungen erfordern würde. Ein Compilerwechsel innerhalb desselben Profils erfordert derzeit `--rebuild`; das Cache-Verhalten wird separat betrachtet.

## English

Compiler selection per build profile belongs in the explicit INI section `[target "NAME".debug]` or `[target "NAME".release]`. `[project] compiler` remains a fallback and `TACK_CC` takes precedence. Tests use the chosen profile compiler and extra `cflags` so a GCC release cannot accidentally be accepted using TinyCC tests. `tack` remains a build driver.

A global-only compiler setting was rejected because each release gate would otherwise need manual environment changes. Switching compilers within one profile currently requires `--rebuild`; cache behavior is a separate concern.
