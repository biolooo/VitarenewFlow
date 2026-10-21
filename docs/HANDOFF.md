# Flow — Einstieg für den Orchestrator

Aktueller Stand 21.10.2026: verbindliche Integrationswurzel ist C:/Users/subje/Documents/Code/vitarenew-flow. Launcher 0.12.33, Desktop 0.4.1. Die lokale Migration übernimmt vorhandene Entwicklung; sie ist keine neue Version und keine Hardwareabnahme.

Lesereihenfolge: AGENTS.md → STATUS.md → MIGRATION.md → ARCHITECTURE.md → DECISIONS.md → ORCHESTRATION.md. PROJECT-COPY.md enthält die ursprüngliche Herkunft, migration-evidence die datierten Nachweise. Der aktuelle Git-Commit benennt die gesicherte Basis; immer zusätzlich den Arbeitsbaum prüfen.

Der neue Nutzerchat heißt Flow-Orchestrator (01a15efd-a7e4-7193-abe3-afde29bead5d). Er hat ohne alten Chat die Quell-/Releasevergleiche reproduziert. Der alte Entwicklerchat und Legacyordner bleiben Referenzen; künftig nicht dort parallel entwickeln oder automatisch zurücksynchronisieren.

## Lokale Prüfung

- WSL: `bash scripts/verify-local.sh`. Vorhandener SDK unter /home/maxim/codex-3ds-work/toolchain, über FLOW_TOOLCHAIN_ROOT konfigurierbar. Der Aufruf erstellt frisches isoliertes SDK-Staging und protokolliert unter .test-build/. Alte absolute Generatoren nicht ausführen.
- Desktop: `desktop/source/build.ps1`, anschließend `desktop/SD-Karte/3DS Library PC.exe --self-test <isolierter Reportpfad>`. Kein echtes SD-Laufwerk verwenden.
- Build-/Fixtureerfolg ist kein Beleg für aktuelle Konsolenfunktion. Siehe STATUS für .27–.33 und offene Fehler.

## Nächster Auftrag

Erst den aktuellen Git-Zustand und Migrationsbericht lesen. Keinen offenen Featurepunkt automatisch starten. Hardwarefeedback .27–.33 ist noch offen; insbesondere POWER wiederholt, HOME/Spielstart, Theme-Musik, DS/Komponentenprüfung und Desktop-Reparatur. N64-Rücksprung bleibt ohne konkrete Logs ungeklärt. Startanimation ist trotz Entfernungswunsch noch vorhanden. Originale Standardmusik ist nicht fertig.

Bei einem konkreten Entwicklungsauftrag: begrenzten Implementierer und unabhängigen lesenden Prüfer einsetzen, Dateieigentum und Commitbasis benennen, Schreibaufgaben bei Parallelität in getrennten Worktrees isolieren. Hauptchat hält Entscheidungen und kurze Ergebnisse. Nach Abschluss STATUS/DECISIONS/HANDOFF aktualisieren. Kein automatischer Release/Push/SD-Transfer und keine NAND-Reparatur.

## Grenzen

Nicht jede historische Chatentscheidung ist rekonstruierbar, das Modell der zweiten Konsole ist unbekannt. Toolchain und vollständige angepasste Emulator-Buildumgebungen sind externe lokale Abhängigkeiten; sie wurden nicht neu installiert oder pauschal versioniert. Ihr Inventar steht im Migrationsbericht. Ein frischer Worktree enthält Projektquellen und die portierte Prüfung, aber nicht SDK/Adapter-SDKs oder private Daten.
