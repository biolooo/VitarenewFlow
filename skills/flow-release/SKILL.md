---
name: flow-release
description: VitarenewFlow-Launcher als geprüftes OTA-/UniStore-Update veröffentlichen. Verwenden bei „als Update veröffentlichen“, „neue Flow-Version bereitstellen“ oder „Flow-Release“. Umfasst Version, Changelog, Build, reproduzierbares Quellpaket, Releasekanäle und öffentliche Downloadprüfung.
---

# Flow-Update veröffentlichen

Arbeite im aktuellen VitarenewFlow-Repository nach AGENTS.md, docs/HANDOFF.md, docs/STATUS.md und docs/MIGRATION.md. Repo-Pfad, neueste Version und Toolchain nicht aus alten Chats übernehmen. Der bekannte PC-1-Worktree ist `C:/Users/subje/.codex/worktrees/error-reports/vitarenew-flow`; auf PC 2 kann der Pfad anders sein.

## Umfang und Basis

Eine ausdrückliche Nutzeraufforderung zur Veröffentlichung autorisiert den passenden Commit, Fast-forward-Push auf main und Pages-Deployment dieses Updates; nicht erneut dieselbe Freigabe erfragen. Bloße Featurearbeit oder erfolgreiche Tests autorisieren keine Veröffentlichung. Ohne Freigabe das fertige Paket lokal vorbereiten. Ein Releaseauftrag startet keine offenen Backlogfeatures und umfasst keine SD-/NAND-/Bootänderungen.

Prüfe Arbeitsbaum und Staging, hole Remote-Refs und vergleiche HEAD mit origin/main sowie relevanten PC-2-Branches. Erhalte fremde Änderungen; keine Resets, automatischen Stashes, Force-Pushes oder pauschales Staging. Bei Divergenz getrennt vergleichen und integrieren; kein fremder Commit darf beim Push verloren gehen. Nicht gepushte Arbeit auf PC 2 ist hier unsichtbar. Nutze die abgesicherte aktuelle Basis und die Rollenregeln aus docs/ORCHESTRATION.md; Releasekanäle und Git haben je nur einen Schreibenden.

## Version, Hinweise und Prüfung

1. Ermittle die nächste freie Patchversion anhand Runtime, `docs/updates/latest.txt`, UniStore und vorhandener Releaseverzeichnisse. Ändere niemals eine bereits veröffentlichte Versionsmappe.
2. Aktualisiere Launcher/Makefile, main.c-Berichtversion, app_update.c, components.c und app_log.c konsistent. Desktop bleibt außerhalb des Releaseumfangs, sofern nicht beauftragt.
3. Pflege ein bis drei zutreffende DE/EN-Neuerungen in `release-notes/<Zielversion>.json` als gemeinsame Changelogquelle. Führe `python scripts/generate-release-notes.py --version <Zielversion>` aus. Dies erzeugt die installierten Hinweise; Paketierung erzeugt daraus README/FEATURES-DE und notes.txt. verify-local, Paketierung und Pages prüfen fehlende, veraltete oder nicht passende Hinweise. Keine manuell gepflegte Versionsliste in main.c/settings_menu.c ergänzen. Keine alten Features als neu ausgeben. Hardwareabnahme nicht aus Builds ableiten.
4. Baue frisch mit `wsl -d Ubuntu -u root -- bash <WSL-Repo-Pfad>/scripts/verify-local.sh`, optional FLOW_TOOLCHAIN_ROOT. Nutze nur den vorhandenen geprüften SDK; keine alten work-Generatoren. Prüfe bei Receiveränderungen separat dessen Linux-Unittests und Schema-Kompatibilität; Serverdeployment ist nur nötig, wenn das Update es erfordert und es autorisiert ist.
5. Prüfe `scripts/prepare-report-release.py` VOR Aufruf: es war bisher versionsgebunden. Ziel/Previous, Texte und Quellenfilter für diesen Release anpassen, nicht blind wiederverwenden. Paketierung darf nur exakt zum abschließenden Build passende Quellhashes akzeptieren. Releaseasset-Anzahl aus Skript/Manifest prüfen; bis .37 neun Dateien/acht Manifesteinträge; ab .38 zehn Dateien/neun Einträge einschließlich notes.txt und vier Kanälen.

## Exaktes Paket

Paket muss App-3DSX/SMDH, vorhandene Boot-/Shortcut-/GM9-Begleiter, zutreffende Hinweise, Quellen und SHA256.json konsistent enthalten. Unveränderte Begleiter aus geprüftem Vorgänger übernehmen. Vier Kanäle: latest.txt, VitarenewFlow.unistore, VitarenewFlow-info.json, install.html. Prüfe Zielversion, URLs, Bytes, Hashes, UniStore-Revision, Aktionen und Installationspfade. Keine Datenbanken, Rohreports, ROMs, Saves, Schlüssel, SDKs oder lokalen Sicherungen ins Paket/Git.

Baue das EXAKTE fertige Quell-ZIP in einem neuen isolierten SDK-Verzeichnis nach dessen RELEASE-BUILD.md; nicht nur den normalen Checkout noch einmal bauen. Stage-Suite bestehen lassen und resultierende 3DSX/SMDH byteweise mit den gelieferten Dateien vergleichen. Logs und Stagingpfad außerhalb Git unter .test-build speichern. Prüfe, dass vorherige veröffentlichte Releases bytegleich zum Ausgangscommit bleiben. Alle Paket-/Kanalbytes müssen auch nach Git-Staging unverändert bleiben (.gitattributes).

## Veröffentlichung und Nachweis

Stelle vor dem Push aktuellen Remotezustand, Diff, gewählte Dateien, Manifest und Reproduktion fest. Committe gezielt. Nutze einen normalen Fast-forward-Push des geprüften Commits nach main; bei Ablehnung wegen neuer Remotecommits neu vergleichen. Kein GitHub-Release/Tag nötig, wenn der bestehende Pages-/UniStore-Weg genügt.

Prüfe den zu diesem Commit gehörenden Pages-Workflow über GitHub Actions. Öffentlich lesbarer API-Weg: `https://api.github.com/repos/biolooo/VitarenewFlow/actions/runs`; bei veraltetem API-Cache einen frischen Queryparameter nutzen. Erfolg gilt erst bei conclusion=success und tatsächlich abrufbaren passenden Bytes. Ein frischer 404 während Deployment ist kein fertiges Ergebnis. Bei Fehlern Diagnose und gezielte Korrektur statt Endlosschleife.

Führe [scripts/verify_public.py](scripts/verify_public.py) mit `--root <Repo> --version <Zielversion> --output <lokaler Nachweis>` aus. Es liest alle aktuellen Assets einschließlich notes.txt und vier Kanäle über die öffentlichen URLs und vergleicht Bytes mit dem lokalen Paket. Erst dann Download/OTA als verfügbar melden. Einen Nachweis zu Commit, Workflow, Hashes und verbleibender Hardwareabnahme in docs/release-evidence und HANDOFF/STATUS/Release-Dokument führen. Ein anschließender reiner Nachweiscommit darf mit `[skip ci]` die unveränderten Payloads ohne redundantes Deployment dokumentieren.

Antworte knapp auf Deutsch: Version, App-Menüpfad zum Update, [Installationsseite](https://biolooo.github.io/VitarenewFlow/updates/install.html), relevante Neuerungen und offene Konsolenabnahme. HOME-Shortcut lädt denselben SD-3DSX-Pfad wie OTA; keine neue Shortcut-CIA bei jedem Update erforderlich.
