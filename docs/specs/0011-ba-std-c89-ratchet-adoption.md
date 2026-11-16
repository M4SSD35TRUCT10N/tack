# SPEC 0011 — tack: Test- und Coverage-Ratchets / Test and coverage ratchets

**Status:** Entwurf / draft. Eine Coverage-Baseline wurde NICHT GEMESSEN.

## Deutsch

Alle bestehenden C89-Regressionstests gehören zum Test-Ratchet. Nach Abnahme dürfen sie nicht ohne dokumentierte und ausdrücklich freigegebene Vertragsänderung entfernt oder abgeschwächt werden.

Eine Coverage-Baseline wird ausschließlich durch einen tatsächlich ausgeführten, reproduzierbaren, `tack`-gesteuerten instrumentierten Testlauf begründet. Es gilt der höchste akzeptierte Wert auf vergleichbarer Grundlage. Coverage wird als exaktes Verhältnis `covered/total` geführt, nicht über gerundete Prozentwerte. Änderungen an Nenner, Filtern, Compiler oder Messprofil erfordern Dokumentation und separate Freigabe. Eine niedrigere Baseline ist keine Fehlerbehebung.

**Offenes Gate:** Es gibt keinen akzeptierten, aktuell gemessenen High-Water-Ausgangswert. Weder `0%` noch `100%` dürfen als Platzhalter verwendet werden. Vollständige BA-STD-C89-Konformität und Releasefreigabe erfordern tatsächliche Messung, Ratchet-Gate und Owner-Abnahme. Grüne Funktionstests allein erfüllen dieses Gate nicht.

## English

All existing C89 regression tests belong to the test ratchet. Once accepted, they must not be removed or weakened without an explicitly approved, documented contract change.

A coverage baseline must come from an actually executed, reproducible, `tack`-driven instrumented test run. The highest accepted value under comparable conditions is the high-water mark. Record coverage as the exact `covered/total` ratio, not a rounded percentage. Changes in the denominator, filters, compiler or measurement profile require documentation and separate approval. Lowering the baseline is not a repair.

**Open gate:** No accepted, currently measured high-water baseline exists. Neither `0%` nor `100%` may be used as a placeholder. Full BA-STD-C89 compliance and release approval require actual measurement, a ratchet gate and owner acceptance. Green functional tests alone do not satisfy this gate.
