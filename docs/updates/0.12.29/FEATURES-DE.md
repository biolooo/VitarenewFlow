# Minimalinstallation und vollständige Testkarte – Stand 0.12.29

Die Minimalinstallation benutzt denselben Launcher-Code. Unterschiede entstehen
vor allem durch zusätzliche Dateien und Programme auf der bisherigen Testkarte.

| Bereich | Neue Konsole / Minimalinstallation |
|---|---|
| Desktop-Verwaltung | Ab 0.12.29 in der App herunterladbar. Windows-App liegt anschließend im SD-Hauptverzeichnis. Spiele importieren, CIAs bereitstellen, Cover zuordnen/automatisch suchen/verkleinern. |
| GB/GBC/GBA, N64, DS | Separat im Menü Emulatoren herunterladen und einrichten. Der Menüeintrag aus 0.12.20 ist in 0.12.29 korrigiert. Hardwareabnahme der Downloads steht aus. |
| SNES, NES, Virtual Boy | Ab 0.12.29 ebenfalls im Emulator-Menü als separate Pakete verfügbar. Ursprüngliche Lizenzbedingungen und Quellcode bleiben erhalten. |
| Eigene Spiele, Spielstände und Cover | Werden nicht mitgeliefert und nicht von der alten Karte übernommen. Cover-Suche aktuell über die Desktop-App, nicht direkt auf dem 3DS. |
| Originale animierte 3DS-Banner | Abspiel- und Exportcode vorhanden. Vorbereitete .lbn-Caches der alten Karte fehlen auf der neuen. Automatische Konvertierung auf der Konsole/über die Desktop-Verwaltung noch nicht integriert. Ohne Cache wird ein Icon/Cover verwendet. |
| Eigene Themes | Installierte HOME-Themes werden gelesen. Anemone und Themes werden nicht von unserem Installer eingerichtet. |
| Standard-HOME-Musik | Noch generell offen; Roharchiv-Vorbereitung ist kein funktionierender Musikplayer. Theme-Musik ist implementiert. |
| Audio-Voraussetzung | Ab 0.12.29 wird die DSP-Firmware bei Bedarf lokal aus dieser Konsole vorbereitet. Kein Firmware-Download. Klicks gehören zur Basis-App. |
| Spielmodule kopieren | Geführter GodMode9-Ablauf und Skript vorhanden. GodMode9 selbst muss installiert sein; bisher kein Download-/Einrichtungsassistent dafür. Spielstände der Module werden nicht automatisch übernommen. |
| Weitere Homebrew-Apps | FBI, Anemone, Checkpoint usw. werden nicht mitgeliefert. Diese sind nur für ihre jeweiligen Zusatzfunktionen erforderlich. |
| PC-Weiterspielen / Save-Synchronisation | Noch generell nicht umgesetzt, auch nicht Bestandteil der alten Desktop-Verwaltung. |

Im Launcher-Code enthalten: Spielekategorien, Touch, L/R, Infos mit X, 3DS-/Modulstart,
CIA-Installation mit Seed-Reparatur und CIA-Bereinigung, DE/EN, Updates,
DS-Diagnose und optionaler Autostart. Hardware- und Konfigurationsvoraussetzungen
bleiben bestehen; das ist noch keine vollständige 1.0-Abnahme.


# VitarenewFlow 0.12.29

## Neue Spiele-Suche
- Lupe links in der Kopfleiste öffnet eine Suche über alle Kategorien und Homebrew-Apps.
- Touch-Tastatur mit sofortigen Treffern, Zahlen/Sonderzeichen und Löschen.
- Titel, Wortteile, Wortreihenfolge, zusammengezogene Wörter, Akzente und Tippfehler werden berücksichtigt. Exakte Treffer zuerst. Alle Suchwörter müssen passen.
- System-Abkürzungen kombinierbar, z.B. `mario gba` oder `zelda n64`.
- Oben Vorschau; unten drei Treffer pro Seite mit System und Startpfeil. Steuerkreuz hoch/runter wechselt die Auswahl, A oder rechter Startpfeil startet. Antippen eines Namens wählt nur aus.
- B oder Schließen kehrt zur vorherigen Kategorie/Auswahl zurück. X löscht einen Buchstaben, Y leert die Suche. Bis 32 Zeichen. Suche läuft offline nur bei Eingaben, ohne Dateien zu durchsuchen oder herunterzuladen.
- Keine Suche nach freien Beschreibungen oder unbekannten Alternativtiteln ohne entsprechende Metadaten.

