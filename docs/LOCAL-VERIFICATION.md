# Lokale Migrationsprüfung — 21.10.2026

## Tatsächlich ausgeführt

- WSL aus neuer Wurzel: `bash scripts/verify-local.sh`. Frisches SDK-Staging; Quellhashes vor dem Build und nachher mit dem Ziel verglichen. Native 3DSX/SMDH mit devkitARM -Wall/-Wextra/-Werror gebaut.
- Komponenten-/Desktop-/DS-Runtime-Fixtures mit UBSan: EXE/Altnamen, fehlende Sidecars, Cache/Neuprüfung, IO-Fehler, Touch/X/Downloadgrenzen; relevante kopierte Quellen vor Ausführung auf Gleichheit geprüft.
- TWL: tatsächliche twl_repair.c/twl_advice.c gegen Hostmocks, Indikation/Consent, Whitelist, Journal/IO, 128 Empfehlungskombinationen. Keine echte Konsole/NAND.
- HOME-Ladeanzeige: 12 vorhandene Hostfixture-Szenarien. Suche: aktuelle game_search.c/home_controls.c, Relevanz/Tippfehler/Unicode/Grenzen und 20.000 Fuzzfälle. ROM-Preflight: quellverankerter Fragmenttest. POWER: vorhandene Mockfixture (20 wiederholte OS-Anfragen/Abbruch und 2.000 Folgeframes).
- Windows Desktop .4.1 frisch gebaut. `--self-test` in synthetischer temporärer Umgebung: **108 Prüfungen bestanden**. Keine Live-Coversuche/Netzwerktests und keine echte SD.
- 100 Launcher-C/H-Dateien weiterhin identisch zu Legacy .33. Desktopquellen unverändert .4.1. Alle acht Einträge des bestehenden .33-Release-Hashmanifests geprüft. Keine unerwartete Änderung vorhandener Dateien außerhalb der protokollierten Migrationspfade.

## Gefundener Migrationsfehler

Der erste native Build scheiterte an data/README.md, das durch den alten *.*-Filter als Binärressource behandelt wurde. Der Makefilefilter wurde auf *.bin begrenzt. Der vollständige native Build und alle oben genannten Fixtures wurden danach erfolgreich erneut ausgeführt. Keine Änderung der Anwendungslogik/Version.

## Belege und Grenzen

Versionierter Bericht: migration-evidence/local-verification-2026-10-21.json. Ausführliche lokale Logs/Artefakte liegen ignoriert in .test-build/verification-20261021-170850, Desktopreport in .test-build/desktop-self-test.json. Die Stagingherkunft steht dort in source-hashes.json/staging-path.txt. Der neue Runner verwendet keine alten Quellorte und überschreibt kein vorhandenes SDK-Projekt.

Die Release-Testquellen sind unverändert archiviert. Manche Fixtures enthalten extrahierte Logik; Quellanker decken zentrale Komponenten-/Preflightfunktionen ab, nicht jede Zeile jeder Fixture. Kopierte HOME-/POWER-/Input-Fixtures sind keine vollständigen Laufzeittests. Reale Audioausgabe, Boot, HOME-Rückkehr, Spiele/Module und DS-Modus brauchen explizite Konsolenabnahme. Keine private Theme-Fixture kopiert oder neu getestet. Kein Native-CIA-/Emulator-Neubau und keine Stable-/Veröffentlichungsfreigabe.

## Unabhängiger Review

Lesender Prüfer ohne Änderungen: 100 Launcher-C/H gleich Legacy .33, Desktopquellen gleich Sicherung, alle 1.205 erfassten Snapshotdateien unverändert; 19 Übernahmedateien hashgleich, Makefilekorrektur dokumentiert. Quellhashes passen zum Prüfbericht, keine fehlenden lokalen Markdownlinks oder Unicode-Ersatzzeichen. Zwei veraltete STATUS-Aussagen korrigiert. Keine weiteren belegten Migrationsmängel. Finaler Git-Stand wird separat geprüft.
