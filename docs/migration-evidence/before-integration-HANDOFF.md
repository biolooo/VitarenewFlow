# Einstieg für einen neuen Orchestrator

Dokumentationsaufnahme 2026-10-21. **Kein Featurestart vor eindeutigem Quellstand.**

## Lesereihenfolge

1. [PROJECT-COPY](../PROJECT-COPY.md), [AGENTS](../AGENTS.md): unabhängiger Snapshot, ungesicherten Stand erhalten.
2. [STATUS](STATUS.md): Funktions-/Abnahmestand und vorgeschlagene Prioritäten.
3. [MIGRATION](MIGRATION.md): Quellen/Diffs, Werkzeuge, noch nicht ausgeführte Schritte.
4. [ARCHITECTURE](ARCHITECTURE.md), [DECISIONS](DECISIONS.md): Datenflüsse und Entscheidungen/überholte Ideen.
5. [ORCHESTRATION](ORCHESTRATION.md), optional [PLANS](../PLANS.md).
6. [Belegübersicht](migration-evidence/README.md).

## Kurzstand und Konflikte

Ziel `C:/Users/subje/Documents/Code/vitarenew-flow`: main/f8851dad9c14f96d8243fdd130a5d72d4d99548c, viele ungesicherte Änderungen, Launcher .32. Originalwurzel `C:/Users/subje/Documents/Codex/2026-10-13/kan`: Launcher .33, 100 C/H identisch mit Repository-Staging und .33-Quell-ZIP. Im Ziel sechs C-Dateien plus Makefile anders und .33-Releaseordner fehlt.

Desktop Ziel/Repo .4.1 entspricht dem veröffentlichten Desktop-Paket; separate alte Desktopkopie/Snapshot .4.0 nicht darüberkopieren. Empfehlung: Ziel als künftige Integrationswurzel, .33-Launcher gezielt nach Review ergänzen, .4.1 Desktop erhalten. Noch nicht verbindlich bestätigt/übernommen.

Keine ausdrückliche Abnahme .27–.33. N64-Rücksprung der neuen Konsole ohne Logs offen; Startanimation noch vorhanden trotz Entfernungswunsch; originale Standardmusik offen. Weitere UI-Wünsche sind Backlog, kein Auftrag.

AGENTS.md/PROJECT-COPY.md und Snapshot ignoriert; .gitignore unverändert. Regeln später gezielt versionieren. Keine Secrets, ROMs, Saves, SD-Backups/private Musik übernehmen. Historische README/ROADMAP/RELEASE-STATUS-Texte sind erhalten und markiert, keine Stable-Freigabe.

## Direkt kopierbarer Startprompt / lesende Akzeptanz

```text
Übernimm zunächst nur Bestands- und Verständnisprüfung für VitarenewFlow.
Ziel: C:\Users\subje\Documents\Code\vitarenew-flow
Legacy: C:\Users\subje\Documents\Codex\2026-10-13\kan
Lies PROJECT-COPY.md und AGENTS.md, dann docs/HANDOFF.md, STATUS.md,
MIGRATION.md, ARCHITECTURE.md, DECISIONS.md und ORCHESTRATION.md.
Dieser lesende Einzelauftrag startet keine Agenten/Chats, übernimmt keine
Quellen, führt keine Builds/Tests mit Schreibeffekten aus und verändert
weder Git, SD, System noch Veröffentlichungen. Keine Secrets lesen/kopieren.
Prüfe lesend HEAD/Branch/Status/Ignore-Regeln gegen migration-evidence.
Reproduziere Hash-/Inhaltsvergleich der 100 Launcher-C/H-Dateien und Makefiles,
vier Desktopvarianten und des .33-Quellarchivs. Bestätige oder widerlege
sechs Launcher-Diffs, Ziel .32, Legacy .33 und Desktop Ziel .4.1 vs .4.0.
Prüfe Release-Hashes sowie neue/fehlende Dateien und Änderungen seit Aufnahme.
Keine Test-Generatoren starten; einige patchen Quellen/Versionen/alte Pfade.
Erkläre 3DSX vs native CIA vs Shortcut, HOME/POWER/Autoboot, DS-Runtime vs
Paketprüfung, TWL-Indikation, Desktop-Reparatur und offene Hardwaregates.
.27–.33 nicht als hardwarebestanden behandeln. Keine neue Veröffentlichung.
Berichte Befunde, Widersprüche, Basisempfehlung und minimalen nächsten
Sicherungs-/Quellabgleichauftrag. Stoppe vor Sicherung, Git-Operationen,
Implementierung und Featurestart, bis die autoritative Basis bestätigt ist.
```

Akzeptanz: Der neue Orchestrator reproduziert die Unterschiede selbstständig, kennt private Ausschlüsse, erhält fremde Änderungen und benennt offene Entscheidungen, ohne aus Code/Build Hardwarezusagen abzuleiten. Abweichende Livebefunde gehen vor diesen Dokumenten und werden erklärt, nicht automatisch repariert.

## Rekonstruktionsgrenzen

Sichtbare Nutzeranforderungen, Ablehnungen, einzelne Berichte, Code und lokale Artefakte sind gesichert. Frühe Versionszuordnung, vollständige Testkonstellationen und Zweitkonsolenmodell teilweise unbekannt. Verdichtete ältere Chatteile liefern nicht jede Toolausgabe. Logs/Releaseberichte nicht komplett neu ausgeführt; private SD-/Audio-Fixtures absichtlich nicht übernommen. Kein aktueller Konsolen-/GitHub-Livezustand in dieser Phase als bestanden geprüft. Ein anderer Chat kann neuere Entscheidungen kennen: vor Umschaltung mit dem Koordinator abstimmen.

Dieser Auftrag endet nach Dokumentation. Keine Quellübernahme, Git-Sicherung, weitere Agenten oder Implementierung freigegeben.
