# Architektur und tatsächliche Datenflüsse

Stand: Dokumentationsaufnahme 2026-10-21. Quellkonflikte und Hashbelege: [MIGRATION](MIGRATION.md). Funktions-/Abnahmestand: [STATUS](STATUS.md). Die folgenden Pfade sind relativ zum Zielprojekt, sofern nicht ausdrücklich Legacy genannt. Aktuelle Basis 0.12.33; die neue Desktop-Erkennung ist im Ziel integriert. Historische Vergleichsbelege dokumentieren den vorherigen .32-Snapshot.

## Programm und Zustandsverwaltung

`launcher/source/main.c` scannt installierte SD-/NAND-Anwendungen, CIA-Eingang, ROM-Verzeichnisse, Homebrew und eingesteckte Karten. `library_model.*` trennt 3DS von Homebrew und fasst GB/GBC/GBA in einer Ansicht zusammen. `catalog_format.*`, `cartridge.*`, `game_info.*` liefern Identität/Metadaten. L/R wechseln Kategorien, X öffnet Spielinformationen. Kopfzeile, Coverflow, Suche und Menüseiten werden in `ui.c` gezeichnet; Touchzuordnung und Zustandsübergänge liegen überwiegend in `main.c`.

**Zentrale Konfliktdateien:** `main.c` und `ui.c` sind große, gemeinsam genutzte Integrationsstellen. Änderungen an Eingabe, APT, Komponenten und Layout treffen sich dort. Eine Aufgabe besitzt diese Dateien exklusiv; ein zweiter Agent liest oder arbeitet in einem getrennten Worktree. Module/Interfaces vor Parallelisierung festlegen.

`game_search.*` normalisiert Suchbegriffe, bewertet Treffer und baut Vorschläge. Die aktuelle Suche benutzt oben Suchtext und Ergebnisliste mit Cover, Titel und Kategorie; unten Tastatur und drei Vorschläge. Dies ist eigene lokale Rangfolge, kein Shopify-Dienst oder externes KI-Modell.

## Darstellung und Performance

`coverflow.*`, `cover_loader.*`, `ui_texture.*`, `ui_font.*` und `ui_runtime.*` koordinieren Touch/Karussell, Hintergrund-Decoding, Texturatlas und GPU-Lebensdauer. Der Cover-Worker liefert CPU-Daten; GPU-Arbeit bleibt im UI-Pfad. Cache/Anforderungsprioritäten und Logmetriken sind vorhanden, aber die Aufnahme liefert keinen aktuellen belastbaren FPS-Benchmark. Die Desktop-Kompression reduziert Bilder für den kleinen Bildschirm.

`banner_export.*`, `banner_format.*`, `banner_player.*` verarbeiten vorbereitete Originalbanner-Caches (`.lbn`). Fehlende Caches können auf Icon/Cover zurückfallen. Eine vollständige automatische Konvertierung beliebiger neu hinzugefügter 3DS-Spiele auf der Konsole ist damit nicht nachgewiesen. Originale animierte HOME-Banner sind die Nutzerentscheidung, keine 3D-Schachtelmodelle.

## 3DSX, CIA und HOME-Shortcut

Die regulär aktualisierte Anwendung ist `sdmc:/3ds/3ds-library/3ds-library.3dsx`. Ein historisch vorhandener vollständiger nativer CIA-Build wird über `scripts/build-cia.ps1` aus ELF/SMDH/Banner gebaut. Er ist nicht mit dem kleinen aktuellen HOME-Shortcut gleichzusetzen.

`launcher/forwarder/source/main.c` implementiert den HOME-Shortcut: er prüft die 3DSX, setzt `hb:ldr`-Argumente/Ziel und stellt die vorherige Rosalina-Takeover-Auswahl per Rückgabeparameter wieder her (`forwarder_restore.*`). Die UniStore-Installation liefert Shortcut-CIA, 3DSX, SMDH, Boot-Helfer und Kopierskript. Spätere normale Flow-Updates tauschen die 3DSX aus; nicht für jede Änderung eine neue native CIA installieren. `native_handoff.*` prüft beim nativen CIA-Kontext den vorhandenen Rosalina-Zieltitel. Nie ohne Kontextwechsel 3DSX-, native-CIA- und Shortcut-Tests gleichsetzen.

## APT, HOME, POWER und Autoboot

