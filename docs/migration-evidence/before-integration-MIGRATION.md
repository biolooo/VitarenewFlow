# Migration — Vorschlag, noch nicht ausgeführt

Ziel: `C:/Users/subje/Documents/Code/vitarenew-flow`.
Legacywurzel: `C:/Users/subje/Documents/Codex/2026-10-13/kan`.
Aufnahme 2026-10-21. Dokumentation erlaubt keinen Quellabgleich, Build, Git-Sicherung oder Release. Regeln [AGENTS](../AGENTS.md), Ablauf [ORCHESTRATION](ORCHESTRATION.md).

## Quellen und Konflikte

Die SHA256-Berichte in [migration-evidence](migration-evidence/) unterscheiden Inhalt, neue/fehlende Dateien und geschützten Ausgangsstand. Keine Auswahl anhand von Änderungsdatum.

| Quelle → Ziel | Befund | Empfohlene Variante, NICHT übernommen |
|---|---|---|
| Legacy outputs/3ds-library/source → Ziel launcher/source | 100 C/H, 94 gleich, sechs geändert: app_log.c, app_update.c, components.c, l10n.c, main.c, ui.c. Keine neuen/entfernten C/H. | Legacy .33 nach Einzelreview; kein komplettes Überschreiben |
| Legacy outputs/3ds-library/Makefile → Ziel launcher/Makefile | .33 gegen .32, Hash in versions.json | Zum selben Quellstand gehörendes Makefile, Versionsstrings gemeinsam prüfen |
| Legacy Repository launcher/source → Legacy Launcher source | 100/100 gleich, auch zum .33-Quell-ZIP | Bestätigte zweite Source-Kopie; lokaler Git-HEAD f8851da bleibt alter Stand |
| Legacy Launcher → Ziel local-development-snapshot/3ds-library/source | Dieselben sechs Differenzen wie Hauptziel | Historischer Snapshot, keine neueste Quelle |
| Legacy Repository desktop/source → Ziel desktop/source | 12/12 gleich, .4.1 | Behalten, entspricht veröffentlichtem desktop-1-Quellpaket |
| Legacy outputs/3ds-library-pc/source → Ziel desktop/source | Vier Differenzen: AssemblyInfo.cs, MainWindow.xaml, Program.cs, Tests.cs; .4.0 vs .4.1 | Nicht über Ziel kopieren; unabhängige Änderungen separat reviewen |
| Legacy Desktop → Ziel local-development-snapshot/3ds-library-pc/source | 12/12 gleich, .4.0 | Getrennte historische Referenz |
| Legacy Repository setup/scripts/launcher-tests/.github → Ziel | Je 5/5, 5/5, 58/58, 1/1 gleich | Erhalten, Alter/Abdeckung prüfen; Host-Tests nicht vollständig für .27–.33 |
| Legacy Launcher branding/data → Ziel launcher/branding/data | Sechs Branding- und zwei Datendateien gleich; zusätzliches Ziel-README | Keine Übernahme nötig. Daten-README beschreibt verworfenen Fallback, nicht aktiven Musikplayer |
| Legacy Repository docs/updates → Ziel | 173 gleich, vier Kanäle anders; neun Dateien unter .33 fehlen im Ziel; keine Ziel-exklusiven Release-Dateien | Später komplettes unveränderliches .33-Paket + zusammengehörige Kanäle; keine Veröffentlichung implizieren |
| SD-Staging/ELF/andere Binärkopien | Erzeugte Outputs außerhalb normaler Quellen | Getrennt behandeln, frisch aus freigegebener Quelle bauen und Hash zuordnen |

Desktop-Unterschied: .4.0 startet die lokale Setup-EXE; .4.1 verweist auf Einrichtung/Downloads/Updates auf der Konsole. Letzteres entspricht dem späteren Nutzerwunsch. [desktop-archive-comparison.json](migration-evidence/desktop-archive-comparison.json) belegt die vier .4.1-Dateien im öffentlichen Quellpaket.

Weitere tatsächliche Quellen: outputs/emulator-adapters enthält modifizierte Upstream-Archive, separates Legacy-NES-SDK, N64-picaGL/ImGui, BUILD-MANIFEST und Hinweise. outputs/mgba-library, outputs/ds-runtime, outputs/banner-tools, outputs/home-start-test sowie work/mgba-build, work/emulators/mgba, work/ds enthalten spezialisierte Quellen/Hilfen. outputs/emulator-testpaket/SD-Staging sind keine pauschale Source-of-truth. Die fünf Ziel-Snapshotordner (Launcher, Desktop, Banner-Tools, HOME-Test, mGBA) ersetzen weder vollständige Adapterquellen noch Toolchain. SD-Backups/private Fixtures sind ausdrücklich kein Migrationsmaterial.

