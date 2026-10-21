# 0.12.35 — Fehlerberichte, OTA-Testversion

Am 21.10.2026 hat der Nutzer die Aktivierung des eigenen öffentlichen HTTPS-Empfangs und die .35-OTA-Testveröffentlichung ausdrücklich freigegeben. Basis ist `815da5a` mit dem Stand von PC 2. Desktop .4.1 und alte Versionspakete bleiben unverändert. Noch keine Abnahme auf echter Konsole.

## Umfang

Touchbutton Melden, Beschreibung über Systemtastatur, sichtbare Diagnosevorschau und ausdrückliche Sendebestätigung. Nur begrenzte strukturierte Felder; keine Rohlogs/ROM-Namen/SD-Pfade. HTTPS mit Zertifikatsprüfung und begrenzter Laufzeit; 201 nur nach persistenter Speicherung beim eigenen Empfänger. Während Suche, Installation, Startübergabe, Download, POWER und TWL-Bestätigung ist der Button nicht verfügbar; nach Rückkehr wieder zugänglich. Desktop und abgestürzte Fremdemulatoren haben keine neue Meldeoberfläche.

## Empfang und lokale Nachweise

Receiver 0.1.0 läuft nicht privilegiert und mit privater SQLite-Datenbank auf dem Server im freigegebenen Benutzerverzeichnis. Öffentliche Route nur zusätzlich unter /flow-reports auf dem bisherigen HTTPS-Port 10000. Root-Gateway und private Serverlistener bleiben erhalten. Gesundheit, Neustartregel, Verzeichnisrechte und HTTPS mit dem eingebetteten App-CA-Bundle geprüft; synthetischer Bericht per 201 bestätigt und in SQLite nachgewiesen. Keine Kundenberichte gelesen.

Nativer Build und bisherige Regressionen sowie neue Formatter-/UTF-8-/Puffer-, Modal-, Übersetzungs- und Transporttests werden vor Paketierung aus dem aktuellen Stand ausgeführt. Receiver-Suite: 18 Tests. Das fertige Quell-ZIP wird anschließend frisch entpackt und daraus gebaut; 3DSX und SMDH müssen bytegleich zum Paket sein. SHA256.json benennt die ausgelieferten acht Assets. Boot-Helfer, Shortcut-CIA und GM9-Skript werden unverändert aus dem bisherigen Testpaket übernommen.

Veröffentlichung erst nach unabhängigem lesenden Review. Nach Push muss der passende Pages-Lauf erfolgreich sein und die öffentlichen Paket-/Kanalbytes müssen mit dem geprüften lokalen Stand übereinstimmen. Ein grüner alter Lauf genügt nicht. Diese Dokumentation selbst behauptet keinen Konsolenpass.

## Auf dem 3DS testen

Menü → Konsole & Start → Updates → Aktualisieren → Neu öffnen. Danach Version 0.12.35 prüfen. Melden öffnen, mit X eine kurze Testbeschreibung ohne private Angaben schreiben, Vorschau ansehen, A zum Prüfen und nochmals A zum Senden. Empfang als separaten Bericht prüfen. Danach Zurück/erneut Öffnen, DE/EN, Suchvorschläge, L/R, Spielstart, WLAN aus und HOME/POWER beim Versand testen. Modell und Autoboot an/aus mitmelden. Auf gesunder Konsole keine TWL-Reparatur ausführen.

Bei unklarer Empfangsbestätigung kann erneutes Senden einen doppelten Bericht erzeugen; Abbrechen nimmt bereits gespeicherte Berichte nicht zurück. Keine automatische Übermittlung. Der App-Updater erhält die vorherige 3DSX für den manuellen Rückweg. Kein Stable-Release und keine neue Boot-/NAND-Funktion.
