# Geräteprüfung für den Backlogstand

Noch nicht ausgeführt. Entwicklungsbranch `codex/backlog-improvements`, Ausgangsbasis `eba8f03`; genaue Testartefakthashes vor Übertragung festhalten. Öffentliche 0.12.33 und lokaler Entwicklungsdiff sind verschiedene Stände. Kein automatischer SD-Transfer oder Release.

Pro Fall Datum, Modell, Region, Luma-Version, Startweg (3DSX/Shortcut/native CIA), Autoboot, verwendetes Artefakt und Ergebnis notieren. Die Versionsanzeige allein identifiziert einen unveröffentlichten Entwicklungsdiff nicht.

| Fall | Erwartung | Ergebnis |
| --- | --- | --- |
| Spielstart mit alten appearance.txt-Werten 0/1 | Kein Animations-Toggle und keine künstliche Wartezeit; echte Vorbereitung/Seedhinweise bleiben | offen |
| About/Support DE/EN | Text vollständig auf beiden Screens, Navigation/Zurück per Taste und Touch | offen |
| Neuerungen DE/EN ein-/ausklappen | Version eindeutig, keine Änderungen unbekannter Remoteversion behauptet, Updateaktionen weiter bedienbar | offen |
| Neues Spiel ohne .lbn, beschädigter Cache, Titelwechsel | Icon/Cover-Fallback, keine falschen Banner, keine Hänger | offen |
| 1/100/1024 Einträge, schnelle Kategorien/Touchwechsel | Auswahl korrekt, Speicher stabil; performance.log auswerten, keine vom PC abgeleiteten FPS nennen | offen |
| POWER → B → POWER wiederholt | Dialog erneut nutzbar; echte Beendigung bleibt möglich | offen |
| HOME vor/nach erstem nativen Start, Karte/digital | Rückweg und Eingaben funktionieren mit/ohne Autoboot | offen |
| DS, Homebrew und jeder Emulator | Start, Rückkehr und zweiter Start, vorhandene Saves erhalten | offen |
| Theme-Musik kalt/Loop/Sleep/Resume | Reale Ausgabe und Lautstärke prüfen; originale Standardmusik bleibt separat offen | offen |
| Frische unterstützte Konsole: UniStore → Flow → Komponenten/Desktop → OTA | Vollständiger Ablauf und Abbruch-/Platzmangel-/fehlende-Dateien-Fälle | offen |

DS-Diagnose auf gesunder Konsole nur prüfen/abbrechen. Keine künstliche Beschädigung und keine NAND-Löschung für diese Abnahme.

## N64-Rücksprung gezielt eingrenzen

Nach einem reproduzierten Fehlstart sichern: `/3ds/n64-library/last-run.log`, `/3ds/3ds-library/handoff.log` (gegebenenfalls handoff.previous.log), session.log, Konsolenmodell/Startkontext und SHA256 der installierten n64-library.3dsx. Keine ROM-Datei erforderlich. Das Adapterlog unterscheidet Kernel-/ROMFS-/picaGL-/UI-/Systeminitialisierung, ROM-Öffnen und CPU/VBlank-Phasen. Erst anhand des letzten erreichten Schritts einen Fix ableiten; erfolgreicher Flow-Preflight ist kein erfolgreicher Emulatorstart.

## Originale HOME-Standardmusik

Es existieren weder eingebauter originaler Musiktitel noch ein CSAR/CSEQ-Instrumenten-/Sequenzplayer. Der vorhandene begrenzte private Archivleser ist nicht im Produktionspfad aufgerufen. Für die Implementierung fehlen ein überprüfter Zugriff auf das konkrete HOME-Archiv, Format-/Instrumentendaten und Gerätevalidierung. Keine Nintendo-Musik verteilen, keinen verworfenen synthetischen Ersatz aktivieren. Aktive Theme-Musik ist davon unabhängig.
