# VitarenewFlow 0.12.8 – vollständige Abnahme

Stand: 18.10.2026. Lokale Prüfungen sind keine Konsolentests. Pro Punkt PASS/FAIL/NICHT GETESTET eintragen. Bei Freeze oder Datenverlust stoppen, Konsole neu starten und SD-Logs sichern. Keine weitere Featureentwicklung bis zur Auswertung.

## 0. Installation und Ausgangszustand
- [ ] SD sicher auswerfen; Netzteil anschließen. Spielstände und NAND-Backup vorhanden.
- [ ] In FBI `/cias/VitarenewFlow.cia` über die bestehende App installieren. Nur SD-Kopieren aktualisiert die installierte CIA NICHT.
- [ ] App aus HOME starten: Version **0.12.8** prüfen. Auch 3DSX über Homebrew Launcher einmal öffnen.
- [ ] Autostart ist beim Update deaktiviert. Erst nach den ersten Tests aktivieren: neuer Boot-Helfer wird dabei übernommen.
- [ ] Bestehende Spiele, Covers, Einstellungen und Spielstände erhalten.

## Priorität dieses Builds: bitte zuerst diese Folge ausführen
1. Neue CIA in FBI installieren und 0.12.8 prüfen; zunächst Autostart aus.
2. Super Princess Peach und ein zweites bisher funktionierendes DS-Spiel starten.
3. DS beenden und Flow ohne Ausschalten erneut öffnen: kein schwarzer Dauerbildschirm. Dreimal wiederholen.
4. Einstellungen > HOME-Menü öffnen: normales HOME erreichbar.
5. Autostart aktivieren: Einstellung wird gespeichert, App bleibt offen. Konsole selbst neu starten.
6. Dieselben zwei DS-Spiele aus Autostart starten; kurz spielen und speichern.
7. Kalt in Flow booten, HOME drücken: kurzer Neustart ins echte HOME. Alternativ Einstellungen > HOME-Menü öffnen testen.
8. Flow aus HOME öffnen, danach neu starten: Autostart wieder aktiv.
9. Power im Autostart-Modus: Dialog statt sofortigem HOME-Neustart; Ausschalten testen.
10. Autostart deaktivieren: App bleibt offen. Selbst neu starten, normales HOME erscheint.
Fehlercode und Spielname bei jedem Fehler notieren. Übergaben bleiben jetzt in `/3ds/3ds-library/handoff.log` und ggf. `handoff.previous.log` erhalten, auch wenn Flow später erneut gestartet wird.

## 1. DS – zuerst die gemeldete Regression
- [ ] Bereits reparierte Konsole: kein wiederholter Reparaturdialog nach App-Start.
- [ ] Falls Diagnose erscheint: X neu prüfen, Werte/Fehler fotografieren; B schließt sie. Nicht erneut reparieren, wenn DS funktioniert.
- [ ] Mindestens drei bisher funktionierende DS-ROMs direkt starten, je 2 Minuten spielen.
- [ ] DS speichern, neu öffnen, Spielstand vorhanden.
- [ ] DS Download Play aus dem echten HOME-Menü startet.
- [ ] DS-Modul, sofern vorhanden, erkennen und starten.
- [ ] App dreimal neu öffnen: alter Reparaturhinweis bleibt geschlossen.
- [ ] Diagnose in Einstellungen: unbekannter Prüffehler wird nicht als bewiesener Defekt behandelt. Diagnose sperrt keine ROM-Starts.
- [ ] Bei tatsächlichem DS-Startfehler verständliche Meldung und erreichbare Diagnose. Ein einzelnes defektes ROM ist kein Reparaturgrund.
Eine NAND-Reparatur gehört NICHT zum normalen Abnahmetest. Sie entfernt Systemtitel und verlangt ein Systemupdate. Nur bei wirklich defektem DS-System separat testen.

