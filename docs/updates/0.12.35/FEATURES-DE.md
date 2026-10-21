# VitarenewFlow 0.12.35 — Fehlerberichte als Testversion

- Fehlerbericht per sichtbarem „Melden“-Button bzw. über Unterstützung öffnen.
- Technische Angaben prüfen und optional selbst beschreiben, was zuvor geschah.
- Erst die ausdrückliche Zustimmung und „Senden“ übertragen einen Bericht per HTTPS.
- Übertragen werden nur Appversion, Fehlerbereich/-code, Modell, Region, Autoboot,
  Startkontext, eine grobe Aktivitätskategorie und die freiwillige Beschreibung.
- Keine automatischen Rohlogs, Spieltitel/-pfade, Seriennummern, ROMs, Saves oder
  Schlüssel. Bitte auch in den freiwilligen Text keine privaten Daten eingeben.
- Kein automatisches Nachsenden. Ein Sendeversuch kann abgebrochen werden.
- Empfangsdienst: geprüfte JSON-Felder, begrenzte Anfragen, SQLite-Speicherung;
  reguläre Aufbewahrung 30 Tage, keine dauerhaften IP-/Header-/Anfrageprotokolle.

## Auf echter Konsole prüfen

Nach Veröffentlichung: Menü → Konsole & Start → Updates → Aktualisieren → Neu öffnen.
Die installierte Version muss 0.12.35 anzeigen. Kein Konsolenpass ist vorweggenommen.
Pro Fall Modell, Version, Startkontext, Region und Autoboot an/aus festhalten.

1. Deutsch/Englisch: Fehler melden bei Spielstart-, Update-/Komponenten- und
   SD-/DS-Fehlern sowie über Unterstützung erreichbar; Text und Touchflächen prüfen.
   Auf gesunder Konsole keine NAND-Beschädigung oder TWL-Reparatur provozieren.
2. Vor dem Senden technische Vorschau prüfen, optionalen Text eingeben/ändern.
   Zurück vor der Zustimmung sendet nichts. Ein Abbruch kann bereits gespeicherte Berichte nicht zurücknehmen. Ohne Zustimmung ist kein Versand möglich.
3. Mit WLAN genau einen eigenen Testbericht senden, Erfolgsanzeige und tatsächlichen
   Empfang im Betreiberzugriff abgleichen. Erfolg erst nach gespeicherter Annahme.
4. WLAN aus, falsches Datum/HTTPS, Serverfehler und begrenzte Anfragen prüfen:
   verständlicher Fehler, kein falscher Erfolg und kein automatischer Wiederholversand.
5. Während des Sendens abbrechen und erneut öffnen; Bedienung bleibt reaktionsfähig.
   Nach einem Netzabbruch kann der Server den Bericht bereits gespeichert haben.
6. HOME/POWER, Suspend/Resume, Spielstart und bestehende Reparatur-/Updateaktionen
   mit Autoboot an/aus prüfen. Ein Bericht darf keine gefährliche Aktion bestätigen.
7. Eingabegrenze und Umlaute prüfen. Keine echten privaten Daten für Tests verwenden.

Die .34-Verbesserungen bleiben erhalten. Originale HOME-Standardmusik, N64 und
Save-Sync werden mit dieser Version nicht neu implementiert. Keine Stable-1.0-Freigabe.
Das 3DSX-Update verändert keine Spiele, Saves oder Bootkonfiguration. Nicht während
der Installation ausschalten oder SD entfernen; vorherige 3DSX als manuellen Rückweg
unter /3ds/3ds-library/3ds-library.3dsx.previous.3dsx erhalten.

## Quellen und Begleiter

Nativer Build und Hostprüfungen müssen vor dem Packen bestanden sein. Dazu gehören
Berichtformat/UTF-8/Puffergrenzen, Zustimmung/Touch, HTTPS-Transport und bestehende
Komponenten-/TWL-/HOME-/POWER-/Suche-/Banner-/Menü-Fixtures. Hostmocks ersetzen keine
echte OS-/WLAN-/Konsolenabnahme. Receiver-Unittests separat ausführen.

Boot-3DSX, Shortcut-CIA und GM9-Skript stammen unverändert aus .34. Es gibt keine
neue Boot-/NAND-Funktion. Das Quellpaket enthält aktuelle Launcherquellen, Scripts,
Tests, Lizenzen, Projektdokumentation und den getrennten Empfangsdienst samt Compose.
Empfangsdienst nicht ungeprüft öffentlich starten. SDK/Emulator-SDKs separat;
RELEASE-BUILD.md und docs/PC2-SETUP.md beschreiben den Quellpaket-Build.
GPL-3.0-only für eigenen Code; Fremdkomponenten behalten ihre Bedingungen.
Keine Spiele, Saves, Schlüssel, Datenbanken, private Musik oder lokalen Nutzerdaten.
