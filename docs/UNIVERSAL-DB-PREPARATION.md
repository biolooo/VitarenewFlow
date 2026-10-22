# Universal-DB: technische Vorbereitung für VitarenewFlow

Stand22.10.2026, Basis `c53d944`, Remote-main identisch, bestehende Nutzeränderungen zu Beginn keine. Auftrag: Einreichung vorbereiten, nicht absenden. Dies ist ein interner Prüfbericht zum eigenen Flow-Projekt, kein zur Übernahme bestimmter Universal-DB-Beitrag und kein PR-/Issue-Text. Es wurden weder Fork/PR/Issue noch GitHub-Release/Tag oder neue öffentliche Dateien erstellt.

## Aktuelle Regeln und Grenze der Vorbereitung

[CONTRIBUTING](https://github.com/Universal-Team/db/blob/master/CONTRIBUTING.md), [App Request](https://db.universal-team.net/app-request), [App Template](https://github.com/Universal-Team/db/wiki/App-Template), jeweils22.10.2026 gelesen. SHA256 der gelesenen CONTRIBUTING: `41d09ef45bb588d564737184d0faf261c64e9325cb1ca4b0a67349dab469efea`.

Universal-DB erlaubt KI-unterstützte Apps bei zutreffender Deklaration. Es untersagt KI-generierte Beiträge/Assets für die Datenbank selbst sowie KI-verfasste PR-/Issue-Beschreibungen. Deshalb hier bewusst kein generierter `source/apps/vitarenewflow.json`-Entwurf und kein Einreichungstext. Diese müssen vom Nutzer selbst oder den Maintainern nach deren Regeln erstellt werden. Die technischen Fakten unten dienen der eigenen Prüfung, nicht als fertiger kopierbarer Antrag.

Regeln betreffen unter anderem Rechte an mitgelieferten Inhalten, sinnvolle Funktion, 3DS/DS-Plattform und zutreffende KI-Deklaration. Öffentliche Quellen und GitHub-Releases sind Empfehlungen; ein GitHub-Release ist dort kein generelles Pflichtkriterium. Aufnahme/Fristen entscheidet das Universal-Team. Keine künstliche Pflichtversion1.0 aus den Regeln abgeleitet.

## Verifizierte Fakten zum eigenen Projekt

| Punkt | Befund |
|---|---|
| Name/Entwickler | VitarenewFlow / VitaRenew |
| Repository | https://github.com/biolooo/VitarenewFlow, öffentlich, Standardbranch main |
| Plattform | Nintendo3DS/2DS-Homebrew; kein eigenständiges DS-Programm |
| Aktueller Downloadstand | 0.12.38, öffentliche Testversion; keine vollständige aktuelle Hardwareabnahme |
| Eigene Lizenz | GPL-3.0-only laut README/LICENSE; Fremdkomponenten gesondert |
| Supportkanal | Öffentliches GitHub-Issue-Tracking aktiviert |
| KI-Nutzung | Umfangreiche KI-Unterstützung; eigener UniStore kennzeichnet bereits `llm_generation=yes`, nicht minor |
| ShortcutTitleID | `000400000F104200`; eigenständiger Loader, keine vollständige native Flow-App |
| ShortcutUniqueID | `0xF1042`, dezimal987202; kein Treffer im aktuellen Katalog unter unique_ids; keine globale Reservierung/Kollisionsfreiheit behauptet |
| Downloadweg bisher | GitHub Pages und eigener UniStore, noch keine GitHub-Releases/Tags |
| Bestehender DB-Eintrag/Antrag | Livekatalog und GitHub-Issuesuche nach VitarenewFlow ohne Treffer am Prüftag |

Öffentliche eigene Bilddateien bytegleich geprüft: [Icon48×48](https://raw.githubusercontent.com/biolooo/VitarenewFlow/main/launcher/branding/icon.png), [Banner256×128](https://raw.githubusercontent.com/biolooo/VitarenewFlow/main/launcher/branding/banner.png). Diese sind Branding, kein erfundener Konsolenscreenshot. Echte Screenshots für den Datenbankeintrag wären optional; Hardwarebelege fehlen weiterhin.

## Installationsverhalten, das die Aufnahme erhalten muss

Die installierte Shortcut-CIA lädt fest `sdmc:/3ds/3ds-library/3ds-library.3dsx`. Eine automatische Standardinstallation der CIA allein oder ein 3DSX an einem anderen Pfad ergibt deshalb keinen gleichwertigen Installationsweg. Der bestehende eigene UniStore lädt fünf Dateien, verschiebt vier an ihre vorgesehenen Orte und installiert die CIA. Autostart wird dabei nicht aktiviert.

| Vorhandenes Asset | Vorhandener Zielpfad / Verarbeitung |
|---|---|
| VitarenewFlow.3dsx | `/3ds/3ds-library/3ds-library.3dsx` |
| VitarenewFlow.smdh | `/3ds/3ds-library/3ds-library.smdh` |
| VitarenewFlow-Shortcut.cia | `installCia: /3ds/3ds-library/updates/universal-updater/VitarenewFlow-Shortcut.cia` |
| VitarenewFlow-Boot.3dsx | `/3ds/3ds-library/boot-dispatcher.3dsx` |
| VitarenewFlow-Copy.gm9 | `/gm9/scripts/VitarenewFlow-Copy.gm9` |

Quelldaten: `docs/updates/VitarenewFlow.unistore`, `launcher/forwarder/source/main.c`, `launcher/branding/application.rsf`. Mit einem Maintainer muss geklärt werden, wie dieser bestehende Mehrdateiweg im offiziellen Katalog erhalten bleibt. Keinen AI-generierten Customscript-Beitrag an Universal-DB einreichen. Als technische Alternative kann ein Maintainer den vorhandenen Installer prüfen/übernehmen oder der Nutzer den Eintrag selbst nach dem offiziellen Schema erstellen.

## Downloadinventar für eigene Releasevorbereitung

Alle Assets liegen unter `https://biolooo.github.io/VitarenewFlow/updates/0.12.38/`. Bisher kein GitHub-Release vorhanden. Ein zusätzliches GitHub-Release mit unverändert geprüften direkten App-/Begleitdateien würde den empfohlenen Standardworkflow erleichtern; Erstellung/Upload ist ein gesonderter externer Schritt, hier nicht erfolgt. Source-ZIP, Hinweise und Manifest müssen dabei zu derselben Version gehören.

| Asset | SHA256 |
|---|---|
| FEATURES-DE.md | `876a316067f2b0cdb3f43190dd587be99c96bc3e4951fc041b0cf45129352808` |
| README.md | `876a316067f2b0cdb3f43190dd587be99c96bc3e4951fc041b0cf45129352808` |
| VitarenewFlow-Boot.3dsx | `14d18b52fe4c55973f3664a34649a0ef6a5dda5440a2a408486a8684c29bf277` |
| VitarenewFlow-Copy.gm9 | `c1495d3a1575c000f41828ced398088bda115784eb0a57fe0a3539d5c1cab530` |
| VitarenewFlow-Shortcut.cia | `d8e53523627d300aa6954ea3864af0e062bea8f4c03cab34ea1a78e0260ef097` |
| VitarenewFlow-source.zip | `db7144c679ebd1e0ec1ff95a2e567a8f2e2d316d13f2487e3440eb9a9e97d4a2` |
| VitarenewFlow.3dsx | `6bc8b9735bcec49fcef778c010352369b4915336fcf7d61b3deea97a3a515abf` |
| VitarenewFlow.smdh | `dc1719fd3323242a8a8bb5d84caa91b24a69fb32c0b86fd39674e40143210ea9` |
| notes.txt | `648ad3db490ddc17e270439f5b488c3713c6fadf932750ca03338832629dfe05` |

## Vor der tatsächlichen Einreichung offen

- [ ] Frische Konsoleninstallation einschließlich HOME-Shortcut und einmaligem OTA-Test protokollieren; kein Buildpass als Hardwarepass ausgeben.
- [ ] Mitgelieferte und nachgeladene Fremdkomponenten anhand `docs/COMPONENT-AUDIT.md` und `licenses/NOTICES.md` vollständig zuordnen. Integrität ist nachgewiesen, vollständige Distributions-/Quellzuordnung nicht. Nicht als erledigte Freigabe darstellen.
- [ ] Für automatische Erfassung geeigneten Downloadweg bestimmen: empfohlenes GitHub-Release oder mit Maintainer abgestimmter Pages-/Customscript-Weg. Keine automatische CIA-only-Installation akzeptieren.
- [ ] Menschlich erstellten Eintrag über offizielles App-Request-Formular beziehungsweise Maintainer erstellen lassen; KI-Nutzung zutreffend angeben und nur passende Assets auswählen.
- [ ] Kurzen PR-/Issue-Text selbst verfassen. Kein von einem LLM formulierter Antrag.
- [ ] Tatsächliches Einreichen ausdrücklich beauftragen; erst nach Annahme darf Anleitung/Shop behaupten, Flow sei im Standardstore per Lupe auffindbar.

Lokale Nachweise der lesenden API-Prüfungen unter `.test-build/universal-db-preparation/live-check.json`. Öffentliche .38-Assets bleiben unverändert. Keine Kundendaten, Rohreports, ROMs oder Schlüssel in diesen Unterlagen.
