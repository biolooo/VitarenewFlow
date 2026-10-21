# TEST 0.12.6 – HOME und Homebrew bei Autostart

## Vor dem Update

Autostart in der bisherigen App deaktivieren, bevor die SD aktualisiert wird. Der Boot-Helfer hat sich geändert; das Setup verweigert andernfalls gegebenenfalls das Update. Nach dem Kopieren die neue `/cias/VitarenewFlow.cia` mit FBI installieren und Version 0.12.6 prüfen.

## Hintergrund und Musik

- [ ] Ohne Custom Theme: heller neutraler Hintergrund mit feinem Raster, keine mintfarbenen Kreise.
- [ ] Keine selbst komponierte Fallback-Musik mehr.
- [ ] Ein installiertes Theme mit Musik spielt nach Appstart und nach „Theme neu laden“ weiterhin.

Originale Nintendo-Standardmusik ist noch nicht fertig. Der bisherige RomFS-Zugriff schlug fehl. Für die nächste Diagnose bitte die neue CIA einmal bei normal laufendem HOME-Menü ohne Custom Theme öffnen, 15 Sekunden warten, schließen und die SD zurückgeben. `session.log` enthält jetzt bei einem Mount-Fehler Titel, Medium und Fehlercode. Eine erfolgreich gelesene private Archivkopie liegt unter `/3ds/3ds-library/music/home-source.bcsar`. Diese Dateien nicht veröffentlichen. Es ist kein PC-Import als Nutzerfunktion vorgesehen.

## Homebrew

- [ ] Autostart nach dem Update neu aktivieren und vollständig neu starten.
- [ ] Mindestens zwei installierte Homebrew-Apps aus der Library starten: richtige App erscheint, kein Rücksprung zu VitaRenewFlow.
- [ ] Homebrew Launcher aus der Library öffnen: echter Homebrew Launcher erscheint.
- [ ] Einen Emulator mit Spiel starten und den bisherigen Rückkehrweg testen.
- [ ] Nach Rückkehr eine weitere App starten; kein Freeze.

## HOME bei Autostart

- [ ] Nach einem kalten Autostart HOME drücken. Falls HOME nicht geladen ist, erscheint „HOME-Menü öffnen“.
- [ ] A wählen: Autostart wird pausiert und die Konsole startet ins normale HOME-Menü neu.
- [ ] Ein Spiel oder eine Homebrew-App aus HOME starten.
- [ ] VitaRenewFlow aus HOME erneut öffnen: Autostart wird ohne sofortigen Neustart wieder aktiviert.
- [ ] HOME bleibt jetzt in dieser Sitzung erreichbar.
- [ ] Vollständig ausschalten/einschalten: VitaRenewFlow startet wieder automatisch.

Die Pause dauert bis zum nächsten Öffnen von VitaRenewFlow. Wird die Library nicht geöffnet, bleiben weitere Starts im normalen HOME-Menü. Das ist ein bewusst sichtbarer Wiederherstellungsweg, kein unsichtbarer Einmal-Boot-Trick.

## Fehlerfälle

Fremd veränderte Boot-Dateien oder ein beschädigter Wiederaufnahme-Marker werden nicht automatisch überschrieben. Bei einem Fehler bitte Bildschirmmeldung und `session.log` sichern. Der nächste echte Konsolentest ist erforderlich; lokale Simulationen beweisen die Hardwareübergänge nicht.
