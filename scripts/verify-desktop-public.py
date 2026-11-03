#!/usr/bin/env python3
"""Verify all published Desktop package/companion bytes against local evidence."""
import argparse,hashlib,json,urllib.request
from datetime import datetime,timezone
from pathlib import Path

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[1]);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    folder=args.root/'docs/updates/desktop-0.4.2';packages=json.loads((folder/'package.json').read_text());names={'package.json'}
    for package in packages:
        for entry in package['files']:
            path=folder/entry['name']
            if path.name!=entry['name'] or path.is_symlink() or path.stat().st_size!=entry['bytes'] or hashlib.sha256(path.read_bytes()).hexdigest()!=entry['sha256']:raise ValueError('Invalid local package contract')
            names.add(entry['name'])
    evidence={'version':'0.4.2','checked_at':datetime.now(timezone.utc).isoformat(),'files':{}}
    for name in sorted(names):
        path=folder/name;expected=hashlib.sha256(path.read_bytes()).hexdigest();size=path.stat().st_size
        url='https://biolooo.github.io/VitarenewFlow/updates/desktop-0.4.2/'+name
        request=urllib.request.Request(url,headers={'User-Agent':'Flow-Desktop-release-verifier','Cache-Control':'no-cache'})
        sha=hashlib.sha256();received=0
        with urllib.request.urlopen(request,timeout=25) as response:
            if response.geturl()!=url:raise ValueError('Unexpected redirect')
            while True:
                chunk=response.read(1024*1024)
                if not chunk:break
                received+=len(chunk)
                if received>size:raise ValueError('Oversize public response')
                sha.update(chunk)
        if received!=size or sha.hexdigest()!=expected:raise ValueError('Public Desktop bytes differ: '+name)
        evidence['files'][name]={'size':size,'sha256':expected};print('PASS Desktop',name,flush=True)
    args.output.write_text(json.dumps(evidence,indent=2)+'\n')
if __name__=='__main__':main()
