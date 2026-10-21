# Historische Releaseberichte — nicht der aktuelle Projektstand

Aufnahme 2026-10-21: Der bisherige Text bleibt vollständig erhalten. Für aktuellen Funktions-/Abnahmestand ausschließlich [STATUS](STATUS.md), für Entscheidungen [DECISIONS](DECISIONS.md), für Quellübernahme [MIGRATION](MIGRATION.md) verwenden. Alte Testgates, Standardmusik-, TWL- und Veröffentlichungsangaben sind historisch und dürfen nicht ohne Prüfung als aktuelle Anleitung ausgeführt werden.

<details>
<summary>Historische Releaseberichte (unverändert erhaltener Text)</summary>

# Release status — 0.12.6 test package

Implemented: CIA/3DSX app, branding, DE/EN app, VitaRenew panel, optional autoboot, 3DS/DS cartridge detection/start, guided GodMode9 copying, desktop game/cover management and compression, portable Windows installation/repair, five-asset UniStore generation. Own-code GPL-3.0 selected by the user. No final 1.0 is published.

Local validation: native build/CIA service ACL; actual system/card dispatch simulations; verified copying policy; Windows installer 18 real filesystem checks with complete payload; Desktop 108 checks; UniStore 15 offline checks; screenshot review of installer. Release assets are allowlisted and checksummed. No games, saves, console keys, SD boot/config files, commercial cover/banner caches or GodMode9 binary are bundled.

Still required:

1. TEST-0.12.1, including outstanding POWER/reboot/cartridge tests from 0.12.0, on New 3DS XL.
2. Windows setup on a second already-CFW SD; fresh end-user emulator dependencies verified.
3. Optional adapter pack licensing/source audit. Snes9x has separate non-commercial terms; own-code GPL does not replace those. The base installer therefore reports missing adapters explicitly.
4. Complete binary dependency review and final hardware acceptance before a stable tag/download/UniStore goes live. Universal-DB submission stays a separate human-authored step under that project's rules.

Deferred by previous scope: automatic PC emulator/save synchronization and cartridge-save migration. Copying a cartridge game does not claim that its save was migrated. Existing desktop save discovery is read-only.

## 0.12.2 local preparation

GodMode9 records a bounded versioned receipt before the first prompt and only reports DS/CIA success after verified output commit. The launcher acknowledges the receipt once, opens the matching category and reports incomplete/unconfirmed attempts in DE/EN. No automatic installation, arbitrary receipt path execution or save migration. Console acceptance remains pending; see TEST-0.12.2.

0.12.3: DS diagnosis and guided repair implemented; intent/audit journaling, fixed system-title scope, background progress, reboot/settings/update handoff and mandatory DS test confirmation. Local checks pass; hardware repair acceptance pending. See DS-REPAIR.md and acceptance/TEST-0.12.3.md.

0.12.4: active-theme BCSTM audio implemented and locally tested. Original default HOME audio is NOT yet implemented; private console source acquisition needs a real-console run before native sequence-player work continues. No end-user PC import. Theme shuffle remains pending. See MENU-MUSIC.md and acceptance/TEST-0.12.4.md.

0.12.5: owner approved original bundled fallback theme/music (no PC import or Nintendo audio). CTR cartridges now use normal APT application jumps; DS module route unchanged. Startup theme audio retries are bounded. Console HOME-return and audio acceptance remain pending. See acceptance/TEST-0.12.5.md.

## 0.12.6 supersedes the 0.12.5 fallback decision

Owner rejected the synthesized default track and requested original HOME music. The synthesized fallback is no longer linked or played. A light neutral grid replaces the circles. Original HOME audio is NOT finished; console archive access still requires diagnosis. Installed homebrew uses immediate APT application jumps; the configured HBL has an explicit original-3DSX target. The boot dispatcher selects HBL when HOME is already registered. HOME escape pauses managed autoboot and reboots normally; reopening the launcher resumes autoboot without immediate reboot. Hardware acceptance pending.

## 0.12.7 DS regression
DS title-presence checks no longer block launches. Version Data absence is not counted as a core-title failure. Pending repair rechecks core titles and acknowledges a valid journal without NAND writes; malformed markers are retained. Original default music remains unfinished. Hardware acceptance pending.

## 0.12.8 handoff regression fix

