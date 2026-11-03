"""Build, exercise and package one native, self-contained Desktop 0.4.2 target.
No releases/channels/SD cards are modified. Output stays in .test-build.
"""
import argparse, hashlib, json, os, platform, plistlib, shutil, subprocess, tarfile, zipfile
from pathlib import Path
VERSION = '0.4.2'
RIDS = ('osx-arm64', 'osx-x64', 'linux-x64')
def run(args, cwd):
    subprocess.run([str(a) for a in args], cwd=cwd, check=True)
def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--rid',required=True,choices=RIDS);args=parser.parse_args()
    root=Path(__file__).resolve().parents[1];out=root/'.test-build/desktop-release'/args.rid
    if out.exists():raise ValueError('Output exists; use a fresh checkout/staging directory.')
    out.mkdir(parents=True);publish=out/'publish';dotnet=os.environ.get('DOTNET','dotnet')
    hostarch='arm64' if platform.machine().lower() in ('arm64','aarch64') else 'x64'
    if not args.rid.endswith(hostarch):raise ValueError('Native smoke tests require a matching target architecture.')
    project=root/'desktop/portable/VitarenewFlow.Desktop.csproj'
    run([dotnet,'publish',project,'-c','Release','-r',args.rid,'--self-contained','true','-p:RestoreLockedMode=true','-p:DebugType=None','-o',publish],root)
    executable=publish/'VitarenewFlow.Desktop'
    for mode in ('self-test','ui-test','ui-test-headed'):
        run([executable,'--'+mode,out/(mode+'.json')],root)
        if not json.loads((out/(mode+'.json')).read_text()).get('passed'):raise ValueError('Smoke report failed: '+mode)
    sources={}
    for folder in (root/'desktop/source',root/'desktop/portable'):
        for path in folder.rglob('*'):
            rel=path.relative_to(root)
            if path.is_file() and not any(part in ('bin','obj') for part in rel.parts) and path.suffix in ('.cs','.csproj','.json','.xaml','.png','.manifest','.ps1','.md'):
                sources[rel.as_posix()]=digest(path)
    (out/'source-hashes.json').write_text(json.dumps(sources,indent=2)+'\n')
    notices=root/'desktop/portable/NOTICES.txt'
    if args.rid.startswith('osx-'):
        bundle=out/'VitarenewFlow Desktop.app';macos=bundle/'Contents/MacOS';shutil.copytree(publish,macos)
        info={'CFBundleExecutable':'VitarenewFlow.Desktop','CFBundleIdentifier':'de.vitarenew.flow.desktop','CFBundleName':'VitarenewFlow Desktop','CFBundleDisplayName':'VitarenewFlow Desktop','CFBundlePackageType':'APPL','CFBundleShortVersionString':VERSION,'CFBundleVersion':VERSION,'LSMinimumSystemVersion':'15.0','NSHighResolutionCapable':True}
        (bundle/'Contents/Info.plist').write_bytes(plistlib.dumps(info))
        shutil.copyfile(notices,macos/'NOTICES.txt');shutil.copyfile(root/'LICENSE',macos/'LICENSE.txt')
        # Remove only build-copy Finder metadata from this newly generated bundle.
        # Do not remove quarantine attributes or alter any existing application.
        for attribute in ('com.apple.FinderInfo','com.apple.ResourceFork'):
            subprocess.run(['/usr/bin/xattr','-r','-d',attribute,str(bundle)],cwd=root,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=False)
        run(['/usr/bin/codesign','--force','--deep','--sign','-',bundle],root)
        run(['/usr/bin/codesign','--verify','--deep','--strict',bundle],root)
        run([macos/'VitarenewFlow.Desktop','--self-test',out/'bundle-self-test.json'],root)
        name='VitarenewFlow-Desktop-macOS-'+hostarch+'.zip';archive=out/name
        run(['/usr/bin/ditto','-c','-k','--sequesterRsrc','--keepParent',bundle,archive],root)
    else:
        folder=out/'VitarenewFlow-Desktop';shutil.copytree(publish,folder)
        shutil.copyfile(notices,folder/'NOTICES.txt');shutil.copyfile(root/'LICENSE',folder/'LICENSE.txt')
        name='VitarenewFlow-Desktop-linux-x64.tar.gz';archive=out/name
        with tarfile.open(archive,'w:gz') as tar:tar.add(folder,arcname=folder.name)
    if archive.stat().st_size>=95*1024*1024:raise ValueError('Desktop archive exceeds the chosen 95 MiB publication bound.')
    (out/'asset.json').write_text(json.dumps({'version':VERSION,'rid':args.rid,'name':name,'bytes':archive.stat().st_size,'sha256':digest(archive),'url':'https://biolooo.github.io/VitarenewFlow/updates/desktop-'+VERSION+'/'+name,'target':'sdmc:/Library-PC/'+name,'self_contained':True,'developer_id_notarized':False,'native_tests':True},indent=2)+'\n')
    print('PASS: native engine, headless/headed UI, self-contained archive',name,archive.stat().st_size)
if __name__=='__main__':main()
