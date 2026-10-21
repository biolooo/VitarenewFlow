# Flow — Einstieg für den Orchestrator

Aktueller Stand 21.10.2026: verbindliche Integrationswurzel ist C:/Users/subje/Documents/Code/vitarenew-flow. Launcher 0.12.33, Desktop 0.4.1. Die lokale Migration übernimmt vorhandene Entwicklung; sie ist keine neue Version und keine Hardwareabnahme.

Lesereihenfolge: AGENTS.md → STATUS.md → MIGRATION.md → ARCHITECTURE.md → DECISIONS.md → ORCHESTRATION.md. PROJECT-COPY.md enthält die ursprüngliche Herkunft, migration-evidence die datierten Nachweise. Der aktuelle Git-Commit benennt die gesicherte Basis; immer zusätzlich den Arbeitsbaum prüfen.

Der neue Nutzerchat heißt Flow-Orchestrator (01a15efd-a7e4-7193-abe3-afde29bead5d). Er hat ohne alten Chat die Quell-/Releasevergleiche reproduziert. Der alte Entwicklerchat und Legacyordner bleiben Referenzen; künftig nicht dort parallel entwickeln oder automatisch zurücksynchronisieren.

## Lokale Prüfung

### Zweiter PC (vorbereitet am 21.10.2026)

Der vollständige lokale Entwicklungsstand wird für den Gerätewechsel auf dem GitHub-Branch `codex/pc2-handoff` bereitgestellt. Remote-`main` ist ein anderer Stand: für den Einstieg ausdrücklich diesen Übergabe-Branch klonen. Ein Push auf `main` kann Pages veröffentlichen; keine automatische Zusammenführung. Der bisherige absolute Windows-Pfad beschreibt PC 1. Auf PC 2 ist dessen ausgewählter lokaler Checkout die Arbeitswurzel.

`scripts/setup-second-pc.ps1` klont diesen Branch in ein neues Verzeichnis, verweigert vorhandene Zielordner und prüft die acht .33-Releasehashes. Standardziel ist `Documents/Code/vitarenew-flow` im jeweiligen Benutzerprofil. Alternativ direkt `git clone --single-branch --branch codex/pc2-handoff https://github.com/biolooo/VitarenewFlow.git <neuer-Zielordner>` verwenden. Danach den Ordner in Codex als lokales Projekt öffnen und die Einstiege lesen. Chatdaten, Kontenanmeldungen und lokale Skills werden dabei nicht kopiert; die Projektdateien reichen zur fachlichen Übernahme.

Vor einem Build auf PC 2 Werkzeuge prüfen: Git, Windows .NET Framework/WPF sowie WSL Ubuntu und ein kompatibler devkitARM-chroot mit Hostcompiler/Python. WSL-Installation kann Administratorbestätigung und Neustart auf PC 2 erfordern. Den SDK-Pfad über `FLOW_TOOLCHAIN_ROOT` setzen; kein SDK oder private Konfiguration gehört ins Git. Der bisherige Basiscontainer war `devkitpro/devkitarm@sha256:15b79ce75822c289538d8153da5fa7aafe5e6adc32ad8a575a197beca0f0761b` (Linux amd64). Das Image allein belegt noch nicht die vollständige Hosttestumgebung; auf PC 2 frisch bauen und prüfen. Keine komplette WSL-Installation, Benutzerprofile, Schlüssel oder private Fixtures von PC 1 kopieren. Vollständige Adapter-SDKs bleiben wie in MIGRATION beschrieben separat.

Arbeitswechsel: immer nur ein PC schreibt an derselben Aufgabe. Änderungen prüfen, gezielt committen und auf den vereinbarten Entwicklungsbranch pushen; am anderen PC zunächst sauberen Status prüfen und nur per Fast-forward aktualisieren. Bei divergierten oder uncommitteten Ständen stoppen und vergleichen, niemals automatisch resetten/stashen oder force-pushen. Nicht gleichzeitig auf PC 1 `main` und auf PC 2 dem Übergabe-Branch unterschiedliche Fortsetzungen derselben Aufgabe geben.

Startauftrag für Codex auf PC 2: „Richte VitarenewFlow hier als lokalen Entwicklungsplatz ein. Klone https://github.com/biolooo/VitarenewFlow.git, Branch codex/pc2-handoff, in einen neuen Ordner unter Documents/Code. Erhalte vorhandene Ordner. Lies AGENTS.md und docs/HANDOFF.md. Prüfe Releasehashes, Git-Zustand und fehlende Buildwerkzeuge; richte die benötigte lokale Umgebung ein und prüfe den nativen Build/Hosttests sowie Desktop .4.1 isoliert. Bei Windows-Administratorbestätigung oder Neustart melde dich. Keine Features, Veröffentlichung, main-Push, SD-/NAND-Aktionen oder Übernahme privater Daten.“

- WSL: `bash scripts/verify-local.sh`. Vorhandener SDK unter /home/maxim/codex-3ds-work/toolchain, über FLOW_TOOLCHAIN_ROOT konfigurierbar. Der Aufruf erstellt frisches isoliertes SDK-Staging und protokolliert unter .test-build/. Alte absolute Generatoren nicht ausführen.
- Desktop: `desktop/source/build.ps1`, anschließend `desktop/SD-Karte/3DS Library PC.exe --self-test <isolierter Reportpfad>`. Kein echtes SD-Laufwerk verwenden.
- Build-/Fixtureerfolg ist kein Beleg für aktuelle Konsolenfunktion. Siehe STATUS für .27–.33 und offene Fehler.

## Nächster Auftrag

PC 2 ist inzwischen lokal eingerichtet und geprüft: WSL2/Ubuntu auf E:, gepinnter devkitARM-Chroot, nativer 3DSX-Build und vorgesehene Hosttests erfolgreich. Desktop .4.1 besteht 108 Selbsttests. Reproduzierbare Befehle und genaue Grenzen: [PC2-SETUP.md](PC2-SETUP.md). Keine neue Hardwareabnahme oder Veröffentlichung. GitHub-Schreibzugriff bleibt separat zu klären.

Erst den aktuellen Git-Zustand und Migrationsbericht lesen. Keinen offenen Featurepunkt automatisch starten. Hardwarefeedback .27–.33 ist noch offen; insbesondere POWER wiederholt, HOME/Spielstart, Theme-Musik, DS/Komponentenprüfung und Desktop-Reparatur. N64-Rücksprung bleibt ohne konkrete Logs ungeklärt. Startanimation ist trotz Entfernungswunsch noch vorhanden. Originale Standardmusik ist nicht fertig.

Bei einem konkreten Entwicklungsauftrag: begrenzten Implementierer und unabhängigen lesenden Prüfer einsetzen, Dateieigentum und Commitbasis benennen, Schreibaufgaben bei Parallelität in getrennten Worktrees isolieren. Hauptchat hält Entscheidungen und kurze Ergebnisse. Nach Abschluss STATUS/DECISIONS/HANDOFF aktualisieren. Kein automatischer Release/Push/SD-Transfer und keine NAND-Reparatur.

## Grenzen

Nicht jede historische Chatentscheidung ist rekonstruierbar, das Modell der zweiten Konsole ist unbekannt. Toolchain und vollständige angepasste Emulator-Buildumgebungen sind externe lokale Abhängigkeiten; sie wurden nicht neu installiert oder pauschal versioniert. Ihr Inventar steht im Migrationsbericht. Ein frischer Worktree enthält Projektquellen und die portierte Prüfung, aber nicht SDK/Adapter-SDKs oder private Daten.