## Prüfung
Native Build, separater Quellpaket-Build, Relevanz-/Tippfehler-/Unicode-Tests, 20.000 Zufallseingaben, Tastatur-Grenzen und tatsächliche Katalog-/Input-Integration lokal geprüft. Host-Vorschau mit angenäherter Schrift. Echtkonsolen-Abnahme steht aus.

## Testliste auf dem 3DS
- Lupe öffnet die Suche. Touch-Buchstaben aktualisieren die Treffer sofort.
- `mario`, `mraio`, `kart mario`, `pokemon`, `pokmeon` testen, sofern passende Titel vorhanden sind.
- Treffer aus verschiedenen Kategorien, `mario gba`, ein langes Spiel und mehr als drei Treffer prüfen.
- Hoch/runter und Touch wählen aus; obere Vorschau stimmt. A und Startpfeil starten das ausgewählte Spiel.
- Zahlen, Leerzeichen, Löschen, X/Y und leere Trefferliste prüfen.
- B/Schließen stellt vorherige Kategorie/Auswahl wieder her.
- Deutsch/Englisch, Spielinstallation/Emulator-Einrichtung aus Suchtreffern und HOME-Rückkehr prüfen.


# VitarenewFlow 0.12.29

## Suche auf beiden Bildschirmen
- Oben: Suchtext und drei Treffer mit Cover links, Spielname rechts und kleiner Kategorie darunter. Weitere Treffer per Steuerkreuz oder Touch-Pfeilen unten.
- Unten: größere, versetzte QWERTZ-Tastatur mit Zahlen/Sonderzeichen, Leerzeichen und Löschen.
- Drei Wortvorschläge aus passenden installierten Spieltiteln. Antippen ersetzt das gerade getippte Wort und setzt ein Leerzeichen; danach werden passende Folgewörter angeboten. Es erscheinen nur verfügbare Vorschläge, keine erfundenen Titel.
- A / Öffnen startet den markierten Treffer. B / Zurück stellt die vorherige Kategorie und Auswahl wieder her. X löscht einen Buchstaben, Y leert das Suchfeld.
- Tippfehlertoleranz und Offline-Suche aus 0.12.24 bleiben erhalten. Keine animierten Banner während der Suchliste; Cover werden über den bestehenden Hintergrundlader bereitgestellt.

## Test auf der Konsole
- Lupe öffnen: Suchtext und Treffer oben, Tastatur unten.
- Einen vorhandenen Titel tippen; Treffer zeigen korrektes Cover, Name und Kategorie.
- `mario` oder einen anderen vorhandenen Titel eingeben und einen Wortvorschlag antippen: das Wort wird ergänzt, ohne vorhandene Suchwörter zu löschen.
- Nach einem Leerzeichen Folgewörter prüfen. Bei wenigen Treffern bleiben ungenutzte Vorschlagsfelder leer.
- Mehr als drei Treffer durchgehen: Steuerkreuz und Touch-Pfeile bewegen die Markierung und blättern die Liste.
- A / Öffnen startet genau den markierten Titel; keine Touch-Auswahl auf dem oberen Bildschirm nötig.
- Zahlen, Löschen, X/Y, keine Treffer und B / Zurück testen.
- Deutsch/Englisch und normalen Kategorie-Spielstart prüfen.

## Lokal geprüft
Native Build und separater Quellpaket-Build; Relevanz-/Unicode-/Tippfehler-/Grenztests, 20.000 Zufallseingaben und aus dem echten Code extrahierte Katalog-/Bedienungslogik einschließlich Wortvorschlägen. Zwei-Bildschirm-Vorschau mit angenäherter PC-Schrift. Echtkonsolen-Abnahme steht aus.
Der separat gemeldete N64-Laufzeitfehler ist mit diesem UI-Update nicht behoben; die Logs der betroffenen Konsole fehlen noch.


# VitarenewFlow 0.12.29

## Power-Fenster
Vollflächiger Dialog im Stil des Menüs mit klaren Aktionszeilen: HOME-Menü (A), Neustart (X), Homebrew Launcher (Y) und Ausschalten (SELECT). Alle Zeilen sind per Touch bedienbar. Ausschalten ist farblich abgesetzt.

B oder der große Abbrechen-Button schließt den Dialog und lässt die vorherige Ansicht geöffnet. Es wird dabei weder die App verlassen noch ein Neustart oder Ausschalten angefordert. B hat Vorrang vor gleichzeitig gedrückten Aktionstasten.

Die noch offene HOME-/POWER-Anfrage im app-internen APT-Zustand wird beim Abbrechen ebenfalls zurückgesetzt, damit der Dialog nicht sofort wieder aufgeht. Eine neue Anfrage kann ihn erneut öffnen. Bereits angeordnete echte Systembeendigungen werden nicht aufgehoben.

