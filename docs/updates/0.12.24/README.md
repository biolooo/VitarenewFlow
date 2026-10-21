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
