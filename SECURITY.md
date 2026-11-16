# Security / Sicherheit

DE: `tack` verarbeitet Projektpfade, Targetnamen, Compilerprogramm und Konfiguration als potenziell fehlerhafte Eingaben. Standardmäßig sind sichere Zielpfade verpflichtend; `allow_unsafe_paths = yes` ist nur für ausdrücklich freigegebene kontrollierte Sonderfälle zulässig. `TACK_CC` ist kein Kanal für Flags. Keine ungeprüften Live-/Destruktivoperationen. Sicherheitskorrekturen benötigen gezielte negative Tests und unverfälschte Abnahmeberichte. Siehe `docs/security/README.md`.

EN: Treat project paths, target identifiers, compiler program and configuration as untrusted input. Safe paths are the default; unsafe mode requires explicit authorization. TACK_CC is not a flag container. Destructive/live operations require explicit approval. Security fixes need negative tests and authentic evidence. See `docs/security/README.md`.
