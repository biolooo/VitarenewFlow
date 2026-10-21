# TEST 0.12.4 – Menümusik

Entwicklungsstand: Theme-Musik ist implementiert. Die automatische originale Standardmusik ist noch nicht fertig. Für Nutzer ist kein PC-Import vorgesehen; zunächst muss das originale Klangarchiv auf echter Hardware geprüft werden. Theme-Shuffle wird vorerst nicht übernommen, damit kein falscher alter Musiktitel spielt.

## Installation

SD sichern, Windows-Testpaket entpacken und Setup ausführen. Anschließend `/cias/VitarenewFlow.cia` mit FBI installieren, um die HOME-Menü-App zu aktualisieren. Beide Startwege sollten 0.12.4 zeigen.

## Theme-Wiedergabe

- [ ] Ein einzelnes Theme mit funktionierender Musik über Anemone installieren. Im echten HOME-Menü kontrollieren, dass die Musik spielt.
- [ ] VitarenewFlow starten: dieselbe Theme-Musik spielt in richtiger Geschwindigkeit, ohne Verzerrung und in Stereo, soweit die Quelldatei Stereo enthält.
- [ ] Mindestens zwei Schleifendurchläufe abwarten. Kein Knacken, falscher Startpunkt oder längere Pause am Übergang.
- [ ] Cover schnell durchwischen und Kategorien wechseln: kein Stottern, keine deutlich schlechtere Bedienung.
- [ ] Deckel schließen: Musik pausiert. Öffnen: Wiedergabe setzt fort.
- [ ] Spiel starten: Menümusik endet und überlagert das Spiel nicht. App erneut öffnen: Musik spielt wieder.
- [ ] HOME verlassen und zurückkehren sowie POWER testen: kein Freeze.
- [ ] Anderes Theme installieren; im Launcher „Theme neu laden“ wählen. Die neue Musik spielt, die alte endet vollständig.
- [ ] Mehrfach App starten/schließen; kein paralleler zweiter Musiktitel.

Bei fehlendem Ton X-Spielinfos öffnen: „Menümusik“ zeigt den Status. „DSP-Audio nicht verfügbar“ und ein entsprechender Logeintrag können auf fehlende `/3ds/dspfirm.cdc` hinweisen. Die App soll trotzdem bedienbar bleiben.

## Nötiger Entwicklungstest für originale Standardmusik

- [ ] Im echten HOME-Menü das normale Standardtheme wählen, dann den Launcher als CIA aus dem HOME-Menü starten.
- [ ] Etwa 15 Sekunden warten. In diesem Entwicklungsstand kann noch Stille herrschen; das ist noch kein fertiger Standardmusik-Player.
- [ ] App schließen und SD wieder in den PC stecken. `session.log` und gegebenenfalls `/3ds/3ds-library/music/home-source.bcsar` für die weitere Entwicklung bereithalten.

Die App liest dafür das Klangarchiv der eigenen Konsole und legt eine geprüfte private SD-Kopie an. Sie schreibt nichts in NAND, verändert keine Theme-Extdata und lädt keine Musik herunter. Die Datei wird nicht im öffentlichen Paket mitgeliefert. Dies ist eine Entwicklungsprüfung, kein geplanter PC-Vorbereitungsschritt für Nutzer.
