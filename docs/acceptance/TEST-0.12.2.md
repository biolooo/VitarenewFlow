# VitarenewFlow 0.12.2 – Testpaket

Noch keine stabile Version 1.0.0. Diese Version ergänzt die Rückmeldung nach dem GodMode9-Kopierassistenten. Die offenen POWER-/Neustart-/Modultests aus 0.12.0/0.12.1 bleiben erforderlich.

## Installation

- Windows-Setup mit eingesteckter CFW-SD öffnen und Installation/Reparatur durchführen.
- Für die HOME-Menü-App anschließend /cias/VitarenewFlow.cia mit FBI installieren. Nur die 3DSX zu ersetzen aktualisiert die CIA-App nicht.
- Der vorhandene GodMode9-Payload und die Emulatoren bleiben Voraussetzung. Setup ersetzt weder boot.firm noch boot.3dsx und ändert keine Luma-Konfiguration.

## Neue Kopier-Rückmeldung

- [ ] DS-Modul: X-Infos → X-Kopierassistent → Neustart mit START → GodMode9 → HOME → Scripts → VitarenewFlow-Copy. Kopie abschließen, neu starten und Launcher öffnen. Die DS-Kategorie öffnet sich mit Bestätigung; neue ROM sichtbar und startbar.
- [ ] 3DS-Modul: denselben Ablauf durchführen. Nach verifizierter CIA öffnet sich die 3DS-Kategorie mit Installationshinweis. Grauen Titel auswählen, Installationsdialog prüfen, installieren und starten. Bereits installierte Titel bleiben ohne doppelten CIA-Eintrag.
- [ ] Kopieren abbrechen: beim nächsten Launcherstart erscheint „Modulkopie nicht bestätigt“. Keine unvollständige Datei als fertiges Spiel sichtbar.
- [ ] Vorhandene Kopie erneut versuchen: keine Überschreibung, keine falsche Erfolgsmeldung.
- [ ] Nach einer Rückmeldung Launcher erneut öffnen: alte Bestätigung erscheint nicht wieder.
- [ ] Dieselben Rückmeldungen auf Englisch prüfen; Sprache bleibt nach Neustart erhalten.
- [ ] Modulspielstand bleibt erhalten. Die Kopie übernimmt den Modulspielstand noch nicht automatisch.

## Weiterhin Pflicht vor 1.0

- [ ] Autostart aktivieren/deaktivieren und echte Neustarts prüfen; kein unerwarteter Rücksprung oder Freeze.
- [ ] POWER, HOME, Schlafmodus sowie zehn Spielstarts/Rückkehr nacheinander prüfen.
- [ ] Vollständige finale Checkliste in docs/acceptance/FINAL-1.0.md durchführen.

Lokale Prüfungen ersetzen nicht die Ausführung von GodMode9 und der App auf echter Hardware. Bei Fehlern bitte Ablauf und /3ds/3ds-library/session.log sichern. Eine eventuell vorhandene copy-result.txt ebenfalls sichern.
