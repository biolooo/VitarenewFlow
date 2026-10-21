# Projektstatus und Abnahme

Aktualisiert nach lokaler Migration am 21.10.2026. Verbindlicher Arbeitsort ist dieses Repository. Launcher 0.12.33 und Desktop 0.4.1; alte Kopien unverändert. Lokale Migrationsprüfungen stehen in LOCAL-VERIFICATION.md, Quelle/Sicherung in MIGRATION.md. Hardwareabnahmen bleiben wie unten dokumentiert offen.

## Aktuelle Basis

- 100 C/H-Dateien identisch zum Legacy-.33-Stand und Releasequellarchiv. Makefile nur um den nachgewiesenen README-Buildfilter ergänzt.
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

1. **Migration lokal durchgeführt:** verbindliche Basis Launcher .33/Desktop .4.1, Sicherung und neue isolierte Prüfbefehle vorhanden; keine Featurearbeit automatisch gestartet. Nachweise in MIGRATION und LOCAL-VERIFICATION.
2. **P1 Hardwarefeedback .27–.33:** ausdrücklich ausstehend. Primärmodell New 3DS XL; Zweitkonsole unbekannt. Keine weitere Featurekette als Ersatz für Abnahme.
3. **P1 Startanimation entfernen:** ausdrücklich gewünscht, nicht erledigt. `settings_menu.c` MENU_ANIMATION, `main.c` Flag/Prefs und `launchFrame<36` bestehen. Echte Lade-/Seed-Anzeigen und animierte Originalbanner erhalten.
4. **P1 N64-Zweitkonsole:** nach installiertem Emulator Blackscreen und Rückkehr zu Flow. Betroffene konkrete Logs fehlen. Modell, Adapterhash, Argumente und private ROM-Identität im gesonderten Diagnoseauftrag prüfen. Älterer Fehlender-Emulator-Fix löst dies nicht nachweislich.
5. **P1 Standardmusik:** originale HOME-Musik ohne PC gewünscht, nicht fertig. Synthetischer Fallback verworfen. Ohne gültiges Theme aktuell Klicks; .30 Theme-Decoder lokal verbessert, Hardwarefeedback fehlt.
6. **P1 1.0-Vertrieb:** saubere Zweitkonsole komplett via UniStore einrichten und Emulator-/Shortcut-/OTA-Fehlerfälle abnehmen. Lizenz-/Quellpaketzuordnung der Adapter prüfen. Eigener UniStore ist keine Universal-DB-Aufnahme.
7. **P2 UI:** About/Support modernisieren, ausklappbare kurze Update-Neuerungen. Einfacher Rückweg nach nativen Spielen teilweise via Shortcut vorhanden, Verhalten mit Autoboot/ohne Shortcut noch konkret prüfen.
8. **P2 Banner/Performance:** neue Spiele ohne .lbn, Coverqualität und große Bibliotheken prüfen. Vollständige automatische Banner-Konvertierung/FPS-Ziele nicht fertig oder freigegeben.
9. **Später entscheiden:** PC-Emulator-/Save-Sync, Save-Konvertierung und Modul-Saves. Kopie eines Moduls übernimmt nicht automatisch dessen Save.
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
