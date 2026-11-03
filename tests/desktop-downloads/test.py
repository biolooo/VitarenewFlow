"""Verify real Desktop catalog generation, runtime guards and current menu input.
Only synthetic package bytes and an isolated filesystem are used.
"""
from pathlib import Path
import copy, hashlib, importlib.util, json, os, re, subprocess, tempfile
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('desktop_catalog_generator',ROOT/'scripts/generate-desktop-catalog.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)

def function(text,name):
    match=re.search(r'\b'+re.escape(name)+r'\s*\([^;{}]*\)\s*\{',text)
    assert match,name
    pos=text.index('{',match.start());depth=0
    for token in re.finditer(r'"(?:\\.|[^"\\])*"|\x27(?:\\.|[^\x27\\])*\x27|/\*.*?\*/|//[^\n]*|[{}]',text[pos:],re.S):
        if token[0]=='{':depth+=1
        elif token[0]=='}':
            depth-=1
            if depth==0:return text[match.start():pos+token.end()]
    raise AssertionError(name)

with tempfile.TemporaryDirectory(prefix='flow-desktop-downloads-') as temporary:
    root=Path(temporary);folder=root/'docs/updates'/module.DIRECTORY;folder.mkdir(parents=True)
    try:module.generate(root);raise AssertionError('Missing actual packages accepted')
    except ValueError:pass
    packages=[]
    for platform,name,label,archive in module.PLATFORMS:
        files=[]
        for filename in (archive,*module.COMPANIONS):
            data=('synthetic Desktop fixture: '+filename).encode();(folder/filename).write_bytes(data)
            files.append(dict(name=filename,url=module.BASE+filename,target='sdmc:/Library-PC/'+filename,bytes=len(data),sha256=hashlib.sha256(data).hexdigest()))
        packages.append(dict(name=name,version=module.VERSION,platform=platform,files=files))
    manifest=folder/'package.json';manifest.write_text(json.dumps(packages))
    module.generate(root);module.generate(root,check=True);module.generate(root,embedded=True)
    # Source-only verification is explicit, full-check never tolerates missing bytes.
    archive=folder/module.PLATFORMS[0][3];original=archive.read_bytes();archive.unlink()
    module.generate(root,embedded=True)
    try:module.generate(root,check=True);raise AssertionError('Missing payload accepted')
    except ValueError:pass
    archive.write_bytes(original+b'changed')
    try:module.generate(root,check=True);raise AssertionError('Changed payload accepted')
    except ValueError:pass
    archive.write_bytes(original)
    changes=[lambda x:x[0].update(platform='windows'),lambda x:x[0].update(version='0.0.0'),lambda x:x[0]['files'][0].update(url='https://elsewhere.invalid/app.zip'),lambda x:x[0]['files'][0].update(target='sdmc:/Library-PC/../bad'),lambda x:x[0]['files'][0].update(bytes=0),lambda x:x[0]['files'][0].update(sha256='f'*64),lambda x:x[0]['files'].pop()]
    for change in changes:
        altered=copy.deepcopy(packages);change(altered);manifest.write_text(json.dumps(altered))
        try:module.generate(root,check=True);raise AssertionError('Invalid metadata accepted')
        except ValueError:pass
    manifest.write_text(json.dumps(packages));module.generate(root,check=True)
    generated=root/'launcher/source/desktop_catalog_generated.h';generated.write_bytes(generated.read_bytes()+b'changed')
    try:module.generate(root,embedded=True);raise AssertionError('Stale compiled header accepted')
    except ValueError:pass
    module.generate(root)
    # Do not accidentally test a stale real generated header from the checkout.
    catalog=root/'component_catalog.c';catalog.write_text((ROOT/'launcher/source/component_catalog.c').read_text())
    runtime=(ROOT/'launcher/source/components.c').read_text();main=(ROOT/'launcher/source/main.c').read_text()
    fragments='\n'.join(function(runtime,name) for name in ('components_definition','components_desktop_platform','components_set_desktop_platform','probe_present','components_availability','components_recheck','components_start'))
    fragments=fragments.replace('components_definition(', 'components_definition(',1)
    signatures={
        'components_definition':'const ComponentDef *','components_desktop_platform':'unsigned ',
        'components_set_desktop_platform':'bool ','probe_present':'static ComponentAvailability ',
        'components_availability':'ComponentAvailability ','components_presence_detail':'const char *','components_recheck':'bool ','components_start':'void '}
    fragments='\n'.join(signatures[name]+function(runtime,name) for name in signatures)
    start=main.index('            ComponentStatus cs=components_status();',main.index('}else if(componentPage){'))
    end=main.index('}else if(updatePage){',start)
    menu=main[start:end]
    prefix=r'''
#define _POSIX_C_SOURCE 200809L
#include "components.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
typedef unsigned long long u64;
typedef void *Thread;
static Thread worker;
static unsigned state,received,total,selected,desktop_platform,active_desktop_platform;
static const ComponentDef *active_definition;
static bool cancelled,ioFailure;
static int diagnostic_code;
static int present_cache[COMPONENT_COUNT]={-1,-1,-1,-1,-1,-1,-1};
static const char *presence_detail[COMPONENT_COUNT];
static unsigned clears,downloads,checks,commits;
static int checked_stat(const char *path,struct stat *st){if(ioFailure&&strstr(path,"macOS")){errno=EIO;return -1;}return stat(path,st);}
#define stat(p,s) checked_stat(p,s)
typedef unsigned DsRuntimeState;
#define DS_RUNTIME_READY 0
#define DS_RUNTIME_UNAVAILABLE 4
static DsRuntimeState ds_runtime_check(void){return DS_RUNTIME_READY;}
ComponentStatus components_status(void){ComponentStatus s={.state=state};return s;}
void components_stop(void){worker=NULL;}
void components_clear(void){components_stop();state=COMP_IDLE;clears++;}
static void run(void *unused){(void)unused;}
static Thread threadCreate(void (*fn)(void*),void *argument,unsigned stack,unsigned priority,int affinity,bool detached){(void)fn;(void)argument;(void)stack;(void)priority;(void)affinity;(void)detached;downloads++;return (void*)1;}
static void publish(ComponentState next,const char *message){(void)message;state=next;}
static void app_log(const char *message){assert(message);checks++;}
'''
    input_prefix=r'''
enum {KEY_A=1,KEY_B=2,KEY_X=4,KEY_L=8,KEY_R=16,KEY_UP=32,KEY_DOWN=64,KEY_TOUCH=128,KEY_LEFT=256,KEY_RIGHT=512};
static unsigned componentSelected=6;static bool componentPage=true,installing;static u64 componentCheckedUntil;
static bool aptIsHomeAllowed(void){return true;}static bool aptIsSleepAllowed(void){return true;}
static void aptSetHomeAllowed(bool value){(void)value;}static void aptSetSleepAllowed(bool value){(void)value;}
static const char *tr(const char *value){return value;}static void draw(bool loading,float value,const char *text){(void)loading;(void)value;(void)text;}
bool components_commit(void){commits++;state=COMP_DONE;return true;}void components_cancel(void){cancelled=true;}
static u64 osGetTime(void){return 1;}static void menu_music_click(void){}static void app_update_stop(void){}
#define ERROR_REPORT_DOWNLOADING 1
static void recordActivity(unsigned activity){(void)activity;}
static void input(unsigned keys,int x,int y){struct{int px,py;}pos={x,y};
'''
    tests=r'''
#undef stat
static void file(const char *path){char tmp[256];snprintf(tmp,sizeof(tmp),"%s",path);for(char *p=tmp+1;*p;p++)if(*p=='/'){*p=0;mkdir(tmp,0700);*p='/';}FILE *f=fopen(path,"wb");assert(f);fputs("fixture",f);fclose(f);}
int main(void){
 assert(COMPONENT_COUNT==7&&DESKTOP_PLATFORM_COUNT==4);
 const ComponentDef *windows=component_desktop_definition(0);assert(windows==component_definition(6)&&windows->count==4);
 assert(!strcmp(windows->files[0].target,"sdmc:/VitarenewFlow Desktop.exe"));assert(!strcmp(windows->files[0].sha,"3881305b2372d5a96021959a64046be82b3a3778e4ebb063dfb7597470a949be"));
 for(unsigned platform=1;platform<4;platform++){const ComponentDef *d=component_desktop_definition(platform);assert(d&&d->count==4);for(unsigned i=0;i<4;i++){assert(strstr(d->files[i].url,"/desktop-0.4.2/")&&strstr(d->files[i].target,"sdmc:/Library-PC/")==d->files[i].target&&strlen(d->files[i].sha)==64&&d->files[i].bytes>0);}}
 assert(!component_desktop_definition(4));assert(component_desktop_hit(12,54)==0&&component_desktop_hit(307,165)==3);assert(component_desktop_hit(11,54)==-1&&component_desktop_hit(308,54)==-1&&component_desktop_hit(12,79)==-1&&component_desktop_hit(12,170)==-1);
 assert(component_desktop_step(0,-1)==3&&component_desktop_step(3,1)==0);
 file("sdmc:/3DS Library PC.exe");assert(components_availability(6)==COMP_AVAILABLE);
 assert(components_set_desktop_platform(1));assert(components_availability(6)==COMP_MISSING);
 file(components_definition(6)->files[0].target);assert(components_set_desktop_platform(1)&&components_availability(6)==COMP_AVAILABLE);
 assert(components_set_desktop_platform(2)&&components_availability(6)==COMP_MISSING);
 ioFailure=true;assert(components_set_desktop_platform(1)&&components_availability(6)==COMP_UNCHECKED);ioFailure=false;
 assert(components_set_desktop_platform(1));components_start(6);const ComponentDef *snapshot=active_definition;assert(snapshot==component_desktop_definition(1)&&active_desktop_platform==1&&total>0);
 assert(!components_set_desktop_platform(2)&&active_definition==snapshot&&desktop_platform==1);
 state=COMP_READY;assert(!components_set_desktop_platform(2)&&active_definition==snapshot);
 state=COMP_DONE;assert(components_set_desktop_platform(3)&&state==COMP_IDLE&&components_availability(6)==COMP_MISSING);
 assert(!components_set_desktop_platform(4)&&desktop_platform==3);
 input(KEY_UP,0,0);assert(desktop_platform==2);input(KEY_R,0,0);assert(desktop_platform==3);input(KEY_DOWN,0,0);assert(desktop_platform==0);
 input(KEY_TOUCH,100,54+29,0);return 0;
}
'''
    tests=tests.replace('input(KEY_TOUCH,100,54+29,0);return 0;','''input(KEY_TOUCH,100,54+29);assert(desktop_platform==1);
 unsigned before=downloads;input(KEY_TOUCH,150,80);assert(downloads==before&&desktop_platform==1);
 input(KEY_X|KEY_A|KEY_R,0,0);assert(downloads==before&&desktop_platform==1&&checks>0);
 input(KEY_A,0,0);assert(downloads==before+1&&state==COMP_DOWNLOAD&&active_desktop_platform==1);input(KEY_DOWN,0,0);assert(desktop_platform==1);
 state=COMP_READY;input(KEY_R,0,0);assert(commits==1&&desktop_platform==1);state=COMP_IDLE;
 puts("PASS: actual Desktop definitions/generator/selection/presence/start snapshot and current menu input, four OS choices, busy guards, exact targets, unchanged Windows");return 0;''')
    test=root/'test.c';test.write_text(prefix+fragments+'\n'+input_prefix+menu+'\n}\n'+tests)
    flags=[os.environ.get('CC','cc'),'-D_DARWIN_C_SOURCE','-std=c11','-O2','-Wall','-Wextra','-Werror','-fsanitize=undefined','-I'+str(root/'launcher/source'),'-I'+str(ROOT/'launcher/source')]
    subprocess.run(flags+[str(test),str(catalog),'-o',str(root/'test')],check=True)
    subprocess.run([str(root/'test')],cwd=root,check=True)
    legacy = root/'legacy-test'
    subprocess.run(flags+['-I'+str(ROOT/'tests/release-0.12.33/component-runtime/mock'),str(ROOT/'tests/release-0.12.33/component-runtime/test.c'),str(catalog),'-o',str(legacy)],check=True)
    subprocess.run([str(legacy)],cwd=root,check=True)
    assert 'CURLOPT_TIMEOUT,selected==6?900L:180L' in runtime
    assert 'const ComponentDef *d=active_definition;' in runtime[runtime.index('static void run('):runtime.index('void components_start(')]
    assert 'const ComponentDef *d=active_definition;' in runtime[runtime.index('bool components_commit('):]
    print('PASS: generator rejects missing/changed bytes, invalid metadata and stale headers; source-only mode explicit; runtime worker/commit retain selected snapshot; TLS/hash downloader unchanged')
