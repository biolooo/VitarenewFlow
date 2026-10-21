# Finale Abnahme — VitarenewFlow 1.0.0

Aktueller Einstieg für den PC-2-Backlogstand: [BACKLOG-PC2.md](BACKLOG-PC2.md). Die folgenden älteren versionsbezogenen Testgates bleiben historisch; aktuelle Reparaturgrenzen aus STATUS/DS-REPAIR beachten und keine NAND-Aktionen daraus automatisch ausführen. Kein Häkchen wurde ohne Nachweis ergänzt.

Geplante Gesamtabnahme. Diese Liste behauptet nicht, dass alle Funktionen bereits fertig sind. Erst nach den bestandenen Pflichtpunkten wird 1.0 veröffentlicht. TEST-0.11.1 wurde vom Nutzer vollständig bestanden; als Nächstes TEST-0.12.3 inklusive der offenen Konsolentests aus 0.12.0 durchführen.

## Installation

- [ ] GitHub-Seite, Downloads, Versionen und SHA256 stimmen.
- [ ] Anleitung auf frischer unterstützter SD nachvollziehbar; vorhandene Daten bleiben erhalten.
- [ ] CIA installieren, starten, aktualisieren und deinstallieren; Spiele/Saves bleiben erhalten.
- [ ] CIA und 3DSX finden dieselben Daten und Einstellungen.
- [ ] Fehlende SD/Dateien/Emulatoren und unterbrochene Installation erzeugen verständliche Meldungen.

## Gestaltung und Bedienung

- [ ] Name, VF-Icon, Lorbeerkranz und Titelgrafik korrekt; kein innerer Transparenzfehler.
- [ ] Theme-Hintergrund, Datum, Uhrzeit und Akku lesbar.
- [ ] Touch-Swipe, Steuerkreuz, L/R, X-Infos, Y-Übersicht und Öffnen funktionieren.
- [ ] 3DS/Homebrew getrennt; GB/GBC/GBA gemeinsam.
- [ ] Originalbanner stimmen zur Auswahl; fehlende Cover haben einen klaren Ersatz.
- [ ] Manuelle und automatische Desktop-Coverpflege samt Komprimierung funktionieren ohne falsche Zuordnung oder Datenverlust.
- [ ] Deutsch/Englisch vollständig und ohne abgeschnittene Texte; Sprachwahl bleibt gespeichert.
- [ ] VitaRenew-Bereich freiwillig, lesbar und ohne störende Einblendungen während der Nutzung.

## Spiele und Saves

- [ ] Mindestens zwei 3DS-Titel und ein Homebrew-Titel aus CIA und 3DSX starten.
- [ ] DS, GBA, GB, GBC, N64, SNES, NES und Virtual Boy jeweils testen.
- [ ] Direktstart öffnet richtige ROM und richtigen Spielstand.
- [ ] Rückkehr funktioniert je Adapter; abweichende Wege sind dokumentiert.
- [ ] Nach Rückkehr weiterer Spielstart; kein Freeze beim Wechsel/Ausschalten.
- [ ] CIA-Dialog: Größe, Speicher, Zeit, Fortschritt, Abbruch und Fehlermeldungen stimmen.
- [ ] CIA erst nach verifiziertem Installationserfolg gelöscht; installiertes Spiel startet.
- [ ] Seed-Prüfung/Reparatur und Offline-Start mit gültigem Seed funktionieren.

## Autostart — implementiert, Konsolentest 0.12.1 ausstehend

- [ ] In Einstellungen aktivieren/deaktivieren; betroffene Dateien nachvollziehbar gesichert.
- [ ] Vollständig ausschalten/einschalten startet VitarenewFlow zuverlässig.
- [ ] HOME und Homebrew Launcher bleiben erreichbar.
- [ ] Fehlende App/defekte Konfiguration verursacht keine unauflösbare Startschleife.
- [ ] Deaktivieren stellt vorherige Konfiguration wieder her; boot.firm unverändert.

## Spielmodule — 0.12.0 implementiert, Hardwareabnahme ausstehend

- [ ] 3DS/DS erkennen; Name/Icon/Quelle anzeigen; Einstecken/Entfernen korrekt behandeln.
- [ ] Physisches Spiel starten; digitale Installation eindeutig unterscheiden.
- [ ] Kopieren zeigt Format, Ziel, Speicherbedarf und realen Fortschritt.
- [ ] Abbruch, Platzmangel, Entnahme und Lesefehler hinterlassen keine vermeintlich fertigen Kopien.
- [ ] Kopie verifiziert und aus Library startbar; Hinweis auf separate Spielstandübernahme verständlich.
- [ ] Bestehende ROMs, CIAs, Installationen und Saves nicht überschrieben.
- [ ] Falls GodMode9/Reboot nötig: Ablauf verständlich und nach Rückkehr fortgesetzt.

## Stabilität und Veröffentlichung

- [ ] 15 Minuten Coverflow/Kategoriewechsel ohne Hänger oder wachsenden Ressourcenbedarf.
- [ ] Zehn Spielstarts/Rückkehr nacheinander; HOME, Schlafmodus und POWER reagieren.
- [ ] Performance-Logs ausgewertet; Modellgrenzen und Ladezeiten dokumentiert.
- [ ] Öffentliches Paket enthält keine Spiele, BIOS/Schlüssel, Saves oder privaten Logs.
- [ ] Quellen/Lizenzen aller ausgelieferten Komponenten vollständig; eigene Code-Lizenz GPL-3.0 festgelegt; Fremdkomponenten geprüft.
- [ ] GitHub-Seite, Anleitung, Paket und Version 1.0.0 stimmen überein.

Abnahmeprotokoll: Datum, Konsolenmodell, Luma-Version, Build, Prüfer und offene Fehler.

## Universal-Updater — vor Freigabe zusätzlich testen

- [ ] Eigener UniStore per URL/QR erreichbar; Icon/Version korrekt.
- [ ] CIA-Installation liefert auch die für Emulator-Rückkehr benötigte 3DSX/SMDH.
- [ ] 3DSX-Installation und Update auf Folgeversion funktionieren.
- [ ] Download-Abbruch/Platzmangel werden verständlich behandelt; Saves, Cover, Einstellungen und Boot-Dateien bleiben erhalten.
- [ ] Standardkatalog-Aufnahme separat beantragt, KI-Mitwirkung korrekt angegeben.

- [ ] DS repair: TEST-0.12.3 including reboot, System Update and successful DS test.
