# Flow — Einstieg für den Orchestrator

## Freigegebene Bereitstellung .36 — 21.10.2026

Nutzer hat aktuellen Download einschließlich Veröffentlichung ausdrücklich genehmigt. Schema-2-Receiver 0.2.0 läuft gesund am bestehenden Endpunkt; Schema 1 und Schema 2 mit leerem Kommentar öffentlich per HTTPS/App-CA angenommen (201) und SQLite-Speicherung geprüft. Vorherige Konfiguration und Image 0.1.0 bleiben als Rückweg erhalten. Paket .36 wird aus verification-20261021-205317 veröffentlicht; abschließender öffentlicher Bytenachweis folgt separat. Keine .36-Hardwareabnahme.

HOME-Shortcut und Boothelfer laden `/3ds/3ds-library/3ds-library.3dsx`; OTA ersetzt genau diese Datei. Der korrekte Shortcut startet daher neue Versionen ohne neue CIA-Installation. Installation über den aktuellen UniStore, nicht über historische vollständige native CIA-Builds. Spieleinstallation DS/3DS und HOME-Icon-Hardwareprüfung sind im TODO erfasst; keine neue Spielequelle umgesetzt.

## Aktuell: lokale Weiterentwicklung 0.12.36

Der Nutzer hat einen Testbericht auf .35 abgeschickt; dessen Empfang wurde bestätigt (New 3DS XL, EUR, Homebrew, Autoboot an). Das ist keine vollständige Konsolenabnahme.

.36 verschiebt den regulären Zugang nach Menü → Über & Unterstützung → Problem melden (Touch oder X). Unerwartete Fehler erhalten einen direkten Fallbackbutton. Technische Diagnose wird unabhängig vom optionalen Kommentar erfasst und mit Y vor dem bewussten Versand angezeigt. Fehlerbereich/-code und Diagnose bleiben gemeinsam am Fehlerzeitpunkt eingefroren.

Stand: lokal implementiert, noch nicht veröffentlicht oder auf echter Konsole getestet. Der neue abwärtskompatible Schema-2-Receiver muss vor .36-Gerätetest/Veröffentlichung aktualisiert und öffentlich geprüft werden; der laufende .35-Receiver akzeptiert nur Schema 1. Desktop .4.1 und öffentliche .35-Pakete bleiben unverändert. Die folgenden .35-Angaben sind historischer Veröffentlichungsstand.

Aktueller Stand 21.10.2026: verbindliche Integrationswurzel ist C:/Users/subje/Documents/Code/vitarenew-flow. Launcher 0.12.35 als öffentlich bytegeprüfter OTA-Teststand, Desktop 0.4.1. Keine Hardwareabnahme vorweggenommen.

## Aktiver Auftrag: Fehlerberichte (21.10.2026)

Der Auftrag basiert auf Remote-main `815da5a` (PC-2-Arbeit/0.12.34 integriert), nicht auf dem veralteten lokalen PC-1-main .33. Isolierter angehängter Worktree `error-reports`, Launcher 0.12.35. [ERROR-REPORTS](ERROR-REPORTS.md) dokumentiert UI, Datenschutz, Empfangsdienst, Tests und Hardwareplan. Der Nutzer hat den öffentlichen Serverempfang im Benutzerverzeichnis und die OTA-Testveröffentlichung ausdrücklich freigegeben. Empfang läuft unter /home/maxim/apps/vitarenew-flow-reports; synthetischer HTTPS-Bericht mit App-CA-Bundle angenommen und in SQLite bestätigt. Paketnachweise: RELEASE-0.12.35.md. Keine automatische weitere Veröffentlichung oder SD-Übertragung. Desktop .4.1 unverändert.

Vor jeder weiteren Aufgabe Remote-Refs/Branches gegen den lokalen Stand prüfen (AGENTS Regel 8). Nicht gepushte Arbeit auf PC 2 bleibt separat abzuklären. Alte Angaben zu `codex/pc2-handoff` unten beschreiben den ursprünglichen Gerätewechsel, nicht die heutige jüngste Quellbasis. Vollständige historische .34-Veröffentlichung heute nicht erneut live auditiert; der neue Featureauftrag ändert keine bestehenden Kanäle.

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

OTA-Testversion 0.12.35 ist veröffentlicht: Releasecommit 2ada1cd, Pages-Lauf 39651893937 erfolgreich, alle neun Paketdateien und vier Kanäle öffentlich bytegleich geprüft. Empfangsdienst aktiv; synthetischer HTTPS-Bericht gespeichert und bestätigt. [RELEASE-0.12.35](RELEASE-0.12.35.md), [ERROR-REPORTS](ERROR-REPORTS.md) und [öffentlicher Nachweis](release-evidence/0.12.35.json).

Nächster Haltpunkt ist die ausdrückliche Konsolenabnahme: Menü → Konsole & Start → Updates → Aktualisieren → Neu öffnen; Version .35 prüfen. Melden, Beschreibung, zweimal A und tatsächlichen Berichtempfang sowie WLAN-aus/Abbruch/HOME/POWER/DE-EN/Rückkehr prüfen. Modell/Region/Startweg/Autoboot notieren. Gesunde Konsole nicht per TWL-Reparatur verändern. Keine Hardwarezustimmung aus dem Build oder fehlendem Feedback ableiten.

PC 2 und PC 1 müssen vor Weiterarbeit den aktuellen Remote-main abrufen und eigene Änderungen erhalten. Das historische codex/pc2-handoff ist kein heutiger Einstieg mehr; aktuelle Basis ist der veröffentlichte main. Noch nicht gepushte Arbeit eines anderen PCs bleibt separat abzuklären. N64-/Save-Sync-Themen bleiben vertagt, originale HOME-Standardmusik offen. Kein weiterer Backlogauftrag automatisch gestartet.

## Grenzen

Nicht jede historische Chatentscheidung ist rekonstruierbar, das Modell der zweiten Konsole ist unbekannt. Toolchain und vollständige angepasste Emulator-Buildumgebungen sind externe lokale Abhängigkeiten; sie wurden nicht neu installiert oder pauschal versioniert. Ihr Inventar steht im Migrationsbericht. Ein frischer Worktree enthält Projektquellen und die portierte Prüfung, aber nicht SDK/Adapter-SDKs oder private Daten.
