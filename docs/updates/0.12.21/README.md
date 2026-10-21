# VitarenewFlow 0.12.21 — Desktop download and menu correction

Update inside Flow: Menu > Konsole & Start > Updates > Aktualisieren > Neu öffnen.

Menu > Spiele verwalten now contains four valid entries: rescan, DS diagnosis,
emulator downloads and Desktop-Verwaltung. Fixes 0.12.20's missing emulator menu
entry and mismatched row count. Regression test checks all four real actions.

Desktop-Verwaltung > Installieren downloads VitarenewFlow Desktop 0.4.1 for Windows
into the SD root, plus instructions, GPL license and corresponding source.
Power off, insert the SD into a Windows PC and open VitarenewFlow Desktop.exe.
This Windows program does not run on the console. It imports own games/CIAs,
manages covers, searches missing covers and compresses images. It does not include
PC emulators or save synchronization. Launcher setup now points to the console's
in-app update/setup menus instead of requiring a separate missing Windows setup.

All package URLs/targets/sizes/hashes are compiled allowlists. Downloads are staged
and SHA-256 checked before replacing files; prior files retained as .flow-backup.
Existing games, saves, settings and covers are not part of the download replacement.
The Desktop package does not install any NAND titles or modify boot files.

Available emulator setup remains GB/GBC/GBA, N64 and DS. DS installs only the
verified YANBF bootstrap title 0004800546574452. SNES, NES and Virtual Boy automatic
downloads remain disabled pending redistribution license review. No games, keys,
BIOS, user covers or Nintendo music are bundled. See FEATURES-DE.md for the full
fresh-install gap analysis; this is not stable 1.0 or full hardware acceptance.

Test on console:
- Update to 0.12.21 and reopen.
- All four Spiele verwalten entries are accessible; emulator screen opens.
- Desktop download completes and reports saved to SD.
- Cancel and retry; offline error and retry work.
- Re-download preserves own games, saves and covers.
- On Windows, run the EXE from SD root; import own game, find/assign its cover,
  reinsert SD into console, rescan, verify game and cover.

Local validation: native -Werror build; 108 Windows desktop checks including real
WPF button flow, import/cover safeguards and compression; actual installer/input
UBSan tests including root filenames with spaces, Desktop exclusion from NAND and
emulator-folder setup, four valid menu actions, cancel/retry/state guards and batch
rollback faults. This is not a real-console pass.

Own code GPL-3.0-only. Preferred source: VitarenewFlow-source.zip. Desktop source:
../desktop-1/VitarenewFlow-Desktop-source.zip. Emulator sources: ../components-1/README.md.
Build native app: make -C launcher with documented devkitPro dependencies.
HOME shortcut/autoboot helper unchanged from 0.12.19. The initial minimal installation
and HOME shortcut passed the user's hardware test; new downloads need acceptance.
