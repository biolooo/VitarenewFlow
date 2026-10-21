"""Prepare versioned 3DSX updater files. This script never publishes."""
import argparse
import hashlib
import re
import shutil
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('--version', required=True)
p.add_argument('--binary', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a = p.parse_args()
if not re.fullmatch(r'(0|[1-9][0-9]{0,2})\.(0|[1-9][0-9]{0,2})\.(0|[1-9][0-9]{0,2})', a.version):
    p.error('Expected bounded major.minor.patch version')
data = a.binary.read_bytes()
if not 32 <= len(data) <= 8 * 1024 * 1024 or data[:4] != b'3DSX':
    p.error('Invalid or oversized 3DSX')
version_dir = a.output / a.version
version_dir.mkdir(parents=True, exist_ok=True)
target = version_dir / 'VitarenewFlow.3dsx'
if target.exists() and target.read_bytes() != data:
    p.error('Versioned artifacts are immutable; choose a new version')
if target.resolve() != a.binary.resolve():
    shutil.copyfile(a.binary, target)
manifest = (f'VFUPDATE1\nversion={a.version}\nsize={len(data)}\n'
            f'sha256={hashlib.sha256(data).hexdigest()}\n'
            f'url=https://biolooo.github.io/VitarenewFlow/updates/{a.version}/VitarenewFlow.3dsx\n')
(a.output / 'latest.txt').write_bytes(manifest.encode('ascii'))
print(manifest, end='')
