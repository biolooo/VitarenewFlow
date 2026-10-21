# VitarenewFlow 0.12.34 — Testversion

- Spiel-Startanimation samt Menüoption und künstlicher Mindestwartezeit entfernt.
- Echte Vorbereitungs-, Seed- und HOME-Anzeigen sowie Originalbanner bleiben erhalten.
- Über/Unterstützung übersichtlicher, vollständig Deutsch/Englisch.
- Updates: kurze lokale Neuerungen der installierten Version per X oder Touch ausklappen.
- Alte appearance.txt bleibt erhalten; Sprach-/Ansichtseinstellungen bleiben unabhängig.

## Auf dem 3DS testen

Nach Veröffentlichung: Menü → Konsole & Start → Updates → Aktualisieren → Neu öffnen.
Die installierte Version muss 0.12.34 anzeigen. Bei HTTPS-Problemen Datum und WLAN prüfen.
1. Anpassen: nur Sprache und Theme, kein Startanimationsschalter.
2. Spiel starten: kein Logo und keine künstliche Verzögerung; echte Vorbereitung bleibt sichtbar.
3. Über/Unterstützung und Updates in Deutsch/Englisch auf vollständigen Text und Touch prüfen.
4. X/Touch klappt die Neuerungen auf/zu; Prüfen/Aktualisieren/Zurück bleiben bedienbar.
5. Fehlender Banner nutzt weiter Icon/Cover. HOME, POWER und erneuten Spielstart prüfen.

Keine neue originale HOME-Standardmusik und kein neuer N64-Fix. N64 und Save-Sync sind vertagt.
Spiele, Saves, Cover und Boot-Konfiguration werden durch dieses 3DSX-Update nicht verändert.
Nicht während der Installation ausschalten oder die SD entfernen. Der Updater erhält die vorherige
3DSX als /3ds/3ds-library/3ds-library.3dsx.previous.3dsx für den manuellen Rückweg.

## Nachweise und Begleiter

Frischer nativer Build und Hosttests inklusive Menü/DE-EN/Touch, Bannerparser, Paketintegrität,
Komponenten, TWL-Mocks, HOME, Suche, ROM-Preflight und POWER lokal bestanden.
Kein Konsolenpass vorweggenommen. Boot-3DSX, Shortcut-CIA und GM9-Skript sind bytegleich
aus 0.12.33 übernommen; keine neue System-/Boot-/NAND-Funktion.
VitarenewFlow-source.zip enthält aktuelle Launcherquellen, Builddateien, Tests und Lizenzen.
devkitARM/3DS-Bibliotheken separat einrichten; siehe docs/PC2-SETUP.md im Quellpaket.
Launcher: GPL-3.0-only; Fremdkomponenten behalten ihre eigenen Bedingungen.
Keine Spiele, Saves, Schlüssel oder Nintendo-Musik enthalten. Kein Stable-1.0-Release.
