# Projektstatus und Abnahme

## About-Layoutkorrektur nach .36 — lokal

Frischer nativer Build und alle vorgesehenen Hostprüfungen bestanden: `.test-build/verification-20261021-211513`.

Nutzer meldete gedrängte About-Seite nach Hinzufügen des Reportbuttons. Identität, Version, VitaRenew-Zeile und Website stehen nun zentriert untereinander in einer gemeinsamen Karte. Der Meldebutton verwendet eine dezente eigene Zeile mit X-Tastenmarkierung statt einer vollflächigen Akzentfläche. Keine gedrängte horizontale Website-/Textzeile mehr. Report-/Zurück-Touchflächen unverändert. DE/EN-Layoutvorschau mit Inter-Schrift und den nativen Glyphenvorschüben visuell geprüft; kein Konsolenscreenshot oder Hardwarepass. Öffentliche .36-Pakete unverändert, noch nicht veröffentlicht.

Changelog für nächsten Release: Über & Unterstützung mit klarer Textanordnung und dezenterem Meldebutton überarbeitet (EN: Refined About & support layout and report button). Gitbasis 46e69ee, Remote-main vor Beginn 2f4356c; kein neuer PC-2-Remotecommit. Nicht gepushte PC-2-Arbeit bleibt hier unsichtbar.

Veröffentlichung .36 bestätigt: Releasecommit e417163, Pages-Lauf 39658148277 erfolgreich. Alle neun Paketdateien und vier Updatekanäle öffentlich bytegleich geprüft. Nachweis: [0.12.36](release-evidence/0.12.36.json). Receiver 0.2.0 gesund; keine .36-Konsolenabnahme. PC 2 kann den veröffentlichten main nach Prüfung eigener Änderungen abrufen. Die folgenden Vorbereitungsangaben sind datierter Verlauf.

## Freigegebene Bereitstellung .36 — 21.10.2026

Nutzer hat aktuellen Download einschließlich Veröffentlichung ausdrücklich genehmigt. Schema-2-Receiver 0.2.0 läuft gesund am bestehenden Endpunkt; Schema 1 und Schema 2 mit leerem Kommentar öffentlich per HTTPS/App-CA angenommen (201) und SQLite-Speicherung geprüft. Vorherige Konfiguration und Image 0.1.0 bleiben als Rückweg erhalten. Paket .36 wird aus verification-20261021-205317 veröffentlicht; abschließender öffentlicher Bytenachweis folgt separat. Keine .36-Hardwareabnahme.

HOME-Shortcut und Boothelfer laden `/3ds/3ds-library/3ds-library.3dsx`; OTA ersetzt genau diese Datei. Der korrekte Shortcut startet daher neue Versionen ohne neue CIA-Installation. Installation über den aktuellen UniStore, nicht über historische vollständige native CIA-Builds. Spieleinstallation DS/3DS und HOME-Icon-Hardwareprüfung sind im TODO erfasst; keine neue Spielequelle umgesetzt.

## Aktueller Auftrag: Reportbedienung und Diagnose .36

Lokal umgesetzt: regulärer Report im Unterstützungsmenü statt neben R; direkter Fallback bei unerwarteten Fehlern; automatische begrenzte Diagnose auch ohne Kommentar. Diagnose enthält Laufzeit, freien SD-Speicher, freien linearen RAM und die letzten acht erlaubten Aktionskategorien. Keine Namen, Pfade, Titel, Rohlogs oder eindeutigen Gerätekennungen. Fehlerdaten bleiben am Ereignis eingefroren; Y zeigt die Vorschau. Versand erfordert weiterhin bewusste Zustimmung.

Nutzer hat .35-Testversand bestätigt; realer Eingang verifiziert. Keine vollständige Hardwareabnahme. .36 ist nicht veröffentlicht; Schema-2-Receiverupgrade und öffentliche Ende-zu-Ende-Prüfung stehen vor dem Gerätetest aus. Frischer nativer .36-Build und alle vorgesehenen Hostprüfungen bestanden: `.test-build/verification-20261021-205317`. Receiver-Suite unter Linux: 26 Tests bestanden. Lesender Abschlussreview ohne Blockade; präzise gespeicherte Fehlerdaten werden beim Öffnen beibehalten. Windows-Python zeigte bei der absichtlich ungültigen Chunked-Anfrage einen Verbindungsabbruch statt auslesbarer HTTP400; die Linux-Zielumgebung besteht diesen Test. Changeloghinweise .36 DE/EN manuell aktualisiert; automatische Generierung bleibt der vorgemerkte TODO.

