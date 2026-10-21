# VitarenewFlow

Nintendo-3DS-Spielbibliothek mit Coverflow, Suche, DS-/Emulator-Startwegen und Windows-Verwaltung. Entwicklung von VitaRenew. Aktuelle lokale Integrationsbasis: **Launcher 0.12.33 / Desktop 0.4.1**, weiterhin Teststand ohne abschließende Hardwareabnahme.

## Einstieg

[Aktueller Status](docs/STATUS.md) · [Orchestrator-Übergabe](docs/HANDOFF.md) · [Architektur](docs/ARCHITECTURE.md) · [Entscheidungen](docs/DECISIONS.md) · [Migration](docs/MIGRATION.md)

Dieses Repository ist der verbindliche Entwicklungsort. Legacyordner und local-development-snapshot sind historische Referenzen. Datenpfade bleiben aus Kompatibilitätsgründen unter /3ds/3ds-library. Keine Spiele, Saves, BIOS oder Schlüssel werden mitgeliefert.

## Lokal bauen und prüfen

Unter WSL mit dem vorhandenen devkitPro/chroot-SDK: `bash scripts/verify-local.sh`. FLOW_TOOLCHAIN_ROOT kann den SDK-Pfad überschreiben. Der Aufruf baut aktuelle Quellen in frischem isoliertem Staging und führt ausgewählte vorhandene Hostfixtures aus. Ergebnisse liegen ignoriert unter .test-build/. Voraussetzungen und Grenzen: [Migration](docs/MIGRATION.md), [lokale Nachweise](docs/LOCAL-VERIFICATION.md).

Direkter 3DS-Build bei korrekt gesetztem DEVKITPRO/DEVKITARM: `make -C launcher`. Benötigt libctru, citro2d/3d, libpng, curl, mbedTLS und zlib. Desktop unter Windows: `desktop/source/build.ps1` (.NET Framework 4/WPF); Ausgabe desktop/SD-Karte/3DS Library PC.exe. Native CIA benötigt makerom/bannertool, siehe scripts/build-cia.ps1. Angepasste Emulatoren haben zusätzliche separate Upstream-Buildumgebungen.

## Abnahme und Mitarbeit

Build-/Hosttesterfolg ist keine Konsolenabnahme. POWER/HOME, Audio, DS/Module und frische Einrichtung müssen separat auf Hardware geprüft werden. Offene Punkte stehen im Status. Kein automatischer Release oder SD-Transfer nach einem Build. Agenten lesen [AGENTS.md](AGENTS.md) und [Orchestrierung](docs/ORCHESTRATION.md).

Eigener Code GPL-3.0-only, siehe LICENSE und licenses/NOTICES.md. Fremdkomponenten behalten ihre jeweiligen Bedingungen. Alte README-Texte sind als historischer Beleg unter docs/migration-evidence/before-integration-README.md erhalten.
