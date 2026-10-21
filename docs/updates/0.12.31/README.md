# VitarenewFlow 0.12.31

## Emulator-Erkennung und Erneut prüfen
- DS-Bereitschaft und DS-Spielstart nutzen jetzt dieselbe read-only Runtime-Prüfung: installierter DS-Starttitel 0004800546574452 plus _nds/ntr-forwarder/sdcard.nds und _nds/nds-bootstrap-release.nds als reguläre, nicht leere Dateien.
- Eine nicht mehr vorhandene CIA-Installationsdatei oder fehlende Download-Begleittexte lösen keine falsche Fehlt-Anzeige mehr aus. Die bisherige Erkennung verlangte sämtliche Download-Dateien, obwohl sie zum Start nicht alle nötig sind.
- Bei den anderen fünf Emulatoren zählt die tatsächlich von Flow verwendete Startdatei. Begleitdateien bestimmen die Startbereitschaft nicht. Der Installer lädt und prüft weiterhin das vollständige Paket einschließlich Lizenztexten.
- Fehlende DS-Komponenten werden konkret benannt. Ein Fehler bei der Titelabfrage wird nur nach erfolgreicher vollständiger Titellistenprüfung als nicht installiert gewertet. Sonst erscheint Unklar; eine Neuinstallation wird bei ungeklärtem Status gesperrt.
- Neuer Touch-Button Erneut prüfen, alternativ X. Er prüft alle sechs Emulatoren neu, ohne Download, Installation oder Änderungen an Spielen/Spielständen. Geprüft bestätigt den Abschluss kurz sichtbar. Beim Öffnen der Emulatorseite wird ebenfalls frisch geprüft. L/R und Installieren/Neu einrichten bleiben getrennte Buttons.
- Ergebnisse bleiben bis zur erneuten Prüfung zwischengespeichert; keine wiederholten NAND-Abfragen pro Frame. Details werden im session.log protokolliert.

## Testliste auf der Konsole
1. Über den App-Updater auf 0.12.31 aktualisieren und Flow neu öffnen. Menü -> Spiele verwalten -> Emulatoren: Nintendo DS soll auf der bereits funktionierenden Konsole Bereit anzeigen. Darunter soll DS-Starttitel und beide Loader vorhanden stehen.
2. Erneut prüfen antippen: kurz Geprüft, gleiche korrekte Ergebnisse; kein Download- oder Installationsdialog. Anschließend mit X wiederholen.
3. L/R wechseln und alle sechs Einträge prüfen. Vorhandene Flow-Emulatoren sollen Bereit anzeigen. Fehlende Startdateien sollen Fehlt mit verständlichem Hinweis anzeigen.
4. Ein DS-Spiel starten: Start und HOME-Rückkehr wie bisher. Einen vorhandenen Game-Boy-/SNES-/NES-Titel ebenfalls starten.
5. Optional auf der minimal eingerichteten Konsole: einen fehlenden Emulator installieren, danach Erneut prüfen. Der Status soll ohne App-Neustart Bereit werden.
6. Theme-Musik und Power -> B -> Power als kurze Regression prüfen.

Keine echten Loader-Dateien oder installierten Titel auf einer funktionierenden SD zum Test löschen. Fehlende/leere Dateien, Ordner statt Dateien, fehlende Starttitel, AM-Servicefehler, unvollständige Titellisten und Cache-Aktualisierung sind lokal mit synthetischen Fixtures geprüft.

## Lokal geprüft
Der tatsächliche alte Probe-Code reproduziert die falsche Fehlt-Anzeige bei vorhandener Runtime und fehlenden Download-Beigaben. Neue tatsächlich aus dem Launcher extrahierte DS-/Komponentenfunktionen bestehen Runtime-/Cache-/Fehlerfälle, 1000 gecachte Statusabfragen ohne weitere Titel-IPC, Busy-Sperren, read-only Prüfungen und den gemeinsamen DS-Start-Preflight. Die echte Eingabeverarbeitung besteht Touch/X, X+A/R ohne versehentliche Installation/Seitensprung, Button-Grenzen, L/R/A, unklaren Status, laufenden Download und Commit sowie die bestehenden Desktop-Bedienelemente. Tests laufen unter UndefinedBehaviorSanitizer. Native Build und isolierter Build aus dem veröffentlichten Quellpaket werden geprüft. DE/EN-Ansicht aus dem tatsächlichen UI-Code kontrolliert. Echtkonsolen-Abnahme steht aus.

Musik-Fix 0.12.30, HOME-Ladeanzeige und Power-Fix bleiben enthalten. Keine Änderung an DS-Loadern, Luma oder Bootdateien. Desktop-Erkennung und TWL-Diagnose werden hier nicht umgebaut. Bereit bedeutet Runtime vorhanden; es ist keine Zusage zur Kompatibilität jedes einzelnen ROMs und keine Versions-/Hashprüfung bestehender Fremdinstallationen.
