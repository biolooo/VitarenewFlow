# PC 2: Entwicklungsumgebung und Prüfung

Stand 21.10.2026, Basis `3b6ad02` (Übergabebranch `codex/pc2-handoff`), Launcher 0.12.33 / Desktop 0.4.1. Einrichtung und Prüfungen auf PC 2 abgeschlossen; keine neue Konsolenabnahme.

## Lokale Umgebung

- Projekt: `C:\Users\Maxim\OneDrive\Documents\ChatGPT\Vitarenew - Flow`.
- WSL 3.0.3.0, Kernel 6.18.54.1-1, WSL2 und VM-Plattform aktiviert; erforderlicher Windows-Neustart durchgeführt.
- Distribution `Ubuntu-24.04`, Ubuntu 24.04.5 LTS, installiert unter `E:\WSL\VitaRenewFlow-Ubuntu`.
- SDK-Chroot: `/opt/vitarenew-sdk/bundle/rootfs` innerhalb dieser Distribution, außerhalb Git und OneDrive.
- Unveränderte SDK-Basis: `devkitpro/devkitarm@sha256:15b79ce75822c289538d8153da5fa7aafe5e6adc32ad8a575a197beca0f0761b`, linux/amd64. Mit Skopeo unter Erhalt der Digests heruntergeladen und mit umoci entpackt. Image und Herkunft unter `/opt/vitarenew-sdk/image` und `image-inspect.json` erhalten.
- Chroot: Debian 12, devkitARM GCC 16.1.0, Host-GCC 12.2.0, Python 3.11.2 und Make 4.3. Vorhandene 3DS-Bibliotheken verwendet, kein SDK-Upgrade. Hostcompiler mit UBSan separat erfolgreich geprüft.
- In Ubuntu installiert: `skopeo`, `umoci`, `python3`, `ca-certificates` samt Paketabhängigkeiten. Docker ist nicht erforderlich.
- Für die später ergänzten Banner-Hosttests wurde im Debian-Chroot zusätzlich `zlib1g-dev` (1:1.2.13.dfsg-1) installiert; die devkitARM-/3DS-Pakete blieben unverändert. Paketverwaltung benötigt ein funktionsfähiges `/dev/null` im Chroot; hierfür wurde es gezielt temporär gebunden und danach ausgehängt. Signierte Paketindizes wurden erfolgreich geprüft, keine Signaturprüfung deaktiviert.

Buildbefehle laufen gezielt als Linux-root, weil der vorhandene Runner einen Chroot benötigt. Eine interaktive Ubuntu-Benutzereinrichtung mit Passwort war dafür nicht erforderlich. Windows-Administratorrechte sind für die laufenden Prüfungen nicht nötig.

## Wiederholbare Prüfung aus PowerShell

Im Projektordner:

```powershell
wsl -d Ubuntu-24.04 -u root --exec env FLOW_TOOLCHAIN_ROOT=/opt/vitarenew-sdk/bundle/rootfs bash scripts/verify-local.sh
```

WSL übernimmt das aktuelle Windows-Arbeitsverzeichnis. Der Runner erstellt jedes Mal frisches isoliertes SDK-Staging und schreibt Ergebnisse nach `.test-build/verification-*`. Keine alten Generatoren verwenden. Die erzeugte 3DSX wird nicht automatisch installiert oder veröffentlicht.

Desktopprüfung aus dem Projektordner:

```powershell
New-Item -ItemType Directory -Force desktop/SD-Karte, .test-build/pc2-initial | Out-Null
& ./desktop/source/build.ps1
$exe = Join-Path $PWD 'desktop/SD-Karte/3DS Library PC.exe'
$report = Join-Path $PWD '.test-build/pc2-initial/desktop-self-test.json'
$testProcess = Start-Process -FilePath $exe -ArgumentList @('--self-test', ('"' + $report + '"')) -WindowStyle Hidden -Wait -PassThru
if ($testProcess.ExitCode -ne 0) { throw 'Desktop-Selbsttest fehlgeschlagen' }
Get-Content -LiteralPath $report -Raw
```

Der Desktop-Selbsttest verwendet synthetische temporäre Daten. Sein Bericht und Prozess-Exitcode sind maßgeblich; keine echte SD auswählen.

## Tatsächlich erfolgreich ausgeführt

- Frischer 3DSX-/SMDH-Build mit `-Wall -Wextra -Werror` im gepinnten SDK.
- Quellabgleich der kopierten Komponenten-/Start-Testfragmente bestanden.
- Hostfixtures mit UBSan: Komponenten/DS-Runtime und Eingabe, TWL-Diagnose/Reparatur gegen Mocks einschließlich 128 Empfehlungskombinationen, zwölf HOME-Ladeszenarien, Suche mit 20.000 Fuzzfällen, ROM-Preflight und POWER mit 20 Abbruchzyklen und 2.000 Folgeframes.
- Vollständiger Runner mit Exitcode 0 und abschließendem PASS.
- Desktop 0.4.1: Build und 108 Selbsttests bereits auf PC 2 bestanden.
- Alle acht vorhandenen Release-Manifesthashes 0.12.33 vor Einrichtung geprüft; ausgelieferte Pakete bleiben unverändert.

Nachweise: `.test-build/verification-20261021-183300/verification.log`, `source-hashes.json`, `staging-path.txt`, `artifact-hashes.txt`, erzeugte `3ds-library.3dsx` und `.smdh`. SDK-Einrichtung: `.test-build/pc2-initial/sdk-setup.log` und `prepare-sdk.sh`. Desktopbericht: `.test-build/pc2-initial/desktop-self-test.json`. Diese maschinenspezifischen Daten sind ignoriert.

Nach dem Build wurden alle 110 erfassten Quell-/Assethashes gegen die Arbeitskopie und alle acht Releasehashes erneut geprüft: unverändert. Der unabhängige lesende Review bestätigte Quellhashes, Artefakthashes und PASS-Nachweise. Sein Hinweis zum zuverlässigen Warten auf den Desktop-Testprozess ist im obigen Befehl berücksichtigt.

## Grenzen und Zusammenarbeit

Hosttests ersetzen keine echte Konsole. Native CIA, Boot-Dispatcher, Shortcut und vollständige angepasste Emulatoradapter wurden in diesem Auftrag nicht neu gebaut; deren separate Umgebungen bleiben eigenständige Aufgaben. Keine ROMs, Saves, Schlüssel oder private Musik übernommen. Keine Veröffentlichung, SD-/NAND-Aktion, kein Push und kein main-Merge.

Öffentlicher Fetch funktioniert. GitHub-Schreibzugriff ist noch nicht nachgewiesen; die früher gespeicherte Anmeldung lieferte HTTP 401. Vor späterem Push erneut anmelden. Der Projektordner liegt unter OneDrive; dessen Synchronisationszustand wurde nicht bestätigt. Für PC-Wechsel nur einen Schreibenden je Aufgabe, Git-Status prüfen und Remote-Stände vergleichen; keine automatische Rücksynchronisation von PC-1-Altpfaden.
