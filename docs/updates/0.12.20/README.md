# VitarenewFlow 0.12.20 — Emulator setup test

Universal-Updater fresh installation and HOME shortcut have passed the user's real-console test in 0.12.19. This version adds optional in-app downloads for GB/GBC/GBA, Nintendo 64 and DS.

Update inside Flow: Menu > Konsole & Start > Updates > Aktualisieren > Neu öffnen.
Then Menu > Spiele verwalten > Emulatoren. Select a system using touch or Up/Down,
press Installieren / A. L/R changes the page. Each package displays its download size.
The app verifies all downloads by exact byte count and SHA-256 before replacing files.
Previous emulator files are retained as .flow-backup. Configs and saves are not replaced.
DS also installs only the verified YANBF bootstrap title 0004800546574452, not a TWL system repair.

SNES, NES and Virtual Boy downloads are not released pending redistribution-license review. Existing manually installed adapters remain usable. No ROM, BIOS, key or save files are included.

Place your own games in /roms/gba, /roms/gb, /roms/gbc, /roms/n64 or /roms/nds.
Use Spiele neu einlesen after adding games. These directories are created by setup.

Hardware checklist:
- Update from 0.12.19; reopen and verify 0.12.20.
- Download GB/GBA, N64 and DS on the fresh console; progress updates and status becomes Ready.
- Start an own game from each system. Save, reopen and check the save.
- Reinstall a package and verify existing saves/configs remain.
- Cancel a download with B or touch. Retry and complete successfully.
- Disable WLAN before starting a download; get a useful error and retry after reconnecting.
- HOME, sleep, closing/reopening and DE/EN still work.
- Do not power off or remove SD during the brief setup/DS installation step.

Local checks: native -Werror build; actual batch-commit code under UBSan with each rename failure for fresh and existing installs, corrupted staging, DS install failure and cancelled state; actual menu-input tests including touch cancel and unavailable packages; existing menu/translation checks; compiled catalog hash/size checks. These do not replace the pending hardware test.

Own code GPL-3.0-only. Corresponding source: VitarenewFlow-source.zip.
Optional emulator sources and original notices: ../components-1/README.md.
Build: make -C launcher using devkitPro/devkitARM and documented dependency packages.
Build tools are not bundled. HOME shortcut and optional autoboot helper are unchanged from 0.12.19.
