# Belege zur Dokumentationsaufnahme

Erfasst 2026-10-21 aus lokalen Dateien/Git-Leseabfragen. Keine Builds/Anwendungstests, Git-Mutationen, SD-Operationen oder Veröffentlichungen ausgeführt. Berichte sind Vergleichsmaterial, kein Beweis für Hardwarefunktion.

- `scope.json`: Zeitpunkt, Scope, Ausschlüsse, .33-Quell-ZIP-Vergleich.
- `comparisons.json`: relative Dateimengen, gleiche/geänderte/neue/fehlende Dateien, SHA256 für Unterschiede, absolute Quell-/Zielwurzeln.
- `versions.json`: Makefile-/Manifest-Versionen und Hashes.
- `desktop-archive-comparison.json`: welche Desktopvariante den vier .4.1-Archivdateien entspricht.
- `development-inventory.json`: Hash/Größe ausgewählter Entwicklungs-/Werkzeug-/Test-/Releaseartefakte. SD-Backups und private Audiofixtures ausgeschlossen. Kein vollständiges privates Medieninventar.
- `absolute-paths.json`: konkrete Altpfadtreffer mit Datei/Zeile; kein automatisches Umschreiben.
- `git-before.json`: lokaler Branch/HEAD/Origin (keine Credentials), vollständiger anfänglicher Status beider Repositorykopien. Kein Fetch/Remoteabgleich.
- `protected-before.json`: SHA256 geschützter Nicht-Dokumentdateien, Ziel-Gitdaten und Legacy-Quellen für abschließende Unverändert-Prüfung. Keine Dateiinhalte/Secrets kopiert.
- `preserved-documents.json`: ursprüngliche und neue Hashes der historisch eingerahmten Dokumente. Alttexte vollständig erhalten, nicht als heute richtig ausgegeben.
- `validation.json`: nachgelagerte Dokumentations-/Pfad-/Schutzprüfung, kein neuer Anwendungstest.

Historische lokale Public-Prüfprotokolle und Tests bleiben im alten work-Verzeichnis; sie wurden nicht pauschal kopiert. Ein zukünftiger Orchestrator muss Liveänderungen seit dieser Aufnahme erneut feststellen.
