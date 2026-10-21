from pathlib import Path
s=Path('outputs/3ds-library/source/main.c').read_text()
launch=s[s.index('static Result launchTitle'):s.index('static void installProgress')]
prefix=r'''
#include <3ds.h>
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
typedef unsigned short u16;
#define MEDIATYPE_GAME_CARD ((FS_MediaType)2)
#include "cartridge.h"
#include "library_model.h"
#include "ds_backend.h"
#define tr(x) (x)
#define CIA_ERROR_INVALID ((Result)-70)
typedef struct {u64 id;FS_MediaType media;int system;bool pending,homebrew,cartridge;char path[512],key[256];} Game;
static bool uiReleased,home=true,hb=true,initOK=true,transfer;
static int queryRC,prepareRC,jumpRC,nsRC,rebootRC,dsRC;
static unsigned stopped,started,prepared,jumped,rebooted,waited,exited,nsOpened,nsClosed,dsCalls;
static u64 expectedId;static FS_MediaType expectedMedia;
static jmp_buf escape;
static unsigned language=2;static Cartridge card;
static int bootState,hbPrepRC,hbCalls,chainCalls,configRC,emulatorRC;
static const char *backendPaths[SYSTEM_COUNT]={0};
static void app_log(const char *p){assert(p);}
static void app_handoff_log(const char *p){assert(p);}
static void menu_music_stop(void){}
static int aptGetMenuAppID(void){return 257;}
static Result APT_IsRegistered(int id,bool *out){assert(id==257);*out=home;return queryRC;}
static Result nsInit(void){nsOpened++;return nsRC;}
static void nsExit(void){nsClosed++;}
static Result NS_RebootToTitle(u8 media,u64 id,u8 mem){assert(uiReleased&&stopped==1&&media==expectedMedia&&id==expectedId&&mem==0);rebooted++;return rebootRC;}
static void svcSleepThread(long long time){assert(time==100000000);waited++;if(transfer)longjmp(escape,2);}
static void svcExitProcess(void){assert(uiReleased&&stopped==1&&jumped==1&&!started);exited++;longjmp(escape,1);}
static void ui_shutdown(void){assert(prepared==1||nsOpened==1);stopped++;}
static bool ui_init(void){assert(uiReleased&&stopped==1);started++;return initOK;}
static void ui_runtime_reset(void){}
Result APT_PrepareToDoApplicationJump(u8 flags,u64 id,u8 media){assert(!flags&&!stopped&&id==expectedId&&media==expectedMedia);prepared++;return prepareRC;}
Result APT_DoApplicationJump(const void *p,u32 n,const void *h){assert(uiReleased&&stopped==1&&n==0x300);for(unsigned i=0;i<n;i++)assert(!((const u8*)p)[i]);for(unsigned i=0;i<0x20;i++)assert(!((const u8*)h)[i]);jumped++;return jumpRC;}
Result AM_GetTitleInfo(FS_MediaType media,u32 n,u64 *id,AM_TitleInfo *out){assert(media==expectedMedia&&n==1&&*id==expectedId);out->titleID=*id;out->size=100;return 0;}
Result cartridge_read(Cartridge *out,unsigned lang){assert(lang==2);*out=card;return 0;}
Result ds_prepare_launch(const char *p){(void)p;dsCalls++;return dsRC;}
static bool envIsHomebrew(void){return hb;}
static int autoboot_status(void){return bootState;}
static Result native_handoff_target(u64 *id){*id=0x000400000D921E00ULL;return configRC;}
static Result homebrew_prepare_launch(const char *p){assert(!strcmp(p,"sdmc:/3ds/3ds-library/autoboot-hbmenu.3dsx"));hbCalls++;return hbPrepRC;}
static void aptSetChainloader(u64 id,u8 media){assert(id==0x000400000D921E00ULL&&media==MEDIATYPE_SD);chainCalls++;}
static Result emulator_prepare_launch(const char *p,const char *rom,bool archive){(void)p;(void)rom;assert(archive);return emulatorRC;}
static void draw(bool a,float b,const char *c){(void)a;(void)b;(void)c;}
'''
tail=r'''
static void reset(void){uiReleased=false;home=hb=initOK=true;transfer=false;queryRC=prepareRC=jumpRC=nsRC=rebootRC=dsRC=0;stopped=started=prepared=jumped=rebooted=waited=exited=nsOpened=nsClosed=dsCalls=0;bootState=hbPrepRC=hbCalls=chainCalls=configRC=emulatorRC=0;}
static void accepted(Game *g,bool cold){home=!cold;transfer=cold;int outcome=setjmp(escape);if(!outcome){launch(g);assert(!"Accepted handoff must never return to main cleanup");}assert(stopped==1&&!started);if(cold)assert(outcome==2&&rebooted==1&&!prepared&&!jumped&&!exited&&nsOpened==1&&nsClosed==1);else assert(outcome==1&&prepared==1&&jumped==1&&exited==1&&!nsOpened&&!rebooted);}
int main(void){
 Game g={.system=SYSTEM_DS};expectedId=DS_BOOTSTRAP_ID;expectedMedia=MEDIATYPE_NAND;
 reset();accepted(&g,false);assert(dsCalls==1);
 reset();accepted(&g,true);assert(dsCalls==1);
 reset();dsRC=-30;assert(launch(&g)==-30&&!stopped&&!prepared&&!nsOpened);
 reset();queryRC=-11;assert(launch(&g)==-11&&!stopped&&!prepared&&!nsOpened);
 reset();prepareRC=-12;assert(launch(&g)==-12&&!stopped&&!jumped);
 reset();jumpRC=-13;assert(launch(&g)==-13&&started==1&&!uiReleased&&!exited);
 reset();jumpRC=-13;initOK=false;assert(launch(&g)==-13&&uiReleased);
 reset();home=false;nsRC=-14;assert(launch(&g)==-14&&!stopped&&!rebooted);
 reset();home=false;rebootRC=-15;assert(launch(&g)==-15&&started==1&&!uiReleased&&!waited);
 reset();home=false;assert(launch(&g)==-105&&waited==100&&started==1&&!uiReleased&&!exited);
 g.system=SYSTEM_3DS;g.id=expectedId=0x000400000012BE00ULL;g.media=expectedMedia=MEDIATYPE_SD;
 reset();accepted(&g,false);reset();accepted(&g,true);
 reset();g.homebrew=true;accepted(&g,false);reset();g.homebrew=true;accepted(&g,true);
 reset();g.id=expectedId=0x000400000D921E00ULL;bootState=1;assert(!launch(&g)&&hbCalls==1&&!stopped);hb=false;assert(!launch(&g)&&chainCalls==1);g.homebrew=false;
 reset();g.cartridge=true;g.system=SYSTEM_DS;strcpy(g.key,"cart-ABCE");card=(Cartridge){.present=true,.ds=true};strcpy(card.code,"ABCE");expectedId=0;expectedMedia=MEDIATYPE_GAME_CARD;transfer=true;home=true;
 if(!setjmp(escape)){launch(&g);assert(0);}assert(rebooted==1&&!prepared&&!dsCalls); // DS card must reboot even with HOME
 reset();card.present=false;assert(launch(&g)==-103&&!stopped);
 reset();g.cartridge=false;g.system=SYSTEM_GBA;assert(!launch(&g)&&!stopped&&!rebooted);hb=false;assert(!launch(&g)&&chainCalls==1);
 puts("PASS real launch code: DS CIA and coldboot routes; graphics released before IPC; exactly one handoff; failed prepare preserves UI; rejected/timeout handoff restores UI; native/homebrew/card/emulator regressions");
}
'''
Path('work/v0137/launch_test.c').write_text(prefix+launch+tail)
