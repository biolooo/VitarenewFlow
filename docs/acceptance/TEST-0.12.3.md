# TEST 0.12.3 – DS-Diagnose und Reparatur

Testversion: lokal geprüft, noch keine Hardwareabnahme. Die Reparatur entfernt ausgewählte DS-Systemtitel; anschließend ist ein Nintendo-Systemupdate zwingend.

## Installation

Windows-Testpaket entpacken und Setup auf der CFW-SD ausführen. Für die HOME-Menü-App anschließend `/cias/VitarenewFlow.cia` mit FBI installieren. CIA und 3DSX sollten 0.12.3 anzeigen.

## Diagnose ohne Systemänderung

- [ ] HOME-Icon > Einstellungen > DS-Diagnose und Reparatur öffnen. Loader, Startdateien und Systemtitel werden angezeigt.
- [ ] X prüft erneut; B schließt ohne Systemänderung.
- [ ] Deutsch, Englisch und Touch-Aktionen sind verständlich und lesbar.
- [ ] Vorhandene DS-Spiele starten weiterhin. Bei einem Startfehler führt X aus der Fehlermeldung zur Diagnose.
- [ ] Fehlende Loader-Dateien werden von fehlenden Systemtiteln unterschieden. Ein einzelner ROM-Fehler löst keine automatische Reparatur aus.

## Reparatur nur bei tatsächlich defektem DS-System

Ein verwertbares GodMode9-NAND-Backup bereithalten, Netzteil anschließen und Internet für das Systemupdate sicherstellen. Backup und Wiederherstellung erfolgen nicht automatisch. Bei Regionwechsel oder bereits fehlgeschlagenem Systemupdate zuerst die passende Anleitung prüfen.

- [ ] B bricht vor der Reparatur ohne Systemänderung ab.
- [ ] A allein auf der letzten Bestätigungsseite startet keine Reparatur. Erst START halten + A führt sie aus.
- [ ] Danach zuerst neu starten. Die App führt keine erneute Deinstallation aus.
- [ ] Nach Neustart erscheint die Nachkontrolle. Y öffnet die Systemeinstellungen. Andere Einstellungen > Systemupdate ausführen, auch bei aktueller Firmware.
- [ ] Danach Launcher öffnen und X zur Prüfung drücken. Fehlende Titel lassen den Vorgang offen.
- [ ] B verschiebt die Nachkontrolle; sie erscheint beim nächsten Appstart erneut.
- [ ] DS-Spiel und Nintendo DS Download Play testen. Erst nach Erfolg zurückkehren und A „Test bestanden“ drücken.
- [ ] Nach dieser Bestätigung erscheint der Hinweis nicht erneut.

Bei Fehlern keine erneute Reparatur durchführen. Fehlercode und `/3ds/3ds-library/twl-repair.log` sichern. Das Protokoll bleibt über App-Neustarts erhalten. Vorhandene Titel beweisen keine vollständige Integrität; der echte DS-Test bleibt erforderlich.

Anleitung: https://3ds.hacks.guide/troubleshooting-post-install.html

Die bisherigen offenen POWER-/Autostart-/Modulkopiertests bleiben Bestandteil der finalen Abnahme.