Aktualisiert nach lokaler Migration am 21.10.2026. Verbindlicher Arbeitsort ist dieses Repository. Launcher 0.12.33 und Desktop 0.4.1; alte Kopien unverändert. Lokale Migrationsprüfungen stehen in LOCAL-VERIFICATION.md, Quelle/Sicherung in MIGRATION.md. Hardwareabnahmen bleiben wie unten dokumentiert offen.

## Fehlerberichte — aktiver Entwicklungsstand

Am 21.10.2026 auf Basis Remote-main `815da5a` mit PC-2-Stand 0.12.34 begonnen. Launcher 0.12.35 ist der lokale neue Entwicklungsstand im isolierten Worktree `error-reports`. Strukturierter Berichtsdialog, bewusste Zustimmung, HTTPS-Versandmodul und eigener SQLite-Empfangsdienst sind implementiert. Öffentlicher Empfang im freigegebenen Server-Benutzerverzeichnis läuft; synthetischer HTTPS-Bericht gespeichert und 201 bestätigt. Endpunkt in .35 konfiguriert, OTA-Testveröffentlichung ausdrücklich freigegeben. Native Builds und Hosttests ersetzen weder öffentliche Ende-zu-Ende-Prüfung noch Konsolenabnahme. Desktop .4.1 unverändert. Details: [ERROR-REPORTS](ERROR-REPORTS.md).

## Veröffentlichung .35 bestätigt

Releasecommit 2ada1cd, Pages-Lauf 39651893937 erfolgreich. Alle neun .35-Paketdateien plus vier öffentliche Kanaldateien bytegleich mit dem lokalen geprüften Commit; alle acht Manifesthashes bestätigt. Der eigene Empfangsdienst ist aktiv und ein synthetischer HTTPS-Bericht gespeichert. Konsolenabnahme von .35 ist ausdrücklich noch offen. Neuester Einstieg ist Remote-main; ältere .34-/Migrationsangaben unten sind datierter Verlauf. [Nachweis](release-evidence/0.12.35.json).

## Aktuelle Basis

- Testreleasevorbereitung 0.12.34 auf `codex/test-release-0.12.34`: Versionen, OTA-/UniStore-Kanäle und Paket lokal konsistent, exaktes Quell-ZIP baut identische 3DSX/SMDH. Noch nicht veröffentlicht; main-Merge-Freigabe und öffentliche Prüfung ausstehend. Details: [RELEASE-0.12.34.md](RELEASE-0.12.34.md).

- Lokaler Backlogdiff auf `codex/backlog-improvements` ab `eba8f03`: Spiel-Startanimation/Toggle und künstliche Mindestwartezeit entfernt, About/Support erneuert und lokale versionsgebundene Neuerungen ergänzt (DE/EN). Nativer Build und erweiterter Hostrunner bestanden; noch nicht veröffentlicht oder auf Konsole abgenommen. Details: [BACKLOG-WORK.md](BACKLOG-WORK.md).

- PC 2 (21.10.2026): WSL2/Ubuntu und gepinnter devkitARM-Chroot eingerichtet; frischer 3DSX-/SMDH-Build, Quellankerprüfung und vorgesehene Hostfixtures bestanden. Desktop .4.1 mit 108 Selbsttests bestanden. Einzelheiten/Befehle in [PC2-SETUP.md](PC2-SETUP.md); keine zusätzliche Konsolenabnahme.

- Gesicherte Migrationsbasis: 100 C/H-Dateien identisch zu Legacy .33 und Releasequellarchiv, Makefile mit README-Buildfilter. Der darüberliegende lokale Backlogdiff verändert UI-/Menüquellen; er ist nicht bytegleich zum unveränderten öffentlichen .33-Quellpaket.
- Desktop .4.1 im Ziel erhalten; die separate .4.0-Kopie bleibt historisch.
- Lokales .33-Paket und vier Kanaldateien übernommen und geprüft, kein neuer öffentlicher Release.
- AGENTS/PROJECT-COPY werden versioniert. SDK, Sicherung, Snapshot und Prüfergebnisse bleiben extern/ignoriert.
- Den gesicherten Basiscommit mit `git log -1` und Abweichungen mit `git status` prüfen. Ein alter f8851da-Worktree repräsentiert diesen Stand nicht.

