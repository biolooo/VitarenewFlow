# Additional optional Flow adapters

Free, separate downloads for SNES (Snes9x 3DS 1.61), NES (VirtuaNES 3DS 1.02 / Flow 0.7.0) and Virtual Boy (Red Viper 1.3.4).
These are the same adapter binaries as the existing console-tested SD installation.
No games, BIOS, firmware, saves or game covers are included.

Original upstream terms are retained, not replaced by the launcher's GPL license:
- SNES: [original notices](snes-NOTICES.txt), [modified source](snes-source.tar.gz).
- NES: [upstream readme/notices](nes-NOTICES.txt), [modified source](nes-source.tar.gz), [legacy libctru runtime source](nes-runtime-source.tar.gz).
- Virtual Boy: [Reality Boy/Red Viper notices and credits](vb-NOTICES.txt), [modified source](vb-source.tar.gz).

SNES and Reality Boy terms include non-commercial restrictions; consult the original notices for the full conditions. No new blanket license is asserted for upstream code. Sources retain in-file and third-party notices. Excluded .git metadata, prebuilt outputs/build-tool executables and ROM/save files are not build source.

Build instructions: original Makefiles in each source tree. SNES: make 3dsx USE_CUSTOM_CITRO3D=0. NES: make -f virtuanes-make CTRULIB=<separate included legacy libctru build>. Virtual Boy: make with devkitPro/devkitARM and included dependencies. Launcher return path: /3ds/3ds-library/3ds-library.3dsx; L+R+Y passes through normal save/cleanup routines. The NES legacy runtime retains its own HOME-return behavior.

Install devkitPro build tools separately; the source mirrors do not redistribute prebuilt makerom/bannertool executables.
