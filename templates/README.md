# tack templates (DE/EN)  v0.7.24+

Recommended layout: place this `templates/` directory next to your `src/` directory (repo root). `tack init` can create this folder for new projects.

---

## Deutsch

Empfohlenes Layout: Lege den Ordner `templates/` neben den Ordner `src/` im Repo-Root ab. `tack init` kann diesen Ordner für neue Projekte anlegen.

### tack.ini Beispiel

```ini
[doc]
template = templates/tack_template_min.html
css      = templates/tack_doc.css

[bom]
template = templates/tack_template_min.html
css      = templates/tack_doc.css
```

### Optional: Template mit Inline-JS-Suche

Nutze:

```ini
template = templates/tack_template_search_inline.html
```

Hinweise:
- Die Inline-Suche filtert Navigationslinks und findet Überschriften auf der aktuellen Seite.
- Eine Suche über mehrere Seiten erfordert einen generierten `search-index.json`. Ein Platzhalter ist enthalten.
- Die mitgelieferte `tack_doc.css` enthält jetzt Grundunterstützung für `prefers-color-scheme`, `prefers-contrast: more` und `forced-colors: active`, damit DOC/BOM-Ausgaben in Hell/Dunkel/High-Contrast ohne JavaScript besser lesbar bleiben.

---

## English

Recommended layout: place the `templates/` directory next to `src/` in the repo root. `tack init` can create this directory for new projects.

### tack.ini example

```ini
[doc]
template = templates/tack_template_min.html
css      = templates/tack_doc.css

[bom]
template = templates/tack_template_min.html
css      = templates/tack_doc.css
```

### Optional: template with inline JS search

Use:

```ini
template = templates/tack_template_search_inline.html
```

Notes:
- The inline search filters navigation links and finds headings on the current page.
- Cross-page search requires a generated `search-index.json`. A placeholder is included.
- The shipped `tack_doc.css` now includes baseline support for `prefers-color-scheme`, `prefers-contrast: more`, and `forced-colors: active` so DOC/BOM output remains readable in light/dark/high-contrast setups without JavaScript.
