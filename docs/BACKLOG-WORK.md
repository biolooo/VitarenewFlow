# Backlog-Auftrag vom 21.10.2026

Nutzerauftrag: offene To-dos ansehen und umsetzen. Ausgangsbasis `eba8f03`, sauberer Arbeitsbaum; Entwicklungsbranch `codex/backlog-improvements`. Der Auftrag erlaubt lokale Entwicklungsarbeit und Prüfung, keine Veröffentlichung, keinen main-Merge und keine SD-/NAND-Eingriffe. Historische Roadmap bleibt Referenz; maßgeblich sind STATUS und DECISIONS.

## Akzeptanz und Zuständigkeiten

| Punkt | Nachweis / nächster Schritt | Zuständigkeit |
| --- | --- | --- |
| Spiel-Startanimation entfernen | Kein Toggle/Logo/künstlicher Mindestframe-Wait; bestehende Einstellungen kompatibel; Seed-/HOME-Ladezustände und Originalbanner bleiben | UI-Implementierer exklusiv für main.c/ui.c/settings/l10n |
| About/Support | Lesbare strukturierte DE/EN-Ansicht, Bedienung und Rückweg erhalten | derselbe Implementierer |
| Kurze ausklappbare Neuerungen | Nur bekannte echte Versionshinweise, keine erfundenen Änderungen unbekannter Remoteversionen; Updateprüfungen unverändert | derselbe Implementierer |
| Banner/Cover/große Bibliotheken | Reale Parser mit synthetischen gültigen/defekten/fehlenden Daten prüfen; PC-Zeit kein Konsolen-FPS-Nachweis | separater Validierungs-Worktree |
| Paket-/Quellzuordnung | Lokale Manifestgrößen/Hashes und vorhandene Source-/Lizenzbegleiter prüfen, ungeklärte Zuordnung offen ausweisen | Validierungsagent und lesender Prüfer |
| Standardmusik | Vorhandenen HOME-Archivzugriff/Player und notwendige Formatdaten prüfen; keine privaten Audiodaten verteilen, keinen synthetischen Ersatz einführen | lesender Prüfer, Orchestrator |
| N64-Rücksprung | Code-/Adapterprüfung; tatsächliches Modell, Adapterhash und Fehlstartlogs fehlen bisher | lesender Prüfer; Nutzerangaben angefragt |
| Hardware .27–.33 / frische UniStore-Einrichtung | Version/Modell/Region/Autoboot und konkrete Fälle auf echter Konsole protokollieren | benötigt Konsolenzugang, keine fingierte Abnahme |
| Save-/PC-Sync, Konvertierung, Modul-Saves | Bisher ausdrücklich vertagte Produktentscheidung; Nutzer nach Planung gefragt | Entscheidung ausstehend |
| Technische Schulden | Konkrete Testlücken/Drift und veraltete Aussagen beheben; keine pauschale riskante main.c/ui.c-Umschreibung | Orchestrator/Prüfer |

## Prüfweg

Implementierer liefert fokussierten Diff und relevante Hostprüfungen. Anschließend unabhängiger lesender Review und Integration des separaten Validierungs-Worktrees. Frischer nativer Build/Hostrunner im gepinnten SDK, zusätzliche neue Prüfungen und Releasebytevergleich. Ergebnisse, offene Grenzen und Hardwareabnahme getrennt in STATUS/HANDOFF führen. Keine Versions-/Kanalbytes nur für lokale Entwicklung ändern.

## Rückweg

Keine fremden Änderungen überschreiben oder stashen. Änderungen bleiben als fokussierter Branchdiff zur gesicherten Basis prüfbar. Vor einem späteren Release eigener freigegebener Versions-/Paketauftrag; die bisherige öffentliche .33 ist keine Auslieferung dieses Entwicklungsdiffs.

## Ergebnis und Nachweise

- Startanimation/Toggle/Logo/Mindestframe-Wait entfernt. Die nur hierfür verwendete appearance.txt wird weder geändert noch gelöscht; Sprach-/Ansichtsdateien bleiben unabhängig. Echter Vorbereitungs-/Seed-/HOME-Status und Originalbanner bleiben.
- About/Support strukturiert in DE/EN, Rückweg erhalten. Updatepanel zeigt per X/Touch ausklappbare lokale Highlights für bekannte installierte 0.12.33. Unbekannte Versionen ergeben keine Notes. Keine Änderung an Download-/Manifest-/Installationslogik.
- Tatsächliche Menü-/Highlightdaten-/Touchgrenzen-/Übersetzungstests mit UBSan bestanden. Neue Banner-/Bibliotheksmodultests prüfen gültige Stereo-Pixeldaten, kaputte/trunkierte Header/Indizes/Frames und 1024 unterschiedliche Coverpfade. Keine Fullscan-/GPU-/FPS-Behauptung.
- 25 lokale Paketdateien stimmen mit Manifestgröße/SHA256 überein, 19 benannte Source-/Noticebegleiter vorhanden. Fünf ursprüngliche Python-Tests im Gesamtcheck bestanden; anschließend um Ablehnung leerer Manifeste ergänzt und alle sechs Python-Tests erfolgreich erneut ausgeführt. Aktuelle Zuordnung samt alten Fehlverweisen in COMPONENT-AUDIT.md, falsche Architektur-/Noticeaussagen korrigiert. Geschützte Releasebytes erhalten.
- Reproduzierbarer Gesamtcheck mit erweitertem scripts/verify-local.sh: Exitcode 0, kompletter nativer 3DSX-/SMDH-Build und alle bisherigen Hostfixtures plus neue Prüfungen bestanden. Nachweis: `.test-build/verification-20261021-185456/` mit package-verification.log, verification.log, source-hashes.json, staging-path.txt und artifact-hashes.txt.
- Erste integrierte Bannerprüfung scheiterte an fehlenden Host-zlib-Headern. Ausschließlich Debian zlib1g-dev ergänzt, Paketindizes mit funktionalem temporär gebundenem /dev/null erfolgreich signaturgeprüft; danach vollständiger Lauf bestanden. Kein devkitARM-Upgrade.
- Unabhängiger lesender Review: keine weiteren Code-/Prüferblocker. Leere Paketmanifeste werden nach ergänztem Fehlerfall abgewiesen, gezielter Nachlauf mit sechs Tests in package-followup.log. Alle 110 erfassten Buildquellen/-assets stimmen nach dem Build mit source-hashes.json überein; acht Releasehashes und geschützte Releasepfade unverändert. Pixel-/Konsolenprüfung bleibt offen.

N64 wurde vom Nutzer ausdrücklich bis zu späterem Test/Logs vertagt; Save-/Sync/Konvertierungs-/Modul-Save-Themen sollen offen bleiben. Hardwarematrix in acceptance/BACKLOG-PC2.md ist vorbereitet, aber nicht ausgeführt. Originale HOME-Standardmusik bleibt offen: vorhandener privater Archivleser ist nicht aufgerufen; CSAR/CSEQ-/Instrumentenplayer und überprüfte reale Daten fehlen. Ein aktiviertes unbewiesenes Fallback wäre keine Erfüllung dieser Anforderung. Vollständige Source/Binary-Reproduzierbarkeit der Emulatoradapter bleibt ebenso offen. Der Gesamtbacklog ist deshalb nicht als vollständig abgeschlossen markiert.
