"""Compile current main.c's real ROM callback with the real scanner and fixtures."""
from pathlib import Path
import os, subprocess, tempfile
root=Path(__file__).resolve().parents[2]
source=(root/'launcher/source/main.c').read_text()
game=source[source.index('#define MAX_GAMES '):source.index('static Game games[')]
hashes='#define ROM_PATH_SLOTS ' in source
callback=source[source.index('#define ROM_PATH_SLOTS ' if hashes else 'static bool romFound('):source.index('static bool dsScanContinue(')]
callback=('#define ROM_PATHS_BEGIN() romPathsBegin()\n' if hashes else '#define ROM_PATHS_BEGIN() ((void)0)\n')+callback
prefix=r'''
#include "rom_library.h"
#include "library_model.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
typedef uint64_t u64;typedef uint16_t u16;typedef unsigned FS_MediaType;
typedef struct {unsigned dummy;} CiaPackage;
'''
tests=r'''
static bool proceed(void *unused){(void)unused;return true;}
int main(void){
 gameCount=198;for(unsigned i=0;i<gameCount;i++){games[i].system=SYSTEM_3DS;games[i].id=i+1;}
 DsScan s;ROM_PATHS_BEGIN();rom_library_scan(NULL,&s,romFound,proceed,NULL);
 assert(MAX_GAMES==1536&&gameCount==1044&&!s.limited&&!s.cancelled);
 unsigned counts[SYSTEM_COUNT]={0};for(unsigned i=0;i<gameCount;i++){assert(!games[i].icon);counts[games[i].system]++;if(i>=198){assert(!strncmp(games[i].path,"sdmc:/",6));assert(strlen(games[i].path)<512);}}
 assert(counts[SYSTEM_3DS]==198&&counts[SYSTEM_DS]==131&&counts[SYSTEM_GBA]==234&&counts[SYSTEM_N64]==21&&counts[SYSTEM_SNES]==214&&counts[SYSTEM_NES]==222&&counts[SYSTEM_VB]==24);
 // Complete paths determine identity; duplicate traversal never creates copies.
 unsigned before=gameCount;assert(romFound("sdmc:/GBA/f000.gba",NULL)&&gameCount==before);
 assert(romFound("sdmc:/GBA/other/f000.gba",NULL)&&gameCount==before+1);
 assert(!strcmp(games[before].path,"sdmc:/GBA/other/f000.gba"));assert(!strcmp(games[before].key,"f000.gba"));
 while(gameCount<MAX_GAMES){char path[80];snprintf(path,sizeof(path),"sdmc:/NES/added-%u.nes",gameCount);assert(romFound(path,NULL));}
 assert(!romFound("sdmc:/NES/overflow.nes",NULL)&&gameCount==1536);
 char tooLong[600];memset(tooLong,'x',sizeof(tooLong));strcpy(tooLong+sizeof(tooLong)-5,".gba");gameCount=198;ROM_PATHS_BEGIN();assert(romFound(tooLong,NULL)&&gameCount==198);
 puts("PASS actual main ROM callback:198 installed+846 detected ROMs=1044, correct systems/original paths, basename cover contract, zero ROM icons, fullpath dedup,1536 ceiling");
 return 0;
}
'''
with tempfile.TemporaryDirectory(prefix='flow-romcallback-') as temporary:
    folder=Path(temporary)
    for path,ext,count in [('GBA','.gba',234),('NES','.nes',222),('SNES','.sfc',214),('Virtual Boy','.vb',24),('Games/games ds','.nds',131),('3ds/DaedalusX64/Roms','.n64',21)]:
        directory=folder/'sdmc:'/path;directory.mkdir(parents=True,exist_ok=True)
        for i in range(count):(directory/f'f{i:03}{ext}').touch()
    test=folder/'test.c';test.write_text(prefix+game+'\nstatic Game games[MAX_GAMES];static unsigned gameCount;\n'+callback+tests)
    flags=[os.environ.get('CC','cc'),'-D_DARWIN_C_SOURCE','-D_POSIX_C_SOURCE=200809L','-std=c11','-O2','-Wall','-Wextra','-Werror','-fsanitize=undefined','-I'+str(root/'launcher/source')]
    subprocess.run(flags+[str(test),str(root/'launcher/source/rom_library.c'),str(root/'launcher/source/ds_library.c'),str(root/'launcher/source/library_model.c'),'-o',str(folder/'test')],check=True)
    subprocess.run([str(folder/'test')],cwd=folder,check=True)
assert source.count('rom_library_scan(&dsRoots,&dsScan,romFound,dsScanContinue,NULL);')==1
assert 'ds_library_scan(&dsRoots' not in source
assert 'dsScan.cancelled?' in source and 'dsScan.limited?' in source
assert 'if(!dsScan.limited&&!dsScan.cancelled&&gameCount<MAX_GAMES)status=tr("Spieleliste aktualisiert")' in source
print('PASS: current main integrates one shared scan, with explicit incomplete/cancel/library-limit status')
