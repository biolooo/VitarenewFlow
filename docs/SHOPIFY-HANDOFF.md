# Flow-Webseite und Shopify-Abstimmung

Nutzerauftrag 22.10.2026: „die seite soll eine eigene seo optimierte seite sein welche über einen banner auf der homepage verlinkt ist. Der homepage banner soll erst relativ weit unten kommen.“ Außerdem ausdrücklich: „Weiße den Shopify Orchestrator selbst an, auch wenn wichtige änderungen kommen welche auf der website erwähnt werden sollten stoße den Orchestrator an damit er diese änderungen durchführt.“ Diese direkte menschliche Autorisierung gilt für gezielte Nachrichten an den Shopify-Orchestrator im beschriebenen Umfang; keine zusätzliche Nachrichtenfreigabe je Release nötig.

Zielchat: VitaRenew Shopify Orchestrator, threadId `01a15efd-26f7-7b50-9a66-afc183f93433`, host `local`. Projekt C:/Users/subje/Documents/Code/vitarenew-shopify. Am22.10.2026 per send_message_to_thread mit vollständigem Auftrag beauftragt. Eigene SEO-Landingpage mit Anleitung; Homepagebanner im unteren Bereich unter den wesentlichen Verkaufsbereichen. Kein Hero, kein direkter GitHub-Link statt eigener Seite. Mobile/Desktop prüfen. Aktuelle Seitenadresse und Bannerposition sind vom Shopify-Chat zu ermitteln; Beauftragung ist keine fertige Website.

Die bisher angeforderte Vorschau vor Veröffentlichung gilt weiter, sofern Nutzer oder vorhandene Shopify-Freigaben sie nicht ersetzen. Der Flow-Orchestrator überschreibt kein fremdes Theme und nimmt keine pauschale Live-Freigabe an.

## Auslöser für Folgeauftrag

Nach öffentlich verifiziertem Flow-Release oder bestätigter wesentlicher Änderung prüfen, ob Website betroffen ist: Installation/Voraussetzungen, Menünavigation, Download-/QR-/Storeadresse, unterstützte Systeme oder relevante bekannte Einschränkungen, HOME-Icon/Updates, Reportzugang, Wechsel von Teststatus zu1.0. Reine interne Refactorings oder unveränderte Versionsnummern ohne Nutzen für die Seite nicht melden. Unveröffentlichte Entwicklungen ausdrücklich als Entwurf kennzeichnen, niemals schon als verfügbar bewerben.

Bei Bedarf mit send_message_to_thread an den Zielchat einen kompakten Umsetzungsauftrag senden: öffentlich bestätigte Änderung, Quelle/Releasebeleg, betroffene Abschnitte, gewünschter Endzustand und bisherige Freigabegrenzen. Homepagebanner nicht je Release unnötig ändern. Keine Nutzerkommentare, Reports oder privaten Daten weitergeben. Status bei Bedarf lesend prüfen, keine automatisch angelegten neuen Chats. Kommunikationsdatum, Inhalt und Ergebnis kurz hier bzw.imRelease-Handoff festhalten, damit keine doppelten Aufträge entstehen.

## Installationsquellen

Technische Referenz: https://biolooo.github.io/VitarenewFlow/updates/install.html
QR: https://biolooo.github.io/VitarenewFlow/updates/VitarenewFlow-QR.png
Store: https://biolooo.github.io/VitarenewFlow/updates/VitarenewFlow.unistore

Eine feste Appversion in Evergreen-Anleitung vermeiden. Teststatus unter1.0 klar nennen, eigene Spiele separat, keine Spiele/BIOS/Saves enthalten. Lokale .38-Remoteinfos sind bis bestätigtem Release nicht als öffentlich nutzbar darzustellen.

## Releasehinweis22.10.2026 — .38

