# VitarenewFlow 0.12.27

## Power-Menü nach Abbrechen erneut öffnen
- Beim Abbrechen wird jetzt auch der Power-Zustand im System quittiert (APT Utility 0 / ClearPowerButtonState), bevor der lokale Dialogzustand gelöscht wird. In 0.12.26 fehlte diese Systemquittierung; ein weiterer Power-Druck konnte deshalb ausbleiben.
- B und Touch-Abbrechen bleiben erhalten. Bei einer fehlgeschlagenen Systemantwort bleibt der Dialog offen und zeigt am Abbrechen-Button eine erneute Versuchsmöglichkeit. Der Rückgabecode wird im session.log protokolliert.
- Tatsächliche Beendigungsanforderungen des Systems werden nicht aufgehoben.

Nur Punkt 10 der zuletzt besprochenen Aufgabenliste ist in diesem Update geändert. Theme-Musik, HOME-Ladeanzeige, Startanimation, DS-Diagnose, Komponenten-Erkennung, Desktop-Manager, HOME-Shortcut, Support-Seite und Update-Neuerungen folgen separat.

## Testliste auf der Konsole
1. Mit aktiviertem Autostart starten, Power kurz drücken und B drücken.
2. Power erneut drücken: das Fenster soll wieder erscheinen. Dies fünfmal wiederholen.
3. Den gleichen Ablauf mit dem Touch-Button Abbrechen testen.
4. Aus Suche und Einstellungen öffnen und abbrechen: vorherige Ansicht und Auswahl bleiben erhalten.
5. Nach dem Abbrechen ein Spiel starten und HOME-Rückkehr prüfen.
6. Ausschalten und Neustart einzeln testen; anschließend den Ablauf ohne Autostart wiederholen.

## Lokal geprüft
Der echte bisherige Cancel-Helper aus 0.12.26 reproduziert in einem Modell des systemseitigen Power-Latches den verlorenen zweiten Tastendruck. Die neue tatsächliche Cancel-Funktion, Hauptschleife und Eingabeverarbeitung bestehen 20 OS-abhängige Wiederholungen mit 2000 Folgeframes, Touch, Fehlerrückgabe/erneutem Versuch, Vorrang von B, echten Close-Anforderungen und normaler HOME-Übergabe. Native Build und separater Quellpaket-Build erfolgreich. Keine Echtkonsolen-Abnahme vorweggenommen.

APT Utility 0 ist als ClearPowerButtonState dokumentiert: https://docs.mikage.app/APTAppletUtility/ (Spiegel der 3dbrew-Dokumentation). Die Original-Lizenz von libctru steht weiterhin in launcher/source/flow_apt.c. Keine Änderung an Luma oder der Boot-Konfiguration; Update über die .3dsx-Updatefunktion.
