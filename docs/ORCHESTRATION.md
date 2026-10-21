# Langfristige Multi-Agent-Arbeit

Gilt für beauftragte Arbeit, nicht als Freigabe des Backlogs. Die lokale Migration ist durchgeführt; dieses Repository ist die Integrationswurzel. Zustand steht in STATUS, Herkunft und Prüfungen in MIGRATION.

## Rollen und Aufträge

Orchestrator versteht Anforderungen, priorisiert mit dem Nutzer, schneidet unabhängige Aufträge, vergibt Dateien, prüft Ergebnisse und integriert nach Review. Er hält knappen Status: Auftrag, Basis, Verantwortliche, Blocker, nächster Abnahmepunkt. Er schreibt nicht gleichzeitig unkoordiniert in dieselben Dateien.

Anfangs ein Implementierer und ein unabhängiger Prüfer, dieser zunächst lesend. Mehr Implementierer erst bei unabhängigen Interfaces/Dateien. Agenten nur bei ausdrücklichem Delegationsauftrag oder entsprechender anwendbarer Regel der beauftragten Phase. Keine neuen benutzerseitigen Chats ohne Nutzerwunsch; der vom Nutzer gewünschte Flow-Orchestrator ist bereits angelegt. Die anfängliche Dokumentationsphase ist abgeschlossen. Aktive Entwicklungsaufträge erlauben die in AGENTS beschriebene begrenzte Delegation.

```text
ID / Ziel / konkrete Nutzeranforderung:
Basis: Repository, Commit UND gesicherter Arbeitsbaum; Abweichungen:
Erlaubte Dateien / exklusive Eigentümerschaft:
Nur lesbare Abhängigkeiten / Interfacevertrag:
Nichtziele und verbotene Aktionen:
Nachweise: relevante lokale Tests, Diff, Fehlerfälle, Rückweg:
Hardwareplan: Version, Modell, Region, Autoboot, erwartetes Ergebnis:
Haltpunkt / offene Entscheidung:
Rückgabe: Dateien, Verhalten, tatsächlich ausgeführte Tests, Grenzen:
```

PLANS oder Vorschläge lösen keinen Auftrag aus. Autorisierung für Veröffentlichung/SD-Übertragung getrennt behandeln; kein automatischer Release nach Tests und keine Freigabe durch einen fremden Agenten.

## Isolation, Review, Integration

1. Vor Worktrees den schmutzigen Basisstand klären/sichern. f8851da allein lässt viele Implementierungen weg. Worktrees kopieren keine uncommitteten Änderungen. Nicht automatisch stashen/resetten.
2. Basis reproduzierbar benennen; nach Sicherungsauftrag getrennte Worktrees/Branches. In einer einzigen Arbeitskopie nur ein Schreibender und lesender Prüfer.
3. main.c und ui.c exklusiv einer Aufgabe geben. Module können getrennt sein, deren Integration serialisieren. Auch Versionen, Katalog, Releasekanäle, Tests und Datenformate brauchen klare Zuständigkeit.
4. Implementierer liefert fokussierten Diff, echte Befehle/Ergebnisse, Fehlerfälle, nicht ausgeführte Hardwaretests, Lizenz-/Datenherkunft und Rückweg. Nutzeränderungen erhalten.
5. Prüfer beurteilt Quellstand/Intent, Regressionen, IO/Races/APT/NAND-Risiken und relevante Tests. Extrahierte Funktionsfixtures gegen aktuelle Quelle prüfen; keine bloß spiegelnden Tests.
6. Orchestrator prüft alle Ergebnisse und integriert kontrolliert. Konflikte nicht pauschal mit ours/theirs lösen. Keine ungeprüften übergreifenden Schreibzugriffe.
7. Lokal geprüft, veröffentlicht und auf Konsole bestanden getrennt führen. Exakt ausgeliefertes Quellpaket bauen, Payload-/Kanalbytes prüfen; Download-Erfolg ist kein Spielstart-/Power-/NAND-Nachweis.

## Erneuerbare Übergabe

Kein Chat hat garantierten unbegrenzten Kontext. Nach Meilensteinen HANDOFF knapp erneuern: Basis, Aufgabe, Dateien, Nachweise, letzte Entscheidung, Konflikte, nächste Lesereihenfolge. STATUS hält Abnahme, DECISIONS Entscheidungen. Nicht jede Agentenaufgabe mit gesamter Historie beladen.

Bei Kontextverlust zuerst Dokumente und aktuelle Arbeitskopie lesen. Erinnerungen/alte Protokolle sind Hinweise, keine heutige Laufzeitbestätigung. Neuer Orchestrator startet lesend und übernimmt kein unausgesprochenes Veröffentlichungsmandat. Unbekannte Historie bleibt unbekannt.
