# VitarenewFlow 0.11.2 — optionaler Autostart

0.11.1 wurde vom Nutzer vollständig auf der Konsole abgenommen. Dieser neue Schritt benötigt einen eigenen Boot-Test, bevor Spielmodule/Kopieren folgen. Noch keine finale 1.0.

## Vorbereitung

- CIA-Nutzung: `/cias/VitarenewFlow-0.11.2.cia` mit FBI installieren. Die 3DSX-Version und der Starthelfer sind schon aktualisiert.
- Autostart ist bei der Übertragung ausgeschaltet geblieben. Erst deine Bestätigung in der App aktiviert ihn.
- Unterstützter Teststand: New 3DS XL, Luma 13.4, config_version 3.13, normaler Anwendungsspeicher, installierter Rosalina/Homebrew-Titel.

## Pflichtprüfungen

1. Einstellungen → Weitere Einstellungen → Autostart. B bricht ab. Es darf keine Boot-Änderung erfolgen.
2. Autostart mit A bestätigen. Erfolgsnachricht erscheint. Konsole vollständig ausschalten und neu einschalten: VitarenewFlow öffnet sich.
3. Drei vollständige Aus-/Einschaltzyklen durchführen.
4. HOME drücken: Das originale HOME-Menü bleibt erreichbar. Schlafmodus und POWER prüfen.
5. Einschalten und B bis zum Homebrew Launcher halten. Der bisherige Homebrew Launcher öffnet sich; von dort VitarenewFlow starten.
6. In automatisch gestarteter VitarenewFlow-Version START bzw. Einstellungen → Weitere Einstellungen → Schließen wählen: Homebrew Launcher öffnet sich, keine Neustartschleife.
7. Ein 3DS-Spiel und ein GBA-Spiel aus dem automatisch gestarteten Launcher öffnen. GBA-Rückkehr per L+R+Y prüfen und danach ein weiteres Spiel starten.
8. Sprache wechseln und App neu starten: Einstellung bleibt erhalten.
9. Autostart in denselben Einstellungen deaktivieren, vollständig ausschalten und einschalten: normales HOME-Menü öffnet sich. Homebrew Launcher startet wie vorher.
10. Optional erneut aktivieren und deaktivieren, um wiederholte Verwendung zu prüfen.

## Sicherungen und Wiederherstellung

Bei Aktivierung werden angelegt:

- `/3ds/3ds-library/autoboot-hbmenu.3dsx`: bisherige `/boot.3dsx`, bleibt erhalten.
- `/3ds/3ds-library/autoboot-config.original.ini`: ursprüngliche Luma-Konfiguration, bleibt erhalten.

Aktivierung ändert nur `/boot.3dsx` und den Wert `autoboot_mode` in `/luma/config.ini`. `boot.firm`, der Rosalina-Titel, Speichermodus, Spiele und Saves werden nicht geändert. Deaktivierung stellt die bisherige `/boot.3dsx` wieder her und setzt Autostart aus; andere zwischenzeitliche Luma-Einstellungen bleiben erhalten. Nach Bestätigung vollständig neu starten; vorher keine alte Konfiguration über Rosalina speichern.

Falls der Start scheitert: Beim Einschalten SELECT halten, in Luma „Hbmenu autoboot“ auf „off“ setzen und speichern. Anschließend am PC die gesicherte `autoboot-hbmenu.3dsx` als `/boot.3dsx` zurückkopieren (vorhandene Datei vorher sichern). Falls ein Stromausfall einen Dateitausch unterbricht, auch eine vorhandene `.vrflow-old` sichern und wiederherstellen. Nicht `boot.firm` ersetzen. Bei fremd veränderter Boot-Datei verweigert die App automatisches Überschreiben.

Der normale Homebrew-Starttitel führt bei aktivem Autostart ebenfalls zu VitarenewFlow; B beim Öffnen führt zum bisherigen Homebrew Launcher. Fehlt die VitarenewFlow-3DSX, versucht der Starthelfer automatisch den gesicherten Homebrew Launcher. Diesen Fehlerfall lokal geprüft; für den Konsolentest keine App-Dateien löschen.

## Lokal geprüft

Nativer Build und CIA-Struktur; tatsächlicher Starthelfer inklusive B/fehlender App/fehlgeschlagener IPC-Übergabe; FAT-Dateitausch mit zehn Aktivierungs-/Deaktivierungszyklen; exakte Rücksicherung; simulierte Schreibfehler; fremde Boot-Dateien; Erhalt fremder Konfigurationsänderungen; tatsächliche Menüeingaben; vorhandene Sprach-/Start-/Installationsregressionen. Hardware-Ergebnis steht aus.
