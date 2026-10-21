# VitarenewFlow 0.12.32

## DS-Diagnose: passende Hilfe statt pauschalem TWL Fix
- Neues vollflächiges DE/EN-Fenster mit zwei Statuszeilen, kurzer Empfehlung und passenden Aktionen.
- Die Prüfung verändert nichts am System. Fehlende Loader führen zur Emulator-Einrichtung, fehlende erforderliche DS-Systemtitel zur Systemupdate-Anleitung. Unbekannte oder nicht unterstützte Systeme erhalten keine Reparaturfreigabe.
- Vorhandene Systemtitel beweisen nicht, dass DS funktioniert. TWL Fix wird erst nach ausdrücklicher Bestätigung angeboten, dass alle DS-Spiele UND Nintendo DS Download Play auch außerhalb Flow ausfallen. Ein einzelnes nicht startendes Spiel reicht nicht.
- Eine fehlende optionale Version Data wird weiterhin nicht als DS-Defekt ausgelegt.
- Vor einer Reparatur werden System und Voraussetzungen erneut geprüft. Eine physische Kombination START halten + A ist erforderlich; Touch kann keine Systemtitel löschen. Zurück, Schließen und Erneut prüfen verwerfen vorherige Bestätigungen.
- Bestehende Reparaturjournale bleiben erhalten. Nach einer begonnenen Reparatur führt die Ansicht zum Neustart und anschließenden Systemupdate. Keine automatische Wiederholung einer Löschung.
- Fehlende Voraussetzung, fehlgeschlagene Abfrage und SD-Schreibfehler haben konkrete Hinweise; technische Details bleiben in den Logs.

## Testliste auf einer funktionierenden Konsole
1. Über den App-Updater auf 0.12.32 aktualisieren, Flow neu öffnen und DS-Diagnose aufrufen.
2. DS-System und Flow-Loader sollten vorhanden sein. Erwartet: Kein TWL Fix empfohlen. Keine Aufforderung, direkt Systemtitel zu löschen.
3. Erneut prüfen antippen und X testen: Status wird frisch abgefragt; kein Download, keine Installation.
4. DS-Modus testen öffnen: kurze Frage zu allen DS-Spielen und Nintendo DS Download Play außerhalb Flow. Mit B zurückgehen; das Kreuz oben rechts schließt das Fenster.
5. Optional nur den Dialog prüfen: Ja, beides zeigt die letzte Reparaturbestätigung. Auf einer funktionierenden Konsole NICHT START+A drücken, sondern mit B abbrechen. Danach muss eine neue Bestätigung erforderlich sein.
6. Sprache auf Englisch wechseln: Texte und Buttons vollständig lesbar. Wieder auf Deutsch wechseln.
7. Ein DS-Spiel starten und mit HOME zurückkehren. Theme-Musik und Power -> B -> Power kurz nachprüfen.

Keine Dateien oder Systemtitel auf einer funktionierenden Konsole zum Test löschen. TWL Fix bleibt für einen tatsächlich außerhalb Flow defekten DS-Modus vorgesehen. Nach der Entfernung sind Neustart und Systemupdate erforderlich; vorab NAND-Backup, Internetzugang und Stromversorgung sicherstellen. Eine erkannte vorhandene Runtime ist keine Kompatibilitätsgarantie für jedes ROM.

## Lokal geprüft
Tatsächlicher Reparaturcode und Eingabeverarbeitung unter UndefinedBehaviorSanitizer: 128 Entscheidungskombinationen, fehlende/ungültige Loader, fehlende erforderliche und optionale Titel, AM/CFG/APT-Abfragefehler, unbekannte Systeme, regionale/modelabhängige genaue Titelliste, veraltete Prüfergebnisse, fehlende Zustimmung, Touch-Sperre, START+A, Zurück/Schließen, bestehende/defekte Journale und IO-Fehler. Keine echte NAND-Löschung ausgeführt.
18 DE/EN-Vorschauen aus dem tatsächlichen UI-Code mit den ausgelieferten Inter-Schriftmetriken und Atlas auf vollständige Texte und Grenzen geprüft. Nativer Build und separater Build sowie Tests aus dem exakten Quellpaket. Echtkonsolen-Abnahme steht aus.

Emulator-Erkennung 0.12.31, Musik-Fix 0.12.30, HOME-Ladeanzeige und Power-Fix bleiben enthalten. Keine Änderung an DS-Loadern, Luma oder Bootdateien.
