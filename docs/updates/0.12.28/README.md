# VitarenewFlow 0.12.28

## Theme-Musik: BCSTM-Kanalabstand korrigiert
- Der Decoder las die SEEK-Eintragsgröße als Abstand zwischen den Audiokanälen. Gültige Theme-Dateien konnten deshalb als ungültig abgelehnt werden, obwohl das Laden aus dem installierten Theme erfolgreich war.
- Nutzdatenlänge, Samplezahl und gepolsterter Kanalabstand des letzten Audioblocks werden jetzt getrennt aus den richtigen Feldern gelesen. Stereo und Schleifen bleiben erhalten.
- Bei ungültigen Dateien enthält session.log jetzt die erkannten Audioparameter. Die eingebauten Klicks bleiben auch ohne abspielbare Musik verfügbar.
- Lautstärken unverändert: Musik 28 Prozent, Klickkanal 85 Prozent.

## Testliste
1. Mit aktivem Anemone-Theme und Autoboot kalt starten: Musik ohne Theme neu laden hörbar?
2. Spiele und Kategorien wechseln: Klicks weiterhin gut hörbar?
3. Musik über einen vollständigen Schleifendurchlauf hören: keine Sprünge, Verzerrungen oder Kanalwechsel?
4. Konsole zuklappen/öffnen sowie HOME-Rückkehr testen: Musik setzt korrekt fort?
5. Ein anderes Theme mit Musik testen, danach ein Theme ohne Musik: keine Abstürze, Klicks bleiben verfügbar.
6. Den gleichen Kaltstart ohne Autoboot testen.

## Lokal geprüft
Vorheriger Decoder lehnt die neue synthetische BCSTM-Regressionsdatei ab. Korrigierter Decoder besteht Stereo-DSP-ADPCM, PCM16 mit gepolstertem letzten Kanalblock, Sample-genaue Schleifen, unabhängige SEEK-Metadaten, defekte Größen, verkürzte Dateien und 20000 Mutationen unter UndefinedBehaviorSanitizer. Audio-Worker, Theme-Auswahl und read-only Extdata-Zugriffe bestehen die bestehenden Tests. Native Build und separater Build des veröffentlichten Quellpakets werden geprüft. Echtkonsolen-Abnahme steht noch aus; die konkrete Musikdatei der Testkonsole war während dieser Arbeit nicht verfügbar.

Nur die Theme-Musik ist neu geändert. Der Power-Fix aus 0.12.27 bleibt enthalten. Kein neues Fallback-Theme und keine Nintendo-Musik im Downloadpaket.

Formatabgleich mit den unabhängigen Readern von VGAudio und Anemone3DS:
https://github.com/Thealexbarney/VGAudio/blob/master/src/VGAudio/Containers/NintendoWare/Structures/StreamInfo.cs
https://github.com/astronautlevel2/Anemone3DS/blob/master/source/music.c
