# VitarenewFlow 0.12.11 – HOME vor nativem Spielstart laden

Hardwarebefund: 0.12.10 startet das Modul, HOME blockiert trotzdem. Der korrigierte Speichertyp hat das Problem nicht gelöst. Protokolle zeigen weiterhin den direkten NS-Neustart ohne vorhandene HOME-Sitzung.

Neuer Ablauf für native CTR-Titel (Modul, digital und Homebrew-CIA): APT_PrepareToDoApplicationJump aufrufen, damit APT HOME laden kann. Danach auf APT_IsRegistered(HOME)=true warten. Dieses Signal steht für die APT-Registrierung/Enable, nicht bloß für einen erfolgreichen Prozess-Start. Erst danach Grafiken freigeben und APT_DoApplicationJump ausführen. Auch im Autostart erfolgt kein direkter Neustart ins native Spiel mehr. Es gibt keinen unsicheren NS-Ausweichpfad für native Titel.

Die Vorbereitung zeigt einen Ladebildschirm. Nach spätestens 10 Sekunden ohne HOME-Registrierung bleibt die UI erhalten und zeigt eine verständliche Meldung. Nach Registrierung wird HOME auch in Flow wieder freigegeben; schlägt die eigentliche Übergabe fehl, kann HOME in dieser Sitzung genutzt werden. DS/TWL und Emulatorpfade bleiben getrennt.

Primärquellen: https://github.com/azahar-emu/azahar/blob/master/src/core/hle/service/apt/applet_manager.cpp (PrepareToDoApplicationJump, Enable) und https://github.com/devkitPro/libctru/blob/master/libctru/source/services/apt.c (aptScreenTransfer wartet ebenfalls auf APT_IsRegistered).

## Gezielte Testfolge
- [ ] In FBI /cias/VitarenewFlow.cia installieren und Version 0.12.11 prüfen.
- [ ] Autostart aktivieren und Konsole vollständig neu starten.
- [ ] Das betroffene 3DS-Modul starten. Es lädt nach der HOME-Vorbereitung.
- [ ] HOME drücken: HOME erscheint und bleibt bedienbar. Spiel fortsetzen.
- [ ] HOME erneut drücken, Spiel schließen und Flow wieder öffnen.
- [ ] Modulstart, HOME, Fortsetzen und Schließen zweimal ohne Ausschalten wiederholen.
- [ ] Konsole neu starten und dieselbe Folge erneut durchführen.
- [ ] Digitales 3DS-Spiel aus kalt gestartetem Autostart-Flow starten und HOME testen.
- [ ] Autostart deaktivieren, Konsole neu starten; Modulstart und HOME weiterhin korrekt.
- [ ] Ein DS-Spiel und ein Game-Boy-Spiel zur Regression starten; gespeicherten Spielstand laden.

Bei einer HOME-Vorbereitungsfehlermeldung den genauen Text notieren und SD zurückgeben. Das Spiel soll dann nicht blind starten. Bei einem erneuten Freeze Spielname und ersten oder wiederholten Start notieren. Protokoll: /3ds/3ds-library/handoff.log und handoff.previous.log. Erwartet werden 'APT jump prepare', 'HOME readiness confirmed' und 'Handoff APT result'; für native Spiele kein NS-Reboot mehr.

Lokale Tests: realer Launch-Code mit verzögerter HOME-Registrierung, Bereitschaft genau an der Frist, niemals eintretender Registrierung, Abfragefehlern, Vorbereitungsfehlern und abgelehnter Übergabe; Modulidentität und installierte Titel; DS-/Emulatorregressionen; DE/EN-Texte; ARM-Build; CIA-Inhalt und Dienste; Installer/Pakete. Hardwareerfolg bleibt bis zur Testfolge offen.
