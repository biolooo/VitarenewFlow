# Portable Windows setup

Setup 0.1.0 embeds launcher/CIA, boot helper, GodMode9 assistant, Desktop 0.4.0 and DE/EN instructions. It detects an existing FAT32 Luma SD; it does not install CFW or format media. App files are backed up; game/config/boot paths are excluded. Failed replacements attempt rollback. Reparse points are rejected; the volume serial is checked before operations. Final files are SHA256 verified. CIA installation still happens with FBI on console.

Build: `build.ps1 -PayloadDirectory <payload> -OutputFile <setup.exe>`. Payload manifest.json contains Path/Resource/SHA256 entries. Everything is embedded in one EXE. Requires Windows .NET Framework/WPF. Installer.cs + Tests.cs compile separately as console tests. `--preview <png>` renders read-only; `--sd I:` preselects a card.

Emulators are separate until their license/source distribution is approved. Diagnostics report file presence, not proven compatibility. Disable autoboot before updating a changed boot helper. Native 0.12.1 keeps a snapshot of the active helper so independently staged helper updates remain identifiable.
