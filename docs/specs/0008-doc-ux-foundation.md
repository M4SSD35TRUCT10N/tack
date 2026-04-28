# SPEC 0008 — tack-doc-UX-Grundlage / tack doc UX foundation

**Status:** angenommen / accepted
**Serie:** v0.8.1-dev
**Typ:** Dokumentations-UX-Spezifikation / documentation UX specification

---

## Deutsch

### Ziel

`tack doc` soll als kleine, offline-fähige Doku-Ausgabe deutlich besser lesbar werden, **ohne** einen vollständigen Markdown-zu-HTML-Konverter einzubauen.

### Umsetzungsstand

Die Markdown-lite-Ausgabe, bessere Typografie/CSS, seitenbezogene ToC, Current-Page-Markierung und gruppierte Navigation für `docs/`-Unterverzeichnisse sind umgesetzt. Die optionale JS-Suche wird bewusst separat in SPEC 0009 beschrieben.

### Umfang

Umgesetzt ist strukturierte Ausgabe für:

- Überschriften (`#` bis `######`)
- Absätze
- ungeordnete Listen
- geordnete Listen
- fenced code blocks (```)

Zusätzlich werden kleine Inline-Elemente erkannt:

- Inline-Code mit Backticks
- Markdown-Links `[Text](URL)`
- einfache Auto-Links für `http://` und `https://`

Weitere UX-Bestandteile:

- seitenbezogene ToC aus erkannten Überschriften bis Ebene 3
- deutlich verbesserte Navigation mit Current-Page-Markierung
- Gruppierung von `docs/`-Unterverzeichnissen als eigene Dokumentgruppen in Navigation und Index, zum Beispiel `ADR` und `SPEC`
- CSS-Nachzug für bessere Typografie, Zeilenumbruch, Abstände und Dark-Mode-Tauglichkeit

### Nicht-Ziele

- kein vollständiger Markdown-Parser
- keine vollständige CommonMark-Kompatibilität
- keine Pflicht zu JavaScript
- keine Abhängigkeit von externen Web-Frameworks

### Leitplanke

`tack doc` bleibt **CSS-first**, **offline-fähig** und **klein**. Struktur wird dort erkannt, wo sie für Lesbarkeit und Navigation den größten Nutzen bringt. Unterverzeichnisse unterhalb von `docs/` sind dabei nicht nur Ablage, sondern Teil der erzeugten Dokumentorganisation.

---

## English

### Goal

`tack doc` should become a much better offline-friendly reading experience **without** introducing a full Markdown-to-HTML converter.

### Implementation status

Markdown-lite output, improved typography/CSS, page-local ToC, current-page marking, and grouped navigation for `docs/` subdirectories are implemented. The optional JS search is intentionally described separately in SPEC 0009.

### Scope

Structured output is implemented for:

- headings (`#` to `######`)
- paragraphs
- unordered lists
- ordered lists
- fenced code blocks (```)

Small inline elements are also detected:

- inline code via backticks
- Markdown links `[text](URL)`
- simple auto-links for `http://` and `https://`

Additional UX elements:

- page-local ToC from detected headings up to level 3
- improved navigation with current-page marking
- grouping of `docs/` subdirectories as document groups in navigation and index, for example `ADR` and `SPEC`
- CSS refresh for typography, wrapping, spacing, and dark-mode friendliness

### Non-goals

- no full Markdown parser
- no full CommonMark compatibility
- no JavaScript requirement
- no dependency on external web frameworks

### Guardrail

`tack doc` remains **CSS-first**, **offline-friendly**, and **small**. Structure is recognized only where it provides the biggest benefit for readability and navigation. Subdirectories below `docs/` are treated as part of the generated documentation organization, not just as storage.
