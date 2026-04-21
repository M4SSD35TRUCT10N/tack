# SPEC 0001 — Bilinguale Dokumentationspolitik / Bilingual documentation policy

**Status:** angenommen / accepted  
**Serie:** v0.8.0-dev  
**Typ:** Dokumentationsspezifikation / documentation specification

---

## Deutsch

### Ziel

Ab der 0.8-Serie wird normative Projektdokumentation **bilingual (DE/EN)** gepflegt.

### Geltungsbereich

Diese Anforderung gilt mindestens für:

- `README.md`
- `FAQ.md`
- `ROADMAP.md`
- `RELEASENOTES.md`
- Dokumente in `docs/specs/`
- Dokumente in `docs/adr/`, soweit sie für die Produkt- oder Architekturgeschichte relevant sind

### Regeln

1. **Gleicher Informationsgehalt**  
   Deutsch und Englisch müssen inhaltlich deckungsgleich bleiben. Stilistische Unterschiede sind erlaubt, Informationslücken nicht.

2. **Ein Dokument statt Sprach-Split**  
   Normative Dokumente werden grundsätzlich in **einer Datei** mit klar getrennten DE/EN-Abschnitten gepflegt. Getrennte Sprachdateien sollen vermieden werden, damit keine Drift entsteht.

3. **Root-Dokumente bleiben Einstiegspunkte**  
   `README.md`, `FAQ.md`, `ROADMAP.md` und `RELEASENOTES.md` bleiben die primären Nutzerdokumente. `docs/` ergänzt diese um Spezifikationen und Entscheidungen.

4. **Spezifikation vor großer Änderung**  
   Bei neuen Grundsatzthemen soll zuerst ein passendes Dokument in `docs/specs/` oder `docs/adr/` entstehen und erst danach Produktionscode geändert werden.

5. **Keine C89-/Fail-fast-Verwässerung durch Doku**  
   Dokumentation darf den Projektrahmen nicht weichzeichnen. Aussagen zu C89, Portabilität, fail-fast, data-only-Konfiguration und ähnlichen Kernprinzipien müssen konsistent bleiben.

### Nicht-Ziele

- Keine Pflicht, jeden historischen Kleintext oder jeden Inline-Kommentar sofort zweisprachig nachzuziehen.
- Keine künstliche Verdopplung von technischen Beispielen, wenn ein einziges Beispiel für beide Sprachabschnitte ausreicht.

---

## English

### Goal

Starting with the 0.8 series, normative project documentation is maintained **bilingually (DE/EN)**.

### Scope

This requirement applies at least to:

- `README.md`
- `FAQ.md`
- `ROADMAP.md`
- `RELEASENOTES.md`
- documents under `docs/specs/`
- documents under `docs/adr/` where they matter to product or architecture history

### Rules

1. **Same information content**  
   German and English must remain aligned in meaning. Stylistic differences are fine; information gaps are not.

2. **One document instead of language splits**  
   Normative documents should normally live in **one file** with clearly separated DE/EN sections. Separate language files should be avoided to reduce drift.

3. **Root documents stay the entry points**  
   `README.md`, `FAQ.md`, `ROADMAP.md`, and `RELEASENOTES.md` remain the primary user-facing documents. `docs/` complements them with specifications and decisions.

4. **Specify before large changes**  
   New foundational topics should first receive a matching document in `docs/specs/` or `docs/adr/` before production code changes land.

5. **No dilution of C89/fail-fast principles through docs**  
   Documentation must not soften the project frame. Statements about C89, portability, fail-fast behavior, data-only configuration, and similar core principles must stay consistent.

### Non-goals

- No requirement to retroactively bilingualize every small historical text or every inline comment immediately.
- No artificial duplication of technical examples when a single example is sufficient for both language sections.
