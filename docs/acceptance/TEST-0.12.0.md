# VitarenewFlow 0.12.0 — Module und Systemsteuerung

Noch keine finale 1.0. Native Builds und Simulationen sind bestanden; echtes Kopieren/Booten und POWER benötigen diese Konsolenabnahme.

## Update

1. `/cias/VitarenewFlow-0.12.0.cia` mit FBI installieren, falls du die HOME-Menü-App nutzt. Sie enthält jetzt zusätzlich die benötigten NS-/PTM-Dienstrechte. Die 3DSX-Datei ist bereits aktualisiert.
2. Version in den Einstellungen prüfen. Boot-Dateien, Luma-Konfiguration und GodMode9.firm wurden beim SD-Update nicht ersetzt. Der Kopierassistent liegt zusätzlich unter `/gm9/scripts/VitarenewFlow-Copy.gm9`.

## A — Neustart und POWER zuerst

1. Autostart aktivieren. Nach A muss ein tatsächlicher Neustart stattfinden und VitarenewFlow erscheinen. Nicht nur ins HOME-Menü zurückfallen.
2. Autostart deaktivieren. Nach A muss ein tatsächlicher Neustart ins HOME-Menü stattfinden.
3. Autostart wieder aktivieren und nach dem Kaltstart POWER drücken. Wenn HOME bereits läuft, erscheint das normale Systemmenü. Ist HOME noch nicht geladen, erscheint stattdessen ein eigener Dialog: A ausschalten, X neu starten, Y Homebrew Launcher. Es darf keinen unendlichen schwarzen Bildschirm geben.
4. Ausschalten, Neustart, HOME-Taste sowie Zuklappen/Aufklappen prüfen. Beim direkten Boot ohne HOME kann auch HOME den eigenen Systemdialog öffnen; diese Einschränkung ist noch kein vollständiger HOME-Menü-Ersatz.
5. Falls die Systemanfrage nach fünf Sekunden zurückkehrt, Fehlermeldung und genaue Startart notieren (CIA/3DSX/Autostart). Die App protokolliert Dienstfehler, HOME-Registrierung und Übergabe.

## B — Modulerkennung und Start

1. Ein 3DS-Modul einstecken. Innerhalb einiger Sekunden erscheint unter 3DS ein Eintrag mit `[Modul]`. Name/Icon prüfen. Eine digitale Installation desselben Spiels darf daneben erhalten bleiben.
2. Modul mit A starten. Richtiger Titel und Modul-Spielstand müssen geöffnet werden.
3. Zur Library zurückkehren, Modul im Hauptbildschirm entnehmen. Der Modul-Eintrag verschwindet; andere Spiele bleiben vorhanden.
4. Mit einem DS-Modul wiederholen. DS-Kategorie, Titel/Icon und physischer Start prüfen.
5. X öffnet beim Modul die Infos. Dort X erneut drücken: Kopierassistent erscheint. B bricht ihn ab.

## C — Geführtes Kopieren mit GodMode9

Der Assistent startet nicht selbstständig ARM9-Code in unserer App. Er führt durch einen echten Neustart in den vorhandenen GodMode9-Payload. Diesen Teil bitte erst nach A und B testen.

1. Beim Modul X → X öffnen. START halten und A drücken; START während des Neustarts weiter halten. Falls Luma mehrere Payloads zeigt, GodMode9 wählen.
2. In GodMode9 HOME → Scripts → `VitarenewFlow-Copy` wählen. Das angezeigte Modul prüfen und bestätigen.
3. DS: Kopie wird zunächst als `.nds.part` geschrieben. Erst nach SHA256-Vergleich mit dem Modul wird sie zu `.nds` umbenannt. Danach neu starten, Modul entnehmen, DS-Kopie aus der Library starten.
4. 3DS: CIA wird in `/gm9/out` erstellt und von GodMode9 verifiziert. Erst dann kommt sie unter einem freien Namen nach `/cias`. Neu starten, Modul entnehmen, ausgegrauten Titel in der Library installieren und starten.
5. Eine bereits vorhandene DS-Zieldatei bzw. eine passende vorhandene CIA unter `/gm9/out` führt zum Abbruch. Vorhandene Spiele werden nicht überschrieben. Fehler und B-Abbruch dürfen keine ungeprüfte Datei als fertiges Spiel erscheinen lassen. `.part` bleibt zur Prüfung liegen.
6. Kopiertes Spiel einmal ohne Modul starten. **Modul-Spielstände werden nicht automatisch übernommen.** Für Save-Übernahme Checkpoint separat verwenden; unser Assistent schreibt keine Modul-Saves und greift nicht schreibend auf NAND zu.

GM9 zeigt eigenen Fortschritt, Speicher-/Lesefehler und Abbruchdialoge. Bei 3DS-CIAs kann erneut gebaut werden, wenn die bisherige fertige CIA schon nach `/cias` verschoben wurde; vorhandene Dateien dort bleiben durch freie Zieldateinamen erhalten. Bei fehlendem Platz wird kein Erfolg gemeldet. Keine Kartenentnahme während einer laufenden Kopie erzwingen.

## D — Regression

Ein digitales 3DS-Spiel und GBA-Spiel starten; GBA-Rückkehr L+R+Y prüfen. Sprachwechsel, Coverflow und normales Beenden testen. Beide Sprachfassungen der neuen Dialoge prüfen. Das GodMode9-Skript verwendet vorerst Englisch.

## Recovery

Bei Startproblemen SELECT beim Einschalten halten und „Hbmenu autoboot“ ausschalten. Die gesicherte `/3ds/3ds-library/autoboot-hbmenu.3dsx` bleibt erhalten. Bitte nach dem Test die SD samt Logs wieder einstecken. Bei einem Fehler notieren: Abschnitt/Schritt, Spielmodul, Startart und letzter sichtbarer Text.

Technische Quellen: [libctru PTM-Systemdienst](https://github.com/devkitPro/libctru/blob/master/libctru/source/services/ptmsysm.c), [APT-HOME-Übergabe](https://github.com/devkitPro/libctru/blob/master/libctru/source/services/apt.c), [GodMode9-Skriptbefehle](https://github.com/d0k3/GodMode9/blob/master/arm9/source/utils/scripting.c), [CIA-Erstellung](https://github.com/d0k3/GodMode9/blob/master/arm9/source/utils/gameutil.c).