`main.c` (`prepareNativeHome`, `ensureNativeHome`, `launchTitle`, `openHomeMenu`) prüft HOME-Registrierung. Auf dem eingeführten Coldboot-Pfad wird der geprüfte regionale HOME-Prozess mit `NS_LaunchTitle` gestartet und auf Registrierung gewartet (bis etwa 10 Sekunden), bevor native APT-Übergabe vorbereitet wird. Das unterscheidet sich von den inzwischen überholten direkten NS-/Speichermodus-Hypothesen für CTR-Karten. `ui_runtime.*` und Aufräumpfade verhindern unpassende GPU-/APT-Übergaben.

`flow_apt.c` ist eine ausdrücklich veränderte Kopie des libctru-APT-Moduls mit erhaltenem Original-Lizenztext. Beim Abbrechen des POWER-Dialogs quittiert `flow_apt_cancel_power_request` den Systemzustand; bloß den lokalen Dialog zu schließen reicht nicht. Echte Systembeendigung wird nicht unterdrückt. HOME-Ladeansicht wird vor blockierenden Aufrufen dargestellt; während einer blockierenden OS-Übernahme kann nur das letzte Bild sichtbar bleiben.

`autoboot.*` verwaltet SD-seitige Konfiguration/Backups und den Boot-Helfer, nicht `boot.firm` oder eine eigene Luma-Version. `launcher/boot-dispatcher/` wählt Flow oder den erhaltenen HBL-Fallback; B ist ein Rettungsweg. Vorhandenes HOME führt zum HBL-Fallback, um eine Rücksprungschleife zu vermeiden. Managed HOME-Ausgang kann Autoboot pausieren und später wieder aufnehmen. Bootdateien, Luma-Konfiguration, Takeover-Titel und aktive Helfer-Snapshots müssen erhalten bleiben. Die Hypothese, ein neues Luma/Rosalina-Modul sei Voraussetzung, ist nicht der implementierte Startweg.

## Spielstart und Installation

- Installiertes digitales CTR-Spiel: Title ID/Medium prüfen, gegebenenfalls Seed prüfen/reparieren (`game_seed.*`), HOME vorbereiten, APT-Übergabe. Eine CIA-Datei wird nie direkt als installiertes Spiel gestartet.
- Ausstehende CIA: `cia_inspect`/`cia_prepare` liefern Identität, Größe/Speicherbedarf; UI zeigt Installationsdialog/ETA. `cia_install` registriert und prüft die Installation. Erst danach Installer entfernen (`cia_remove_installer`), Seed-Ergebnis separat melden. Abbruch-, Commit- und Aufräumphasen unterscheiden.
- Physische Karte: `cartridge_read` ermittelt aktuelle Identität; kein installiertes-SD-Titel-Preflight auf Karten anwenden. CTR- und NTR-Karten haben getrennte Übergaben.
- DS-ROM: `ds_backend.*` prüft DS-Starttitel `0004800546574452` und die nicht leeren Dateien `_nds/ntr-forwarder/sdcard.nds`, `_nds/nds-bootstrap-release.nds`; `ds_path.*`/`ds_profile.*` schreiben gezielte ROM-/Kompatibilitätsprofile. Fremde Spielprofile und Saves bewahren. Ein sporadischer White-Screen beweist keinen TWL-Systemdefekt.
- Andere ROMs/Homebrew: `rom_backend.*` validiert 3DSX/ROM, überträgt Argumente vor Ziel an `hb:ldr`. mGBA benötigt seinen SD-Archivpfad. Angepasste Emulatoren besitzen einen eigenen Rückkehr-/Save-Cleanup-Pfad; L+R+Y ist kein systemweiter Hook für native 3DS-/DS-Spiele.

## Komponenten, DS-Diagnose und Reparatur

`component_catalog.*` enthält festgelegte HTTPS-URLs, Größe, Ziel und SHA256 für sechs Emulator-Komponenten plus Desktop. Pakete: `components-1`, `retro-1`, `desktop-1`; nicht ungefragt durch Upstream-"latest" ersetzen. Angepasste Builds und Lizenztexte gehören zusammen. `components.*` lädt im Worker, prüft staged Dateien und ersetzt sie mit Backup/Rollback. Präsenzprüfung bleibt separat von vollständiger Paketprüfung. DS benutzt dieselbe Runtime-Prüfung wie der Start; fehlende Download-CIA/Begleittexte bedeuten nicht "DS fehlt".

0.12.33 ergänzt im Legacy-Code die Desktop-Erkennung über aktuelle/alte EXE und einen Prüfbutton; diese Änderungen fehlen im Ziel-Snapshot noch. Dateipräsenz ist keine Ausführbarkeitsprüfung unter Windows.

