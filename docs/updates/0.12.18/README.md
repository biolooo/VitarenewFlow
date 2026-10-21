# VitarenewFlow 0.12.18 — in-app update test

Development test, not the stable 1.0 release. Bootstrap 0.12.17 is installed through the Homebrew Launcher; Settings > Console & startup > Updates downloads this 0.12.18 build over verified HTTPS. The two builds have the same functionality and differ only in their version identifier to test a real update.

3DSX only. Existing games, saves, covers, settings, root boot files and CIA installations are not updated. The previous binary is retained as `/3ds/3ds-library/3ds-library.3dsx.previous.3dsx`. In case of an interrupted FAT rename, restore that file to `3ds-library.3dsx` using a PC. Do not remove the SD or power off during installation. Re-downloads are checked against the published size and SHA256 before and after replacement.

Update checks are manual. No background telemetry. A valid console date and working Wi-Fi are required for HTTPS certificate checks. The first real-console online update is still pending acceptance.

Corresponding source, build files, original assets and license texts: `VitarenewFlow-source.zip` alongside this binary. Own code: GPL-3.0-only. DevkitPro dependencies are installed separately: libctru, citro2d/3d, libpng, zlib, curl 8.4.0, Mbed TLS 2.28.8. No games, saves, keys, emulator binaries or Nintendo music are included.

Build with devkitPro: `make -C launcher`. Host update tests: compile tests/update-test.c together with launcher/source/update_format.c and launcher/source/update_store.c using C11 and UBSan.