Shopify-Orchestrator nach erfolgreichem Pages-Lauf39758606547 und14öffentlichen Bytevergleichen über .38 informiert: Zielupdateinfos per X/Touch, blätterbare DE/EN-Ansicht, einmalige Aktualisierung von .37 nötig. Installationsquellen und Teststatus bleiben. Konkrete Vorschauadresse im Shopify-Chat angefordert, noch nicht bestätigt. Keine private Reportdatenweitergabe.

Vorschau inzwischen erreichbar und perHTTP200 mit passendem Seitentitel/Installationsinhalt bestätigt. Unveröffentlichter Entwurf im Theme206275248469; vollständiger temporärer Vorschau-Link privat unter `.test-build/shopify-preview-link.json` sowie im Shopify-Chat, nicht im öffentlichen Git. Mobile/Banner-QA beim Abruf noch in Arbeit. Keine Website-Livefreigabe oder fertige SEO-Abnahme behauptet.

## Releasehinweis 27.10.2026 — .39

Nach öffentlicher Byteprüfung aller zehn Assets/vier Kanäle (Pages40486000717) den bestehenden Shopify-Orchestrator zur Anleitungsergänzung beauftragt: DS-Unterordnersuche, eigene Spieleordner, neue hilfreiche Leeransicht. Evergreen-Anleitung ohne Versionszwang, bestehende Vorschaugrenzen. Keine private Reportinformation versendet; Websiteumsetzung nicht bestätigt.

## Folgeauftrag vorbereitet, Zustellung offen · 03.11.2026 — .41 / Desktop .4.2

Launcher 0.12.41 und Desktop 0.4.2 sind öffentlich bereitgestellt: Pages-Lauf
`41633256738` zu Releasecommit `19597a5` erfolgreich, alle 21 Paket-/Kanaldateien
öffentlich bytegleich geprüft. Referenzen: [Installation](https://biolooo.github.io/VitarenewFlow/updates/install.html),
[Launcherhinweise](https://biolooo.github.io/VitarenewFlow/updates/0.12.41/README.md),
[Desktoppakete](https://biolooo.github.io/VitarenewFlow/updates/desktop-0.4.2/package.json).

Vorbereiteter Folgeauftrag für die bestehende SEO-Anleitung: Im Desktopabschnitt
**Menü → Spiele verwalten → Desktop-Verwaltung** mit Auswahl **Windows**, **macOS ·
Apple Silicon**, **macOS · Intel** und **Linux · x64** erklären. Windows bleibt bei
seinem bisherigen Paket. Mac/Linux werden als Archiv unter `Library-PC` auf der SD
abgelegt und am Computer entpackt. Voraussetzungen präzise nennen: macOS 15 oder
neuer; Linux x64 wurde nativ auf Ubuntu 24.04 geprüft. Die Mac-App ist ad-hoc
signiert, nicht Apple-notarisiert; mögliche Sicherheitsrückfragen transparent
benennen, keine globalen Sicherheitsmechanismen abschalten lassen. Im
Spieleordnerabschnitt die automatische Suche in üblichen Spielewurzeln samt
Unterordnern und die Grenze von 1536 Bibliothekseinträgen erläutern. Eigene Spiele
bleiben separat, es werden keine Spiele, BIOS oder Saves mitgeliefert. Keine feste
Appversion in die Evergreen-Anleitung aufnehmen. Teststatus unter 1.0 und bisherige
Vorschau-/Live-Grenzen beibehalten; Homepagebanner nicht unnötig ändern.

**Technisch noch nicht zugestellt:** Der autorisierte bestehende Windows-Zielchat
`01a15efd-26f7-7b50-9a66-afc183f93433` (`local`) ist auf diesem Mac nicht auffindbar
(`read_thread`: „No Codex thread found“). Keine Nachricht gesendet, keinen neuen
Sidebar-Chat angelegt und keine Websiteänderung vorgenommen. Der Entwurf bleibt
bis zum Zugriff auf den bestehenden Shopify-Orchestrator offen; dies blockiert
den verifizierten Flow-/Desktop-Release nicht. Keine privaten Kartendaten,
Spielnamen oder Reports wurden weitergegeben.
