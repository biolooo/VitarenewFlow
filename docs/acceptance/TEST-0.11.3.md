# VitarenewFlow 0.11.3 — Neustart nach Autostart-Änderung

Beobachtung 0.11.2: Nach Deaktivierung Freeze beim Verlassen/POWER. SD-Prüfung bestätigt korrekt wiederhergestellte boot.3dsx und autoboot_mode=off. Letzter Logeintrag „APT suspend“; die genaue interne Ursache ist damit noch nicht bewiesen.

Die App fordert nach erfolgreicher Autostart-Änderung jetzt unmittelbar einen Hardware-Neustart an, statt mit dem bisherigen Boot-Zustand weiterzulaufen und später über HOME/POWER zu wechseln. Vorher werden Dateien geschlossen, die Ansicht gespeichert, Cover-/Banner-Worker beendet und GPU-Ressourcen freigegeben. Der Bestätigungsdialog kündigt den sofortigen Neustart an. Ein abgelehnter Neustart-Aufruf erzeugt einen Fehlerdialog, sofern die Oberfläche wiederhergestellt werden kann.

## Test

1. `/cias/VitarenewFlow-0.11.3.cia` mit FBI installieren. Bei 3DSX-Nutzung ist die App bereits aktualisiert. Version oben in den Einstellungen prüfen.
2. Autostart → B: Abbruch ohne Neustart oder Boot-Änderung.
3. Autostart aktivieren → A: Konsole startet selbst neu und öffnet VitarenewFlow.
4. Autostart deaktivieren → A: Konsole startet selbst neu und landet im HOME-Menü. Es ist kein manuelles Ausschalten erforderlich; kein Freeze.
5. Aktivieren/Deaktivieren noch zweimal wiederholen. Falls die Konsole nicht von selbst neu startet, notieren, welcher Text zuletzt sichtbar war.
6. Danach ein 3DS- und ein GBA-Spiel starten, GBA-Rückkehr prüfen. Sprachwechsel und normales Beenden bei ausgeschaltetem Autostart prüfen.

Bei Startproblemen SELECT beim Einschalten halten, „Hbmenu autoboot“ in Luma ausschalten. Die Sicherung `/3ds/3ds-library/autoboot-hbmenu.3dsx` bleibt erhalten. Die Übertragung aktiviert Autostart nicht und ändert weder boot.3dsx noch boot.firm oder Luma-Konfiguration.

Lokal bestanden: nativer Build; Reihenfolge GPU-Freigabe → Reset → Prozessende; abgelehnter Reset und UI-Wiederherstellung; tatsächliche Bestätigungslogik (Abbruch/Erfolg/Fehler); bestehende Autostart-Dateitransaktionen und Startregressionen. Echtes Neustart-Verhalten muss auf der Konsole getestet werden.

Technische Referenz: [libctru APT_HardwareResetAsync](https://libctru.devkitpro.org/apt_8h_source.html).
