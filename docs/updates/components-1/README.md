# VitarenewFlow optional emulator components

These packages contain no games, keys, BIOS files, saves or commercial cover art.
Versioned files are immutable. The app uses a compiled allowlist of HTTPS URLs,
SD targets, exact sizes and SHA-256 hashes in packages.json.

## Game Boy / Game Boy Color / Game Boy Advance
mGBA 0.10.5, Flow adapter 3. MPL-2.0, retained LICENSE.
Upstream: https://github.com/mgba-emu/mgba/tree/0.10.5
Corresponding modified source: [gba-source.tar.gz](gba-source.tar.gz).
Source archive excludes .git and cinema regression ROM/save/image corpus, not build code.
Flow adapter implements ROM arguments and return to the launcher.
Build using devkitARM/libctru and the included 3DS build instructions.

## Nintendo 64
DaedalusX64-3DS 1.0, Flow fix 0.6.1, GPL-2.0. Retained copying.txt.
Upstream: https://github.com/masterfeizz/DaedalusX64-3DS
Modified corresponding source: [n64-source.tar.gz](n64-source.tar.gz).
Dependencies: [picaGL source](picaGL-revamp-source.tar.gz),
[imgui source](imgui-picagl-main-source.tar.gz), their original notices retained.
Optional commercial cover previews and .git are excluded; no compile source removed.
Build using included Makefile/devkitARM instructions and these dependency sources.

## Nintendo DS
NTR Forwarder SD-30, nds-bootstrap 2.17.0, YANBF bootstrap 1.6.3.
Original GPL notices included in ds-LICENSE.txt and the source archives.
Corresponding sources:
- [NTR Forwarder SD-30](ds-ntr-forwarder-source.zip), https://github.com/RocketRobz/NTR_Forwarder/tree/SD-30
- [nds-bootstrap 2.17.0](ds-bootstrap-source.zip), https://github.com/DS-Homebrew/nds-bootstrap/tree/v2.17.0
- [YANBF 1.6.3](ds-yanbf-source.zip), https://github.com/YANBForwarder/YANBF/tree/v1.6.3
Build with the included upstream Makefiles and devkitARM dependencies.
The installer verifies the fixed YANBF title ID before NAND installation. It does
not remove or repair any Nintendo TWL system titles.

SNES, NES and Virtual Boy automatic downloads remain disabled pending redistribution
license review. Existing manually installed adapters remain supported.

The nds-bootstrap source mirror excludes optional screenshots and 200th release promotional wallpaper/logo art, not build sources.
