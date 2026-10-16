# Road to 1.0

| Requirement | Current state | Release gate |
|---|---|---|
| GitHub repository/page | Source, site and documentation prepared | Live page/downloads and clean-SD install verified |
| Installable CIA | Built; local structure and routing tests | Installation and game starts on New 3DS XL |
| Name/icon/title artwork | Approved concept C integrated; alpha corrected | HOME banner and startup art accepted |
| German/English | 155 translations and saved preference | All console dialogs checked in both languages |
| VitaRenew area | Optional settings panel | Readable in both languages |
| Optional boot into launcher | Researched; follows CIA test | Backup/restore, disable, HOME recovery and full power cycles |
| Physical games and copying | Not implemented | 3DS/DS detection/start, verified copy, cancellation, space and removal errors |

## Immediate test gate

Complete TEST-0.11.0 before boot changes and cartridge backup. Host tests cannot prove native CIA lifecycle, service permissions or handoffs.

GodMode9 runs in an ARM9 environment; its cartridge implementation cannot simply be linked into this ARM11 app. Investigate a supported ARM11 route or an explicit GodMode9-assisted flow. If rebooting is required, the user must understand the step and the copy must be verified before success is shown.

Luma supports autoboot through its homebrew takeover title. Integration must preserve the original boot.3dsx, HBL and Rosalina configuration, validate targets and provide recovery. Do not replace boot.firm.