## 2. 3DS, Homebrew und Module
- [ ] Drei digitale 3DS-Spiele starten, darunter Harvest Moon und ein frisch installiertes Spiel.
- [ ] Jeweils HOME drücken: echtes HOME reagiert, Spiel kann fortgesetzt und normal beendet werden.
- [ ] Zwei installierte Homebrew-CIAs starten; kein Rücksprung in Flow.
- [ ] Homebrew Launcher öffnen; echte App-Liste erscheint, kein Flow-Kreislauf.
- [ ] Zwei 3DSX-Homebrews öffnen; Rückweg gemäß App funktioniert.
- [ ] 3DS-Modul einstecken/entfernen: Liste und Informationen aktualisieren sich ohne Freeze.
- [ ] 3DS-Modul starten, HOME drücken, fortsetzen und schließen – kein Freeze.
- [ ] Modul-Kopierassistent zeigt Speicherbedarf und Wechsel nach GodMode9 verständlich. Abbruch vor Kopieren funktioniert.
- [ ] Optional: eigenes DS-/3DS-Modul vollständig kopieren, Ergebnis prüfen, ROM bzw. CIA erscheint, Installation/Start funktionieren. Originalmodul und dessen Save bleiben erhalten. Modul-Spielstandübernahme ist nicht zugesagt.

## 3. Emulatoren und Rückkehr
Für jede Zeile: Start, 2 Minuten Eingabe/Ton, Save, erneuter Start mit Save und Rückkehr prüfen.
- [ ] GB
- [ ] GBC
- [ ] GBA: L+R+Y führt in Flow zurück.
- [ ] SNES: L+R+Y führt in Flow zurück.
- [ ] N64: Start und Rückweg gesondert protokollieren; nicht pauschal aus GBA ableiten.
- [ ] NES: beide Bildschirme reagieren; kein schwarzer Touch-Menü-Bildschirm. Rückweg genau protokollieren (HOME oder Flow).
- [ ] Virtual Boy: L+R+Y führt in Flow zurück.
- [ ] DS-Rückweg gemäß TWiLight/Loader dokumentieren. Eine universelle systemweite Tastenkombination ist nicht zugesagt.

## 4. Autostart und HOME
- [ ] Autostart aktivieren: Einstellung gespeichert, App bleibt ohne Neustart bedienbar; erst beim selbst ausgelösten Neustart startet Flow automatisch.
- [ ] Im Autostart-Modus zwei Homebrew-CIAs, HBL und eine 3DSX-App starten: kein Flow-Kreislauf.
- [ ] Im Autostart-Modus digitales 3DS-Spiel, Modul, DS und GBA öffnen.
- [ ] HOME bei kalt gestarteter Flow-App: kurzer Neustart ins echte HOME; Autostart bis zum nächsten Öffnen von Flow pausiert. Alternativ Einstellungen > HOME-Menü öffnen.
- [ ] Flow aus HOME öffnen: Autostart wird wieder aktiv, ohne sofortigen Neustart; HOME bleibt in dieser Sitzung erreichbar.
- [ ] Nächster Kaltstart landet wieder in Flow.
- [ ] Wird Flow nach HOME-Pause nicht geöffnet, bleibt auch der nächste Start im HOME-Menü. Diese Pause ist beabsichtigt.
- [ ] Beim Kaltstart B gedrückt halten: ursprünglicher Homebrew Launcher erreichbar.
- [ ] Autostart deaktivieren: Einstellung gespeichert, App bleibt offen. Selbst neu starten: normales HOME, kein Freeze. Drei Ein/Aus-Zyklen.

## 5. Power, Schlafmodus und Stabilität
- [ ] Power-Taste aus normal gestartetem Flow: Systemdialog bedienbar, Ausschalten funktioniert.
- [ ] Power-Taste aus Autostart-Flow: eigener Dialog, Neustart und Ausschalten funktionieren.
- [ ] Konsole zuklappen, 30 Sekunden warten, öffnen: Bild und Eingabe reagieren; Musik setzt korrekt fort.
- [ ] HOME zurück zu Flow dreimal, Schlafmodus dreimal; kein eingefrorener Bildschirm/Ton.
- [ ] 15 Minuten durch Kategorien und Spiele navigieren; anschließend Spiel starten und normal ausschalten.

