# VitarenewFlow — Entwicklungs-Snapshot

Nintendo-3DS-Bibliothek von [VitaRenew](https://vitarenew.de) für native Spiele und angepasste Emulatoren. Diese Kopie ist **kein aktueller fertiger Release-Checkout**. Zuerst [PROJECT-COPY.md](PROJECT-COPY.md) und [AGENTS.md](AGENTS.md) lesen.

Aktueller dokumentierter Entwicklungs-/Abnahmestand: [STATUS](docs/STATUS.md). Quellen und noch nicht ausgeführter Abgleich: [MIGRATION](docs/MIGRATION.md). Einstieg für neue Chats: [HANDOFF](docs/HANDOFF.md). Ein zutreffender neuer Projekt-/Buildüberblick als separat prüfbarer README-Vorschlag steht in [PROJECT-OVERVIEW](docs/PROJECT-OVERVIEW.md).

Die vorhandene README war bereits ungesichert verändert. Ihr vollständiger Text bleibt unten als Historie erhalten; insbesondere alte Versions-, Musik-, Installer- und Universal-Updater-Aussagen sind keine aktuellen Zusagen. Keine stabile 1.0-Freigabe aus dieser Kopie ableiten.

<details>
<summary>Historische Projekt-README (unverändert erhaltener Text)</summary>

# VitarenewFlow

![VitarenewFlow](docs/assets/title.png)

A Nintendo 3DS game launcher by [VitaRenew](https://vitarenew.de).

**Development milestone 0.12.11 — Windows setup and update distribution prepared; reboot/POWER/cartridge acceptance is still pending.**

Deutsch: Eine gemeinsame Bibliothek für 3DS-Spiele, DS und Emulatoren, mit Coverflow, Originalbanner-Caches, CIA-Installation und Seed-Reparatur. Die bisherigen 3DSX-Spielstarts wurden auf einem New Nintendo 3DS XL getestet. Die neuen Neustart-/POWER-/Modul-Funktionen benötigen noch Konsolentests.

English: One library for 3DS games, DS and emulators, with coverflow, original banner caches, CIA installation and seed repair. Existing 3DSX launch routes were tested on a New Nintendo 3DS XL. New reboot/POWER/cartridge changes still need hardware acceptance.

## Current additions

- Approved VF/laurel branding and startup art without an extra splash delay.
- Persistent English/German language selection in Settings.
- Optional VitaRenew support panel under Settings → More settings.
- Native CIA package with icon and HOME banner.
- Native CIA handoff to the configured Rosalina takeover title for 3DSX emulators.

## Install / Installation

See [test installation](docs/INSTALL.md). This milestone updates an existing project SD; it does not bundle games, artwork, BIOS files, keys or emulator binaries. The emulator integrations require the project's tested adapters. A clean end-user installer follows the remaining hardware milestones.

[Next console test](docs/acceptance/TEST-0.12.11.md) · [Final acceptance checklist](docs/acceptance/FINAL-1.0.md) · [Roadmap](docs/ROADMAP.md)

## Build

- 3DS: devkitPro devkitARM, libctru, citro2d/citro3d, libpng and zlib. Run `make -C launcher` with DEVKITPRO/DEVKITARM configured.
- CIA: makerom 0.19.0 and bannertool 1.2.3 from their official repositories; see `scripts/build-cia.ps1`.
- Windows desktop: .NET Framework 4.x/WPF; run `desktop/source/build.ps1`. Compatible filename: `3DS Library PC.exe`.
- Host feature tests: `sh scripts/test-host.sh` with a C compiler and UBSan available.

Data paths remain under `/3ds/3ds-library` for compatibility.

## Notices

Original project code is licensed under GNU GPL version 3 (GPL-3.0-only); see LICENSE. Branding does not imply third-party endorsement. Third-party components retain their own licenses; see [notices](licenses/NOTICES.md). No GodMode9 implementation is included in the launcher.

## Windows setup

Portable install/repair is implemented in `setup/`. A single setup EXE embeds native files, Desktop 0.4.0 and DE/EN instructions. It supports a fresh already-CFW SD and preserves boot/config/games/saves. Emulator binaries are separate; missing adapters are reported. See [release status](docs/RELEASE-STATUS.md).

Menu music: active single-theme playback. The synthetic default track was removed at the owner's request; original Nintendo default playback remains pending, with a silent fallback. Default background uses a light neutral grid. See docs/MENU-MUSIC.md.

0.12.5: owner approved original bundled fallback theme/music (no PC import or Nintendo audio). CTR cartridges now use normal APT application jumps; DS module route unchanged. Startup theme audio retries are bounded. Console HOME-return and audio acceptance remain pending. See acceptance/TEST-0.12.5.md.

</details>
