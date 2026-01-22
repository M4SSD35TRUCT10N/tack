# Example templates for tack (v0.7.1+)

Recommended layout: place this `templates/` directory next to your `src/` directory (repo root).

## tack.ini example

[doc]
template = templates/tack_template_min.html
css      = templates/tack_doc.css

[bom]
template = templates/tack_template_min.html
css      = templates/tack_doc.css

## Optional: template with inline JS search

Use:
template = templates/tack_template_search_inline.html

Notes:
- The inline search filters navigation links and finds headings on the current page.
- Cross-page search requires a generated `search-index.json`. A placeholder is included.