## 6. GUI, Cover und Performance
- [ ] Touch-Swipes, Steuerkreuz/Circle Pad, L/R in beide Richtungen, A/Öffnen und X-Infos funktionieren.
- [ ] Kategorien 3DS/Homebrew getrennt; GB/GBC/GBA zusammen; Auswahl bleibt nach Rückkehr erhalten.
- [ ] Texte und lange Titel gut lesbar, keine Überlappung; Datum/Akku plausibel.
- [ ] Cover richtig herum, passende Zuordnung, fehlendes Cover hat sauberen Platzhalter.
- [ ] Originale 3DS-Banner mehrerer Spiele korrekt und flüssig; fehlendes/ungültiges Banner blockiert keine Bedienung.
- [ ] Schnell 20-mal Kategorien wechseln und durch große Liste swipen: kein Freeze, keine falschen Covers.
- [ ] Aktuelle Kategorie lädt zuerst; benachbarte Kategorien anschließend prüfen. Kaltstart und wiederholten Wechsel jeweils subjektiv und zeitlich protokollieren.
- [ ] X-Infos und Einstellungen öffnen/schließen, Hilfe und Supportbereich bedienbar.

## 7. Theme, Musik, Sprache und Einstellungen
- [ ] Aktives Einzeltheme: Hintergrund und Musik schon beim Start, ohne Reload Theme.
- [ ] Reload Theme übernimmt Änderungen; kein mehrfach überlagerter Ton.
- [ ] Ohne Theme: neutraler heller Rasterhintergrund, keine synthetische Ersatzmusik.
- [ ] Theme-Ton bei Spielstart beendet, nach unterstützter Rückkehr wieder vorhanden; Schlaf/HOME kein Dauerton.
- [ ] DE → EN → DE mehrfach wechseln, Dialoge und Neustart prüfen; Sprache bleibt gespeichert.
- [ ] Helligkeitssteuerung nur als bestanden markieren, wenn reale Displayhelligkeit sich ändert; vorhandener wirkungsloser Slider = FAIL.
**Offen:** Originale Nintendo-Standardmusik spielt noch nicht. Das ist kein bestandener Test und verhindert eine Behauptung „alle Bugs behoben“. Für Diagnose Flow-CIA aus normalem HOME 15 Sekunden öffnen, danach SD zurückgeben. Theme-Shuffle-Audio nicht vollständig unterstützt.

## 8. CIA-Installation und Seed
- [ ] Nicht installiertes CIA-Spiel grau in 3DS, Titel/Cover/Dateigröße korrekt.
- [ ] A/Öffnen zeigt Bestätigung mit Größe/Zeitabschätzung; B bricht ohne Installation ab.
- [ ] Erfolgreich installieren: Fortschritt verständlich; Titel danach normal startbar.
- [ ] CIA erst nach bestätigtem Erfolg gelöscht; installierter Titel bleibt nach Rescan/Neustart sichtbar.
- [ ] Bei einem bekannten Seed-Fall mit Internet: Reparaturmeldung, Erfolg und anschließender Start.
- [ ] Ohne Internet bleibt Fehler verständlich und App bedienbar; keine endlose schwarze Anzeige.
- [ ] Fehlgeschlagene Installation behält CIA und bietet verständliche Fehlermeldung. Kein absichtliches Ausschalten während Installation!

## 9. Desktop und Wiederverwendung
- [ ] Desktop-App direkt von SD öffnen; richtige Karte/Bibliothek erkannt.
- [ ] Eigenes zusätzliches Testspiel importieren, richtige Kategorie/Pfad; danach auf Konsole sichtbar und startbar.
- [ ] Fehlendes Cover manuell pflegen; korrekte Zuordnung und Orientierung auf Konsole.
- [ ] Automatische Cover-Suche mit mindestens drei fehlenden Covers: Treffer prüfen, Nichttreffer verständlich, vorhandene manuelle Covers erhalten.
- [ ] Optimierte Cover klein genug und auf Konsole scharf; Import erneut ausführen ohne Duplikate.
- [ ] Setup erkennt Karte, erhält fremde Dateien/Spielstände und beendet sich mit nachvollziehbarem Ergebnis.
PC-Weiterspielen/Save-Synchronisierung ist zurückgestellt und kein fertiges Feature.

## 10. Abschluss
- [ ] Alle FAILs mit Spielname, Version, Startart (CIA/3DSX/Autostart), Tastenschritten und Foto/Video erfassen.
- [ ] SD wieder einstecken; `/3ds/3ds-library/session.log`, `performance.log`, `twl-repair.log` auswerten lassen. Logs werden teils beim nächsten App-Start ersetzt: nach Fehler möglichst nicht mehrfach neu öffnen.
- [ ] Keine neue Veröffentlichung/Version 1.0, bevor Pflichtfälle auf Hardware bestehen und verbleibende Einschränkungen gemeinsam entschieden wurden.
