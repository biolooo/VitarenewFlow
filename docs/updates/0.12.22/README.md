# VitarenewFlow 0.12.22

Update in Flow: Menu > Konsole & Start > Updates > Aktualisieren > Neu oeffnen.

Six emulator packages are available in Spiele verwalten > Emulatoren. L/R changes
pages. SNES, NES and Virtual Boy use the exact adapters from the earlier tested SD.
Original upstream notices and matching modified source remain with the packages.
No games, BIOS files, keys or Nintendo firmware/music are distributed.

Audio preparation is now part of the base app. Missing DSP firmware is read from
this console's HOME code, decompressed with bounded parsing, checked against its
segment SHA-256 hashes and staged/read back on SD. Valid existing firmware remains
unchanged; invalid files are retained as backups. NAND is accessed read-only.

Clicks are synthesized by Flow and work without theme music. The unrelated HOME
music archive search was removed from audio startup. Selected theme caches,
including valid shuffle slots, are checked and delayed initialization is retried
twice. BCSTM final stereo block padding is handled correctly. Music mix remains
28 percent and clicks 85 percent. Standard Nintendo HOME music is still pending.

Local validation: native build with warnings as errors; 100 audio-worker lifecycle
cycles, with/without BGM, retries, pause/wake and error cleanup; bounded BLZ/DSP and
BGM malformed-input tests; theme extdata mocks; actual DSP storage using a private
console-owned fixture (not included); actual installer rollback and catalog tests.
These checks do not replace the fresh-console hardware test.

Test: update/reopen and verify 0.12.22; listen for clicks; verify installed theme
music without Reload Theme; switch to a basic theme and verify clicks without
stale BGM; download SNES/NES/Virtual Boy; launch, save/reopen and test return.
NES retains its known legacy HOME return behavior. Test HOME/sleep and download
cancel/retry. First local audio preparation can take a few seconds.

Own code GPL-3.0-only. Preferred source: VitarenewFlow-source.zip. Upstream adapter
licenses are not replaced by the launcher license. See ../retro-1/README.md,
../components-1/README.md and ../desktop-1/ for optional component sources/notices.
Build native app with make -C launcher and the documented devkitPro dependencies.

Format references: zoogie/DSP1, LumaTeam/Luma3DS DSP dump, and Anemone3DS themes.h.
The new bounded readers and storage integration are independently authored.
