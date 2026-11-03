#!/usr/bin/env python3
"""Embed verified, immutable portable Desktop assets in the 3DS component catalog.
Never invent hashes or enable a platform whose actual package bytes are absent.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re

VERSION = '0.4.2'
DIRECTORY = 'desktop-' + VERSION
BASE = 'https://biolooo.github.io/VitarenewFlow/updates/' + DIRECTORY + '/'
PLATFORMS = (
    ('osx-arm64', 'VitarenewFlow Desktop macOS Apple Silicon', 'macOS arm64', 'VitarenewFlow-Desktop-macOS-arm64.zip'),
    ('osx-x64', 'VitarenewFlow Desktop macOS Intel', 'macOS Intel', 'VitarenewFlow-Desktop-macOS-x64.zip'),
    ('linux-x64', 'VitarenewFlow Desktop Linux x64', 'Linux x64', 'VitarenewFlow-Desktop-linux-x64.tar.gz'),
)
COMPANIONS = ('LIESMICH.txt', 'NOTICES.txt', 'VitarenewFlow-Desktop-source.zip')


def require(value, message):
    if not value:
        raise ValueError(message)


def validated(packages, folder=None):
    require(type(packages) is list and len(packages) == len(PLATFORMS), 'Expected exactly three portable platform packages')
    output = []
    for package, (platform, expected_name, label, archive) in zip(packages, PLATFORMS):
        require(type(package) is dict and set(package) == {'name', 'version', 'platform', 'files'}, 'Invalid Desktop package schema')
        require(package['version'] == VERSION and package['platform'] == platform and package['name'] == expected_name, 'Unexpected platform order/name/version')
        require(type(package['files']) is list and len(package['files']) == 4, 'Each platform requires exactly archive/readme/notices/source')
        files = []
        total = 0
        for entry, name in zip(package['files'], (archive, *COMPANIONS)):
            require(type(entry) is dict and set(entry) == {'name', 'url', 'target', 'bytes', 'sha256'}, 'Invalid Desktop file schema')
            require(entry['name'] == name and entry['url'] == BASE + name and entry['target'] == 'sdmc:/Library-PC/' + name, 'Unexpected Desktop URL or target')
            size, sha = entry['bytes'], entry['sha256']
            require(type(size) is int and 0 < size <= 256 * 1024 * 1024, 'Invalid Desktop asset size')
            require(type(sha) is str and re.fullmatch('[0-9a-f]{64}', sha), 'Invalid Desktop SHA256')
            if folder is not None:
                path = folder / name
                require(path.is_file() and not path.is_symlink() and path.stat().st_size == size, 'Missing or changed Desktop asset: ' + name)
                digest = hashlib.sha256()
                with path.open('rb') as stream:
                    for block in iter(lambda: stream.read(1024 * 1024), b''):
                        digest.update(block)
                require(digest.hexdigest() == sha, 'Desktop SHA256 mismatch: ' + name)
            files.append(entry)
            total += size
        require(total <= 0xffffffff, 'Desktop total exceeds the downloader counter')
        output.append((label, files))
    return output


def header(packages):
    quote = lambda text: json.dumps(text, ensure_ascii=False)
    lines = ['/* Generated from verified Desktop package bytes; do not edit. */', '#pragma once']
    for index, (_, files) in enumerate(packages):
        lines.append('static const ComponentFile desktop_portable_files' + str(index) + '[]={')
        for entry in files:
            lines.append(' {' + ','.join((quote(entry['url']), quote(entry['target']), quote(entry['sha256']), str(entry['bytes']))) + '},')
        lines.append('};')
    lines.append('static const ComponentDef desktop_portable_definitions[]={')
    for index, (label, _) in enumerate(packages):
        lines.append(' {' + quote('VitarenewFlow Desktop ' + label) + ',' + quote(label + ' · ' + VERSION) + ',desktop_portable_files' + str(index) + ',4},')
    lines.append('};')
    return ('\n'.join(lines) + '\n').encode('utf-8')


def generate(root, check=False, embedded=False):
    root = Path(root)
    source = root / 'launcher/source/desktop_catalog_manifest.json' if embedded else root / 'docs/updates' / DIRECTORY / 'package.json'
    require(source.is_file() and not source.is_symlink(), 'Missing verified portable Desktop manifest')
    manifest = json.loads(source.read_text(encoding='utf-8'))
    data = header(validated(manifest, None if embedded else source.parent))
    metadata = (json.dumps(manifest, ensure_ascii=False, indent=2) + '\n').encode('utf-8')
    output = root / 'launcher/source/desktop_catalog_generated.h'
    bundled = root / 'launcher/source/desktop_catalog_manifest.json'
    if check or embedded:
        require(output.is_file() and not output.is_symlink() and output.read_bytes() == data, 'Compiled Desktop catalog is missing or stale')
        require(bundled.is_file() and not bundled.is_symlink() and bundled.read_bytes() == metadata, 'Embedded Desktop manifest is missing or stale')
    else:
        require(not output.is_symlink() and not bundled.is_symlink(), 'Refuse symlink catalog output')
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_bytes(data)
        bundled.write_bytes(metadata)
    return data


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument('--check', action='store_true', help='Verify actual package bytes and embedded metadata')
    modes.add_argument('--check-embedded', action='store_true', help='Check source-only package metadata/header without desktop binaries')
    args = parser.parse_args()
    try:
        generate(args.root, args.check, args.check_embedded)
    except (ValueError, OSError, KeyError, TypeError) as error:
        parser.error(str(error))
