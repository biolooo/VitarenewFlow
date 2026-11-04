"""Exercise the current main's view builder, native launch and ROM path index.
Only service mocks and synthetic paths are used; no SD is mounted or accessed.
"""
from pathlib import Path
import os,re,subprocess,tempfile
root=Path(__file__).resolve().parents[2]
source=(root/'launcher/source/main.c').read_text()
def function(name):
    m=re.search(r'(?m)^(?:static\s+)?[^\n;{}]*?\b('+re.escape(name)+r')\s*\([^;\n{}]*\)\s*\{',source);assert m,name
    start=source.index('{',m.start(1));depth=0
    for token in re.finditer(r'"(?:\\.|[^"\\])*"|\x27(?:\\.|[^\x27\\])*\x27|/\*.*?\*/|//[^\n]*|[{}]',source[start:],re.S):
        if token[0]=='{':depth+=1
        elif token[0]=='}':
            depth-=1
            if not depth:return source[m.start(1):start+token.end()]
    raise AssertionError(name)
game=source[source.index('#define MAX_GAMES '):source.index('static Game games[')]
hash_callback=source[source.index('#define ROM_PATH_SLOTS '):source.index('static bool dsScanContinue(')]
backend='\n'.join(line for line in source.splitlines() if line.startswith('static const char *backendNames[') or line.startswith('static const char *backendPaths['))
prefix=r'''
#include "library_model.h"
#include "game_search.h"
#include "game_info.h"
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
typedef uint64_t u64;typedef uint32_t u32;typedef uint16_t u16;
typedef unsigned FS_MediaType;typedef int Result;typedef struct {unsigned unused;} CiaPackage;
#define MEDIATYPE_NAND 0
#define MEDIATYPE_SD 1
#define MEDIATYPE_GAME_CARD 2
#define R_FAILED(r) ((r)<0)
#define R_SUCCEEDED(r) ((r)>=0)
#define CIA_ERROR_INVALID -70
#define DS_BOOTSTRAP_ID 0x0004800546574452ULL
'''
state=r'''
static Game games[MAX_GAMES];static unsigned gameCount,visibleCount,selected,systemView,visible[MAX_GAMES],remembered[VIEW_COUNT];
static bool searchOpen;static char searchQuery[SEARCH_QUERY_SIZE],searchSuggestions[3][SEARCH_QUERY_SIZE];static int searchRanks[MAX_GAMES];
typedef struct {const char *name,*cover,*backend;bool native;const u16 *icon;const char *banner;bool pending;} UiCard;
static UiCard cards[MAX_GAMES],neighborCards[32];static unsigned neighborCount;static const char *searchSystems[MAX_GAMES];
static const char *tr(const char *text){return text;}
static unsigned nativeCalls,dsCalls,seedCalls,hbPrepareCalls,chainloaderCalls,manualSeedCalls;
#define ERROR_REPORT_INSTALLING 1
static void recordActivity(unsigned activity){(void)activity;}static u64 nativeId;static FS_MediaType nativeMedia;static bool nativeReboot;
static bool homebrewContext=false;static int autoboot=1;
static void menu_music_stop(void){}
static void app_log(const char *message){(void)message;}static void app_handoff_log(const char *message){(void)message;}
static void draw(bool loading,float value,const char *text){(void)loading;(void)value;(void)text;}
typedef struct {bool present,ds;char code[5];u64 id;} Cartridge;
static Result cartridge_read(Cartridge *card,unsigned language){(void)card;(void)language;assert(0);return -1;}
static unsigned language=2;
typedef struct {u64 titleID,size;} AM_TitleInfo;
static Result AM_GetTitleInfo(FS_MediaType medium,unsigned count,u64 *id,AM_TitleInfo *info){(void)medium;assert(count==1);info->titleID=*id;info->size=100;return 0;}
static Result launchTitle(u64 id,FS_MediaType medium,bool reboot){nativeCalls++;nativeId=id;nativeMedia=medium;nativeReboot=reboot;return 0;}
static int autoboot_status(void){return autoboot;}
static Result native_handoff_target(u64 *id){*id=0x000400000D921E00ULL;return 0;}
static Result homebrew_prepare_launch(const char *path){assert(path);hbPrepareCalls++;return 0;}
static bool envIsHomebrew(void){return homebrewContext;}
static void aptSetChainloader(u64 id,FS_MediaType medium){(void)id;(void)medium;chainloaderCalls++;}
static Result ds_prepare_launch(const char *path){assert(path&&*path);dsCalls++;return 0;}
static Result emulator_prepare_launch(const char *backend,const char *path,bool gba){(void)backend;(void)path;(void)gba;return 0;}
static Result rom_preflight(const char *backend,const char *path){(void)backend;(void)path;return 0;}
static Result game_seed_ensure(u64 id,FS_MediaType medium,u32 *phase){(void)id;(void)medium;(void)phase;seedCalls++;return 0;}
typedef struct {Game *game;Result result;u32 seedPhase;bool done;} Preflight;
'''
tests=r'''
static void model(void){
 memset(games,0,sizeof(games));gameCount=4;
 games[0]=(Game){.id=0x000400000FF40000ULL,.system=SYSTEM_3DS,.homebrew=true,.dsForwarder=true,.media=MEDIATYPE_SD};strcpy(games[0].name,"Sample Forwarder");strcpy(games[0].key,"000400000FF40000");
 games[1]=(Game){.id=0x00040000001A1E00ULL,.system=SYSTEM_3DS,.homebrew=true,.media=MEDIATYPE_SD};strcpy(games[1].name,"mGBA");
 games[2]=(Game){.id=0x0004000000123400ULL,.system=SYSTEM_3DS,.media=MEDIATYPE_SD};strcpy(games[2].name,"Retail");
 games[3]=(Game){.system=SYSTEM_DS};strcpy(games[3].name,"Sample ROM");strcpy(games[3].path,"sdmc:/Games/sample.nds");
 systemView=VIEW_DS;rebuild();assert(visibleCount==2&&visible[0]==0&&visible[1]==3);assert(!strcmp(cards[0].backend,"YANBF / nds-bootstrap")&&!strcmp(searchSystems[0],"DS"));
 systemView=VIEW_HOMEBREW;rebuild();assert(visibleCount==1&&visible[0]==1);
 systemView=VIEW_3DS;rebuild();assert(visibleCount==1&&visible[0]==2);
 searchOpen=true;strcpy(searchQuery,"nds");rebuild();bool foundForwarder=false,foundRom=false;for(unsigned i=0;i<visibleCount;i++){if(visible[i]==0){foundForwarder=true;assert(!strcmp(searchSystems[i],"DS"));}if(visible[i]==3){foundRom=true;assert(!strcmp(searchSystems[i],"DS"));}}assert(foundForwarder&&foundRom);
 strcpy(searchQuery,"homebrew");rebuild();assert(visibleCount==1&&visible[0]==1);searchOpen=false;
 Preflight forwarder={.game=&games[0]};preflightWorker(&forwarder);assert(forwarder.done&&forwarder.result==0&&seedCalls==0);assert(!launch(&games[0]));assert(nativeCalls==1&&nativeId==games[0].id&&nativeMedia==MEDIATYPE_SD&&!nativeReboot&&dsCalls==0&&hbPrepareCalls==0&&games[0].system==SYSTEM_3DS&&games[0].homebrew);
 Preflight ordinary={.game=&games[1]};preflightWorker(&ordinary);assert(seedCalls==0);assert(!launch(&games[1])&&nativeId==games[1].id&&dsCalls==0);
 Preflight retail={.game=&games[2]};preflightWorker(&retail);assert(seedCalls==1);assert(!launch(&games[2])&&nativeId==games[2].id);
 assert(!launch(&games[3])&&dsCalls==1&&nativeId==DS_BOOTSTRAP_ID&&nativeMedia==MEDIATYPE_NAND);
 Game hbl={.id=0x000400000D921E00ULL,.system=SYSTEM_3DS,.homebrew=true,.media=MEDIATYPE_SD};assert(!launch(&hbl)&&hbPrepareCalls==1&&chainloaderCalls==1);
 visibleCount=1;selected=0;visible[0]=0;importSelectedSeed();assert(manualSeedCalls==0);visible[0]=2;importSelectedSeed();assert(manualSeedCalls==1);
 puts("PASS actual rebuild/search/neighbors and launch/preflight: YANBF in DS, native ID/media launch, no ROM-path or seed routing, ordinary Homebrew/HBL/retail/DS-ROM unchanged");
}
static void paths(void){
 memset(games,0,sizeof(games));gameCount=1;games[0].system=SYSTEM_DS;strcpy(games[0].path,"sdmc:/Games/keep.nds");romPathsBegin();assert(romFound("SDMC:/games/KEEP.NDS",NULL)&&gameCount==1);
 assert(romFound("sdmc:/GBA/one/same.gba",NULL));assert(romFound("sdmc:/GBA/two/same.gba",NULL)&&gameCount==3);assert(!strcmp(games[1].key,games[2].key)&&strcmp(games[1].path,games[2].path));
 // Deliberate same-bucket collisions must still compare full persistent paths.
 gameCount=0;romPathsBegin();unsigned target=romPathHash("sdmc:/NES/collision-0.nes")&(ROM_PATH_SLOTS-1),collected=0;
 for(unsigned i=0;collected<24;i++){char path[96];snprintf(path,sizeof(path),"sdmc:/NES/collision-%u.nes",i);if((romPathHash(path)&(ROM_PATH_SLOTS-1))!=target)continue;assert(romFound(path,NULL));strcpy(path,"temporary buffer overwritten");collected++;}
 assert(gameCount==24);for(unsigned i=0;i<gameCount;i++){char stable[512];strcpy(stable,games[i].path);assert(romFound(stable,NULL)&&gameCount==24);}
 // Approximate the user's mixture without a single filesystem operation.
 memset(games,0,sizeof(games));gameCount=198;romPathsBegin();for(unsigned i=0;i<846;i++){char path[96];snprintf(path,sizeof(path),"sdmc:/GBA/group/game-%04u.gba",i);assert(romFound(path,NULL));}assert(gameCount==1044&&romPathComparisons<5000);printf("PASS actual ROM path index:846 entries use %u full-path comparisons (previous linear path would use524943)\n",romPathComparisons);
 while(gameCount<MAX_GAMES){char path[96];snprintf(path,sizeof(path),"sdmc:/NES/fill-%u.nes",gameCount);assert(romFound(path,NULL));}assert(!romFound("sdmc:/NES/overflow.nes",NULL)&&gameCount==MAX_GAMES);
 char existing[512];strcpy(existing,games[MAX_GAMES-1].path);assert(romFound(existing,NULL));
 gameCount=0;romPathsBegin();assert(romFound(existing,NULL)&&gameCount==1); // abort/restart never retains an old index
 puts("PASS actual ROM path index: case, fullpath identity, collisions, temporary-buffer reuse,1536 cap and reset");
}
int main(void){model();paths();return 0;}
'''
with tempfile.TemporaryDirectory(prefix='flow-yanbf-main-') as directory:
    folder=Path(directory);test=folder/'test.c'
    seed_guard=function('importSelectedSeed').split('    SeedJob job=')[0]+'    manualSeedCalls++;\n}'
    test.write_text(prefix+game+state+backend+'\n'+hash_callback+'\nstatic void '+function('rebuild')+'\nstatic Result '+function('launch')+'\nstatic void '+function('preflightWorker')+'\nstatic void '+seed_guard+'\n'+tests)
    flags=[os.environ.get('CC','cc'),'-std=c11','-O2','-Wall','-Wextra','-Werror','-fsanitize=undefined','-I'+str(root/'launcher/source')]
    subprocess.run(flags+[str(test),str(root/'launcher/source/library_model.c'),str(root/'launcher/source/game_search.c'),str(root/'launcher/source/game_info.c'),'-o',str(folder/'test')],check=True)
    subprocess.run([str(folder/'test')],cwd=folder,check=True)
assert 'library_title_display_system(g->system,g->dsForwarder)' in function('openInfo')
assert 'DS HOME-Verknüpfung' in function('openInfo')
assert '.systemIndex=visibleCount?(unsigned)library_title_display_system(' in function('draw')
scan=function('scan');assert scan.index('memset(romPathSlots')<scan.index('ui_clear_covers')
assert scan.index('qsort(games')<scan.rindex('memset(romPathSlots')
print('PASS: current details use DS presentation, path index cleared before each scan and invalidated after sort')
