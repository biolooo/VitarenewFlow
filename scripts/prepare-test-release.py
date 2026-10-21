"""Prepare a local 0.12.34 test package and channels; never push or deploy."""
import argparse
import hashlib
import json
import shutil
import subprocess
import sys
import zipfile
from datetime import datetime, timezone
from pathlib import Path

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--verification', required=True, type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[1]
updates = root / 'docs/updates'
version = '0.12.34'
release = updates / version
if release.exists():
    p.error('Release directory already exists; inspect and preserve immutable artifacts.')
for name in ('3ds-library.3dsx', '3ds-library.smdh', 'verification.log', 'source-hashes.json'):
    if not (a.verification / name).is_file():
        p.error('Missing verification artifact: ' + name)
if 'PASS: native build and existing component' not in (a.verification / 'verification.log').read_text():
    p.error('Full native/host verification has not passed.')
for name, digest in json.loads((a.verification / 'source-hashes.json').read_text()).items():
    if hashlib.sha256((root / name).read_bytes()).hexdigest() != digest:
        p.error('Build source changed after verification: ' + name)
data = (a.verification / '3ds-library.3dsx').read_bytes()
if data[:4] != b'3DSX' or not 32 <= len(data) <= 8 * 1024 * 1024:
    p.error('Invalid 3DSX.')
if version not in (root / 'launcher/source/app_update.c').read_text():
    p.error('Runtime version mismatch.')
release.mkdir()
shutil.copyfile(a.verification / '3ds-library.3dsx', release / 'VitarenewFlow.3dsx')
shutil.copyfile(a.verification / '3ds-library.smdh', release / 'VitarenewFlow.smdh')
for name in ('VitarenewFlow-Boot.3dsx', 'VitarenewFlow-Shortcut.cia', 'VitarenewFlow-Copy.gm9'):
    shutil.copyfile(updates / '0.12.33' / name, release / name)