## Implementiert, nicht gleich vollständig abgenommen

- Kategorien 3DS, Homebrew, DS, GB/GBC/GBA gemeinsam, N64, SNES, NES, Virtual Boy; Touch-Coverflow, L/R, X-Infos, zweibildschirmige Suche mit drei Vorschlägen.
- Originalbanner-Caches, Cover-Fallback, Desktop-Coverpflege/-Autosuche/-Kompression, asynchrones Cover-Laden mit Prioritäten/Cache und Performance-Logs. Keine aktuelle belastbare FPS-Messung in dieser Aufnahme.
- Ausstehende CIA, Installationsdialog/Größe/ETA, geprüfte Installation, Installer-Entfernung nach Erfolg, Seed-Import/Reparatur.
- Native/DS/Homebrew/angepasste Emulator-Startwege und adapterabhängiges L+R+Y. Kein globaler Rückkehrhook für DS/CTR.
- DE/EN, Branding, Supportbereich, Autoboot/Fallback, explizite HOME-Vorbereitung, HOME-Ladeansicht, abbrechbarer POWER-Dialog.
- Eigener UniStore/Pages, OTA, In-App-Emulator- und Desktop-Downloads. Keine Spiele, BIOS oder Schlüssel enthalten.
- .31 DS-Runtime-Prüfung/Neuprüfung, .32 DS-Diagnose/TWL-Indikation, .33 Desktop-EXE/Altnamen/Prüfen/Reparieren (**.33 jetzt im Ziel**).
- Modulerkennung und geführte GodMode9-Kopie mit geprüftem Beleg. Desktop-Import und älterer optionaler Windows-Paketinstaller. Modul-Saves und PC-Sync sind nicht implementierter Scope.

## Offene Punkte und Prioritäten (Vorschlag, kein Auftrag)

- **TODO Gesamttest auf neuer Konsole (Nutzer 21.10.2026):** Nutzer testet die komplette Einrichtung auf einer frisch eingerichteten, kompatiblen Konsole. Modell/Region, installierte Flowversion, Startweg und Autoboot festhalten. Installation per UniStore, HOME-Shortcut, OTA mit weiterhin aktuellem Icon, Komponenten, vorhandene eigene Spiele je System, Speichern/Rückkehr, DE/EN, Reports mit/ohne Kommentar, WLAN-aus/Abbruch und HOME/POWER durchgehen. DS-/3DS-Direktinstallation erst nach Implementierung abnehmen. Keine CFW-/NAND-Änderung oder gesunde Konsole künstlich beschädigen; Ergebnisse ausdrücklich einzeln notieren.


- **TODO Spieleinstallation (Nutzer 21.10.2026):** DS- und 3DS-Spiele direkt aus dem Flow-Menü auf der Konsole einrichten/installieren. Bestehende lokale CIA-Installation berücksichtigen; DS-Dateistart und echte 3DS-Installation getrennt spezifizieren. Quelle, Dateiformate und Speicherorte vor Umsetzung klären. Dieser Eintrag beauftragt noch keine neue Downloadquelle oder Spieleverteilung.
- **TODO HOME-Starticon prüfen/abnehmen (Nutzer 21.10.2026):** installierter HOME-Shortcut soll den aktuellen Homebrew-Launcher von der SD laden, damit OTA ohne Neuinstallation des Icons greift. Bestehenden Boot-/CIA-Pfad und aktuelle Installation prüfen; Konsolenabnahme nach Update offen.


- **TODO Changelog/Update-Neuerungen (Nutzerauftrag 21.10.2026):** Changelog bei jeder nutzerrelevanten Änderung aktualisieren. Daraus für jede Updateversion automatisch ein bis drei kurze, zutreffende Stichpunkte erzeugen und im Updatebildschirm zur angebotenen Zielversion anzeigen (DE/EN). Eine gemeinsame versionierte Quelle für Appanzeige und Releasehinweise verwenden; keine veralteten fest eingebauten Texte oder erfundenen Änderungen. Paketierung soll bei fehlenden bzw. nicht zur Zielversion passenden Hinweisen abbrechen. Bisherige .35-Stichpunkte sind noch manuell hinterlegt; Automatisierung ist als nächster Featurepunkt vorgemerkt, hier noch nicht implementiert.

