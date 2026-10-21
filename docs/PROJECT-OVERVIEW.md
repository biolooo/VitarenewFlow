# VitarenewFlow — Vorschlag für den künftigen README-Überblick

Separater Vorschlag, weil README bereits umfangreiche ungesicherte Änderungen enthielt. Der Alttext bleibt erhalten. Zustandswahrheit/Versionen nur in [STATUS](STATUS.md); kein neuer vollständiger Release wird hier behauptet.

VitarenewFlow ist ein Nintendo-3DS-Launcher von [VitaRenew](https://vitarenew.de). Eine Bibliothek für installierte 3DS-Spiele, Homebrew, DS und ROMs angepasster Emulatoren, mit Coverflow, Originalbanner-Caches, Suche, CIA-/Seed-Verwaltung und SD-Desktop-Coverpflege. [Architektur](ARCHITECTURE.md) beschreibt tatsächliche Datenflüsse und Grenzen.

## Einstieg und Distribution

Bevorzugter Nutzerweg: bereits gemoddete Konsole → eigener Universal-Updater-Store → Flow → In-App-Einrichtung von Emulatoren/Desktop und spätere OTA-Updates. Vorhandener eigener Store ist nicht gleich Universal-DB-Aufnahme. Keine CFW-Installation, Spiele, BIOS, Keys, Saves oder öffentliche Nintendo-Musik enthalten. Eine vollständige frische Einrichtung auf der Zweitkonsole und aktuelle Hardwaregates bleiben offen.

Die regulär gepflegte App ist 3DSX; HOME-Shortcut startet sie. Historischer nativer CIA-Build ist eine andere Ausführungsart und nicht für jeden normalen OTA-Schritt nötig. Kompatible SD-Daten bleiben unter `/3ds/3ds-library`; aktuelle Desktop-EXE im SD-Root, alter Name wird in .33 erkannt. Älterer Windows-Setupinstaller bleibt optionaler Legacyweg, keine Voraussetzung für die Minimalinstallation.

## Entwicklung / Build, noch nicht ausführen

Diese Kopie ist ein unabhängiger schmutziger Snapshot. Vor Implementierung [MIGRATION](MIGRATION.md) abschließen und autoritative Basis bestätigen. Keine automatische Synchronisation, Git-Sicherung oder Veröffentlichung.

Nach Freigabe braucht der Launcher devkitPro/devkitARM, libctru, citro2d/citro3d, libpng, curl, mbedTLS und zlib. Vorhandenes `make -C launcher` benötigt DEVKITPRO/DEVKITARM. CFLAGS enthalten -Wall/-Wextra/-Werror. Desktop verwendet C#/.NET Framework/WPF über `desktop/source/build.ps1`. Native CIA zusätzlich makerom/bannertool über `scripts/build-cia.ps1`. Angepasste NES-/N64-Builds besitzen separate Abhängigkeiten. Alte WSL/chroot-Skripte schreiben auf absolute Legacypfade.

Konkrete vorhandene Befehle mit Arbeitsverzeichnis, Schreibeffekten und erwartetem Nachweis stehen ausschließlich in [MIGRATION](MIGRATION.md). `scripts/test-host.sh` allein deckt die jüngsten Regressionen nicht ab. Lokale Tests/Builds sind keine Konsolenabnahme und lösen keinen Release aus.

## Mitarbeit und Lizenz

[AGENTS](../AGENTS.md), [ORCHESTRATION](ORCHESTRATION.md), optional [PLANS](../PLANS.md) lesen. Zunächst ein Implementierer und unabhängiger Prüfer auf geklärter Basis. Zentraldateien main.c/ui.c exklusiv koordinieren. Eigener Code GPL-3.0-only, Fremdhinweise erhalten; besonders verändertes libctru und Emulatorbedingungen separat prüfen. Private Spiel-/Konsolendaten gehören nicht in öffentliche Quellen.
