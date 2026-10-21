# Herkunft und aktueller Arbeitsort

Seit der lokalen Migration am 21.10.2026 ist dieses Repository die verbindliche Integrationswurzel: Launcher 0.12.33, Desktop 0.4.1. Der alte Chat-/outputs-Ordner bleibt unverändert als Referenz. Keine automatische Rücksynchronisation. Der lokale Snapshot bleibt historisch und ignoriert.

Sicherung und Übernahmebelege: docs/MIGRATION.md. Aktueller Einstieg: docs/HANDOFF.md. Die folgende ursprüngliche Kopiernotiz ist historisch; ihre damalige Snapshot-/Haltgrenze beschreibt nicht den heutigen Freigabestand.

---

# VitarenewFlow: copied project folder

Snapshot created: 2026-10-21T14:16:04.262098+00:00

The original development chat continues in `C:\Users\subje\Documents\Codex\2026-10-13\kan`.
No original file, Git index, branch or remote was changed by this copy.
This folder is an independent snapshot, not a live synchronized checkout.
The root preserves the existing repository, including its uncommitted changes and origin.
`local-development-snapshot/` preserves the separate launcher, desktop and helper working copies.
The legacy launcher source matched the repository launcher source at copy time; some desktop files differed.
Do not merge these desktop variants by timestamps or overwrite either automatically.
No Flow commits or pushes were made for this migration.

Before continuing development here, explicitly coordinate the switch with the original chat,
compare new changes since this snapshot, and verify build scripts containing absolute legacy paths.
Existing toolchains, old work/ scripts and SD-card contents were not migrated.

## Changes detected after the snapshot

The final comparison already detected further changes in the original 3ds-library working copy.
This snapshot must NOT be treated as the latest authoritative development state.
Before switching, compare all legacy directories again, including newly added or removed files.

- `C:\Users\subje\Documents\Codex\2026-10-13\kan\outputs\3ds-library\3ds-library.elf`
- `C:\Users\subje\Documents\Codex\2026-10-13\kan\outputs\3ds-library\Makefile`
- `C:\Users\subje\Documents\Codex\2026-10-13\kan\outputs\3ds-library\source\app_log.c`
- `C:\Users\subje\Documents\Codex\2026-10-13\kan\outputs\3ds-library\source\app_update.c`
- `C:\Users\subje\Documents\Codex\2026-10-13\kan\outputs\3ds-library\source\components.c`
- `C:\Users\subje\Documents\Codex\2026-10-13\kan\outputs\3ds-library\source\l10n.c`
- `C:\Users\subje\Documents\Codex\2026-10-13\kan\outputs\3ds-library\source\main.c`
- `C:\Users\subje\Documents\Codex\2026-10-13\kan\outputs\3ds-library\source\ui.c`
- `C:\Users\subje\Documents\Codex\2026-10-13\kan\outputs\3ds-library\SD-Karte\3ds\3ds-library\3ds-library.3dsx`
- `C:\Users\subje\Documents\Codex\2026-10-13\kan\outputs\3ds-library\SD-Karte\3ds\3ds-library\3ds-library.smdh`
