# SPDX-License-Identifier: GPL-3.0-only
"""Prepare Windows setup resources from locally built, allowlisted app files."""
import argparse,hashlib,json,shutil,re
from pathlib import Path
def prepare(repo,output,version):
    app=repo/'launcher'
    if not re.fullmatch(r'\d+\.\d+\.\d+',version):raise ValueError('Explicit numeric version required')
    mapping={
        '3ds/3ds-library/3ds-library.3dsx':app/'3ds-library.3dsx',
        '3ds/3ds-library/3ds-library.smdh':app/'3ds-library.smdh',
        '3ds/3ds-library/boot-dispatcher.3dsx':app/'boot-dispatcher/boot-dispatcher.3dsx',
        'cias/VitarenewFlow.cia':app/'VitarenewFlow.cia',
        'gm9/scripts/VitarenewFlow-Copy.gm9':app/'gm9/scripts/VitarenewFlow-Copy.gm9',
        '3DS Library PC.exe':repo/'desktop/SD-Karte/3DS Library PC.exe',
        'Library-PC/INSTALL-DE.txt':repo/'docs/INSTALL-DE.txt',
        'Library-PC/INSTALL-EN.txt':repo/'docs/INSTALL-EN.txt'}
    for src in (repo/'licenses').iterdir():
        if src.suffix in ['.txt','.md']:mapping['Library-PC/licenses/'+src.name]=src
    payload={path:src.read_bytes() for path,src in mapping.items()}
    for path,data in payload.items():
        if path.endswith('.3dsx') and data[:4]!=b'3DSX':raise ValueError('Invalid 3DSX: '+path)
    output.mkdir(parents=True,exist_ok=True);files=[]
    for i,(path,data) in enumerate(payload.items()):
        dst=output/path;dst.parent.mkdir(parents=True,exist_ok=True);dst.write_bytes(data)
        files.append(dict(Path=path,Resource='payload.'+str(i),SHA256=hashlib.sha256(data).hexdigest()))
    (output/'manifest.json').write_text(json.dumps(dict(Version=version+' test package',Files=files),indent=2),encoding='utf-8')
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--version',required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();prepare(Path(__file__).resolve().parents[1],a.output,a.version)