notes = '''# VitarenewFlow 0.12.34 — Testversion

- Spiel-Startanimation samt Menüoption und künstlicher Mindestwartezeit entfernt.
- Echte Vorbereitungs-, Seed- und HOME-Anzeigen sowie Originalbanner bleiben erhalten.
- Über/Unterstützung übersichtlicher, vollständig Deutsch/Englisch.
- Updates: kurze lokale Neuerungen der installierten Version per X oder Touch ausklappen.
- Alte appearance.txt bleibt erhalten; Sprach-/Ansichtseinstellungen bleiben unabhängig.

## Auf dem 3DS testen

Nach Veröffentlichung: Menü → Konsole & Start → Updates → Aktualisieren → Neu öffnen.
Die installierte Version muss 0.12.34 anzeigen. Bei HTTPS-Problemen Datum und WLAN prüfen.
1. Anpassen: nur Sprache und Theme, kein Startanimationsschalter.
2. Spiel starten: kein Logo und keine künstliche Verzögerung; echte Vorbereitung bleibt sichtbar.
3. Über/Unterstützung und Updates in Deutsch/Englisch auf vollständigen Text und Touch prüfen.
4. X/Touch klappt die Neuerungen auf/zu; Prüfen/Aktualisieren/Zurück bleiben bedienbar.
5. Fehlender Banner nutzt weiter Icon/Cover. HOME, POWER und erneuten Spielstart prüfen.

Keine neue originale HOME-Standardmusik und kein neuer N64-Fix. N64 und Save-Sync sind vertagt.
Spiele, Saves, Cover und Boot-Konfiguration werden durch dieses 3DSX-Update nicht verändert.
Nicht während der Installation ausschalten oder die SD entfernen. Der Updater erhält die vorherige
3DSX als /3ds/3ds-library/3ds-library.3dsx.previous.3dsx für den manuellen Rückweg.

## Nachweise und Begleiter

Frischer nativer Build und Hosttests inklusive Menü/DE-EN/Touch, Bannerparser, Paketintegrität,
Komponenten, TWL-Mocks, HOME, Suche, ROM-Preflight und POWER lokal bestanden.
Kein Konsolenpass vorweggenommen. Boot-3DSX, Shortcut-CIA und GM9-Skript sind bytegleich
aus 0.12.33 übernommen; keine neue System-/Boot-/NAND-Funktion.
VitarenewFlow-source.zip enthält aktuelle Launcherquellen, Builddateien, Tests und Lizenzen.
devkitARM/3DS-Bibliotheken separat einrichten; siehe docs/PC2-SETUP.md im Quellpaket.
Launcher: GPL-3.0-only; Fremdkomponenten behalten ihre eigenen Bedingungen.
Keine Spiele, Saves, Schlüssel oder Nintendo-Musik enthalten. Kein Stable-1.0-Release.
'''
(release / 'README.md').write_bytes(notes.encode('utf-8'))
(release / 'FEATURES-DE.md').write_bytes(notes.encode('utf-8'))
# Git inventories paths; current verified working bytes are packaged, never .git,
# local toolchains, build results, private data or old release binaries.
paths = subprocess.check_output(['git', 'ls-files', '-z', '--cached', '--others', '--exclude-standard'], cwd=root).decode().split('\0')
with zipfile.ZipFile(release / 'VitarenewFlow-source.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
    for name in sorted(set(paths)):
        if not name:
            continue
        include = (name.startswith(('launcher/', 'scripts/', 'tests/', 'licenses/'))
                   or name in ('LICENSE', 'README.md', 'AGENTS.md', 'PLANS.md', '.gitattributes', '.gitignore')
                   or (name.startswith('docs/') and not name.startswith(('docs/updates/', 'docs/assets/', 'docs/migration-evidence/')) and name.endswith(('.md', '.txt'))))
        if include:
            archive.write(root / name, name)
    # Preserve the font generator and its original licensed input, which were
    # delivered with .33 but were absent from the migrated checkout.
    with zipfile.ZipFile(updates / '0.12.33/VitarenewFlow-source.zip') as previous:
        for name in ('launcher/tools/make-font.py', 'launcher/tools/fonts/InterVariable.ttf',
                     'launcher/tools/fonts/Inter-LICENSE.txt'):
            if name not in archive.namelist():
                archive.writestr(name, previous.read(name))
    archive.writestr('RELEASE-BUILD.md', '''# Build this exact source package

Use the devkitARM linux/amd64 chroot pinned in docs/PC2-SETUP.md, with host
GCC/UBSan, Python and zlib1g-dev. Extract this ZIP into a NEW directory inside
that chroot (never over an existing SDK project). Enter it with chroot.
From the extracted root, flatten only the archived release fixtures for the
existing stage runner, then execute:

```sh
cp -a tests/release-0.12.33/. tests/
cp scripts/check-fixture-source.py check-fixture-source.py
cp scripts/verify-stage.sh verify-stage.sh
bash verify-stage.sh
```

The stage script sets DEVKITPRO/DEVKITARM/PATH, builds launcher/3ds-library.3dsx
and .smdh and runs real-source anchors, UI/banner and existing hostfixtures.
verify-local.sh is for the complete repository: its package-integrity inventory
requires the separate historical docs/updates distributions omitted from this ZIP.
No SDK, emulator binaries, ROMs, saves, keys or private music are in this package.
The three launcher/tools font input/generator files are retained unchanged from
the .33 source archive; the checked-in font header is used for this build.
''')
manifest = {f.name: hashlib.sha256(f.read_bytes()).hexdigest() for f in sorted(release.iterdir()) if f.is_file()}
(release / 'SHA256.json').write_bytes((json.dumps(manifest, indent=2) + '\n').encode())
subprocess.run([sys.executable, str(root / 'scripts/prepare-app-update.py'), '--version', version,
                '--binary', str(release / 'VitarenewFlow.3dsx'), '--output', str(updates)], check=True)
store = json.loads((updates / 'VitarenewFlow.unistore').read_text(encoding='utf-8-sig'))
store['storeInfo']['revision'] += 1
entry = store['storeContent'][0]
entry['info']['version'] = version
entry['info']['description'] = 'Remove launch animation; refreshed About and expandable installed-version highlights (DE/EN).'
entry['info']['last_updated'] = datetime.now(timezone.utc).isoformat(timespec='seconds').replace('+00:00', 'Z')
install = entry['Installieren / Install']
total = 0
for action in install['script']:
    if action.get('type') == 'downloadFile':
        action['file'] = action['file'].replace('/0.12.33/', '/' + version + '/')
        total += (release / action['file'].rsplit('/', 1)[1]).stat().st_size
install['size'] = str(total) + ' B'
(updates / 'VitarenewFlow.unistore').write_bytes((json.dumps(store, ensure_ascii=False, indent=2) + '\n').encode())
(updates / 'VitarenewFlow-info.json').write_bytes((json.dumps(store['storeInfo'], ensure_ascii=False, indent=2) + '\n').encode())
html = updates / 'install.html'
html.write_bytes(html.read_bytes().replace(b'0.12.33', version.encode()))
print('PREPARED locally; publication and hardware acceptance remain separate:', release)
