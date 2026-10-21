from pathlib import Path
import subprocess
root=Path('/home/maxim/codex-3ds-work/toolchain/project')
src=(root/'source/main.c').read_text()
launch=src[src.index('static Result launch(Game *g){'):src.index('static void installProgress',src.index('static Result launch(Game *g){'))]
test=r'''
#include <3ds.h>
#define tr(x) (x)
#define MEDIATYPE_GAME_CARD ((FS_MediaType)2)
typedef unsigned short u16;
#include "cartridge.h"
static void menu_music_stop(void){}
#include <string.h>
#include <assert.h>
#include "twl_repair.h"
static unsigned twlMissing;void twl_diagnose(TwlReport *r){memset(r,0,sizeof(*r));r->known=true;r->missing=twlMissing;}
#define FLOW_ERROR_HBL_CONFIG ((Result)-90)
#define FLOW_ERROR_HBL_MISSING ((Result)-91)
static int bootState,hbPrepError,hbCalls;static int autoboot_status(void){return bootState;}
static Result homebrew_prepare_launch(const char *p){assert(!strcmp(p,"sdmc:/3ds/3ds-library/autoboot-hbmenu.3dsx"));hbCalls++;return hbPrepError;}
static bool hb=true;static int configResult,emulatorCalls;static bool envIsHomebrew(void){return hb;}
static Result native_handoff_target(u64 *id){*id=0x000400000D921E00ULL;return configResult;}
#include "library_model.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#define CIA_ERROR_INVALID ((Result)-70)
typedef struct {u64 id;FS_MediaType media;int system;bool pending,homebrew,cartridge;char path[512],key[256];} Game;
static bool cardLaunch,transferred;static int nsRC,handoffRC,cardHandoffs,cardReads;static unsigned language=2;static Cartridge fixture;
static int mode,metadataCalls,prepareCalls,jumpCalls,deferredCalls;
static bool uiReleased;static int shutdownCalls,initCalls,exitCalls;static jmp_buf exited;
static void ui_shutdown(void){assert(cardLaunch||(prepareCalls==1&&!jumpCalls));shutdownCalls++;}
static bool ui_init(void){assert(cardLaunch||jumpCalls==1);initCalls++;return mode!=6;}
static void ui_runtime_reset(void){}
static void svcExitProcess(void){assert(uiReleased&&shutdownCalls==1&&jumpCalls==1&&!initCalls);exitCalls++;longjmp(exited,1);}
static u64 expectedId=0x000400000012BE00ULL;static FS_MediaType expectedMedia=MEDIATYPE_SD;
static const char *backendPaths[SYSTEM_COUNT]={0};
void app_log(const char *text){assert(text&&*text);}
Result AM_GetTitleInfo(FS_MediaType media,u32 n,u64 *id,AM_TitleInfo *out){assert(media==expectedMedia&&n==1&&*id==expectedId);metadataCalls++;out->titleID=mode==2?*id+1:*id;out->size=mode==3?0:100;return mode==1?-111:0;}
Result APT_PrepareToDoApplicationJump(u8 flags,u64 id,u8 media){assert(!flags&&id==expectedId&&media==expectedMedia);prepareCalls++;return mode==4?-112:0;}
Result APT_DoApplicationJump(const void *p,u32 n,const void *h){assert(uiReleased&&shutdownCalls==1&&n==0x300);for(unsigned i=0;i<n;i++)assert(!((const u8*)p)[i]);for(unsigned i=0;i<0x20;i++)assert(!((const u8*)h)[i]);jumpCalls++;return mode==5||mode==6?-113:0;}
static void aptSetChainloader(u64 id,u8 media){assert(id==(hb?expectedId:0x000400000D921E00ULL)&&media==expectedMedia);deferredCalls++;}
static unsigned dsCalls;static Result ds_prepare_launch(const char *p){(void)p;dsCalls++;return -21;}
static Result ds_commit_launch(void){assert(!"Unexpected DS dispatch");return -1;}
static void draw(bool loading,float progress,const char *stage){(void)loading;(void)progress;(void)stage;}
static Result emulator_prepare_launch(const char *p,const char *rom,bool gba){(void)p;(void)rom;assert(gba);emulatorCalls++;return mode==7?-117:0;}
'''
test+=r'''
Result cartridge_read(Cartridge*out,unsigned lang){assert(lang==2);*out=fixture;cardReads++;return 0;}
static Result nsInit(void){return nsRC;}static void nsExit(void){}
static Result NS_RebootToTitle(u8 media,u64 id,u8 mem){assert(media==2&&id==0&&mem==0&&uiReleased);cardHandoffs++;return handoffRC;}
static void svcSleepThread(long long t){assert(t==100000000);if(transferred)longjmp(exited,2);}
'''
test+=launch
test+=r'''
static void reset(void){mode=metadataCalls=prepareCalls=jumpCalls=deferredCalls=shutdownCalls=initCalls=exitCalls=0;uiReleased=false;}
static void successful_launch(Game *g){if(!setjmp(exited)){launch(g);assert(!"Successful jump must exit process without returning to normal shutdown");}assert(exitCalls==1&&prepareCalls==1&&jumpCalls==1&&!deferredCalls&&!initCalls);}
int main(void){
 Game blocked={.system=SYSTEM_DS};twlMissing=5;assert(launch(&blocked)==(Result)-21&&dsCalls==1);twlMissing=0;assert(launch(&blocked)==(Result)-21&&dsCalls==2);
 Game g={.id=0x000400000012BE00ULL,.media=MEDIATYPE_SD,.system=SYSTEM_3DS};
 reset();g.pending=true;assert(launch(&g)==CIA_ERROR_INVALID&&!metadataCalls&&!prepareCalls);g.pending=false;
 reset();mode=1;assert(launch(&g)==-111&&metadataCalls==1&&!prepareCalls&&!deferredCalls);
 for(int m=2;m<=3;m++){reset();mode=m;assert(launch(&g)==-2&&!prepareCalls&&!jumpCalls);}
 reset();mode=4;assert(launch(&g)==-112&&prepareCalls==1&&!jumpCalls&&!deferredCalls&&!shutdownCalls);
 reset();mode=5;assert(launch(&g)==-113&&prepareCalls==1&&jumpCalls==1&&!deferredCalls&&initCalls==1&&!uiReleased&&!exitCalls);
 reset();mode=6;assert(launch(&g)==-113&&uiReleased&&initCalls==1&&!exitCalls);
 reset();successful_launch(&g);
 reset();g.homebrew=true;bootState=1;successful_launch(&g);
 reset();u64 savedId=g.id;g.id=expectedId=0x000400000D921E00ULL;hbCalls=0;assert(!launch(&g)&&hbCalls==1&&!prepareCalls&&!deferredCalls);
 reset();hb=false;assert(!launch(&g)&&deferredCalls==1);hb=true;
 reset();hbPrepError=-21;assert(launch(&g)==-21&&!prepareCalls&&!deferredCalls);hbPrepError=0;
 reset();configResult=-90;assert(launch(&g)==-90&&!prepareCalls);configResult=0;
 g.id=expectedId=savedId;bootState=0;
 reset();g.homebrew=false;g.media=expectedMedia=MEDIATYPE_NAND;successful_launch(&g);
 reset();g.system=SYSTEM_GBA;expectedMedia=MEDIATYPE_SD;g.media=MEDIATYPE_SD;assert(!launch(&g)&&emulatorCalls==1&&!deferredCalls);
 hb=false;configResult=-90;assert(launch(&g)==-90&&emulatorCalls==1&&!deferredCalls);
 configResult=0;assert(!launch(&g)&&emulatorCalls==2&&deferredCalls==1);
 reset();mode=7;assert(launch(&g)==-117&&!deferredCalls);

 reset();cardLaunch=true;g.cartridge=true;g.system=SYSTEM_DS;strcpy(g.key,"cart-ABCE");
 fixture=(Cartridge){0};assert(launch(&g)==-103&&!shutdownCalls);
 fixture.present=true;fixture.ds=true;strcpy(fixture.code,"BAD!");assert(launch(&g)==-103&&!shutdownCalls);
 strcpy(fixture.code,"ABCE");nsRC=-120;assert(launch(&g)==-120&&!shutdownCalls);nsRC=0;
 handoffRC=-121;assert(launch(&g)==-121&&initCalls==1&&!uiReleased);handoffRC=0;
 reset();assert(launch(&g)==-105&&initCalls==1&&!uiReleased);
 reset();g.system=SYSTEM_3DS;fixture.ds=false;fixture.id=g.id;transferred=true;
 g.media=expectedMedia=MEDIATYPE_GAME_CARD;successful_launch(&g);assert(uiReleased&&shutdownCalls==1&&prepareCalls==1&&jumpCalls==1);
 assert(cardReads==6&&cardHandoffs==2);
 reset();mode=4;assert(launch(&g)==-112&&prepareCalls==1&&!shutdownCalls&&cardHandoffs==2);
 reset();mode=5;assert(launch(&g)==-113&&initCalls==1&&!uiReleased&&cardHandoffs==2);
 reset();fixture.id++;assert(launch(&g)==-103&&!prepareCalls);
 puts("PASS actual physical launch: removed/wrong card, NS error, rejected handoff, bounded timeout recovery, DS reboot route preserved; CTR card uses normal APT title/media, removal guard, prepare and jump failure recovery");
 puts("PASS actual CIA emulator launch: existing 3DSX route unchanged, configured HBL chainload after successful setup only, configuration/argument failure blocks handoff");
 puts("PASS actual native launch: release UI before jump, process exit without second APT/HBL handoff, exact title/media, zeroed args, preparation failure preserves UI, rejected jump restores UI, recovery failure skips rendering, homebrew route preserved");
}
'''
(root/'v0103').mkdir(exist_ok=True)
(root/'v0103/launch.c').write_text(test)
subprocess.run(['chroot','/home/maxim/codex-3ds-work/toolchain','/bin/bash','-c','set -e;cd /project;gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all -Imock-sdk -Isource v0103/launch.c -o v0103/launch-test;v0103/launch-test'],check=True)

