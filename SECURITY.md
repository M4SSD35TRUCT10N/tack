# Sicherheit / Security

## Deutsch

`tack` behandelt Projektpfade, Targetnamen, Compilerauswahl und Konfiguration als potenziell fehlerhaft. Sichere Zielpfade sind Standard; `allow_unsafe_paths = yes` ist nur für bewusst kontrollierte Sonderfälle vorgesehen. `TACK_CC` ist ausschließlich ein Programmpfad bzw. Name, kein Flag-Container. Destruktive Operationen erfordern Freigabe. Sicherheitsänderungen benötigen negative Tests und nachvollziehbare Abnahmen; Tests und Coverage-Baselines dürfen nicht abgeschwächt werden. Siehe [Sicherheitshinweise](docs/security/README.md).

## English

`tack` treats project paths, targets, compiler selection and configuration as potentially invalid inputs. Safe paths are the default; `allow_unsafe_paths = yes` is for explicitly controlled exceptions only. `TACK_CC` selects an executable name or path, not compiler flags. Destructive operations require approval. Security changes need negative tests and verifiable acceptance; tests and coverage baselines must not be weakened. See [security guidance](docs/security/README.md).
