# Historische DS-Reparaturanleitung — nicht der aktuelle Projektstand

Aufnahme 2026-10-21: Der bisherige Text bleibt vollständig erhalten. Für aktuellen Funktions-/Abnahmestand ausschließlich [STATUS](STATUS.md), für Entscheidungen [DECISIONS](DECISIONS.md), für Quellübernahme [MIGRATION](MIGRATION.md) verwenden. Alte Testgates, Standardmusik-, TWL- und Veröffentlichungsangaben sind historisch und dürfen nicht ohne Prüfung als aktuelle Anleitung ausgeführt werden.

<details>
<summary>Historische DS-Reparaturanleitung (unverändert erhaltener Text)</summary>

# DS system repair

The read-only diagnosis separates the installed YANBF loader, SD runtime files and NAND TWL title inventory. Missing titles block the DS handoff; service errors are not treated as proof of corruption. Title presence alone does not prove integrity.

Repair is an independently authored implementation of the documented TWLFix operation, not a bundled upstream executable. Only a fixed system-title allowlist is accepted. Model and region are rechecked immediately before committing intent. Development hardware and unhandled regions, including Taiwan, are rejected. No tickets, user games or saves are selected for removal.

A verified SD intent record precedes NAND changes. An append-only audit precedes each attempted operation and records its result. Errors stop further operations. Pending or malformed records block destructive retries. In-memory intent also blocks retries if the SD record disappears during the operation. This journal is not a NAND backup or rollback.

The user acknowledges the prerequisites and holds START while pressing A. Reboot and Nintendo System Update remain explicit steps. After reboot the assistant offers a regional installed System Settings handoff, useful for autoboot users. It never starts an unattended update. A fresh title check and the user's confirmation of a successful DS test are both required to finish.

Failed updates and unusual tickets require the upstream troubleshooting flow. No speculative ticket cleanup or automatic repeat is offered.

References:

- https://github.com/MechanicalDragon0687/TWLFix-CFW/blob/master/source/main.cpp
- https://github.com/MechanicalDragon0687/TWLFix-CFW/blob/master/README.md
- https://3ds.hacks.guide/troubleshooting-post-install.html
- https://www.3dbrew.org/wiki/Title_list

Local tests exercise the actual implementation with mocked console services and real filesystem journaling: exact title scope, model/region handling, missing titles, service failures, audit failure, interrupted repair, malformed records, button/touch confirmation guards, settings handoff and rechecks. They do not prove hardware behavior or Nintendo server availability.

</details>
