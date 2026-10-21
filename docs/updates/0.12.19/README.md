# VitarenewFlow 0.12.19 — Universal-Updater test

Development test, not stable 1.0. Install using our own UniStore at https://biolooo.github.io/VitarenewFlow/updates/VitarenewFlow.unistore . The HOME shortcut starts the installed 3DSX through Luma without changing config.ini or root boot files. Future app updates replace the 3DSX. A first real-console shortcut/fresh-install test is pending.

Includes the 3DSX, icon, small CIA HOME shortcut, optional autoboot helper and independently authored GodMode9 assistant script. The installer does not enable autoboot. GodMode9 itself, Homebrew Launcher, emulators and commercial games are not bundled. The emulator download/setup assistant is not implemented yet; direct ROM launch needs the tested adapters. Own code GPL-3.0-only. Corresponding source and all project dependency notices are in VitarenewFlow-source.zip in this directory.

No PC setup is needed for this installer test. Open Universal-Updater, add the above UniStore URL in Settings > Select UniStore > + > URL, choose VitarenewFlow (Test) and Install. Close Universal-Updater and launch the Flow icon from HOME. With no 3DS titles, an empty 3DS category is expected; homebrew is in its own category.

Test: HOME shortcut opens Flow 0.12.19; menus, DE/EN, settings persistence, update check, HOME/sleep and reopening work. Try an installed 3DS title or a physical cartridge. Do not trigger DS repair just to test it. Autoboot and ROM emulators are separate later fresh-console gates. If the shortcut fails, record the displayed code; the 3DSX is also installed for HBL diagnosis.

Build native app: make -C launcher. Build shortcut: make -C launcher/forwarder, then makerom -f cia -o VitarenewFlow-Shortcut.cia -rsf launcher/branding/application.rsf -target t -elf launcher/forwarder/flow-forwarder.elf -icon launcher/3ds-library.smdh -banner launcher/branding/banner.bnr . Install devkitPro and dependency packages separately as documented by the project. Build tools are not bundled.

Luma shared-config layout is private, checked against Luma3DS v13.4. The original shared-config header is included unmodified with its zlib-style notice; the surrounding shortcut/restore implementation is independently authored. Config restoration is limited to an explicit validated shortcut argument in this running session.
