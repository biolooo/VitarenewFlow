# Lokale Flow-Migration — 21.10.2026

Verbindliche Integrationswurzel: dieses Repository. Launcher 0.12.33 aus konsistentem Legacy-Launcher/Repository/Release-Quellpaket; Desktop 0.4.1 im Ziel unverändert. Der Nutzer beauftragte die Migration und ihre Fortsetzung. Die konkrete Quellwahl wurde nach zwei unabhängigen lesenden Prüfungen umgesetzt. Keine neue Featurearbeit, keine Veröffentlichung und keine Änderung der Altstände.

## Sicherung und Übernahme

Vorherige Zielwurzel einschließlich .git und unversionierter Dokumente/Quellen: `C:\Users\subje\Documents\Codex\2026-10-21\referenced-chatgpt-conversation-this-is-an-2\work\flow-before-migration-20261021-170712`. Alle 1020 gesicherten Dateien wurden nach dem Kopieren per SHA256 geprüft. local-development-snapshot ist unverändert am alten Platz und nicht dupliziert. Legacy bleibt unverändert. Kein automatischer Restore/Reset; bei Rückweg einzelne dokumentierte Dateien mit gesicherter Variante vergleichen.

Übernahme: sechs C-Dateien und Makefile; neun Dateien des vorhandenen .33-Releasepakets plus vier zugehörige Kanaldateien. Vorher- und Quellhashes wurden gegen die Aufnahme geprüft. 35 vorhandene öffentliche Testdateien aus .33 wurden unverändert unter tests/release-0.12.33 übernommen. Quelldaten: [integration-2026-10-21.json](migration-evidence/integration-2026-10-21.json).

Migrationskorrektur am Makefile: BINFILES akzeptiert nur *.bin statt *.*. Das zusätzliche Ziel-data/README.md wurde sonst als README.md.o verlinkt und verhinderte den ersten sauberen Build. Die Anwendungs-C/H-Dateien bleiben bytegleich zu .33, Versionsnummer unverändert. Releasearchive/Kanalbytes werden nicht neu gebaut oder umgeschrieben.

AGENTS.md/PROJECT-COPY.md sind jetzt gezielt versionierbar; Snapshot, .test-build, Desktop-/Launcher-Builds und lokale SDK-Konfiguration bleiben ignoriert. Historische Aufnahmeberichte sind unverändert unter migration-evidence; vor der Integration gültige Einstiegstexte stehen dort als before-integration-*.md. Sie sind keine aktuellen Stoppschilder.

## Reproduzierbare Prüfung

Unter WSL aus der neuen Wurzel: `bash scripts/verify-local.sh`. Standard-SDK `/home/maxim/codex-3ds-work/toolchain`, optional FLOW_TOOLCHAIN_ROOT. Der Runner erzeugt einen neuen SDK-Unterordner, kopiert ausschließlich aktuelle Launcherquellen, Branding/Daten und Testquellen, erfasst Quellhashes, baut dort und führt vorhandene Komponenten-/TWL-/HOME-/Suche-/Start-/POWER-Fixtures aus. Logs und Artefakte kommen nach .test-build/verification-*. Kein Cleanup fremder Pfade; Stagingpfad wird protokolliert. Keine alten work/*-Generatoren oder Legacy-Builds werden benutzt.

Komponenten-/Start-Fixtures enthalten kopierte Funktionsfragmente; check-fixture-source.py vergleicht relevante Fragmente vor der Ausführung mit aktuellen Quellen. Andere historische Mockfixtures sind begrenzt und kein vollständiger Systemtest. TWL-Operationen laufen ausschließlich gegen Hostmocks. Private Theme-/Audiofixtures sind nicht migriert.

Windows: `desktop/source/build.ps1` baut Desktop .4.1 in desktop/SD-Karte. `--self-test <isolierter Reportpfad>` verwendet ein synthetisches temporäres Testverzeichnis. Logs/Ergebnisse und konkrete Nachweise stehen in [LOCAL-VERIFICATION.md](LOCAL-VERIFICATION.md). Kein echter Datenträger wird ausgewählt.

## Externe Abhängigkeiten

Vorhandenes WSL Ubuntu/chroot mit devkitPro/devkitARM, libctru, citro2d/3d, libpng, curl, mbedTLS, zlib und Hostcompiler. Keine neue Installation. Native CIA benötigt makerom/bannertool; CIA-Neubau ist für diese Migration nicht nötig. Desktop braucht Windows .NET Framework 4/WPF.

Vollständige angepasste Emulator-Builds bleiben getrennte Aufgaben: Legacy outputs/emulator-adapters, mgba-library, ds-runtime, banner-tools, home-start-test und work/mgba-build/emulators/ds enthalten Werkzeuge, Upstreamquellen und eigene SDKs. NES benötigt älteres SDK, N64 picaGL/ImGui. Die historische Quellen-/Werkzeuginventur und absolute Altpfade stehen in migration-evidence/development-inventory.json und absolute-paths.json. Nicht behaupten, das Repository ersetze diese vollständigen Upstreamumgebungen. Private ROMs/Saves/Schlüssel/Musik wurden nicht übernommen.

## Restliche Grenzen

Keine Konsolenabnahme .27–.33 ergänzt. Kein Stable-Release, kein Push, kein Pages-Workflow gestartet, keine SD-/NAND-/Bootänderung. Historische Releasepakete sind unverändert übernommen, keine neue Lizenzfreigabe der Emulatoren. Künftige Features brauchen eigenen konkreten Auftrag.
