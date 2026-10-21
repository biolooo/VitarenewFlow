# VitarenewFlow 0.12.29

## Ladebildschirm beim Wechsel ins HOME-Menü
- Eigene ruhige HOME-Ansicht auf beiden Bildschirmen, deutsch und englisch: Vorbereitung, Systemübergabe oder bestehender Neustart ins HOME-Menü.
- Die Anzeige wird vor den blockierenden Systemaufrufen eingereiht und auf die Bildschirme gebracht, auch wenn eine anstehende HOME-Anforderung normale Frames bereits sperrt.
- Während der HOME-Registrierungsprüfung bewegt sich ein Warteindikator. Während der anschließenden APT-Systemübergabe bleibt das zuletzt gezeichnete Bild stehen, bis das Betriebssystem die Bildschirme übernimmt. Keine erfundenen Prozentzahlen oder zusätzliche Mindestwartezeit; nur ein Display-Refresh vor dem ersten blockierenden Aufruf.
- Nach Rückkehr oder Fehler wird der Ladezustand entfernt und die normale Ansicht wieder freigegeben. Die vorhandene 10-Sekunden-Grenze der HOME-Registrierungsprüfung bleibt bestehen. Eine echte System-Beendigungsanforderung bricht diese Prüfung ab.

Der bisherige HOME-Startweg und das Verhalten der Emulatoren bleiben erhalten. Der Musik-Fix aus 0.12.28 und der Power-Fix aus 0.12.27 sind enthalten.

## Testliste auf der Konsole
1. Aktualisieren und vollständig neu starten, Autostart eingeschaltet lassen. HOME drücken, bevor ein natives Spiel gestartet wurde: statt der stehenden Bibliothek soll die HOME-Ladeanzeige erscheinen. Falls der bestehende Weg einen Neustart benötigt, soll dies vorher benannt werden.
2. Flow wieder öffnen und HOME erneut drücken. Bereits geladenes HOME soll weiterhin zügig erreichbar sein; keine künstliche mehrsekündige Wartezeit.
3. HOME über Menü sowie über das Power-Fenster öffnen. Dieselbe Ladeansicht soll vor dem Wechsel erscheinen.
4. Flow wieder öffnen: kein Ladebildschirm mehr, Spielauswahl, Suche und Kategorien funktionieren.
5. Direkt nach einem Autostart-Kaltstart ein installiertes 3DS-Spiel und ein 3DS-Modul testen: eventuell nötige HOME-Vorbereitung soll die eigene Anzeige verwenden. Spielstart und HOME-Taste im Spiel bleiben funktionsfähig.
6. Ohne Autostart starten und HOME-Wechsel wiederholen.
7. Power drücken, mit B abbrechen und erneut Power drücken; Theme-Musik und Klicks weiterhin prüfen.

## Lokal geprüft
Zwölf Szenarien mit tatsächlich aus dem aktuellen Launcher extrahierten Funktionen unter UndefinedBehaviorSanitizer: Frame vor APT/NS-Aufrufen, verzögerte Registrierung, 10-Sekunden-Timeout, Startfehler, nicht unterstützte Region, Query-Fehler, Neustartfehler mit UI-Wiederherstellung, Schutz bei Installation/Preflight/DS-Reparatur, echte Beendigungsanforderung, bereits registriertes HOME, Fehler beim HOME-Signal und Power-Dialog ohne registriertes HOME. Der echte Frame-Runtime-Test prüft das Zeichnen trotz anstehender HOME-Anforderung und die Close/Inactive-Sperren. Audioregressionen bestanden. Native Build und isolierter Build des exakt veröffentlichten Quellpakets werden geprüft. DE/EN-Vorschau aus der tatsächlichen Zeichenfunktion lokal kontrolliert.

Die Echtkonsolen-Abnahme steht aus. Während der OS-eigenen Übergabe kann Flow keine Animation fortsetzen, sobald das System die Grafik übernimmt. Dieses Update verkürzt die Systemladezeit nicht; es zeigt verständlich an, worauf gewartet wird. Keine Änderungen an Luma oder den Bootdateien. Update über die vorhandene In-App-Updatefunktion.
