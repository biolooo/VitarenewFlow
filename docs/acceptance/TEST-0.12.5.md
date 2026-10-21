# TEST 0.12.5 – Modulstart und eigenes Fallback-Theme

Die geänderten Abläufe sind lokal geprüft; echte HOME-Rückkehr und Audiowiedergabe müssen auf der Konsole getestet werden.

## Installation

Nach dem SD-Update `/cias/VitarenewFlow.cia` mit FBI installieren. Die HOME-Menü-App und die 3DSX sollen 0.12.5 anzeigen. Die CIA wird durch bloßes Ersetzen der 3DSX nicht aktualisiert.

## 3DS-Modul / HOME

- [ ] Eingestecktes 3DS-Spiel über den Eintrag mit „Modul“ starten. Es muss weiterhin normal starten.
- [ ] HOME drücken: HOME-Menü erscheint ohne Freeze. Spiel fortsetzen, erneut HOME drücken, Spiel schließen.
- [ ] Launcher erneut öffnen, denselben Ablauf mindestens dreimal prüfen.
- [ ] Einmal nach normalem HOME-Start und einmal nach aktivem Launcher-Autostart prüfen.
- [ ] Ein digitales 3DS-Spiel starten und HOME-Rückkehr als Vergleich prüfen.

Die Korrektur betrifft den 3DS-Modulstart. DS-Module benutzen weiterhin ihren bisherigen Startpfad; deren Rückkehrverhalten separat testen, falls der gemeldete Freeze ein DS-Modul betraf.

## Musik und Hintergrund

- [ ] Einzelnes Anemone-Theme mit Musik wählen. Launcher frisch öffnen: Musik startet ohne „Theme neu laden“.
- [ ] Launcher schließen/öffnen und Konsole vollständig neu starten: kein manuelles Neuladen nötig.
- [ ] Standardtheme wählen, Launcher öffnen: eigener heller VitaRenewFlow-Hintergrund und eigene dezente Musik erscheinen automatisch.
- [ ] Mindestens eine Minute hören: Schleife ohne lange Pause oder auffälliges Knacken. Keine PC-Vorbereitung und keine zusätzlichen Musikdateien nötig.
- [ ] Deckel schließen/öffnen: Musik pausiert und setzt fort.
- [ ] Spiel starten: Menümusik endet vollständig. Nach erneutem Öffnen läuft wieder genau eine Musikinstanz.
- [ ] Anemone-Theme wechseln und „Theme neu laden“: neue Musik spielt statt der alten.
- [ ] Cover schnell durchwischen: kein Audio-Stottern und keine deutlich schlechtere Bedienung.

Das Fallback verwendet eine eigene Komposition, nicht die originale Nintendo-Musik. Diese Entscheidung ersetzt das Auslesen des HOME-Klangarchivs; es wird nicht mehr automatisch ausgelesen. Unbekannte oder Shuffle-Themes verwenden weiterhin das Fallback, statt alte Cache-Musik abzuspielen.

Bei Fehlern Spielname, Modulart (3DS/DS), Startweg (CIA/3DSX/Autostart) und Ablauf notieren. Vor erneutem Launcherstart `/3ds/3ds-library/session.log` sichern, damit der letzte Startversuch erhalten bleibt.
