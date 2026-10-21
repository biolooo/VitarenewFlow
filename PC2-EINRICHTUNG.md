# VitaRenewFlow – PC-2-Einrichtung

## Aktueller Abschlussstand

Die Einrichtung wurde nach dem Windows-Neustart am 21.10.2026 abgeschlossen. WSL2/Ubuntu 24.04 liegt auf E:, der gepinnte devkitARM-Chroot unter `/opt/vitarenew-sdk/bundle/rootfs`. Frischer 3DSX-/SMDH-Build und sämtliche vorgesehenen Hosttests sind bestanden. 110 Quell-/Assethashes nach dem Build unverändert, alle acht Releasehashes weiterhin korrekt. Desktop .4.1 hat 108 Selbsttests bestanden. Es ist kein weiterer Neustart nötig.

Aktuelle Befehle, Nachweise und verbleibende Grenzen stehen in [docs/PC2-SETUP.md](docs/PC2-SETUP.md). Dokumentationsänderungen liegen auf `codex/pc2-setup`; kein Push oder Merge. Die folgenden Abschnitte bewahren die frühere Bestandsaufnahme und den Neustart-Haltpunkt und beschreiben nicht mehr den aktuellen Einrichtungsstatus.

Stand: 21.10.2026. Lokale Bestandsaufnahme und Einrichtungsplan; keine Hardwareabnahme.

## Bestätigter Projektstand

- GitHub: https://github.com/biolooo/VitarenewFlow.git (öffentlich).
- Arbeitswurzel: C:\Users\Maxim\OneDrive\Documents\ChatGPT\Vitarenew - Flow.
- Ausgangslage war ein leeres Repository ohne Commits und Remote. Diese Bestandsaufnahme wurde als einzige lokale Datei erhalten.
- origin wurde eingerichtet und gefetcht. Der ausdrücklich für PC 2 vorbereitete Branch codex/pc2-handoff ist lokal ausgecheckt und verfolgt origin/codex/pc2-handoff.
- Geprüfter HEAD: 3b6ad02; beim Abruf 0 voraus / 0 zurück zum Remote-Übergabebranch. main wurde nicht ausgecheckt oder gemergt.
- Projektbasis: Nintendo-3DS-Launcher 0.12.33 und Windows-Desktop 0.4.1. Node, Datenbank und Shopify sind keine belegten Buildabhängigkeiten.
- Gelesen: AGENTS.md, README.md, HANDOFF, STATUS, MIGRATION, ARCHITECTURE, DECISIONS und ORCHESTRATION sowie die relevanten Einrichtungs-/Buildskripte.
- Einstieg für spätere Orchestratoren: AGENTS.md und docs/HANDOFF.md. Status und offene Hardwaretests: docs/STATUS.md. Alte absolute Pfade beschreiben PC 1.

## Tatsächlich geprüft

| Prüfung | Ergebnis |
| --- | --- |
| Öffentlicher Git-Lesezugriff | Erfolgreich, Remote gefetcht |
| Release 0.12.33 | Alle acht Manifest-SHA256-Prüfungen bestanden |
| Windows .NET Framework Compiler/WPF | Vorhanden |
| Desktop-Build | desktop/source/build.ps1 erfolgreich |
| Desktop-Selbsttest | Version 0.4.1, 108 Prüfungen bestanden, Exitcode 0 |
| Desktop-Testbericht | .test-build/pc2-initial/desktop-self-test.json, ignoriert |
| WSL | wsl.exe vorhanden, meldet aber: WSL nicht installiert |
| Launcher-Build/Hosttests | Noch nicht ausgeführt: WSL/SDK fehlen |
| GitHub-Schreibzugriff | Nicht nachgewiesen; gespeicherte Anmeldung liefert HTTP 401 |
| Konsolenprüfung | Nicht durchgeführt |

