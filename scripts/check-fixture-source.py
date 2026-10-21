"""Reject copied release fixtures if their exercised source fragments drift."""
from pathlib import Path
import re

def function(text, name):
    match=re.search(r'\b'+re.escape(name)+r'\s*\([^;{}]*\)\s*\{',text)
    if not match: raise ValueError('missing function '+name)
    start=match.start(); pos=text.index('{',start); depth=0
    token=re.compile(r'"(?:\\.|[^"\\])*"|\x27(?:\\.|[^\x27\\])*\x27|/\*.*?\*/|//[^\n]*|[{}]',re.S)
    for m in token.finditer(text,pos):
        if m.group()=='{': depth+=1
        elif m.group()=='}':
            depth-=1
            if depth==0: return text[start:m.end()]
    raise ValueError('unterminated '+name)

def normal(s):
    s=re.sub(r'/\*.*?\*/|//[^\n]*','',s,flags=re.S)
    return re.sub(r'\s+','',s)

checks=[('launcher/source/ds_backend.c','tests/component-runtime/test.c','ds_runtime_check'),
        ('launcher/source/components.c','tests/component-runtime/test.c','probe_present'),
        ('launcher/source/components.c','tests/component-runtime/test.c','components_recheck'),
        ('launcher/source/rom_backend.c','tests/launch/test.c','rom_preflight')]
for source,test,name in checks:
    assert normal(function(Path(source).read_text(),name))==normal(function(Path(test).read_text(),name)), 'Fixture drift: '+name
print('PASS: copied component/launch source fragments match the checkout')
