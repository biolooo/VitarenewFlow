# Lokale Komponentenprüfung und Quellzuordnung

Stand 21.10.2026, Backlogprüfung auf Basis `eba8f03`. Dies ist eine technische Bestandsaufnahme, keine abgeschlossene Distributionsfreigabe.

Alle 25 über `components-1/packages.json`, `retro-1/packages.json` und `desktop-1/package.json` erfassten lokalen Paketdateien stimmen mit Größe und SHA256 überein. Diese Prüfung belegt Dateiintegrität; sie belegt nicht, dass ein vorliegendes Quellarchiv exakt das veröffentlichte Binary reproduziert.

| Komponente | Vorhandenes Quellmaterial unter docs/updates | Begleittexte |
| --- | --- | --- |
| GBA/GB | components-1/gba-source.tar.gz | gba-LICENSE, gba-ADAPTER.txt |
| DS | components-1/ds-bootstrap-source.zip, ds-ntr-forwarder-source.zip, ds-yanbf-source.zip | ds-LICENSE.txt und getrennte Upstream-Lizenztexte |
| N64 | components-1/n64-source.tar.gz, picaGL-revamp-source.tar.gz, imgui-picagl-main-source.tar.gz | n64-UPSTREAM-copying.txt, n64-ADAPTER.txt |
| SNES | retro-1/snes-source.tar.gz | snes-NOTICES.txt; enthält eigene Snes9x-Bedingungen |
| NES | retro-1/nes-source.tar.gz, nes-runtime-source.tar.gz | nes-NOTICES.txt |
| Virtual Boy | retro-1/vb-source.tar.gz | vb-NOTICES.txt |
| Desktop | desktop-1/VitarenewFlow-Desktop-source.zip | LICENSE.txt, LIESMICH.txt |

## Festgestellte historische Textabweichungen

- components-1/README.md behauptet noch, dass SNES/NES/Virtual-Boy-Downloads deaktiviert seien. Im aktuellen Komponentenkatalog sind sie vorhanden.
- n64-ADAPTER.txt verweist auf `n64-library-source.tar.gz` und nicht enthaltene FIX-/TEST-6.1-Dateien. Das tatsächlich vorhandene Quellpaket heißt `n64-source.tar.gz`.
- gba-ADAPTER.txt nennt ebenfalls einen früheren Quellarchivnamen; vorhanden ist `gba-source.tar.gz`.

Diese ausgelieferten Texte und ihre Manifesthashes bleiben unverändert. Für das nächste ausdrücklich freigegebene Paket müssen Texte/Quellzuordnung zusammen korrigiert und sämtliche betroffenen Manifest-/Kataloghashes konsistent erneuert werden. Keine alte Hashliste auf neue Bytes umdeuten.

## Verbleibende Releasegates

Für jedes angepasste Binary die vollständige Buildumgebung, Revision, Änderungen und Quell-/Lizenzbegleiter zuordnen und reproduzieren. Die separaten Emulator-SDKs sind nicht durch den Launcher-Chroot ersetzt. Die GPL des Launchers ersetzt keine Fremdbedingungen; insbesondere die SNES-Hinweise separat beachten. Erst vollständige Zuordnung plus echte Geräteabnahme erlauben eine gesonderte Freigabeentscheidung.
