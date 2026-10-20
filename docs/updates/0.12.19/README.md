# VitarenewFlow 0.12.19 — Universal-Updater test

Development test, not stable 1.0. Add our own UniStore in Universal-Updater: https://biolooo.github.io/VitarenewFlow/updates/VitarenewFlow.unistore

Installs Flow, a small HOME-menu shortcut, its icon, an optional autoboot helper and our GodMode9 assistant script. Does not enable autoboot or change root boot files or Luma config.ini. The shortcut uses the Luma v13.4 shared-config layout to load the current 3DSX; subsequent app updates stay in-app. Real-console fresh-install and shortcut acceptance is pending.

Emulators, games, BIOS, keys, saves and GodMode9 itself are not included. Emulator setup will be the next in-app milestone. Existing installed 3DS games and physical cartridges can be used for the first test.

All corresponding source, assets, build instructions and dependency notices are included in VitarenewFlow-source.zip. Own code GPL-3.0-only. The original Luma shared-config header retains its zlib-style notice; surrounding shortcut code is independently authored.

Test: install through Universal-Updater, open the HOME icon, check version 0.12.19, menus/languages/settings, HOME and reopening, update check, installed 3DS titles and a cartridge. No PC setup is needed for this test.
