# VitarenewFlow 0.12.33

## Desktop-Manager zuverlässig erkennen
- Vorhandene Windows-App auf SD: Auf SD vorhanden und Reparieren statt Installieren.
- Erkennt VitarenewFlow Desktop.exe und den bisherigen dokumentierten Namen 3DS Library PC.exe im SD-Hauptverzeichnis. Die alte EXE bleibt erhalten; Reparieren ergänzt die aktuelle EXE und das vollständige veröffentlichte Paket.
- Fehlende Anleitung, Lizenzdatei oder Quellpaket erzeugen keine falsche Installieren-Anzeige mehr. Die Reparatur lädt weiterhin alle vier Dateien samt Lizenztext und Quellpaket mit HTTPS und SHA256-Prüfung.
- Nicht leere reguläre EXE erforderlich; eine leere Datei oder ein Ordner zählt nicht. Vorhanden beschreibt die Dateipräsenz, keine Windows-Ausführbarkeits- oder Versionsgarantie.
- SD-Abfragefehler zeigen Status unklar und Erst prüfen. Kein Download bei ungeklärter Erkennung.
- Beim Öffnen frisch prüfen. Zusätzlicher Prüfen-Button und X lesen den Status erneut ein. Kein Download durch Prüfen; Prüfung während Download/Einrichtung gesperrt.
- Bestehende Download-/Rollback-Logik unverändert. Keine Spiele, Cover, Einstellungen oder Saves werden durch die Erkennung geändert.

## Testliste
1. Über Updates auf 0.12.33 aktualisieren und Flow neu öffnen.
2. Menü -> Spiele verwalten -> Desktop-Verwaltung: bei vorhandener PC-App Auf SD vorhanden und Reparieren.
3. Prüfen antippen und X drücken: kurz Geprüft; weiterhin Reparieren, kein Download.
4. Reparieren ausführen: Paket erfolgreich speichern; anschließend weiterhin Auf SD vorhanden und Reparieren.
5. Nach Einsetzen der SD in Windows VitarenewFlow Desktop.exe öffnen. Spiele und Cover müssen erhalten sein.
6. Auf Konsole ohne PC-App muss stattdessen Noch nicht installiert und Installieren erscheinen. Nach Installation wechselt die Anzeige zu Reparieren.
7. B und das Kreuz schließen die Seite; Emulator-Erkennung und DS-Diagnose bleiben wie zuvor.

## Lokal geprüft
Aktuelle tatsächliche Erkennungsfunktionen und Eingabeverarbeitung unter UndefinedBehaviorSanitizer: aktuelle/alte EXE, fehlende Beigaben, leere Datei, Ordner, simulierte SD-IO-Fehler, Cache/Neupruefung, Busy-Sperren, 1000 gecachte Abfragen, X+ A ohne versehentlichen Download, Touch-Grenzen, bestehende DS-/Emulator-Prüfungen. Acht DE/EN-Ansichten mit tatsächlichem UI-Code, ausgeliefertem Inter-Atlas und Schriftmetriken auf vollständige Texte und Bildschirmgrenzen geprüft. Nativer Build und separater Build/Tests aus dem exakten Quellpaket geprüft. Konsole und Windows-App auf echter SD müssen noch abgenommen werden.

DS-Diagnose 0.12.32, Emulator-Erkennung 0.12.31 und Musik-Fix 0.12.30 bleiben enthalten. Keine Änderung an Emulator-Downloads, TWL-Reparatur, Luma oder Autoboot.
