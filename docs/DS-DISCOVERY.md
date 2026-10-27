# DS-Spiele finden — lokale Weiterentwicklung

Auftrag 26.10.2026: Bestehende DS-Dateiablagen robuster unterstützen. Hauptabnahmekriterien des Nutzers sind eine verständliche, passende leere DS-Ansicht und tatsächlich funktionierende Dateisuche. Basis: sauberer Worktree `codex/error-reports-0.12.35`, Commit `01efb07`; Remote-main `c53d944`, vor Beginn frisch abgeglichen. Nicht gepushte PC-2-Änderungen sind hier nicht sichtbar.

## Verhalten und Grenzen

Vorhandene Spiele bleiben an ihren Speicherorten; keine Dateien verschieben oder umbenennen, keine Saves anfassen. Die DS-Suche soll typische Ordner einschließlich Unterordnern sowie bewusst gewählte zusätzliche Ordner berücksichtigen. Begrenzte Suche und Bibliotheksgrenzen dürfen nicht fälschlich als vollständiger Leerbefund erscheinen. Eine vollständige SD-Suche ist eine getrennte spätere Ausbaustufe.

Die leere DS-Ansicht erklärt den nächsten Schritt und bietet Ordnerauswahl und erneutes Einlesen per Touch und Tasten an. Eine leere globale Textsuche bleibt davon getrennt. DS-Startkomponenten bestimmen nicht die Sichtbarkeit vorhandener Spiele. Fehlende Spiele sind kein Grund für eine NAND-/TWL-Reparatur.

## Nachweis und Abnahme

Lokal implementiert als Launcher 0.12.39. Standardordner `/roms/nds`, `/nds`, `/games/nds`; bis acht zusätzliche Wurzeln in `/3ds/3ds-library/ds-folders.txt`, mit FAT-kompatibler Sicherung und Rückleseprüfung. Eigene Wurzeln vor Standardordnern; überlappende Pfade werden dedupliziert, explizite tiefere Wurzeln erhalten ihre eigene Tiefenreichweite. Symlinks werden nicht verfolgt.

Leere DS-Ansicht: A/Touch öffnet unmittelbar den Ordnerbrowser, X/Touch sucht erneut, Y/Touch öffnet getrennte DS-Einrichtung. Menü → Spiele verwalten → DS-Spieleordner verwaltet zusätzliche Ordner einschließlich Entfernen. Browser: A/Touch öffnet Unterordner, B geht zurück, X/Touch übernimmt den aktuellen Ordner. Touch-Pfeile und Steuerkreuz blättern. Nach Übernahme erscheint die DS-Kategorie. Suchgrenzen und Abbruch bleiben sichtbar. Globale Textsuche ist ein eigener Bildschirm, kein versteckter DS-Filter.

Grenzen: acht Unterordnerebenen je Wurzel, 256 Verzeichnisse und 12000 Einträge pro Suche, Browser maximal 96 Unterordner, insgesamt bestehend maximal 1024 Bibliothekseinträge. Gekürzte/abgebrochene Suche wird als unvollständig kenntlich gemacht. Dateierkennung bleibt `.nds`/`.NDS`; keine neue Headerprüfung, keine Archivextraktion. Vorhandene DS-Loader-Pfadgrenze bleibt erhalten und erzeugt vor Startschreibaktionen einen erklärten Fehler bei zu langen Pfaden. Keine neue Reportübertragung oder Schemaänderung.

Frische vollständige Prüfung bestanden: `.test-build/verification-20261026-122036`, SDK-Staging `flow-check-JhioML`. Enthält nativen 3DSX-/SMDH-Build und bestehende Menü-, Update-, Report-, DS-/Komponenten-, TWL-, HOME-, Such-, Start- und POWER-Regressionen. Neuer tatsächlicher Scanner: Rekursion, Überschneidungen, tiefe explizite Ordner, Grenzen, Abbruch, Pfadprüfung, Symlinks, Navigation, Touchgrenzen; FAT-no-overwrite und Fehler beim Austausch/Rückweg gezielt simuliert. DE/EN-Übersetzungstabelle vollständig geprüft.

Ansichtsprüfung aus tatsächlich extrahierten C-Zeichenfunktionen, Schriftmetriken und Inter-Schrift in 320×240: `.test-build/ds-preview/overview.png` (DE/EN-Leeransicht, Ordnerverwaltung und Browser). Hostvorschau, kein Konsolenscreenshot. Lesender Review einschließlich Touch-/Tastenzuständen durchgeführt. Keine Veröffentlichung, kein Push und kein Konsolenpass. Hardwareabnahme: New 3DS XL EUR, Homebrew- und HOME-Shortcut-Start sowie Autoboot aus/an separat erfassen; Standardordner, Unterordner, eigener Ordner, leere Liste, Abbruch, große Bibliothek und Neustart mit gespeicherten Ordnern prüfen. Bestehende DS-Spiele und Saves müssen erhalten bleiben.