DS no longer returns from a successful application jump into ordinary aptExit cleanup. Graphics/music/workers are released first; accepted APT jumps exit the process once. If HOME is not registered (coldboot), NS_RebootToTitle launches the target with a bounded wait and recovery on failure. Physical DS cards keep reboot-to-title. Installed CTR titles share this routing. Missing DS runtime, ROM, bootstrap and handoff errors have distinct messages with codes; missing-bootstrap setup is reachable again.

HOME is visible on the first settings page. Coldboot HOME presses use managed autoboot pause plus restart, while POWER retains a dialog. Autostart toggles save without immediate reboot. Existing HOME uses its standard notification. Handoff logs persist across sessions with bounded rotation. Local mocked IPC/filesystem and ARM tests pass; hardware outcome remains unverified. Original default music is still pending.

API references: https://github.com/devkitPro/libctru/blob/master/libctru/source/services/apt.c and https://github.com/devkitPro/libctru/blob/master/libctru/source/services/ns.c ; standard HOME notification: https://github.com/LumaTeam/Luma3DS/blob/master/sysmodules/rosalina/source/menus.c .

## 0.12.9 physical CTR validation and conservative NTR startup

Hardware feedback on 0.12.8: DS handoff now reaches DS mode and HOME remains functional, but some starts intermittently show white screens; cold restart/second attempt succeeds. Digital CTR launch, CIA installation and Game Boy passed. Physical CTR card returned -2. Persistent logs show accepted NS requests; the latest official SD-30 runtime matches the SD binaries byte-for-byte and all five DS ROMs match their PC source.

Fixed a concrete card validation error: physical cards bypass installed-content metadata checks entirely after their inserted-card identity has been freshly verified. Removed/swapped cards still fail; installed-title identity and size checks remain unchanged. Card identity and requested/returned installed-title IDs, size, media and result are persisted across sessions.

For validated NTR-only retail headers without a per-game INI, create conservative NTR/67 MHz/synchronous-read defaults in the official forwarder's gamesettings directory. Preserve DSi/homebrew/custom profiles, save selection, global configuration and saves. Keep the exact game's previous patch cache as a numbered backup once; subsequent starts reuse the rebuilt cache. Stage/read back configuration writes, recover own interrupted staging, preserve unknown staging data, return a translated error on IO failure. Do not infer a broken TWL system from ROM compatibility.

The NTR profile/cache change is a hardware-unverified compatibility mitigation, not a proven root cause. Need repeated cold/warm starts with and without autoboot and physical CTR+HOME acceptance before release. No stable publication. Original standard HOME music remains pending.

Primary references: https://wiki.ds-homebrew.com/nds-bootstrap/faq.html ; https://github.com/RocketRobz/NTR_Forwarder/blob/SD-30/SD_Card/sd/arm9/source/perGameSettings.cpp ; https://github.com/RocketRobz/NTR_Forwarder/releases/tag/SD-30 .

## 0.12.10 New 3DS coldboot CTR memory mode

0.12.9 hardware: physical CTR card launches, but HOME blocks input while audio continues after autoboot. Logs show HOME absent and successful NS_RebootToTitle using hardcoded appmemtype 0. Correct native CTR reboot requests to use raw appmemtype 6 on New 3DS (0 on Old 3DS), matching Luma configureHomebrewAutobootCtr. Configuration UI index 0 remains unchanged. DS/TWL routes unchanged. Fail model-query before shutdown, persist model/query/memory type. Local launch regression matrix and native build pass; physical hardware HOME return pending. DS white-screen mitigation from 0.12.9 also awaits user results. Primary source: https://github.com/LumaTeam/Luma3DS/blob/master/arm9/source/deliver_arg.c .

## 0.12.11 HOME initialization gate before CTR handoff

Hardware disproved the 0.12.10 memory-mode hypothesis: card loads but HOME still hangs. Replace native CTR coldboot NS reboot with APT preparation, wait for enabled HOME registration (bounded 10s), then normal APT jump. No native NS fallback. On timeout/query/prepare failure keep UI and show a useful message; on successful HOME registration update Flow HOME availability even if handoff is later rejected. DS/TWL/emulator paths unchanged. Tests exercise real launch code with delayed/last-poll/no readiness, faults, card/digital/homebrew and legacy regressions. Hardware acceptance pending. References: https://github.com/azahar-emu/azahar/blob/master/src/core/hle/service/apt/applet_manager.cpp and https://github.com/devkitPro/libctru/blob/master/libctru/source/services/apt.c .

</details>
