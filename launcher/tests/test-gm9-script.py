from pathlib import Path
import shlex,re,fnmatch,hashlib
script=(Path(__file__).resolve().parent.parent/'gm9/scripts/VitarenewFlow-Copy.gm9').read_text()
lines=[shlex.split(line,comments=True) for line in script.splitlines()];lines=[x for x in lines if x]
labels={x[0][1:]:i for i,x in enumerate(lines) if x[0].startswith('@')}
arity={'ask':1,'echo':1,'goto':1,'find':2,'exist':1,'strsplit':3,'set':2,'mkdir':1,'cp':2,'shaget':2,'sha':2,'mv':2,'findnot':2,'buildcia':1,'verify':1,'dumptxt':2}
for line in lines:
 if line[0].startswith('@') or line[0]=='end':continue
 cmd=line[1:] if line[0]=='if' else line
 if cmd[0]=='not':cmd=cmd[1:]
 name,*args=cmd;args=[a for a in args if not a.startswith('-')]
 assert name in arity and len(args)==arity[name],line
 if name=='goto':assert args[0] in labels
assert 'install ' not in script and 'rm ' not in script and 'cp -w' not in script
def run(initial,fail='',cancel=False):
 fs=initial.copy();env={'GM9OUT':'0:/gm9/out'};messages=[];pc=0;steps=0
 def expand(a):return re.sub(r'\$\[([^]]+)\]',lambda m:env[m[1]],a)
 def command(tokens):
  name=tokens[0];before='-b' in tokens;args=[expand(a) for a in tokens[1:] if not a.startswith('-')]
  if name=='ask':return not cancel
  if name=='echo':messages.append(args[0]);return True
  if name=='set':env[args[0]]=args[1];return True
  if name=='strsplit':env[args[0]]=args[1].rsplit(args[2],1)[0 if before else -1];return True
  if name=='dumptxt':
   if fail=='receipt':return False
   fs[args[0]]=(args[1]+'\n').encode();return True
  if name=='mkdir':return True
  if name=='exist':return args[0] in fs
  if name=='find':
   found=sorted(p for p in fs if fnmatch.fnmatchcase(p,args[0]))
   if found:env[args[1]]=found[0]
   return bool(found)
  if name=='findnot':
   for i in range(100):
    p=args[0].replace('??',f'{i:02}')
    if p not in fs:env[args[1]]=p;return True
   return False
  if name=='cp':
   assert args[1] not in fs
   fs[args[1]]=b'partial' if fail=='copy' else fs[args[0]];return fail!='copy'
  if name=='shaget':env[args[1]]=hashlib.sha256(fs[args[0]]).hexdigest();return fail!='hash'
  if name=='sha':return fail!='verify' and hashlib.sha256(fs[args[0]]).hexdigest()==args[1]
  if name=='mv':
   assert args[1] not in fs
   if fail=='commit':return False
   fs[args[1]]=fs.pop(args[0]);return True
  if name=='buildcia':
   if fail=='build':return False
   dest='0:/gm9/out/'+args[0].rsplit('/',1)[-1].rsplit('.',1)[0]+'.standard.cia'
   assert dest not in fs;fs[dest]=b'verifiable CIA';return True
  if name=='verify':return fail!='verify'
  raise AssertionError(tokens)
 while pc<len(lines):
  steps+=1;assert steps<500
  line=lines[pc];pc+=1
  if line[0].startswith('@') or line[0]=='end':continue
  if line[0]=='goto':pc=labels[line[1]]+1;continue
  if line[0]=='if':
   cond=line[1:];neg=cond[0]=='not';cond=cond[1:] if neg else cond
   ok=command(cond);ok=not ok if neg else ok
   if not ok:
    depth=1
    while depth:
     x=lines[pc];pc+=1;depth+=(x[0]=='if')-(x[0]=='end')
  elif not command(line):break
 return fs,messages
ds={'C:/TEST_ABCE.nds':b'DS module data'}
fs,_=run(ds);assert fs['0:/roms/nds/TEST_ABCE.nds']==ds['C:/TEST_ABCE.nds']
for failure in ['copy','hash','verify','commit']:
 fs,msg=run(ds,failure);assert '0:/roms/nds/TEST_ABCE.nds' not in fs and not any('DS copy verified:' in s for s in msg)
initial=ds|{'0:/roms/nds/TEST_ABCE.nds':b'old game'};fs,_=run(initial);assert {k:v for k,v in fs.items() if not k.endswith('copy-result.txt')}==initial
initial=ds|{'0:/roms/nds/TEST_ABCE.nds.part':b'old partial'};fs,_=run(initial);assert {k:v for k,v in fs.items() if not k.endswith('copy-result.txt')}==initial
ctr={'C:/000400000012BE00.3ds':b'3DS module data'};fs,msg=run(ctr);assert fs['0:/cias/VF_CART_00.cia']==b'verifiable CIA'
for failure in ['build','verify','commit']:
 fs,msg=run(ctr,failure);assert not any(p.startswith('0:/cias/') for p in fs) and not any('CIA verified:' in s for s in msg)
initial=ctr|{'0:/gm9/out/000400000012BE00.standard.cia':b'old CIA'};fs,_=run(initial);assert {k:v for k,v in fs.items() if not k.endswith('copy-result.txt')}==initial
fs,_=run(ctr,cancel=True);assert {k:v for k,v in fs.items() if not k.endswith('copy-result.txt')}==ctr
fs,_=run({});assert fs=={'0:/3ds/3ds-library/copy-result.txt':b'VF1 STARTED\n'}
print('PASS actual GM9 script control flow: DS hash/commit, CIA verification, cancellation, read/copy/commit failures, existing/partial destinations, no NAND/save writes; syntax/arity against upstream commands. GM9 hardware execution pending.')

receipt='0:/3ds/3ds-library/copy-result.txt'
assert run(ds)[0][receipt]==b'VF1 DS\n'
assert run(ctr)[0][receipt]==b'VF1 CIA\n'
for source in [ds,ctr]:
 for failure in ['copy','hash','verify','commit','build']:
  fs,msg=run(source,failure)
  if not any('copy verified:' in x or 'CIA verified:' in x for x in msg):assert fs[receipt]==b'VF1 STARTED\n'
 fs,msg=run(source|{receipt:b'VF1 CIA\n'},cancel=True)
 assert fs[receipt]==b'VF1 STARTED\n'
 fs,msg=run(source,fail='receipt');assert fs==source
print('PASS receipts: stale success replaced before cancel; success only after verified commit; unwritable receipt aborts before copying')
