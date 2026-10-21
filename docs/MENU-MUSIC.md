# Historische Musiknotizen — nicht der aktuelle Projektstand

Aufnahme 2026-10-21: Der bisherige Text bleibt vollständig erhalten. Für aktuellen Funktions-/Abnahmestand ausschließlich [STATUS](STATUS.md), für Entscheidungen [DECISIONS](DECISIONS.md), für Quellübernahme [MIGRATION](MIGRATION.md) verwenden. Alte Testgates, Standardmusik-, TWL- und Veröffentlichungsangaben sind historisch und dürfen nicht ohne Prüfung als aktuelle Anleitung ausgeführt werden.

<details>
<summary>Historische Musiknotizen (unverändert erhaltener Text)</summary>

# Menu music development status

0.12.4 implements active single-theme BGM playback. The source is read from the console's theme extdata only when HOME savedata establishes an active non-shuffle custom theme, the body enables BGM and ThemeManage declares a bounded music size. Basic, unknown, silent and shuffled themes never use a stale BgmCache.

The independently authored parser accepts bounded little-endian BCSTM DSP ADPCM, PCM8 and PCM16, plus private local PCM WAV cache data. The renderer produces small stereo PCM buffers and preserves decoder histories at the exact loop sample, including mid-frame ADPCM loop points. Compressed input stays bounded; no entire decoded song is allocated. The worker sleeps between queue checks, pauses on APT sleep/suspend, resumes on restore/wake, and is joined before shutdown or a game handoff. Theme reload stops the previous instance. DSP failure leaves the GUI usable and exposes an audio status in game information.

Original default HOME music is NOT implemented yet. The owner requires direct console playback with no PC import. The development build can read the running HOME title's RomFS and cache its sound archive privately for analysis. The bounded read-only search and CRC-verified SD copy do not modify NAND or theme extdata. Playback of that archive still needs implementation and hardware validation. Theme shuffle is also pending. Do not advertise either as complete.

References for format research only:

- https://github.com/astronautlevel2/Anemone3DS/blob/master/source/music.c
- https://github.com/astronautlevel2/Anemone3DS/blob/master/include/themes.h
- https://github.com/vgmstream/vgmstream/blob/master/src/meta/bcstm.c
- https://www.3dbrew.org/wiki/Home_Menu/Themes

No upstream player implementation, Nintendo music, instrument bank or commercial archive is bundled. Local checks cover actual parser/decoder vectors, exact loops, 20,000 malformed mutations under UBSan, theme-cache selection, worker lifecycle/failure mocks, translation lookup and existing launch regressions. LeakSanitizer was unavailable in the chroot; no successful LeakSanitizer run is claimed. Actual sound, volume, DSP ownership and performance still need console tests.

## 0.12.5: owner-approved built-in fallback

The owner chose an original VitaRenewFlow background and music instead of distributing or extracting Nintendo default music. Both are built in; no PC preparation, external music files or HOME RomFS mount is required. The original 16-second ambient loop is stored as approximately 594 KB of compressed PCM and played by the existing buffered player. Source acquisition is no longer invoked. Active single-theme BGM remains preferred. Startup is delayed until main initialization completes, with at most three source attempts. Theme shuffle remains unsupported and uses our fallback. Artwork uses native vector primitives and existing light UI colors.

## 0.12.6 superseding change

The owner requested original HOME music instead of our composition. Synthesized fallback playback and linkage were removed; the current standard fallback is silent. The neutral grid is HOME-inspired. Active theme music remains. Original HOME playback is not implemented; source acquisition logs the failing mount title/media/result for the next console test. Do not advertise original standard music as working.

</details>