Weitere Hashkontrolle: Forwarder 4/4, Boot-Dispatcher 2/2, GM9-Skript 1/1 und Lizenzdateien 12/12 zwischen Repository-Staging und Ziel identisch. Distribution: vier gemeinsame Dateien identisch; zusätzlich nur ein generierter Python-Bytecodecache im Legacy-Repository (`universal-updater/__pycache__/prepare_store.cpython-312.pyc`), nicht übernehmen. Die genannten WSL-SDK-Verzeichnisse wurden lesend als vorhanden bestätigt.

Zusätzlicher Snapshot-Abgleich: mGBA 8/8, Banner-Tools 551/551 und HOME-Starttest 9/9 Dateien jeweils hashgleich zur entsprechenden alten Ausgabe. Diese Kopien enthalten auch erzeugte/gebündelte Fremdartefakte; Gleichheit macht sie nicht zu einer neuen autoritativen Quellbasis.

## Vorgeschlagene verbindliche Basis

**Noch zu bestätigen:** Ziel als künftige einzige Integrationswurzel. Launcher nach Review aus konsistentem Legacy .33 ergänzen; Desktop Ziel/Repo .4.1 erhalten. Setup, Emulatoradapter, Boot-Helfer und Shortcut separat zuordnen. Bestehende ungesicherte Zieländerungen zuerst inventarisieren/sichern. Weder main/f8851da allein noch die gesamte alte outputs-Struktur ist eine geeignete pauschale Basis.

PROJECT-COPY bleibt erhalten. AGENTS.md/PROJECT-COPY.md werden durch .gitignore Regeln 18/17 ignoriert, Snapshot durch Regel 16. `.gitignore` jetzt unverändert; später Regeln gezielt einschließen statt pauschal entfernen. Vor Git-Sicherung Autorisierung, Datenumfang und Urheberschaft der Änderungen klären.

## Noch auszuführende Schritte / Rückweg

1. Neuer Orchestrator reproduziert den lesenden Akzeptanzauftrag aus HANDOFF, meldet Änderungen seit der Aufnahme.
2. Koordinator/Nutzer bestätigt Basis und separaten Sicherungs-/Integrationsauftrag. Kein Featurestart vorher.
3. Gewünschten schmutzigen Stand inklusive ausgewählter untracked Quellen/Docs autorisiert sichern, private Inhalte ausschließen; überprüfte Ausgangsbasis schaffen. Worktrees aus altem HEAD enthalten uncommittete Implementierungen nicht automatisch.
4. Ein Implementierer gleicht sechs C-Dateien und Makefile ab; Desktop .4.1 erhalten. Aktuelle Tests aus .33-Quellarchiv nachvollziehbar portieren. Zusätzliche fremde Zieländerungen separat behandeln.
5. Jede Übernahme mit Quelle/Ziel/Vorher-/Nachher-Hash/Begründung protokollieren. Keine großflächige Synchronisation, Snapshot nicht überschreiben. Assets, Lizenzen, Helfer/Adapter getrennt behandeln.
6. Skripte auf neue Wurzel portieren, frisches Staging nutzen. Keine historischen Patch-/Releasegeneratoren blind ausführen. Erst dann lokal bauen und gezielt testen.
7. Unabhängiger lesender Review von Diff/Tests/Lizenzen/Artefakten; Orchestrator integriert erst nach Freigabe. Keine automatische Veröffentlichung.
8. Hardwarematrix aus STATUS abnehmen. Eigener Releaseauftrag erst danach; späte alte Pages-Jobs und aktuellen öffentlichen Kanal prüfen.

Rückweg: gesicherte Ausgangsbasis, isolierter Worktree und Hashprotokoll; keine Nutzerdateien per Reset überschreiben. Öffentliches .32/.33 unveränderlich lassen. Quell-/Desktop-Rückweg ist kein Boot-/NAND-Rollback; solche Änderungen brauchen eigenen Auftrag und Wiederherstellung.

## Werkzeuge / absolute Altpfade

Windows PowerShell, Python 3, Git; WSL Ubuntu mit root/chroot. Vorhandener Root `/home/maxim/codex-3ds-work/toolchain` mit `/opt/devkitpro` und devkitARM/3DS-Portlibs. work/toolchain/manifest.json/layers.json identifizieren OCI-Material, nicht automatisch eine neue Installation. SDK-Inhalt wurde hier nicht installiert oder verändert.

