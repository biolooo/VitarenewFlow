# Startzeit prüfen — Launcher 0.12.40

Ziel ist die Zeit vom Öffnen von Flow bis zu einem bedienbaren Spielemenü bei
großer installierter 3DS-Bibliothek. Die lokale Hostprüfung zählt tatsächliche
API-/Dateioperationen; sie liefert keine Startzeit auf einer Konsole.

## Vergleich auf derselben Konsole

Modell, Region, Flow-Version, SD-Karte, Zahl installierter Titel, Sprache,
Startweg (HOME-Shortcut oder Homebrew Launcher) und Autoboot an/aus notieren.
Zwischen verglichenen Starts dieselbe Bibliothek, SD und denselben Startweg
beibehalten. Kaltstart der Konsole und erneutes Öffnen in derselben Sitzung
getrennt dokumentieren. Für einen belastbaren Vergleich je Fall drei Starts
messen und den mittleren Wert verwenden.

1. Falls noch .39 installiert ist: mit Stoppuhr vom Öffnen des HOME-Icons bis
   zur ersten angenommenen Links-/Rechtseingabe messen. Erst danach aktualisieren.
2. Erster .40-Start ohne vorhandenen .40-Titelcache: denselben Zeitraum messen.
   Der erste Start liest Namen und Icons noch live; Bannerexporte und häufige
   Ladeframes entfallen bereits.
3. Flow anschließend mindestens zehn Sekunden ohne Eingabe offen lassen, damit
   der Cache gespeichert werden kann. Bei langsamer SD länger warten: ein festes
   Zeitlimit garantiert keine vollständige Speicherung. Maßgeblich ist die Zeile
   `Title cache: committed` in `sdmc:/3ds/3ds-library/performance.log`.
4. Flow regulär schließen und erneut über denselben Weg öffnen. Stoppuhr wieder
   bis zur ersten angenommenen Eingabe messen. Dies ist der warme Cachefall.
5. Zusätzlich Sprache wechseln, einen Titel installieren/aktualisieren/entfernen
   und die aktuelle Bibliothek prüfen. Ein explizites Aktualisieren der Liste
   liest Titeldaten absichtlich neu und ist kein warmer Startzeitvergleich.

Cacheaufbau startet frühestens nach einer Sekunde Leerlauf nach dem ersten
Menüframe. Pro weiterem Leerlaufframe werden höchstens vier Datensätze geschrieben;
nach zwei Millisekunden wird die Schleife zwischen Datensätzen unterbrochen.
Ein einzelner SD-Zugriff kann länger dauern. Für 1024 Titel sind daher mindestens
256 Schreibframes nötig (bei 60 FPS rund 4,3 Sekunden plus Leerlaufvorlauf),
bei langsamer SD oder Eingaben entsprechend mehr. Die zehn Sekunden sind eine
praktische Warteempfehlung, keine zugesicherte Obergrenze.

## Interne Messwerte

`performance.log` wird lokal ergänzt; die Datei muss nicht veröffentlicht werden.
Sie kann Titeldetails/Pfade aus anderen Diagnosebereichen enthalten und gehört
nicht pauschal in Git oder öffentliche Berichte. Für die Auswertung genügen die
folgenden Zahlenzeilen des betreffenden Starts:

- `Startup ready: total_ms=... titles_cache_hits=... metadata_reads=... entries=...`
  misst ab Eintritt in `main()` bis nach dem ersten Menüframe und Freigabe der
  beim Start gehaltenen Tasten. Sie umfasst keine Zeit vor `main()`, beispielsweise
  den HOME-Shortcut-/Loaderstart. Tasten beim Öffnen loslassen, um diese Messung
  nicht künstlich zu verlängern.
- `Startup titles: ... enum_ms=... cache_ms=... metadata_ms=...` trennt den
  installierten Titelbestand, Cachelesen und fehlende Metadaten.
- `Library scan: total_ms=...` umfasst auch die weiteren Bibliotheksquellen.

Stoppuhr und interne Messung beantworten unterschiedliche Teile der Startzeit.
Für den Nutzer ist die gesamte Zeit vom HOME-Icon bis zur Bedienbarkeit maßgeblich.

## Änderungen und Grenzen

Flow fragt den installierten Bestand weiterhin aktuell ab und prüft Titelversionen
gebündelt. Namen/Icons werden nur bei passendem Medium, Sprache und Titelversion
aus dem validierten Cache verwendet. Fehlerhafte/fehlende Einträge fallen auf
Livezugriffe zurück; Spiele und Spielstände werden nicht verändert.

Rohbanner für die spätere Desktopkonvertierung werden bei explizit geöffneter
Spielinfo exportiert, statt bei jedem Start alle Titel zu durchlaufen. Vorhandene
`.lbn`-Banner und ihre Anzeige bleiben erhalten.

Hostnachweis mit den tatsächlichen Scanfragmenten: 300 Titel kalt 300
Metadaten-/Produktcodeabrufe und acht Ladeframes; warm null Metadaten-, Produktcode-
und Bannerabrufe sowie zehn gebündelte Versionsabrufe. Bei 1024 warmen Titeln
32 Versionsabrufe, ebenfalls null Metadaten-/Produktcode-/Bannerabrufe.
Das belegt weniger blockierende Arbeit, keine konkrete Beschleunigung in Sekunden.
Konsolenabnahme und reale Startzeit bleiben bis zum Gerätetest offen.

## Native Stackprüfung

Der native CI-Build erfasst zusätzlich GCC-Stackberichte (`-fstack-usage`).
Nach dem Stackfix misst CI-Lauf `41451062292` statische Frames: `main` 10584 B,
`scan` 5912 B, `title_cache_read` 4872 B und Cachecallback 32 B. Der Reader hält
seinen serialisierten 4784-B-Record kurzzeitig auf dem Heap und fällt bei fehlendem
Speicher auf Livezugriffe zurück. Der Idlewriter ist eine eigene Funktion.
Die GCC-Berichte sind statische Compilerwerte; sie ersetzen keine Messung des
Stackhöchststands auf echter Konsole und enthalten keine externe Bibliothekskette.
