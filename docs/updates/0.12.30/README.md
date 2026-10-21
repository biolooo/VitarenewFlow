# VitarenewFlow 0.12.30

## Theme-Musik: gültige kurze DSP-ADPCM-Endframes
- Der Decoder akzeptiert jetzt den tatsächlich gespeicherten Umfang des letzten ADPCM-Frames: ein Headerbyte plus die benötigten gepackten Samples. Ein Endframe muss nicht alle 14 Samples enthalten.
- Die bisherigen Versionen lehnten das getestete Theme trotz erfolgreichem Lesen ab: 10926 Samples benötigen 6244 Nutzdatenbytes, nicht 6248 auf volle Frames aufgerundete Bytes. Der getrennte Kanalabstand von 6272 Bytes war korrekt.
- Der Decoder liest nur die für das aktuelle Sample benötigten Bytes. Grenzen für Datei, Datenblock, Kanäle und Nutzdaten bleiben geprüft; auch ein ungepolsterter letzter Kanal direkt am Dateiende wird unterstützt.
- Der Fehler hängt bei der vorliegenden Musikdatei nicht vom Autostart ab. Die Datei wird nun auch lokal vollständig decodiert und über die Schleifengrenze geprüft.

## Lokal geprüft
Mit der privaten Originaldatei des Testgeräts (3340512 Bytes, Stereo, 24100 Hz, 2921134 Samples): vorheriger Decoder lehnt ab; korrigierter Decoder liefert sämtliche Samples und wiederholt den Schleifenanfang samplegenau. Gegenprobe mit unabhängigem FFmpeg-Decoder: gleiche Länge; nach Angleichung der Rundungsregel im separaten Testadapter byteidentische PCM-Ausgabe. Die App behält ihre bisherige Rundungsregel. Die Theme-Musik ist nicht Bestandteil dieses öffentlichen Pakets.

56 synthetische Mono-/Stereo-Fälle für jede Endframe-Länge mit/ohne Kanalpadding und einem letzten Sample direkt am Dateiende bestanden, einschließlich Ablehnung verkürzter/zu kleiner Daten. Bestehende Schleifen-, PCM/WAV-, Fehler- und 20000 Mutationstests sowie Audio-Worker/Theme-IO unter UndefinedBehaviorSanitizer bestanden. Native Build und isolierter Build aus dem veröffentlichten Quellpaket werden geprüft.

## Testliste auf der Konsole
1. Über den Update-Button auf 0.12.30 aktualisieren, anschließend ganz ausschalten und mit eingeschaltetem Autostart starten: die Musik des installierten Themes muss ohne Theme neu laden abspielen.
2. Musik über einen ganzen Schleifendurchlauf hören: keine Aussetzer/Verzerrungen am Ende.
3. Kategorie/Spiel wechseln: Klicks weiterhin hörbar, Lautstärken unverändert.
4. HOME öffnen und Flow erneut starten: Musik wieder hörbar.
5. Ohne Autostart direkt ins HOME-Menü booten und Flow öffnen: ebenfalls Musik hörbar.
6. Zuklappen/Öffnen, Spielstart und Rückkehr sowie Power -> B -> Power prüfen.

Keine Echtkonsolen-Abnahme vorweggenommen. Die HOME-Ladeanzeige aus 0.12.29 und der Power-Fix bleiben enthalten. Keine Änderung am installierten Theme, den Theme-Dateien, Luma oder den Bootdateien. Update ausschließlich über die vorhandene Updatefunktion möglich.