## Umsetzung und Herkunft
Die App enthält die bisher unveränderte, öffentlich verfügbare APT-Implementierung aus devkitPro/libctru mit einer ausdrücklich markierten lokalen Cancel-Erweiterung. Original-Lizenz, Herkunft und Quellhash stehen in launcher/source/flow_apt.c. Der Originalteil wurde bytegleich mit dem zuvor verwendeten Quellstand verglichen. Keine Änderung an Luma, boot.firm oder der SD-Boot-Konfiguration.

## Testliste
- Power drücken: neues Fenster erscheint.
- B drücken: direkt zurück in die vorherige Ansicht, ohne Neustart oder App-Ende. Mindestens 10 Sekunden weiter bedienen.
- Power erneut drücken und den Touch-Button Abbrechen testen. Mehrfach wiederholen.
- Aus normaler Spieleliste, Suche und Einstellungen abbrechen: Ansicht und Auswahl bleiben erhalten.
- Touch-Pfeile/Spielewahl und Spielstart nach dem Abbrechen funktionieren.
- HOME-Menü und Neustart separat prüfen; anschließend Ausschalten testen.
- Je einen Durchlauf mit und ohne Autostart sowie Deutsch/Englisch prüfen.

## Prüfung
Native Build und isolierter Quellpaket-Build. Tatsächliche Cancel-Funktion, Hauptschleife und Dialog-Bedienung lokal mit simulierten APT-Diensten geprüft: latched request, B/Touch, 100 Folgeframes ohne Wiederöffnung, neue Power-Anfrage, Vorrang von Abbrechen, alle Aktionszeilen und Erhaltung echter Close-Anfragen. Die Hardware-Abnahme steht aus.

Der getrennt gemeldete N64-Laufzeitfehler bleibt bis zur Auswertung der betroffenen SD-Logs offen.


# VitarenewFlow 0.12.29

## Power-Menü nach Abbrechen erneut öffnen
- Beim Abbrechen wird jetzt auch der Power-Zustand im System quittiert (APT Utility 0 / ClearPowerButtonState), bevor der lokale Dialogzustand gelöscht wird. In 0.12.26 fehlte diese Systemquittierung; ein weiterer Power-Druck konnte deshalb ausbleiben.
- B und Touch-Abbrechen bleiben erhalten. Bei einer fehlgeschlagenen Systemantwort bleibt der Dialog offen und zeigt am Abbrechen-Button eine erneute Versuchsmöglichkeit. Der Rückgabecode wird im session.log protokolliert.
- Tatsächliche Beendigungsanforderungen des Systems werden nicht aufgehoben.

Nur Punkt 10 der zuletzt besprochenen Aufgabenliste ist in diesem Update geändert. Theme-Musik, HOME-Ladeanzeige, Startanimation, DS-Diagnose, Komponenten-Erkennung, Desktop-Manager, HOME-Shortcut, Support-Seite und Update-Neuerungen folgen separat.

## Testliste auf der Konsole
1. Mit aktiviertem Autostart starten, Power kurz drücken und B drücken.
2. Power erneut drücken: das Fenster soll wieder erscheinen. Dies fünfmal wiederholen.
3. Den gleichen Ablauf mit dem Touch-Button Abbrechen testen.
4. Aus Suche und Einstellungen öffnen und abbrechen: vorherige Ansicht und Auswahl bleiben erhalten.
5. Nach dem Abbrechen ein Spiel starten und HOME-Rückkehr prüfen.
6. Ausschalten und Neustart einzeln testen; anschließend den Ablauf ohne Autostart wiederholen.

## Lokal geprüft
Der echte bisherige Cancel-Helper aus 0.12.26 reproduziert in einem Modell des systemseitigen Power-Latches den verlorenen zweiten Tastendruck. Die neue tatsächliche Cancel-Funktion, Hauptschleife und Eingabeverarbeitung bestehen 20 OS-abhängige Wiederholungen mit 2000 Folgeframes, Touch, Fehlerrückgabe/erneutem Versuch, Vorrang von B, echten Close-Anforderungen und normaler HOME-Übergabe. Native Build und separater Quellpaket-Build erfolgreich. Keine Echtkonsolen-Abnahme vorweggenommen.

APT Utility 0 ist als ClearPowerButtonState dokumentiert: https://docs.mikage.app/APTAppletUtility/ (Spiegel der 3dbrew-Dokumentation). Die Original-Lizenz von libctru steht weiterhin in launcher/source/flow_apt.c. Keine Änderung an Luma oder der Boot-Konfiguration; Update über die .3dsx-Updatefunktion.


# VitarenewFlow 0.12.29

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
