# 0.12.34 OTA-Testfreigabe

Stand 21.10.2026: Paket lokal vorbereitet und geprüft, Veröffentlichung noch nicht erfolgt. GitHub-Pages-Environment erlaubt ausschließlich Branch `main`; kein Deploymentschutz wird dafür geändert. Ein main-Merge benötigt die ausdrückliche Nutzerfreigabe.

## Konkreter Stand

- Änderungen: Startanimation und Mindestwartezeit entfernt, About/Support DE/EN erneuert, ausklappbare lokale Versionshinweise. N64-/Save-Sync-Themen bleiben vertagt, originale HOME-Standardmusik bleibt offen.
- Runtime-/Updater-/Log-/SMDH-Version konsistent 0.12.34. Numerischer Updatevergleich .33 < .34 < .35 geprüft.
- Paket unter `docs/updates/0.12.34`, acht SHA256-Einträge plus Hashmanifest. 3DSX: 1.599.896 Byte, SHA256 `25cb8dc1aca591bf3a10514076308e2f1558c6503a97250fe1f4254f9447c8ac`.
- Boot-3DSX, Shortcut-CIA und GM9-Skript bytegleich aus .33 übernommen; deren Quellen semantisch unverändert. Keine neue Boot-/NAND-/Systemfunktion.
- latest.txt, UniStore/Info und Installationsseite konsistent auf .34 vorbereitet. Alte Versionsverzeichnisse unverändert.

## Prüfung

Erweiterter Runner `.test-build/verification-20261021-190418`: nativer Build und sämtliche bisherigen/neuen Hosttests bestanden. Darunter sechs Paketfehlerpfadtests, 25 Paketdateigrößen/Hashes, Menü/DE-EN/Touch und Versionsvergleiche, Bannerparser/1024 Coverpfade, Komponenten, TWL-Mocks, HOME, Suche, ROM-Preflight und POWER.

Das fertige source.zip wurde frisch in einen isolierten Chroot-Unterordner entpackt. Stage-Runner und Hosttests daraus bestanden; erzeugte 3DSX **und** SMDH sind bytegleich mit den Paketdateien. Nachweis `.test-build/release-0.12.34/source-build.log`. Die drei ursprünglichen Fontgenerator-/Quelldateien sind aus .33 erhalten; fehlendes Forwarder-Icon und GM9-/Branding-Buildbeigaben vollständig im neuen Quellpaket. RELEASE-BUILD.md erklärt den schlanken Quellpaketprüfweg ohne die separat verteilten historischen Emulatorpakete.

## Veröffentlichung und Gerätetest

Der Releasebranch enthält auch die zuvor nur auf `codex/pc2-handoff`/`codex/pc2-setup` gesicherte Migration und Desktop-.4.1-Basis; `main` ist noch nicht auf diesem Integrationsstand. Der Pull Request muss diese Gesamtbasis berücksichtigen, nicht nur die kleine Versionsänderung.

Nach ausdrücklicher main-Merge-/Veröffentlichungsfreigabe: PR kontrolliert integrieren, Pages-Workflow abwarten und anschließend öffentliches latest.txt sowie alle .34-Dateien und Kanalversionen gegen die geprüften Bytes vergleichen. Erst danach „auf dem 3DS testbar“ melden. Auf dem Gerät: Menü → Konsole & Start → Updates → Aktualisieren → Neu öffnen; installierte Version 0.12.34 prüfen, anschließend Testliste aus updates/0.12.34/README.md durchführen.

Kein Stable-Release oder bestandener Konsolentest behauptet. Manueller Rückweg des Updaters: vorhandene `3ds-library.3dsx.previous.3dsx` erhalten; nicht während Installation ausschalten oder SD entfernen.
