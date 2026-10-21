# Minimalinstallation und vollständige Testkarte – Stand 0.12.24

Die Minimalinstallation benutzt denselben Launcher-Code. Unterschiede entstehen
vor allem durch zusätzliche Dateien und Programme auf der bisherigen Testkarte.

| Bereich | Neue Konsole / Minimalinstallation |
|---|---|
| Desktop-Verwaltung | Ab 0.12.24 in der App herunterladbar. Windows-App liegt anschließend im SD-Hauptverzeichnis. Spiele importieren, CIAs bereitstellen, Cover zuordnen/automatisch suchen/verkleinern. |
| GB/GBC/GBA, N64, DS | Separat im Menü Emulatoren herunterladen und einrichten. Der Menüeintrag aus 0.12.20 ist in 0.12.24 korrigiert. Hardwareabnahme der Downloads steht aus. |
| SNES, NES, Virtual Boy | Ab 0.12.24 ebenfalls im Emulator-Menü als separate Pakete verfügbar. Ursprüngliche Lizenzbedingungen und Quellcode bleiben erhalten. |
| Eigene Spiele, Spielstände und Cover | Werden nicht mitgeliefert und nicht von der alten Karte übernommen. Cover-Suche aktuell über die Desktop-App, nicht direkt auf dem 3DS. |
| Originale animierte 3DS-Banner | Abspiel- und Exportcode vorhanden. Vorbereitete .lbn-Caches der alten Karte fehlen auf der neuen. Automatische Konvertierung auf der Konsole/über die Desktop-Verwaltung noch nicht integriert. Ohne Cache wird ein Icon/Cover verwendet. |
| Eigene Themes | Installierte HOME-Themes werden gelesen. Anemone und Themes werden nicht von unserem Installer eingerichtet. |
| Standard-HOME-Musik | Noch generell offen; Roharchiv-Vorbereitung ist kein funktionierender Musikplayer. Theme-Musik ist implementiert. |
| Audio-Voraussetzung | Ab 0.12.24 wird die DSP-Firmware bei Bedarf lokal aus dieser Konsole vorbereitet. Kein Firmware-Download. Klicks gehören zur Basis-App. |
| Spielmodule kopieren | Geführter GodMode9-Ablauf und Skript vorhanden. GodMode9 selbst muss installiert sein; bisher kein Download-/Einrichtungsassistent dafür. Spielstände der Module werden nicht automatisch übernommen. |
| Weitere Homebrew-Apps | FBI, Anemone, Checkpoint usw. werden nicht mitgeliefert. Diese sind nur für ihre jeweiligen Zusatzfunktionen erforderlich. |
| PC-Weiterspielen / Save-Synchronisation | Noch generell nicht umgesetzt, auch nicht Bestandteil der alten Desktop-Verwaltung. |

Im Launcher-Code enthalten: Spielekategorien, Touch, L/R, Infos mit X, 3DS-/Modulstart,
CIA-Installation mit Seed-Reparatur und CIA-Bereinigung, DE/EN, Updates,
DS-Diagnose und optionaler Autostart. Hardware- und Konfigurationsvoraussetzungen
bleiben bestehen; das ist noch keine vollständige 1.0-Abnahme.


# VitarenewFlow 0.12.24

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
