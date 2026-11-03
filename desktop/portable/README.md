# VitarenewFlow Desktop 0.4.2 — macOS and Linux

Native Avalonia desktop port of the existing Windows management application.
The shared Card/CIA/automatic-cover engine remains in `desktop/source`; the
Windows WPF build and its delivered 0.4.1 executable are retained.

Targets: macOS 15+ Apple Silicon and Intel, Linux x64 with glibc/X11 desktop.
Applications are self-contained: users do not need to install .NET.
Extract the downloaded archive to the computer, launch the application and select
the **root of the FAT32 3DS SD volume**, not an arbitrary folder. On Linux the
read-only FAT32/volume check uses `lsblk` from util-linux. Imports and replacement
covers require an explicit action/confirmation; saves are only displayed.

Mac packages have an ad-hoc integrity signature, not Apple Developer ID signing
or notarization. macOS may require explicit approval in Privacy & Security.
Do not disable Gatekeeper globally. The Linux tar archive preserves executable
permissions; run `./VitarenewFlow.Desktop` after extraction to a local folder.

Build with .NET SDK 10.0.403, then restore the committed NuGet lock file:

```
dotnet restore desktop/portable/VitarenewFlow.Desktop.csproj --locked-mode
dotnet run --project desktop/portable/VitarenewFlow.Desktop.csproj -- --self-test REPORT.json
python3 scripts/build-portable-desktop.py --rid osx-arm64
```

Native packaging tests must run on the matching OS/architecture. The build script
runs synthetic engine tests, headless and real-backend GUI checks, then packages
into ignored `.test-build/desktop-release`. Linux headed checks need Xvfb in CI.
No tests import to a real SD card or download game content. Optional
`--probe-volume ROOT REPORT.json` and `--scan-readonly ROOT REPORT.json` report only
volume validation and aggregate discovery counts; reports must be outside the SD.

Dependencies and full notices are in packages.lock.json and NOTICES.txt.
