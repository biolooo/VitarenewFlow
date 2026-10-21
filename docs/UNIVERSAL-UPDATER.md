# Universal-Updater — vorbereitet, noch nicht veröffentlicht

## Zwei Wege

1. Eigener UniStore: Nutzer fügen später unseren Store per URL oder QR-Code hinzu. Dafür ist keine Aufnahme in Universal-DB nötig.
2. Standardkatalog Universal-DB: Nach einer getesteten Veröffentlichung eine Aufnahme beim Universal-Team beantragen. Eine Aufnahme ist nicht garantiert.

Geplante eigene Store-Adresse, derzeit **noch nicht live**:
`https://biolooo.github.io/VitarenewFlow/unistore/VitarenewFlow.unistore`

## Release-Vertrag

Ein freigegebenes GitHub-Release ab 1.0.0 erhält fünf eigenständige Assets mit stabilen Namen:

- `VitarenewFlow.cia`
- `VitarenewFlow.3dsx`
- `VitarenewFlow.smdh`
- `VitarenewFlow-Boot.3dsx`
- `VitarenewFlow-Copy.gm9`

Keine ROMs, kommerziellen CIAs, Spielstände, BIOS-Dateien oder privaten Schlüssel beilegen. Vor Veröffentlichung müssen die Lizenzen der tatsächlich ausgelieferten Komponenten vollständig sein.

Der eigene Store installiert bei der CIA-Option zusätzlich 3DSX/SMDH: Unsere Emulatoren benötigen die 3DSX für die Rückkehr. Ziel bleibt bewusst `/3ds/3ds-library/`, weil die bisherigen Adapter diesen Pfad verwenden. Eigene Cover, Saves, Sprachwahl und Boot-Dateien gehören nicht zum Update. Der eigene Store enthält keine Löschbefehle; die kleine CIA bleibt im Update-Cache, damit sie auch nach einem Fehler verfügbar ist. Emulator-Adapter sind separat; ein vollständiger Ablauf auf einer frischen SD bleibt ein Release-Testpunkt.

## Store erzeugen

1. Erst die finale Hardware-Abnahme bestehen lassen und das freigegebene Release veröffentlichen.
2. Release-Metadaten aus der offiziellen GitHub Releases API als JSON speichern; die fünf Assets herunterladen.
3. Den Generator mit einer gegenüber dem vorherigen Store erhöhten Revision ausführen:

```sh
python distribution/universal-updater/prepare_store.py --release-json release.json --assets release-assets --revision 1 --output prepared-store
```

Der Generator akzeptiert ausschließlich veröffentlichte stabile Releases ab 1.0.0 aus unserem Repository. Download-URLs, Dateigrößen und SHA256-Digests der lokalen Assets müssen mit den GitHub-Metadaten übereinstimmen. Er veröffentlicht nichts. Der Offline-Test prüft die Installationspläne, ersetzt aber keinen Universal-Updater-Konsolentest. Die SHA256-Prüfung erfolgt bei der Vorbereitung; sie ist keine zusätzliche Hash-Prüfung durch Universal-Updater zur Laufzeit.

Nach Prüfung die erzeugten `.unistore`/Info-Dateien und das vorbereitete Icon `VitarenewFlow.t3x` unter `docs/unistore/` veröffentlichen. Erst dann den QR-Code für die echte Adresse bereitstellen und die Projektseite um die Anleitung ergänzen.

## Standardkatalog später anmelden

Die aktuellen CONTRIBUTING-Regeln erlauben KI-unterstützt entwickelte Anwendungen, verlangen aber eine korrekte Kennzeichnung. Unser Projekt fällt wegen umfangreicher Code-Unterstützung und generierter Grafik unter **LLM generation: yes**, nicht minor.

Universal-Team untersagt KI-generierte Beiträge zum Universal-DB-Code sowie KI-formulierte Anfragen-/PR-Beschreibungen. Deshalb liegen hier **kein Universal-DB-Datensatz und kein Einreichungstext**. Die spätere Anfrage bitte selbst formulieren; Repository, getestetes Release, Plattform, Funktionsbeschreibung und KI-Kennzeichnung angeben.

Primärquellen:

- https://github.com/Universal-Team/db/blob/master/CONTRIBUTING.md
- https://github.com/Universal-Team/Universal-Updater/wiki/storeInfo
- https://github.com/Universal-Team/Universal-Updater/wiki/storeContent
- https://github.com/Universal-Team/Universal-Updater/wiki/downloadFile
- https://github.com/Universal-Team/Universal-Updater/wiki/installCia

## Abnahme

- [ ] Store-URL, Info-JSON und T3X liefern HTTP 200.
- [ ] Store per URL/QR hinzufügen; Name, VF-Icon und Version stimmen.
- [ ] CIA-Option installiert HOME-App plus benötigte 3DSX/SMDH.
- [ ] 3DSX-Option landet im kompatiblen Verzeichnis und startet im HBL.
- [ ] Emulatorstart und Rückkehr nach Universal-Updater-Installation funktionieren.
- [ ] Update auf höhere Version wird erkannt; Cover, Saves und Einstellungen bleiben unverändert.
- [ ] WLAN-Abbruch beim Download lässt die zuvor installierten App-Dateien unangetastet; kein falscher Erfolg.
- [ ] Zu wenig Platz/Installationsfehler erzeugt einen sichtbaren Fehler; kein irreführender Abschluss.
- [ ] Keine Boot-Datei oder fremde Anwendung verändert.

Beide Installationsoptionen liefern jetzt auch boot-dispatcher.3dsx und das GodMode9-Kopierskript. boot.3dsx und autoboot-installed.3dsx werden nicht durch den Store überschrieben. Bei einem Upgrade von privaten Testversionen bis 0.12.0 zuerst Autostart deaktivieren. 0.12.1 erkennt den gespeicherten aktiven Helfer unabhängig von der später heruntergeladenen Helfer-Version.