1. **Migration lokal durchgeführt:** verbindliche Basis Launcher .33/Desktop .4.1, Sicherung und neue isolierte Prüfbefehle vorhanden; keine Featurearbeit automatisch gestartet. Nachweise in MIGRATION und LOCAL-VERIFICATION.
2. **P1 Hardwarefeedback .27–.33:** ausdrücklich ausstehend. Primärmodell New 3DS XL; Zweitkonsole unbekannt. Keine weitere Featurekette als Ersatz für Abnahme.
3. **P1 Startanimation entfernen:** lokal umgesetzt und gebaut/geprüft. Toggle, Logo und Mindestframe-Wait entfernt; alte appearance.txt bleibt unangetastet. Echte Lade-/Seed-Anzeigen und Originalbanner bleiben. Konsolenabnahme offen.
4. **P1 N64-Zweitkonsole:** vom Nutzer am 21.10.2026 ausdrücklich bis zu erneutem Test/Logs zurückgestellt. Kein neuer Fix oder Pass behauptet; Diagnosepfade in acceptance/BACKLOG-PC2.md.
5. **P1 Standardmusik:** originale HOME-Musik ohne PC gewünscht, nicht fertig. Synthetischer Fallback verworfen. Ohne gültiges Theme aktuell Klicks; .30 Theme-Decoder lokal verbessert, Hardwarefeedback fehlt.
6. **P1 1.0-Vertrieb:** saubere Zweitkonsole komplett via UniStore einrichten und Emulator-/Shortcut-/OTA-Fehlerfälle abnehmen. Lizenz-/Quellpaketzuordnung der Adapter prüfen. Eigener UniStore ist keine Universal-DB-Aufnahme.
7. **P2 UI:** About/Support und ausklappbare lokale Neuerungen der installierten Version umgesetzt, DE/EN-/Menü-/Touchgrenzen lokal geprüft; Bildschirmabnahme offen. Das VFUPDATE1-Manifest liefert keine Remote-Release-Notes, unbekannte Versionen erhalten keine erfundenen Texte. HOME-/Shortcut-Rückwege weiterhin auf Hardware prüfen.
8. **P2 Banner/Performance:** neue reale Parser-/Bibliotheksmodultests für gültige/defekte/trunkierte Banner und 1024 eindeutige Coverpfade bestanden. Keine GPU-/Fullscan-/FPS-Abnahme; Coverqualität, universelle Bannerkonvertierung und Geräteleistung bleiben offen.
9. **Später entscheiden:** PC-Emulator-/Save-Sync, Save-Konvertierung und Modul-Saves bleiben auf ausdrücklichen Nutzerwunsch (21.10.2026) offen. Kopie eines Moduls übernimmt nicht automatisch dessen Save.
10. **Schulden:** große main.c/ui.c; extrahierte Testkopien können driften; alte Generatoren patchen Quellen; stale SDK-Projektkopien; statische Adapterversionen sind kein Versprechen jeweils neuester Upstreams. Historische Installations-/Fallback-Dokumente nicht als Iststand lesen.

## Explizite Nutzerberichte

Versionen nur dort nennen, wo zuordenbar; kein erfundenes Datum oder Testmodell.

