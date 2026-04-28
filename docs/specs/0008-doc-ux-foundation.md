# SPEC 0008 — tack doc UX foundation

Status: accepted
Serie / series: v0.8.1-dev

## Deutsch

### Ziel
`tack doc` soll als kleine, offline-fähige Doku-Ausgabe deutlich besser lesbar werden, **ohne** einen vollständigen Markdown-zu-HTML-Konverter einzubauen.

### In diesem Schritt umgesetzt
- strukturierte Ausgabe für:
  - Überschriften (`#` bis `######`)
  - Absätze
  - ungeordnete Listen
  - geordnete Listen
  - fenced code blocks (```)
- kleine Inline-Elemente:
  - Inline-Code mit Backticks
  - Markdown-Links `[Text](URL)`
  - einfache Auto-Links für `http://` und `https://`
- seitenbezogene ToC aus erkannten Überschriften (bis Ebene 3)
- deutlich verbesserte Navigation mit Current-Page-Markierung
- Gruppierung von `docs/`-Unterverzeichnissen als eigene Dokumentgruppen in Navigation und Index (z. B. `ADR`, `SPEC`)
- CSS-Nachzug für bessere Typografie, Zeilenumbruch, Abstände und Dark-Mode-Tauglichkeit

### Nicht-Ziele
- kein vollständiger Markdown-Parser
- keine vollständige CommonMark-Kompatibilität
- keine Pflicht zu JavaScript
- keine Abhängigkeit von externen Web-Frameworks

### Leitplanke
`tack doc` bleibt **CSS-first**, **offline-fähig** und **klein**. Struktur wird dort erkannt, wo sie für Lesbarkeit und Navigation den größten Nutzen bringt. Unterverzeichnisse unterhalb von `docs/` sind dabei nicht nur Ablage, sondern Teil der erzeugten Dokumentorganisation.

## English

### Goal
`tack doc` should become a much better offline-friendly reading experience **without** introducing a full Markdown-to-HTML converter.

### Implemented in this step
- structured output for:
  - headings (`#` to `######`)
  - paragraphs
  - unordered lists
  - ordered lists
  - fenced code blocks (```)
- small inline elements:
  - inline code via backticks
  - Markdown links `[text](URL)`
  - simple auto-links for `http://` and `https://`
- page-local ToC from detected headings (up to level 3)
- improved navigation with current-page marking
- grouping of `docs/` subdirectories as document groups in navigation and index (for example `ADR`, `SPEC`)
- CSS refresh for typography, wrapping, spacing, and dark-mode friendliness

### Non-goals
- no full Markdown parser
- no full CommonMark compatibility
- no JavaScript requirement
- no dependency on external web frameworks

### Guardrail
`tack doc` remains **CSS-first**, **offline-friendly**, and **small**. Structure is recognized only where it provides the biggest benefit for readability and navigation. Subdirectories below `docs/` are therefore treated as part of the generated documentation organization, not just as storage.


## Ergänzung / Addendum

DE: Eine optionale kleine JS-Suche als Progressive Enhancement wird in **SPEC 0009** getrennt beschrieben, damit die no-JS-Grundlinie von `tack doc` in dieser Spezifikation klar bleibt.

EN: An optional small JS search as progressive enhancement is described separately in **SPEC 0009**, so that the no-JS baseline of `tack doc` stays clear in this specification.
