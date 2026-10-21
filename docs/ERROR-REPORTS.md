# Nutzerinitiierte Fehlerberichte

## Freigegebene Bereitstellung .36 — 21.10.2026

Nutzer hat aktuellen Download einschließlich Veröffentlichung ausdrücklich genehmigt. Schema-2-Receiver 0.2.0 läuft gesund am bestehenden Endpunkt; Schema 1 und Schema 2 mit leerem Kommentar öffentlich per HTTPS/App-CA angenommen (201) und SQLite-Speicherung geprüft. Vorherige Konfiguration und Image 0.1.0 bleiben als Rückweg erhalten. Paket .36 wird aus verification-20261021-205317 veröffentlicht; abschließender öffentlicher Bytenachweis folgt separat. Keine .36-Hardwareabnahme.

HOME-Shortcut und Boothelfer laden `/3ds/3ds-library/3ds-library.3dsx`; OTA ersetzt genau diese Datei. Der korrekte Shortcut startet daher neue Versionen ohne neue CIA-Installation. Installation über den aktuellen UniStore, nicht über historische vollständige native CIA-Builds. Spieleinstallation DS/3DS und HOME-Icon-Hardwareprüfung sind im TODO erfasst; keine neue Spielequelle umgesetzt.

## .36: aktueller lokaler Entwicklungsstand

Der normale Zugang liegt nun im Menü Über & Unterstützung → Problem melden, per Touch oder X. Der bisherige dauerhafte Headerbutton neben R entfällt. Bei erkannten unerwarteten Fehlern wird zusätzlich ein Meldebutton angezeigt; während kritischer Übergaben bleibt er gesperrt. Ein kompletter Absturz kann keinen Dialog mehr anzeigen.

Schema 2 erweitert die bestehenden strukturierten Daten um Laufzeit in Sekunden, freien SD-Speicher in MiB, freien linearen RAM in KiB und höchstens acht Aktionskategorien. Verfügbarkeitsbits unterscheiden unbekannte Messungen von null. Bei Fehlern werden Werte und Fehlerbereich/-code gemeinsam eingefroren, bei manuellen Meldungen ohne vorherigen Fehler aktuell gemessen. Es werden keine Dateipfade, Spieltitel, Rohlogs oder Gerätekennungen erfasst. Ein optionaler Kommentar ist dafür nicht erforderlich. Y zeigt die technischen Daten vor dem zweistufig bestätigten Versand; automatische Sammlung bedeutet keinen automatischen Versand.

C-Formatter und neuer Receiver unterstützen Schema 2; der Receiver akzeptiert weiterhin Schema 1. 239 C-JSON-Vektoren und 26 Receiver-Tests lokal geprüft. Nicht aktiviert: der laufende öffentliche Receiver entspricht .35 und muss vor .36-Testversand aktualisiert werden. Keine .36-Veröffentlichung und keine .36-Konsolenabnahme. Die nachstehenden Header-/Schema-1-Angaben dokumentieren .35.

Auftrag 21.10.2026. Entwicklungsbasis ist GitHub-main `815da5a` mit den Änderungen von PC 2 und Launcher 0.12.34. Der ältere PC-1-main wurde nicht überschrieben. Entwicklung isoliert im angehängten Worktree `error-reports`; Launcherstand 0.12.35 und OTA-Testveröffentlichung vom Nutzer freigegeben. Noch kein Konsolentest.

## Bedienung und Daten

Der Touchbutton „Melden“ öffnet einen eigenen Dialog, auch im normalen Betrieb für Fehler ohne eigenes Popup. Beschreibung über X und die Systemtastatur; danach Vorschau und zweimalige ausdrückliche A-Bestätigung. B schließt oder bricht einen laufenden Versuch ab. Versand bleibt während Installationen, Downloads, Startübergaben, POWER und der destruktiven TWL-Bestätigung gesperrt. Nach einem Fehler kann später über den normalen Meldebutton der letzte strukturierte Vorfall verwendet werden. Fremde Betriebssystem-/Emulatorabstürze ohne laufende Flow-Oberfläche können keinen Button anzeigen.

Das feste Schema enthält Appversion, Bereich, Fehlercode, Modell/Region, Autoboot, Homebrew-/nativen Startkontext, Aktivitätskategorie und optional bis zu 512 UTF-8-Bytes vom Nutzer eingegebenen Text. Keine Rohlogs, SD-Pfade, Spieltitel, Titel-IDs, Seriennummern, Saves oder Schlüssel werden gesammelt. Freitext kann freiwillig personenbezogene Angaben enthalten: Tastaturhinweis bittet ausdrücklich, darauf zu verzichten. Numerische Codes sind nur verfügbar, wo der Fehlerweg einen liefert; 0 bedeutet nicht erfasst. Statusmeldungen werden niemals ungeprüft übertragen.

Es gibt keinen automatischen Versand, keine persistente Nutzerkennung und keine Hintergrundqueue. Netzwerkfehler behalten den Text im geöffneten Dialog, nicht auf SD. Abbrechen kann einen bereits auf dem Server gespeicherten Bericht nicht zurücknehmen; Wiederholen nach unklarer Bestätigung kann doppelte Berichte erzeugen. Das wird im Dialog angezeigt. Erfolg verlangt eine bestätigte HTTP-201-Antwort. Anfragen sind auf 4095 Bytes, Antworten auf 1024 Bytes und Netzwerklaufzeit auf 20 Sekunden begrenzt. HTTPS prüft Zertifikat/Host; keine Weiterleitungen, Proxies oder Zugangsschlüssel in der App. HOME/POWER beendet den Versandworker vor weiteren Netzwerkjobs.

