# SPEC 0011 — tack BA-STD-C89 test and coverage ratchets

**Status:** DRAFT / ENTWURF — the baseline is NOT MEASURED.

## Deutsch

Für `tack` sind alle bestehenden C89-Regressionstests Bestandteil des Test-Ratchets. Sie werden nach erfolgreicher Abnahme gezählt und dürfen nicht ohne dokumentierte, ausdrücklich freigegebene Vertragsänderung abgeschwächt oder entfernt werden.

Das Coverage-Ratchet erhält eine Baseline erst nach einem tatsächlichen reproduzierbaren, `tack`-gesteuerten instrumentierten Lauf. Es gilt die höchste akzeptierte Coverage auf vergleichbarer Grundlage. Ein neuer höherer akzeptierter Wert ersetzt die vorherige Untergrenze. Coverage wird als exaktes Verhältnis `covered/total` dokumentiert, nicht anhand gerundeter Prozentwerte. Jede Änderung am Nenner, Filter, Compiler oder Messprofil muss offengelegt und getrennt freigegeben werden. Baseline-Minderung ist kein Reparaturweg.

**Blocker:** Es existiert zum Zeitpunkt dieses Entwurfs kein neu gemessener tack-Coverage-Ausgangswert. Weder `0%` noch `100%` dürfen als Ersatz eingetragen werden. Eine vollständige BA-STD-C89-Abnahme von v0.8.3 setzt die tatsächliche Messung und Gate-Einrichtung voraus.

## English

Preserve accepted regression tests as a test ratchet. The coverage ratchet uses the highest actually measured and accepted exact `covered/total` mark under comparable instrumentation. Never invent a baseline or lower the historical high-water mark to get green results. Profile/filter/denominator changes require explicit review. Current baseline is NOT MEASURED, so v0.8.3 is not yet a complete BA-STD-C89 acceptance.
