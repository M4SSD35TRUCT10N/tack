# SPEC 0009 — Optionale JS-Such-Grundlage / Optional JS search foundation

Status: draft
Serie / Series: v0.8.1-dev

## Deutsch

### Ziel
`tack doc` darf optional eine kleine JavaScript-Suche als **Progressive Enhancement** erzeugen.

### Leitplanken
- Default bleibt **ohne JavaScript**.
- Die Suche ist nur aktiv, wenn in `tack.ini` unter `[doc]` explizit `allow_js_search = yes` gesetzt ist.
- Das Suchfeld sitzt in der Kopfzeile, nicht in der linken Navigation.
- Durchsucht wird der statisch erzeugte Dokumentindex aller generierten Seiten.
- Ohne JavaScript bleiben Index, Navigation, Dokumentgruppen und Browser-Suche (`Strg+F`) vollständig nutzbar.
- Es wird **keine** Web-App und **kein** großer Such-Stack eingeführt.

### Mindestumfang
- Suchfeld in der generierten Doku-Navigation
- statisch erzeugte Suchdaten aus den bekannten Dokumentinhalten
- Treffer über Dokumenttitel, Gruppe und Dokumenttext
- Ergebnislinks funktionieren auch auf verschachtelten Seiten unter `docs/...`

### Nicht-Ziele
- keine serverseitige Suche
- keine Abhängigkeit von externen Bibliotheken
- keine Pflicht für JavaScript

## English

### Goal
`tack doc` may optionally generate a small JavaScript search as **progressive enhancement**.

### Guardrails
- The default stays **without JavaScript**.
- Search is only enabled when `allow_js_search = yes` is explicitly set under `[doc]` in `tack.ini`.
- The search field lives in the header, not in the left navigation.
- It searches the static document index across all generated pages.
- Without JavaScript, index, navigation, document groups, and browser search (`Ctrl+F`) remain fully usable.
- This does **not** introduce a web app or a large search stack.

### Minimum scope
- search field in the generated documentation navigation
- statically generated search data from known document content
- matching over document title, group, and document text
- result links work correctly even on nested pages under `docs/...`

### Non-goals
- no server-side search
- no dependency on external libraries
- no JavaScript requirement
