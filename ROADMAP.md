# tack Roadmap

**Backlinks:** [README](README.md) • [FAQ](FAQ.md)

---

## Deutsch

### Done
- **v0.6.0** — `tackfile.c` **on-the-fly**: tack generiert `tackfile.generated.ini` und lädt es als Konfig‑Layer (Fail‑fast bei Fehlern).

### Next (kurzfristig)
- **`--no-code-config`**: `tackfile.c` ignorieren, aber `tack.ini` weiter nutzen (für Teams/CI).
- **Build-All / Test-All**: alle enabled Targets in einem Rutsch bauen/testen (mit sauberem Exitcode).
- **Bessere Doku für Real‑World Ports**: Matrix + Port‑Reports als “offizieller” Prozess (Schema‑F).
- **Optionale `pkg-config` Bridge** (nicht Pflicht): tackfile.c kann bei Bedarf CFLAGS/LIBS abfragen und in generated INI schreiben.

### Ideas / Research (mittelfristig)
#### Package Management (USP, aber nur mit Supply-Chain-Fokus)
Ein möglicher tack‑Ansatz wäre **Source‑Vendoring + Lockfile**:
- `deps.lock` (oder `deps.ini`): URL + Version/Tag + Hash
- “Fetch” als optionaler Schritt (auch offline‑fähig via Cache)
- klare Lizenz-/Provenienz‑Metadaten
- reproduzierbare Builds, keine “magischen” Upgrades

**Wichtig:** Das ist ein großer Schritt (Security, Reproducibility, Policy). Besser klein starten (z.B. nur “pinned fetch + hash verify”) und konsequent dokumentieren.

### Nicht-Ziele (bewusst)
- CMake‑ähnlicher IDE‑Projektgenerator
- “Alles für jeden” — tack bleibt bewusst schlank

---

## English

### Done
- **v0.6.0** — `tackfile.c` **on-the-fly**: tack generates `tackfile.generated.ini` and loads it as a config layer (fail-fast on errors).

### Next (near term)
- **`--no-code-config`**: ignore `tackfile.c` while still reading `tack.ini` (teams/CI).
- **Build-All / Test-All**: build/test all enabled targets with clear exit codes.
- **Better real-world port documentation**: make the matrix + port reports the official “Schema‑F” process.
- **Optional `pkg-config` bridge** (not required): tackfile.c can query CFLAGS/LIBS and write them into the generated INI.

### Ideas / Research (mid term)
#### Package management (USP, but supply-chain first)
A tack-style approach could be **source vendoring + a lockfile**:
- `deps.lock` (or `deps.ini`): URL + version/tag + hash
- optional “fetch” step (offline-friendly via cache)
- license/provenance metadata
- reproducible builds, no surprise upgrades

**Important:** this is a big step (security, reproducibility, policy). Start small (pinned fetch + hash verify) and document aggressively.

### Non-goals (by design)
- a CMake-like IDE project generator
- “everything for everyone” — tack stays intentionally small

---

**Backlinks:** [README](README.md) • [FAQ](FAQ.md)