| Feature/Stand | Modell / Autoboot | Expliziter Bericht | Reichweite |
|---|---|---|---|
| Frühe Demo | Primärkonsole später New 3DS XL benannt / unbekannt | „Alles tests bestanden“ | Früher Meilenstein, kein aktueller Komplettpass |
| GBA/Save | New 3DS XL / unbekannt | „GBA Spiele starten ... gespeichert“ | Historischer Pfad akzeptiert |
| DS/GBA/SNES/VB, später N64/NES | Primärkonsole / unpräzisiert | Einzelstarts positiv; N64 „Funktioniert jetzt“; NES später HOME statt Flow | Keine neue gemeinsame Matrix |
| Cover/Steuerung/Menü | Primärkonsole / unbekannt | „Cover stehen richtig Rum“, „L/R und X funktionieren“, „Das neue Menü gefällt mir, alles funktioniert“ | Ältere UI, Buildnummer nicht eindeutig |
| Sprache | Primärkonsole / unbekannt | „Sprachwechsel funktioniert jetzt und alle Tests bestanden“ | Neue Texte separat prüfen |
| CIA/Seed | Primärkonsole / teils an | Installation wiederholt positiv, „Seed repair hat super funktioniert“ | Frühere Installation/Seed, nicht alle späteren Starts |
| DS mit/ohne Autoboot | New 3DS XL / beide | White-Screen, zweiter Versuch/Neustart teils erfolgreich, HOME wieder möglich | Sporadischer Fehler real |
| CTR-Karte + HOME, alte Versuche | New 3DS XL / an | Spiel läuft, HOME friert Eingabe bei Musik; frühere Fixes erfolglos | NS-/Memory-Mode-Hypothesen widerlegt |
| Kleiner HOME-Test | Primärkonsole / Coldboot-Kontext | Nutzer drückte nur START, nicht HOME | Registrierung im Log beweist HOME-Rückkehr nicht |
| Integrierter expliziter HOME-Start | Primärkonsole / vorangehender Autoboot-Kontext | „Ja funktioniert alles“ | Positiv, genaue Versions-/Karten-/POWER-Matrix unbekannt |
| Klick/Lautstärke | Primärkonsole / unbekannt | Klick akzeptiert, Musik zu laut | Spätere Audioabnahme fehlt |
| OTA | Primärkonsole / unbekannt | „Update ... super Funktioniert“ | Grundweg bestätigt, nicht jeder neue Payload |
| Universal-Updater-Installation | Zweitkonsole Modell unbekannt / unbekannt | „Installation ... funktioniert, die App startet“ | Basiseinstieg, nicht komplette Einrichtung |
| N64 nach Installation | Zweitkonsole / unbekannt | Blackscreen, zurück zu Flow | Offen, Problem-Logs fehlen |

## Jüngste versionsgenaue Nachweise

Die folgende historische Versionsmatrix beschreibt frühere Feature-/Publicprüfungen; neue lokale Migrationsprüfungen stehen separat in LOCAL-VERIFICATION.md. Öffentliche Downloads wurden nicht erneut live geprüft. Release-README ist Autorenbericht, keine Nutzerabnahme.

| Version | Implementierung / lokale Nachweise | Veröffentlichung | Hardware |
|---|---|---|---|
| .27 | APT POWER quittieren, B/Touch/Rearm; work/power-v01227 | Testrelease laut damaligen Protokollen | Modell/Autoboot nicht berichtet; ausstehend |
| .28 | Theme-Auswahl/BCSTM-Endblock/Schleifen; work/music-v01228 | Testrelease | Nutzer meldete Musik weiter defekt; .30 ersetzt Decoderdiagnose |
| .29 | HOME-Ladeanzeige vor APT, Runtimefälle; work/home-loading-v01229 | Testrelease | Keine explizite Abnahme |
| .30 | Kurze ADPCM-Endframes, synthetische Tests und private Theme-/FFmpeg-Gegenprobe; work/music-v01230 | Testrelease, private Musik nicht enthalten | „Während ich die Musik teste“ ist kein Pass |
| .31 | Gemeinsamer DS-Runtimecheck, Cache/Prüfen; work/runtime-v01231 | Neun Assets + vier Kanäle geprüft | Keine explizite Abnahme |
| .32 | Reduziertes Fenster, Indikation/Consent/Revalidation, 128 Kombinationen; work/twl-v01232 | Öffentliche Testversion geprüft | „Während ich das teste“ ist kein Pass; keine echte NAND-Reparatur getestet |
| .33 | Desktop-EXE/Altnamen/IO/Cache/Touch/X, acht DE/EN-Vorschauen; work/desktop-v01233 | Neun Assets + vier Kanäle bytegleich geprüft | Keine explizite Abnahme; lokal übernommen |

## Nächste Hardwareabnahme (eigener Auftrag nach Basisentscheidung)

Pro Fall Version, Modell, Region, Luma, Startkontext und Autoboot an/aus notieren. POWER→B→POWER, HOME vor/nach erstem nativen Start, CTR digital/Karte, DS, Homebrew, L+R+Y je Adapter. Theme kalt/Loop/Suspend; Standardmusik als bekannte Lücke ausweisen. Auf gesunder Konsole DS-Diagnose nur prüfen/abbrechen, kein START+A und keine künstlichen Beschädigungen. Desktop Reparieren und PC-Start, Zweitkonsole kompletter UniStore→App→Emulator/Desktop→OTA-Weg. Ergebnisse je Fall statt pauschalem „alles geht“.