work/toolchain/build.sh schreibt nach chroot/project UND in die Legacywurzel zurück (.3dsx/.smdh/ELF). SOURCE ist absolut auf den alten Ordner gesetzt. Alte work/*/source-build.sh, source-tests.sh, run.sh, preview.sh nutzen `/mnt/c/Users/subje/Documents/Codex/2026-10-13/kan`. Details mit Zeilen: [absolute-paths.json](migration-evidence/absolute-paths.json). Bestehende SDK-Projektkopie entfernt keine alten Quelldateien; frisches Staging bevorzugen.

prepare.py, prepare-tests.py, package.py, package-prepare.py, refresh-draft.py sind nicht generell read-only. .30/prepare-tests.py patcht sogar Versionsstrings und andere Testdateien; prepare-app-update.py verändert latest.txt, prepare-setup.py erzeugt Payload. Ältere .32-Package-Helfer hatten zu korrigierende Versionspfade. Ausgang sind geprüfte Quellen/Quell-ZIP und konkret validierte Befehle, kein blind neu laufender Generator.

Desktop: `%WINDIR%/Microsoft.NET/Framework64/v4.0.30319/csc.exe` plus WPF. Native Make: libctru, citro2d/citro3d, libpng, curl, mbedTLS, zlib. Native CIA zusätzlich makerom/bannertool. Angepasster NES braucht separates altes SDK; N64 picaGL-revamp/ImGui, nicht den überholten frühen picaGL-master-Weg.

GitHub bisher per Nutzerbrowser; keine CLI-Credentials voraussetzen/auslesen. Repository https://github.com/biolooo/VitarenewFlow ; Pages https://biolooo.github.io/VitarenewFlow/ . Quell-ZIP-Veröffentlichung ist nicht gleich versionierter vollständiger Arbeitsbaum.

## Vorhandene spätere Prüf-Befehle

**In dieser Phase nicht ausgeführt. Alle folgenden Builds/Tests schreiben Dateien.** Erst freigegebene Basis/Scratchumgebung benutzen. Hosttests ersetzen Hardware nicht. Noch nicht portierte neue Skripte nicht als existent ausgeben.

| Arbeitsverzeichnis | Befehl | Erwarteter Nachweis / Grenze |
|---|---|---|
| Legacywurzel | `wsl -d Ubuntu -u root -- bash work/toolchain/build.sh` | GCC -Werror, .3dsx/.smdh/ELF und Hash im alten Staging |
| Legacywurzel | `wsl -d Ubuntu -u root -- bash work/desktop-v01233/source-build.sh` | Frischer Build aus genau .33-Quell-ZIP, PASS clean native build |
| Legacywurzel | `wsl -d Ubuntu -u root -- bash work/desktop-v01233/source-tests.sh` | .33-ZIP tests/component-runtime: tatsächliche Probe/Eingabe, Desktop/DS, UBSan, PASS |
| Legacywurzel | `wsl -d Ubuntu -u root -- bash work/twl-v01232/source-tests.sh` | .32-ZIP tests/twl, 128 Entscheidungen/Consent/Whitelist/IO, keine echte NAND-Löschung; später auf Zielquelle beziehen |
| Legacywurzel | `wsl -d Ubuntu -u root -- bash work/power-v01227/test.sh` | Utility-0-Rearm-Hostfixture, kein realer Powerknopf |
| Legacywurzel | `wsl -d Ubuntu -u root -- bash work/home-loading-v01229/test.sh` und `wsl -d Ubuntu -u root -- bash work/home-loading-v01229/runtime.sh` | HOME-Anzeige/Runtime-Fixtures, keine echte OS-Abnahme |
| Legacywurzel | `wsl -d Ubuntu -u root -- bash work/audio-v01222/test.sh` | Audiofixtures, vorher genaue aktuelle Quellen/Tests prüfen |
| Neue Wurzel, nach Basisentscheidung und SDK | `make -C launcher` | Ziel-3ds-library.3dsx/.smdh/.elf; DEVKITPRO/DEVKITARM gesetzt; hier nicht geprüft |
| Neue Wurzel, nach Sicherung | `bash scripts/test-host.sh` | Features/Seeds in .test-build unter UBSan, keine vollständige .27–.33-Suite |
| Neue Wurzel, Windows | `powershell -NoProfile -File desktop/source/build.ps1` | desktop/SD-Karte/3DS Library PC.exe; alter Dateiname auch bei .4.1, Hash/Version prüfen |
| Neue Wurzel, später isolierter Scratchpfad | `& '.\desktop\SD-Karte\3DS Library PC.exe' --self-test '<isolierter Reportpfad>'` | Desktop-Testreport und Scratch; keine Live-SD |
| Neue Wurzel, optionale native CIA | `powershell -NoProfile -File scripts/build-cia.ps1 -Makerom '<makerom.exe>' -Bannertool '<bannertool.exe>'` | CIA/Banner; tatsächliche Paket-Service-ACL prüfen, nicht nur RSF; reguläres OTA braucht keine neue native CIA |

Private .30-Theme-/FFmpeg-Fixtures nicht migrieren/publizieren; synthetische Fixtures verwenden. Neuere Tests liegen im Release-ZIP zusätzlich außerhalb launcher/tests und müssen gezielt integriert werden. Alte SD-Laufwerksbuchstaben niemals blind wiederverwenden.