`twl_advice.*` klassifiziert unbekannt/nicht unterstützt, fehlende Kerntitel, fehlende Loader, unauffällig und bestätigten externen DS-Ausfall. Normale Diagnose liest nur. Fehlende Kerntitel → zuerst Systemupdate; fehlende Loader → Einrichtung. TWL-Fix-Löschung nur nach expliziter Bestätigung, dass sämtliche DS-Spiele **und Nintendo DS Download Play außerhalb Flow** ausfallen; dann erneute Vorprüfung und physisch START halten + A. Touch löst keine Löschung aus. Exakte Titel-Allowlist, Intent-/Auditjournal und Wiederherstellung bleiben in `twl_repair.*`; fehlerhafte/pending Marker werden nicht automatisch gelöscht oder destruktiv wiederholt. Nach begonnener Reparatur Neustart/Systemupdate und echter DS-Test. Kein NAND-Reparaturtest auf einer gesunden Konsole.

## Audio und Themes

`theme_io.*` liest aktive Theme-/HOME-Extdata einschließlich passendem Shuffle-Slot. `theme_music_plan.*` prüft Auswahl/Größen; `bgm_format.*` decodiert BCSTM/PCM und Schleifen; `menu_music.*` betreibt ndsp mit separatem Klickkanal. `audio_firmware.*` bereitet bei Bedarf Firmware lokal vor. Keine privaten DSP-/Theme-Daten veröffentlichen.

Aktuelle Musikmischung: 28 %, Klick 85 %. Klick wird selbst synthetisiert und gehört zur Basis; das ist nicht die verworfene synthetische **Menümusik**. `theme_read_music` liefert zurzeit nur Theme-Musik oder nichts; ohne gültige Musik bleiben Klicks. Originale Nintendo-Standard-HOME-Musik ist weiterhin offen. Ein altes Daten-README und ein unerreichter Fallback-Logtext sind kein Beleg für aktive Standardmusik.

## Desktop, Setup, Module kopieren

`desktop/source/` ist C#/.NET Framework/WPF: SD-Bibliothek importieren, CIAs bereitstellen, Cover auswählen/automatisch suchen/komprimieren, vorhandene Saves anzeigen. Kein freigegebener automatischer PC-Emulator-/Save-Sync. Die 0.4.1-Variante führt Einrichtung/Updates zum 3DS-Menü zurück; 0.4.0 wollte noch die lokale Setup-EXE starten.

`setup/` bleibt ein älterer optionaler Windows-Paketinstaller mit Payloadmanifest/Hashprüfung. Der bevorzugte neue Nutzerweg ist Universal-Updater → App → Emulator-/Desktop-Downloads. Der ältere Installer ist keine Voraussetzung und enthält nicht automatisch die neuesten Artefakte.

Modulkopie ist ein geführter GodMode9-Neustart mit `launcher/gm9/` und überprüfbarem Ergebnisbeleg (`copy_result.*`). Keine GodMode9-ARM9-Implementierung wird in den ARM11-Launcher gelinkt. DS-Ausgabe/CTR-CIA werden vor Erfolg geprüft; daraus folgt keine Übernahme der Modul-Spielstände.

## OTA und Distribution

`app_update.*`, `update_format.*`, `update_store.*` lesen `VFUPDATE1`, begrenzen/prüfen Version, Größe, HTTPS-URL und SHA256, stagen die 3DSX und installieren mit Rückweg. `scripts/prepare-app-update.py` erzeugt unveränderliche Versionsartefakte und verändert das lokale Manifest: kein reiner Test.

`docs/updates/<version>/` enthält neun Release-Dateien einschließlich GPL-Quell-ZIP; vier Kanaldateien sind `latest.txt`, UniStore, Info-JSON und `install.html`. `distribution/universal-updater/` ist Generator/Testmaterial, nicht zwingend der zuletzt veröffentlichte Store. Ein eigener UniStore ist nicht gleich Aufnahme in Universal-DB.

`.github/workflows/deploy-pages.yml` prüft das Manifest/Release-Hashes und veröffentlicht `docs/` via GitHub Pages. Legacy-Browser-/Hashprotokolle bestätigen die bereitgestellte Testversion .33, aber lokale Git-HEADs sind nicht mit diesem Web-Commit synchronisiert. Bei .32/.33 gab es verzögerte/überholte Pages-Aufträge; ein grüner alter Workflow allein beweist keinen aktuellen Kanal. Vor Freigabe öffentliche Bytes/Version prüfen, veraltete Jobs beachten. Keine Veröffentlichung in dieser Migration.
