# VitarenewFlow 0.12.10 – HOME nach Modulstart im Autostart

Hardware 0.12.9: Das Modul startet, aber HOME blockiert Eingaben bei weiterlaufender Musik. Protokoll: HOME fehlt, NS-Neustart akzeptiert.

Korrektur: NS_RebootToTitle verwendet für normale native CTR-Spiele auf New 3DS den rohen FIRM-Speichertyp 6, auf Old 3DS weiterhin 0. Das entspricht Lumas configureHomebrewAutobootCtr. Der Konfigurationswert autoboot_3ds_app_mem_type=0 ist ein Index und bleibt korrekt; die SD-Konfiguration wird nicht geändert. DS-ROMs und physische DS-Karten behalten ihren bisherigen Übergabepfad. Bei fehlgeschlagener Modellabfrage wird vor dem Grafik-Abbau abgebrochen. Protokolle enthalten Modell und tatsächlich übergebenen Speichertyp.

Quelle: https://github.com/LumaTeam/Luma3DS/blob/master/arm9/source/deliver_arg.c . Die Wirkung auf den HOME-Freeze ist noch auf Hardware zu bestätigen.

## Update und Abnahme
- [ ] SD sicher auswerfen und in FBI /cias/VitarenewFlow.cia installieren. Version 0.12.10 prüfen.
- [ ] Autostart aktivieren und die Konsole komplett neu starten.
- [ ] Betroffenes 3DS-Modul direkt aus dem automatisch gestarteten Flow öffnen.
- [ ] Nach 30 Sekunden HOME drücken: HOME erscheint und Eingaben funktionieren.
- [ ] Spiel fortsetzen, kurz spielen, erneut HOME drücken und Spiel schließen.
- [ ] Flow erneut öffnen und denselben Ablauf zweimal ohne Ausschalten wiederholen.
- [ ] Konsole neu starten, Modulstart plus HOME erneut testen.
- [ ] Digitales 3DS-Spiel aus kalt gestartetem Autostart-Flow öffnen: HOME, Fortsetzen und Schließen testen.
- [ ] Autostart ausschalten und Konsole neu starten: Modul plus HOME weiterhin korrekt.
- [ ] Ein DS-Spiel und ein Game-Boy-Spiel starten, vorhandenen Spielstand laden.
- [ ] HOME aus kalt gestartetem Flow, POWER und Zuklappen/Aufklappen weiterhin ohne Freeze.

Bei Fehler: Spielname, Modul/digital, Autostart an/aus und erster Start nach Neustart oder Wiederholung notieren. SD zurückgeben. Erwarteter Log auf New 3DS: new3ds=1, appmemtype=6. Ein erfolgreicher IPC-Aufruf beweist noch keinen funktionierenden HOME-Rückweg.

Lokale Prüfungen: tatsächlicher Launch-Code mit gemockten Systemdiensten (New/Old 3DS, Modul/digital, vorhandenes/fehlendes HOME, Modellabfrage-Fehler, abgelehnte Übergabe und UI-Wiederherstellung), ARM-Build mit Warnungen als Fehler, CIA-Struktur/Inhalt/Dienste, Installer und Paketprüfung. Konsolenabnahme steht aus.
