# VitarenewFlow 0.12.9 – Modulstart und DS-Zuverlässigkeit

Stand: 18.10.2026. Lokale Tests bestanden; das Verhalten auf der Konsole ist noch zu bestätigen.

## Update
- [ ] SD sicher auswerfen und in die Konsole setzen.
- [ ] In FBI `/cias/VitarenewFlow.cia` über die bestehende App installieren.
- [ ] Version **0.12.9** im Launcher prüfen. Eine Dateikopie allein aktualisiert die installierte CIA nicht.
- [ ] Autostart für die erste Testreihe in den Einstellungen deaktivieren und die Konsole selbst neu starten. Das PC-Update verändert den Autostart nicht.

## DS ohne Autostart
- [ ] Das zuvor betroffene DS-Spiel direkt nach dem Einschalten starten (im gespeicherten Startprotokoll steht Super Mario 64 DS USA Rev 1). Spiel muss beim ersten Versuch starten.
- [ ] Zwei Minuten spielen, speichern, mit HOME beenden; Flow erneut öffnen, dasselbe Spiel erneut starten. Dreimal ohne Ausschalten wiederholen.
- [ ] Ein anderes DS-Spiel starten; anschließend ohne Ausschalten zurück zum ersten Spiel wechseln. Kein weißer Bildschirm, kein endloser schwarzer Bildschirm.
- [ ] Auch die anderen DS-Testspiele mindestens einmal starten. Bestehende Spielstände laden weiterhin.

## DS mit Autostart
- [ ] Autostart aktivieren: App bleibt bedienbar. Konsole selbst neu starten, Flow erscheint.
- [ ] Dasselbe betroffene DS-Spiel beim ersten Versuch starten, kurz spielen und speichern.
- [ ] Mit HOME beenden, Flow erneut öffnen und noch zweimal starten, ohne die Konsole auszuschalten.
- [ ] Zwischen zwei DS-Spielen wechseln. Bestehende Spielstände bleiben erhalten.

## Physisches 3DS-Spiel
- [ ] 3DS-Modul aus Flow starten: kein Fehler FFFFFFFE.
- [ ] HOME drücken, Spiel fortsetzen, erneut HOME und Spiel schließen. Danach Flow erneut öffnen.
- [ ] Modulstart auch mit aktiviertem Autostart nach einem Neustart testen.
- [ ] Vor dem Start entferntes oder gewechseltes Modul verursacht eine verständliche Fehlermeldung oder eine korrekt aktualisierte Auswahl, keinen Freeze.

## Regressionen
- [ ] Digitales 3DS-Spiel starten und per HOME verlassen.
- [ ] Frisch installiertes 3DS-Spiel starten; CIA-Installation und Seed-Reparatur bei Bedarf weiterhin möglich.
- [ ] Game Boy und GBA starten, bestehenden Spielstand laden; L+R+Y bei den unterstützten Emulatoren.
- [ ] HOME aus kalt gestartetem Flow bleibt erreichbar; nach erneutem Öffnen von Flow ist Autostart wieder aktiv.
- [ ] Autostart ausschalten: App bleibt offen; nächster Konsolenstart führt ins normale HOME.
- [ ] POWER und Zuklappen/Aufklappen weiterhin ohne Freeze.

## Hinweise und Diagnose
Der DS-Loader war bereits aktuell (NTR Forwarder SD-30 / nds-bootstrap 2.17.0); alle fünf DS-ROMs entsprechen bytegenau den PC-Dateien.
Für reine DS-Spiele mit gültigem Retail-Header und ohne eigene NTR-Forwarder-Einstellungen erstellt Flow beim nächsten Start ein Profil: DS-Modus, 67 MHz, synchroner Kartenlesezugriff, Release-Bootstrap. DSi-Titel, Homebrew und vorhandene Profile bleiben erhalten. Speichernummer/-pfad werden nicht verändert.
Der Patch-Cache genau dieses Spiels wird bei der einmaligen Profilanlage als `.vf-ntr-v1-N.bak` aufbewahrt. Danach wird der neu erzeugte Cache wiederverwendet. Diese Änderung ist ein Kompatibilitätsversuch gegen die sporadischen weißen Bildschirme, kein bereits auf Hardware bewiesener Fix.
Eigene Einstellungen können wie bisher durch Halten von Y während des DS-Loaders geöffnet werden; sie werden von Flow nicht überschrieben.
Die Modulprüfung liest direkt vor dem Start die eingesteckte Karte erneut ein und prüft ihre Identität. Anschließend startet sie vom Kartenmedium, ohne installierte Inhaltsgrößen vorauszusetzen. Entfernte/gewechselte Karten und leere installierte Titel bleiben Fehler.

Bei einem Fehler: Spielname, Autostart an/aus, erster oder späterer Versuch und ungefähre Wartezeit notieren. Bei weißem Bildschirm 30 Sekunden beobachten, dann über HOME beenden, sofern möglich. Vor erneutem Start die SD zurückgeben, damit die letzte DS-Konfiguration erhalten bleibt. Keine TWL-Reparatur für ein einzelnes betroffenes Spiel ausführen.
Protokolle: `/3ds/3ds-library/handoff.log`, gegebenenfalls `handoff.previous.log`, `session.log`, `/_nds/nds-bootstrap.ini` und `/_nds/ntr-forwarder/gamesettings/`.
