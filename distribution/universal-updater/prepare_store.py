"""Prepare our own UniStore. Does not publish, contact Universal-DB or write an SD.

Input: a GitHub Releases API JSON and locally downloaded matching release assets.
Only stable 1.0+ releases are accepted; asset digests must match GitHub metadata.
"""
import argparse
import hashlib
import json
import re
from pathlib import Path

REPO = 'biolooo/VitarenewFlow'
BASE = 'https://biolooo.github.io/VitarenewFlow/unistore/'
APP = 'sdmc:/3ds/3ds-library/'
CACHE = APP + 'updates/'
NAMES = ('VitarenewFlow.3dsx', 'VitarenewFlow.smdh', 'VitarenewFlow.cia', 'VitarenewFlow-Boot.3dsx', 'VitarenewFlow-Copy.gm9')


def store_info(revision):
    if type(revision) is not int or revision < 1:
        raise ValueError('Revision must be a positive integer')
    return dict(title='VitarenewFlow', author='VitaRenew',
                description='VitaRenew game launcher for Nintendo 3DS',
                file='VitarenewFlow.unistore', url=BASE+'VitarenewFlow.unistore',
                infoURL=BASE+'VitarenewFlow-info.json', sheet='VitarenewFlow.t3x',
                sheetURL=BASE+'VitarenewFlow.t3x', version=3, revision=revision)


def entry_info(version, updated):
    return dict(title='VitarenewFlow', author='VitaRenew', version=version,
                description='Game library with coverflow, 3DS titles and emulator launch support.',
                category=['utility'], console=['3DS'], icon_index=0, sheet_index=0,
                title_ids=[0xF1042], installed_files=[APP+'3ds-library.3dsx'],
                last_updated=updated, llm_generation='yes',
                preinstall_message='Requires a supported CFW setup. Emulator adapters are separate. '
                                   'Existing games, covers and saves are not part of this download.')


def prepare(release, assets_dir, revision):
    if release.get('draft') is not False or release.get('prerelease') is not False:
        raise ValueError('Only a published stable release is allowed')
    tag = release.get('tag_name', '')
    match = re.fullmatch(r'v?([1-9][0-9]*)\.([0-9]+)\.([0-9]+)', tag)
    if not match:
        raise ValueError('A stable 1.0.0+ tag is required')
    if release.get('html_url') != f'https://github.com/{REPO}/releases/tag/{tag}':
        raise ValueError('Wrong release repository or tag')
    updated = release.get('published_at', '')
    if not re.fullmatch(r'\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z', updated):
        raise ValueError('Missing release publication timestamp')
    assets = {}
    hashes = {}
    for item in release.get('assets', []):
        name = item.get('name')
        if name not in NAMES:
            continue
        if name in assets:
            raise ValueError('Duplicate release asset: '+name)
        url = f'https://github.com/{REPO}/releases/download/{tag}/{name}'
        if item.get('browser_download_url') != url or item.get('state') != 'uploaded':
            raise ValueError('Untrusted or incomplete release asset: '+name)
        content = (Path(assets_dir)/name).read_bytes()
        digest = hashlib.sha256(content).hexdigest()
        if item.get('digest') != 'sha256:'+digest or item.get('size') != len(content):
            raise ValueError('Release hash/size mismatch: '+name)
        if name.endswith('.3dsx') and content[:4] != b'3DSX':
            raise ValueError('Invalid 3DSX')
        if name.endswith('.smdh') and (content[:4] != b'SMDH' or len(content) != 0x36c0):
            raise ValueError('Invalid SMDH')
        if name.endswith('.cia') and (len(content) < 0x2020 or content[:4] != b'\x20\x20\0\0'):
            raise ValueError('Invalid CIA')
        if name.endswith('.gm9') and (not content.startswith(b'# VitarenewFlow') or b'\0' in content):
            raise ValueError('Invalid GodMode9 assistant')
        assets[name] = item
        hashes[name] = digest
    if set(assets) != set(NAMES):
        raise ValueError('Release must provide standalone CIA, 3DSX, SMDH, boot helper and GodMode9 assistant assets')

    def download(name):
        return dict(type='downloadFile', file=assets[name]['browser_download_url'], output=CACHE+name)

    def move(name, target):
        return dict(type='move', old=CACHE+name, new=APP+target)

    folders = [dict(type='mkdir', directory=CACHE), dict(type='mkdir', directory='sdmc:/gm9/scripts/')]
    files = [NAMES[0], NAMES[1], NAMES[3], NAMES[4]]
    install3dsx = folders + [download(n) for n in files] + [
        move(NAMES[0], '3ds-library.3dsx'), move(NAMES[1], '3ds-library.smdh'),
        move(NAMES[3], 'boot-dispatcher.3dsx'),dict(type='move',old=CACHE+NAMES[4],new='sdmc:/gm9/scripts/VitarenewFlow-Copy.gm9')]
    installcia = folders + [download(n) for n in NAMES] + [
        move(NAMES[0], '3ds-library.3dsx'), move(NAMES[1], '3ds-library.smdh'),
        move(NAMES[3], 'boot-dispatcher.3dsx'),dict(type='move',old=CACHE+NAMES[4],new='sdmc:/gm9/scripts/VitarenewFlow-Copy.gm9'),
        dict(type='installCia', file=CACHE+NAMES[2])]
    content = dict(info=entry_info(tag, updated))
    content['Install / Update CIA + 3DSX'] = dict(type='CIA', size=str(sum(a['size'] for a in assets.values()))+' B', script=installcia)
    content['Install / Update 3DSX'] = dict(type='3DSX', size=str(sum(assets[n]['size'] for n in files))+' B', script=install3dsx)
    return dict(storeInfo=store_info(revision), storeContent=[content]), hashes


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--release-json', type=Path, required=True)
    parser.add_argument('--assets', type=Path, required=True)
    parser.add_argument('--revision', type=int, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    store, hashes = prepare(json.loads(args.release_json.read_text(encoding='utf-8')), args.assets, args.revision)
    # All validation happens before creating or replacing any output.
    args.output.mkdir(parents=True, exist_ok=True)
    for name, value in [('VitarenewFlow.unistore', store), ('VitarenewFlow-info.json', store['storeInfo']), ('release-hashes.json', hashes)]:
        (args.output/name).write_text(json.dumps(value, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print('Prepared own UniStore locally. Nothing published or submitted.')


if __name__ == '__main__':
    main()