## Empfangsdienst

Python-stdlib unter `receiver/error-reports`. Striktes Schema und Größenlimits, SQLite-Transaktion mit FULL-Durability vor 201, keine öffentlichen Lese-/Adminendpunkte und keine IP-/Bodylogs. Berichtaufbewahrung 30 Tage. IP-Limit bleibt nur im RAM; hinter dem Reverseproxy teilen sich Nutzer bewusst das Limit der Proxyadresse, Forwarded-Header werden nicht vertraut. Datenbankzugriff erfolgt nur lokal durch den Betreiber. Bei starkem öffentlichen Missbrauch braucht es zusätzlichen vorgelagerten Schutz; ein mitgelieferter Clientschlüssel wäre kein Geheimnis.

Compose bindet ausschließlich `127.0.0.1:4395`, persistent `./data`, nicht privilegierter Prozess, schreibgeschütztes Containerdateisystem. Öffentliche HTTPS-Route und konkreter Empfangs-URL sind vom Nutzer freigegeben. `FLOW_REPORT_ENDPOINT` ist `https://vitarenew-lager.tail6ccd16.ts.net:10000/flow-reports/v1/reports`. Der Dienst läuft unter `/home/maxim/apps/vitarenew-flow-reports`, Datenbank nur lokal in `data/reports.sqlite3`. Bestehende öffentliche/private Serverrouten bleiben erhalten.

Aktivierung 21.10.2026: Container gesund, Neustartregel unless-stopped, UID/GID 1000:1000, Datenordner 0700, Rootdateisystem read-only geprüft. Zusätzlicher Pfad am vorhandenen öffentlichen 10000-Listener; private Listener und bisheriger Root-Gateway unverändert. Synthetischer HTTPS-Bericht mit exakt dem CA-Bundle der App erhielt 201 und wurde einmalig in SQLite nachgewiesen. Keine Kundenberichte gelesen. Bei Störung nur die eigene Pfadroute oder den eigenen Container stoppen, nicht die gemeinsam genutzten Serverlistener zurücksetzen.

Erststart als Eigentümer des dedizierten App-Verzeichnisses mit `sh start-local.sh`: erstellt `data` mit Modus 0700 und startet Compose unter dessen unprivilegierter UID/GID. Nicht einfach Compose mit fehlendem Bindmount starten, sonst kann ein root-eigener Datenordner entstehen. Das Image selbst verwendet standardmäßig UID 65532; der Starter setzt ausdrücklich die passende lokale Dienst-UID. Keine Datenordner anderer Apps verwenden. Das übliche `/opt/apps` auf dem aktuellen Server benötigt für die erstmalige Verzeichnisanlage sudo; daher wurde das ausdrücklich freigegebene Benutzerverzeichnis verwendet.

## Prüfung und Hardwareplan

`scripts/verify-local.sh` baut aus der aktuellen Worktreequelle in frischem SDK-Staging, einschließlich bisheriger Regressionstests, Formatter-/UTF-8-/Grenztests und Transportfakes (Zustimmung, fehlender Endpunkt, TLS-Policy, 201/200/302/429/500, Fehler, Abbruch und erneutes Öffnen). `python -m unittest discover -s receiver/error-reports -p test_server.py` prüft lokalen HTTP-Empfang/Schema/Persistenz/Retention/Fehlerpfade isoliert. Echte öffentliche HTTPS- und Konsolenübermittlung sind eigene Nachweise, keine Folgerung aus Fakes.

Auf echter Konsole: Modell/Region/Version/Startweg und Autoboot an/aus notieren. Start-, Installations-, Update-, Komponenten- und DS-Fehler sowie normalen Supportfall öffnen; Meldebutton und Zurück prüfen. Text DE/EN einschließlich Umlauten bearbeiten, Vorschau bestätigen, Empfang in privater Datenbank vergleichen. WLAN aus, Serverfehler, Abbruch, erneutes Öffnen und HOME/POWER während Versand prüfen. Keine reale NAND-Reparatur und keine künstliche SD-Beschädigung. Desktop .4.1 ist unverändert; diese Integration betrifft den 3DS-Launcher.

## Ausgeführter lokaler Nachweis

Letzte vollständige Prüfung: `.test-build/verification-20261021-200859`, native 3DSX/SMDH gebaut, bisherige Regressionen und neue Formatter-/Transport-/Modal-/Übersetzungstests bestanden. Alle erfassten Build-Quellhashes stimmen danach mit dem Arbeitsbaum überein. 216 C-JSON-Vektoren vom tatsächlichen Python-Empfängerschema akzeptiert; Receiver-Suite 18 Tests bestanden. Kompakter lokaler Bericht `.test-build/error-reports-verification.json`. Dieser frühere Prüfschritt war noch lokal; die danach freigegebene echte HTTPS-Übermittlung und Bereitstellung sind oben separat dokumentiert. Konsolenabnahme weiterhin offen.

Letzte UI-Korrektur nach Review: Meldebutton während offener Suche ausgeblendet, damit Suchvorschläge erreichbar bleiben. Diese finale main.c wurde im eigenen Staging erneut kompiliert/gelinkt und per SHA256 zugeordnet. Finale lokale 3DSX: `.test-build/error-reports-final.3dsx` (kein ausgeliefertes Release).