Vor dem Desktop-Build wurde desktop/SD-Karte angelegt, da das Buildskript den im frischen Checkout fehlenden Ausgabeordner nicht erzeugt. Ausgabe und Testberichte sind ignoriert; keine Releasebytes verändert. Der Selbsttest verwendet synthetische temporäre Daten, kein echtes SD-Laufwerk.

## Lokale Werkzeuge und Agenten

Git 2.52.0.windows.1, Node 20.14.0, npm 10.8.0, pnpm 11.25.0 und Python 3.12.3 sind vorhanden. VS Code und Shopify CLI sind im PATH gefunden, aber für diesen Build nicht erforderlich. GitHub CLI, Docker, Yarn und uv wurden im PATH nicht gefunden; sie sind nicht pauschal nachzuinstallieren. Git-Autor ist konfiguriert, Credential Helper ist manager. Gespeichertes GitHub-Konto: biolooo. Browser war nicht bei GitHub angemeldet. Eine ergänzende Gmail-Suche scheiterte an einer Connector-Berechtigungs-/Limitmeldung; für den gefundenen öffentlichen Remote nicht erforderlich.

Subagent-Funktionen sind verfügbar. Ein unabhängiger lesender Prüfer hat Buildvoraussetzungen und Plan geprüft; keine parallelen Schreibzugriffe. Projektbezogene Werkzeuge/Skills sind bei Bedarf verfügbar; angebotene MCP-Werkzeuge belegen keine funktionierenden Kontoverbindungen. GitHub-Push benötigt später eine erneute Anmeldung. Keine Zugangsdaten im Bericht speichern.

## Konkrete nächste Einrichtungsschritte

