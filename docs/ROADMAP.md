# Historische Roadmap — nicht der aktuelle Projektstand

Aufnahme 2026-10-21: Der bisherige Text bleibt vollständig erhalten. Für aktuellen Funktions-/Abnahmestand ausschließlich [STATUS](STATUS.md), für Entscheidungen [DECISIONS](DECISIONS.md), für Quellübernahme [MIGRATION](MIGRATION.md) verwenden. Alte Testgates, Standardmusik-, TWL- und Veröffentlichungsangaben sind historisch und dürfen nicht ohne Prüfung als aktuelle Anleitung ausgeführt werden.

<details>
<summary>Historische Roadmap (unverändert erhaltener Text)</summary>

# Road to 1.0

| Requirement | Current state | Release gate |
|---|---|---|
| GitHub repository/page | Source, site and documentation prepared | Live page/downloads and clean-SD install verified |
| Installable CIA | Built; local structure and routing tests | Installation and game starts on New 3DS XL |
| Name/icon/title artwork | Approved concept C integrated; alpha corrected | HOME banner and startup art accepted |
| German/English | 0.11.1 language switching/persistence accepted on console; 167 translations in 0.11.2 | New autoboot dialogs checked in both languages |
| VitaRenew area | Optional settings panel | Readable in both languages |
| Optional boot into launcher | Implemented in 0.11.2; local FAT/rollback/dispatcher tests passed; not activated during SD transfer | TEST-0.11.2: backup/restore, disable, HOME recovery and full power cycles |
| Physical games and copying | 0.12.0: 3DS/DS detection/start and guided GodMode9 copying implemented; local simulations passed | TEST-0.12.0: actual cards, copy verification, cancellation, space/errors; save migration separate |

## Immediate test gate

The user accepted all TEST-0.11.1 checks. Complete [TEST-0.11.2](acceptance/TEST-0.11.2.md) next, before cartridge backup. Host tests cannot prove power-on, HOME recovery or boot-time service availability.

0.11.2 console feedback: disabling restored the boot files correctly but the session froze after APT suspension. 0.11.3 requests an immediate hardware restart after confirmed boot changes, with GPU/worker cleanup and reset-error handling. Complete [TEST-0.11.3](acceptance/TEST-0.11.3.md) before further cartridge work; the exact internal freeze cause is not established by the log.

0.11.3 feedback: config changes succeeded, but the reset path returned to HOME before a later manual reboot; POWER still froze. 0.12.0 uses the PTM MCU request and keeps the process alive. It checks HOME registration before libctru's unbounded screen-transfer wait and provides own power controls if HOME is absent; GPU work is drained before a valid normal HOME transfer. These changes and cartridge functions need [TEST-0.12.0](acceptance/TEST-0.12.0.md). No claim of a hardware-confirmed POWER fix yet.

Copying currently uses the existing GodMode9 payload and an independently authored script. DS files remain .part until a source/copy SHA256 match; 3DS files are built and verified as CIA before moving into /cias. Existing output matches are refused before GodMode9's builder can replace them. The app guides the reboot and then scans the finished files. Cartridge saves are not automatically migrated. This is a hardware-test milestone, not final 1.0 acceptance.

Packaging correction in 0.12.0: makerom's `-desc app:4` overrides the RSF service ACL. Remove that template option so the generated CIA actually contains PTM/NS/AM permissions; inspect both the service ACL and access descriptor in the final package, not only the RSF source.

GodMode9 runs in an ARM9 environment; its cartridge implementation cannot simply be linked into this ARM11 app. Investigate a supported ARM11 route or an explicit GodMode9-assisted flow. If rebooting is required, the user must understand the step and the copy must be verified before success is shown.

Luma supports autoboot through its homebrew takeover title. Integration must preserve the original boot.3dsx, HBL and Rosalina configuration, validate targets and provide recovery. Do not replace boot.firm.

The implementation stages a small boot dispatcher, preserves the existing Homebrew Launcher, leaves the takeover title ID and normal memory mode unchanged, and edits only autoboot_mode after verifying the boot-file swap. Holding B opens the original Homebrew Launcher. Disabling restores the original boot.3dsx while preserving unrelated changes to the live Luma config. Recovery instructions are part of the milestone checklist. Initial supported configuration: Luma 13.4 / config 3.13.

Primary reference: [Luma 13.4 configuration template](https://github.com/LumaTeam/Luma3DS/blob/v13.4/arm9/data/config_template.ini), [Luma homebrew loader](https://github.com/LumaTeam/Luma3DS/blob/v13.4/sysmodules/loader/source/hbldr.c). No Luma source is bundled in the dispatcher.

## Universal-Updater

Eigener UniStore-Generator, Release-Vertrag, VF-Icon und Offline-Tests sind lokal vorbereitet. Noch keine Store-Adresse veröffentlicht und keine Universal-DB-Anfrage abgeschickt. Siehe [Vorbereitung und Abnahme](UNIVERSAL-UPDATER.md).

## 0.12.1 release preparation

Portable Windows setup/repair and Desktop setup entry implemented. Installer: 18 real-filesystem checks including complete package. Desktop: 108 checks. UniStore now includes boot helper and copy script; 15 offline checks. Native autoboot keeps the active helper snapshot to tolerate staged helper updates. Owner selected GPL-3.0-only; LICENSE and dependency notices are included. No 1.0 publication before TEST-0.12.1 and remaining distribution checks. See RELEASE-STATUS.md.

</details>
