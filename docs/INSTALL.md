# Installation — development test 0.11.0

This milestone is not the final 1.0 release. Use the existing project test SD.

1. Keep a backup of the SD. Do not change boot.firm or boot.3dsx for this test.
2. Copy the test package's `3ds/3ds-library/3ds-library.3dsx` and `.smdh` to the same SD folder. Preserve other files.
3. Optionally copy the desktop executable to the SD root.
4. Put `VitarenewFlow-0.11.0.cia` in `/cias` and install through FBI. Title ID: `000400000F104200`; product code: `CTR-H-VRFL`; title version: 176.
5. Open the new HOME-menu icon and complete [the milestone test](acceptance/TEST-0.11.0.md).

The CIA build requires Luma3DS/CFW. Emulator handoffs need the Rosalina takeover title configured in `/luma/config.ini` to be installed. The launcher validates this title and does not modify Luma settings.

## Language and support

Settings → Language selects English/German. Preference is saved at `/3ds/3ds-library/language.txt`. Rescan to refresh installed game names in the selected language.

Settings → More settings → Support the project shows the optional VitaRenew message. This panel sends no automatic web requests or telemetry.

## Pending features

Autostart and physical cartridge copying follow successful CIA acceptance. This package does not bundle emulator adapters or copyrighted game data. Clean-SD installation is not yet supported as a complete end-user flow.

## Reporting failures

Preserve `/3ds/3ds-library/session.log` and `performance.log` after a failure. Starting the launcher again replaces session.log. Report version, console model, entrypoint (CIA/3DSX), game/system and observed behavior.