1. WSL mit Ubuntu einrichten. Administratorbestätigung und gegebenenfalls Neustart durch den Nutzer berücksichtigen. In dieser Bestandsaufnahme keine Installation gestartet.
2. Einen kompatiblen Linux-Chroot mit devkitARM/devkitPro, libctru, citro2d/3d, libpng, curl, mbedTLS und zlib sowie Bash, Python 3, make und Host-C-Compiler mit UBSan vorbereiten. SDK außerhalb Git halten. Den in docs/HANDOFF.md genannten Container-Digest als Ausgangsbasis prüfen; das Image allein ist kein vollständiger Nachweis der Hosttestumgebung.
3. FLOW_TOOLCHAIN_ROOT auf den vollständigen Chroot setzen. scripts/verify-local.sh benötigt Schreibzugriff im Chroot und Berechtigung für chroot; ein gewöhnlicher SDK-Ordner genügt nicht.
4. bash scripts/verify-local.sh in WSL ausführen, frisches isoliertes Staging und Testberichte prüfen. Keine alten work/*-Generatoren verwenden. Native CIA benötigt zusätzlich makerom/bannertool; vollständige Emulatoradapter-Builds haben separate externe Umgebungen.
5. GitHub-Anmeldung für spätere genehmigte Pushes erneuern; öffentlicher Fetch funktioniert bereits. Keine Tokens in Chat, Git oder Berichte kopieren.
6. Vor paralleler PC-Nutzung prüfen, ob OneDrive diesen Ordner synchronisiert. Separate lokale Checkouts und GitHub als gemeinsame Codebasis verwenden. Ein späterer Umzug nach Documents/Code ist noch nicht durchgeführt.
7. Neue Ergebnisse in die vorhandene Dokumentationsstruktur übernehmen; offene Features nicht automatisch starten. Diese Datei ist eine lokale Ergänzung und noch nicht committed.

## Arbeitsregeln und Abnahmegrenzen

Die Regeln aus AGENTS.md und docs/ORCHESTRATION.md gelten. Vor Arbeit Status/Remote prüfen; Änderungen erhalten, Zuständigkeiten exklusiv vergeben, parallele Schreibaufgaben in getrennten Worktrees aus gesichertem Stand. Keine Force-Pushes, automatischen main-Merges, Veröffentlichungen oder SD-/NAND-Aktionen. Der Nutzer verlangt ausdrückliche Freigabe für main-Merges, produktive Deployments, riskante Löschungen/Migrationen und produktive Zugangsdaten-/Shopdatenänderungen. Kein automatischer Remote-Push nach einem erfolgreichen Build.

Bei Entwicklungsaufträgen Anforderungen und Akzeptanzkriterien festlegen, sinnvoll delegieren, relevante Tests und bei größeren Änderungen unabhängiges Review durchführen. Entscheidungen, bekannte Fehler und Übergabe dauerhaft dokumentieren. Fehlendes Hardwarefeedback ist kein Pass.

PC 2 ist für Desktop-Build und Selbsttests geprüft. Vollständige Launcher-Entwicklung bleibt bis erfolgreicher WSL-/SDK-Einrichtung und nativem Build/Hosttests offen. Hardwareabnahmen .27–.33 und vollständige Adapterumgebungen bleiben separat. Keine Installation, kein Commit, Push, Merge oder Deployment in dieser Bestandsaufnahme.

## Fortsetzung: WSL-Einrichtung – Neustart erforderlich

Dieser Abschnitt ersetzt die früheren Aussagen „WSL fehlt“ und „keine Installation“ für den aktuellen Stand.

- Nutzer hat WSL-/devkitARM-Einrichtung, 3DS-Build und Hosttests ausdrücklich beauftragt.
- WSL-Basiskomponente erfolgreich installiert: WSL 3.0.3.0, Kernel 6.18.54.1-1.
- VirtualMachinePlatform mit Administratorrechten geprüft/aktiviert, Zustand Enabled. Enable-WindowsOptionalFeature meldet RestartNeeded=True. Kein Neustart automatisch ausgelöst.
- Firmware-Virtualisierung wurde als aktiviert gemeldet; HypervisorPresent=True. Ein BIOS-Eingriff ist derzeit nicht begründet.
- Ubuntu-24.04-Installationsversuch meldet, dass Änderungen erst nach Neustart wirksam werden. Danach weiterhin keine Distribution registriert und E:\WSL\VitaRenewFlow-Ubuntu noch nicht vorhanden. Nicht als erfolgreiche Ubuntu-Installation behandeln.
- C: hatte vor Einrichtung nur rund 4 GB frei. Linux-Distribution und SDK sollen auf E: liegen (rund 788 GB frei bei Prüfung).
- Administrationsskript und Ergebnis: .test-build/pc2-initial/wsl-install/enable-vm.ps1, enable-vm.log und enable-vm-result.txt (ignoriert). Ergebnis SUCCESS, RestartNeeded=True.

### Nach Nutzer-Neustart fortsetzen

1. Git-Status/Remote und WSL-Version/Status/Distributionsliste erneut prüfen.
2. Wenn weiterhin keine Distribution vorhanden ist und Zielpfad frei ist: `wsl --install Ubuntu-24.04 --location E:\WSL\VitaRenewFlow-Ubuntu --no-launch --web-download`. Installation prüfen, vorhandene Distributionen/Ordner nicht ersetzen.
3. Ubuntu initialisieren und isolierten devkitARM-Chroot aus dem in docs/HANDOFF.md gepinnten Image vorbereiten; Hostcompiler/Python/UBSan ergänzen. SDK außerhalb Git, keine privaten Daten übernehmen.
4. Mit FLOW_TOOLCHAIN_ROOT und erforderlichen Chroot-Rechten scripts/verify-local.sh ausführen; native Buildartefakte, Quellhashes und Hosttestberichte prüfen.
5. Echte Ergebnisse dokumentieren. Launcher-Build und Hosttests sind bis jetzt NICHT ausgeführt. Desktop-Build und 108 Selbsttests aus der ersten Prüfung bleiben erfolgreich.

Kein Commit, Push, Merge, Release, SD-Transfer oder NAND-Eingriff durchgeführt. Aktueller Haltpunkt: Windows-Neustart durch den Nutzer erforderlich.
