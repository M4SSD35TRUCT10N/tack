# ADR 0002 — Compiler per target/profile

Status / Status: PROPOSED / VORGESCHLAGEN (0.8.2-dev)

## Deutsch

Entscheidung: Die Compilerwahl pro Buildprofil gehört in die explizite INI-Sektion `[target "NAME".debug|release]`. Das Projekt besitzt weiterhin eine universelle Fallback-Wahl. `TACK_CC` hat Vorrang. Tests binden sich an das gewählte Zielprofil, damit ein GCC-Release-Build nicht durch einen TinyCC-Testlauf scheinbar abgenommen wird. `tack` bleibt ein reiner Buildtreiber.

Die Alternative eines ausschließlich globalen `compiler`-Wertes wurde verworfen, weil dies für jeden Release-Gate eine manuelle Umgebungsänderung erzwingen würde.

## English

Decision: select compiler per `[target "NAME".debug|release]` section; retain `[project]` fallback and `TACK_CC` override. Tests follow the selected target/profile so GCC releases cannot be accidentally validated by a TinyCC test build. Keep tack a build driver, not an orchestration layer.
