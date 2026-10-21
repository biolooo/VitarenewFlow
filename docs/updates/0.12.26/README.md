# VitarenewFlow 0.12.26

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
