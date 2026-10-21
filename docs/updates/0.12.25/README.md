# VitarenewFlow 0.12.25

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
