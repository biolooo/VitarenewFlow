# VitarenewFlow 0.12.1 — Installer und Release-Vorbereitung

Noch keine finale 1.0. Konsolenprüfung 0.12.0 wurde bisher nicht als bestanden gemeldet; sie bleibt Pflicht. Windows-Setup 0.1.0 und Desktop 0.4.0 sind neu.

## Vorhandene SD

1. `/cias/VitarenewFlow-0.12.1.cia` mit FBI installieren. Die 3DSX ist aktualisiert. Sprache, Covers und Saves prüfen.
2. `3DS Library PC.exe` am PC öffnen. „Installation & Reparatur“ startet `VitarenewFlow Setup.exe` und schließt die Desktop-Verwaltung.
3. Setup zeigt das korrekte Laufwerk und vorhandene/fehlende Komponenten. Deutsch/Englisch wechseln. Dateierkennung beweist noch keine Emulator-Kompatibilität.
4. Installieren / Reparieren ausführen. SHA256-Abschlussmeldung und Sicherungspfad prüfen. Boot, Luma, Games, Saves und Cover bleiben erhalten. Die danach unter `/cias/VitarenewFlow.cia` liegende CIA muss auf der Konsole mit FBI installiert werden.
5. Cover-Automatik und Spieleimport der Desktop-App prüfen.

## Zweite, bereits mit CFW eingerichtete SD (falls vorhanden)

Setup aus dem Download starten, richtige FAT32-Karte mit Nintendo-3DS-Ordner, Luma-Konfiguration und boot.firm wählen. Setup formatiert nichts und installiert keine CFW. Danach CIA mit FBI installieren und Launcher starten: vorhandene 3DS-Spiele müssen erscheinen. Fehlende Retro-Adapter werden ausdrücklich gemeldet. Das Grundpaket enthält noch keinen optionalen Emulator-Pack.

## Konsole — Pflicht aus 0.12.0

- Autostart an/aus: echter Neustart statt HOME-Rücksprung. POWER: normaler Systemdialog oder eigene Optionen A aus / X Neustart / Y Homebrew; kein Freeze.
- HOME, Schlafmodus, normales Schließen und erneuter Start.
- 3DS-/DS-Modul erkennen, entnehmen und starten. X → X öffnet den geführten GodMode9-Assistenten.
- DS kopieren und nach SHA256-Prüfung ohne Modul starten. 3DS als geprüfte CIA kopieren, installieren und ohne Modul starten. Modul-Saves werden nicht automatisch übernommen.
- Digitales 3DS-Spiel, GBA-Start und L+R+Y-Rückkehr.

Ausführliche Modul-Anleitung: TEST-12.0.md. Bei geändertem Starthelfer zunächst Autostart ausschalten. 0.12.1 speichert bei Aktivierung zusätzlich den tatsächlich installierten Starthelfer, damit spätere Updates dessen Erkennung nicht blockieren.

## Lokal geprüft / ausstehend

Installer: 18 echte Dateisystemprüfungen einschließlich vollständigem echtem Paket (Neuinstallation, Update, Hashfehler, gesperrte Datei/Rollback, Boot/Config/Save-Erhalt, Pfadgrenzen). Desktop: 108 Prüfungen. UniStore: 15 Offline-Prüfungen einschließlich Download-Abbrüchen und Zusatzdateien. Native Build, CIA-Rechte und Autostart-Dateitransaktionen mit aktualisiertem Helfer bestanden. Keine dieser Prüfungen ersetzt den Konsolenlauf.

Eigener UniStore/GitHub-Downloads werden erst nach Freigabe veröffentlicht. Unser eigener Code steht wie vom Nutzer gewählt unter GPL-3.0. Optionaler Emulator-Pack braucht vollständige Lizenz-/Quellprüfung. Das Grundpaket behauptet keine fertige Retro-Einrichtung ohne Adapter.
