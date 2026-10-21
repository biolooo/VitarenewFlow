
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
static bool newModel=true;static int modelRC;static unsigned modelQueries;static u8 expectedMem;
static bool uiReleased,home=true,hb=true,initOK=true,transfer;
static unsigned metadataCalls;static u64 metadataSize=100;static bool metadataMismatch;static int metadataRC,cardRC;
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
Result APT_CheckNew3DS(bool *out){modelQueries++;*out=newModel;return modelRC;}
static Result nsInit(void){nsOpened++;return nsRC;}
static void nsExit(void){nsClosed++;}
static Result NS_RebootToTitle(u8 media,u64 id,u8 mem){assert(uiReleased&&stopped==1&&media==expectedMedia&&id==expectedId&&mem==expectedMem);rebooted++;return rebootRC;}
static void svcSleepThread(long long time){assert(time==100000000);waited++;if(transfer)longjmp(escape,2);}
static void svcExitProcess(void){assert(uiReleased&&stopped==1&&jumped==1&&!started);exited++;longjmp(escape,1);}
static void ui_shutdown(void){assert(prepared==1||nsOpened==1);stopped++;}
static bool ui_init(void){assert(uiReleased&&stopped==1);started++;return initOK;}
static void ui_runtime_reset(void){}
Result APT_PrepareToDoApplicationJump(u8 flags,u64 id,u8 media){assert(!flags&&!stopped&&id==expectedId&&media==expectedMedia);prepared++;return prepareRC;}
Result APT_DoApplicationJump(const void *p,u32 n,const void *h){assert(uiReleased&&stopped==1&&n==0x300);for(unsigned i=0;i<n;i++)assert(!((const u8*)p)[i]);for(unsigned i=0;i<0x20;i++)assert(!((const u8*)h)[i]);jumped++;return jumpRC;}
Result AM_GetTitleInfo(FS_MediaType media,u32 n,u64 *id,AM_TitleInfo *out){assert(media==expectedMedia&&n==1&&*id==expectedId);metadataCalls++;out->titleID=*id+(metadataMismatch?1:0);out->size=metadataSize;return metadataRC;}
Result cartridge_read(Cartridge *out,unsigned lang){assert(lang==2);*out=card;return cardRC;}
Result ds_prepare_launch(const char *p){(void)p;dsCalls++;return dsRC;}
static bool envIsHomebrew(void){return hb;}
static int autoboot_status(void){return bootState;}
static Result native_handoff_target(u64 *id){*id=0x000400000D921E00ULL;return configRC;}
static Result homebrew_prepare_launch(const char *p){assert(!strcmp(p,"sdmc:/3ds/3ds-library/autoboot-hbmenu.3dsx"));hbCalls++;return hbPrepRC;}
static void aptSetChainloader(u64 id,u8 media){assert(id==0x000400000D921E00ULL&&media==MEDIATYPE_SD);chainCalls++;}
static Result emulator_prepare_launch(const char *p,const char *rom,bool archive){(void)p;(void)rom;assert(archive);return emulatorRC;}
static void draw(bool a,float b,const char *c){(void)a;(void)b;(void)c;}
static Result launchTitle(u64 id,FS_MediaType media,bool forceReboot){
    bool registered=false;Result r=APT_IsRegistered(aptGetMenuAppID(),&registered);
    char log[160];snprintf(log,sizeof(log),"Handoff target=%016llX media=%u HOME=%u query=%08lX",(unsigned long long)id,(unsigned)media,registered,(unsigned long)(u32)r);app_handoff_log(log);
    if(R_FAILED(r))return r;
    bool reboot=forceReboot||!registered;
    u8 appmemtype=0;
    if(reboot&&(id>>32)==0x00040000ULL){
        /* Raw FIRM appmemtype, not the index from Luma's configuration UI.
           Normal is 0 on O3DS, 6 on N3DS (see Luma deliver_arg.c). Rebooting
           N3DS CTR software with 0 can leave HOME with the wrong memory map. */
        bool new3ds=false;r=APT_CheckNew3DS(&new3ds);
        snprintf(log,sizeof(log),"Handoff model new3ds=%u result=%08lX",new3ds,(unsigned long)(u32)r);app_handoff_log(log);
        if(R_FAILED(r))return r;
        appmemtype=new3ds?6:0;
    }
    r=reboot?nsInit():APT_PrepareToDoApplicationJump(0,id,media);
    snprintf(log,sizeof(log),"Handoff %s prepare=%08lX",reboot?"NS reboot":"APT jump",(unsigned long)(u32)r);app_handoff_log(log);
    if(R_FAILED(r))return r;
    ui_shutdown();uiReleased=true;
    if(reboot){
        r=NS_RebootToTitle(media,id,appmemtype);nsExit();
        snprintf(log,sizeof(log),"Handoff NS appmemtype=%u request=%08lX",appmemtype,(unsigned long)(u32)r);app_handoff_log(log);
        /* NS coordinates termination asynchronously. Do not perform aptExit
           or return through hbmenu while the reboot request is outstanding. */
        if(R_SUCCEEDED(r)){
            for(unsigned i=0;i<100;i++)svcSleepThread(100000000);
            r=(Result)-105;app_handoff_log("Handoff timed out after 10 seconds");
        }
    }else{
        u8 parameters[0x300]={0},hmac[0x20]={0};
        r=APT_DoApplicationJump(parameters,sizeof(parameters),hmac);
        snprintf(log,sizeof(log),"Handoff APT result=%08lX",(unsigned long)(u32)r);app_handoff_log(log);
        if(R_SUCCEEDED(r)){
            app_handoff_log("Handoff accepted; exit process without second APT close");
            svcExitProcess();
        }
    }
    uiReleased=!ui_init();ui_runtime_reset();
    app_handoff_log(uiReleased?"Handoff recovery failed":"Handoff rejected; UI restored");
    return r;
}
static Result launch(Game *g){
    menu_music_stop();
    /* Title presence cannot predict DS compatibility. Let the actual loader run. */
    if(g->pending)return CIA_ERROR_INVALID;
    if(g->cartridge){
        Cartridge card;Result r=cartridge_read(&card,language);
        if(R_FAILED(r)||!card.present||card.ds!=(g->system==SYSTEM_DS)||(card.ds?strcmp(card.code,g->key+5)!=0:card.id!=g->id))return (Result)-103;
        if(card.ds)return launchTitle(0,MEDIATYPE_GAME_CARD,true);
        /* The freshly read card identity is authoritative. Installed-title
           metadata (especially installed size) is not a prerequisite for cards. */
        g->media=MEDIATYPE_GAME_CARD;g->homebrew=false;
        char log[96];snprintf(log,sizeof(log),"Verified CTR cartridge=%016llX; launch from game-card media",(unsigned long long)card.id);app_handoff_log(log);
        return launchTitle(card.id,MEDIATYPE_GAME_CARD,false);
    }
    if(g->system==SYSTEM_3DS){AM_TitleInfo info={0};
    u64 id=g->id;
    Result r=AM_GetTitleInfo(g->media,1,&id,&info);
    char entry[192];snprintf(entry,sizeof(entry),"Launch requested=%016llX returned=%016llX media=%u size=%llu metadata=%08lX",(unsigned long long)id,(unsigned long long)info.titleID,(unsigned)g->media,(unsigned long long)info.size,(unsigned long)(u32)r);app_handoff_log(entry);
        if(R_FAILED(r))return r;
        if(info.titleID!=id || !info.size)return (Result)-2;
        if(g->homebrew&&autoboot_status()==1){
            u64 hbTarget=0;Result check=native_handoff_target(&hbTarget);
            if(R_SUCCEEDED(check)&&hbTarget==id){
                r=homebrew_prepare_launch("sdmc:/3ds/3ds-library/autoboot-hbmenu.3dsx");
                if(R_FAILED(r))return r;
                if(!envIsHomebrew())aptSetChainloader(id,g->media);
                app_log("Homebrew Launcher: explicit original HBL target");return 0;
            }
            if(R_FAILED(check)&&id==0x000400000D921E00ULL)return check;
        }
        return launchTitle(id,g->media,false);}
    if(g->system==SYSTEM_DS){
        app_handoff_log(g->path);
        Result r=ds_prepare_launch(g->path);
        if(R_FAILED(r)){char log[80];snprintf(log,sizeof(log),"DS preflight=%08lX",(unsigned long)(u32)r);app_handoff_log(log);return r;}
        draw(true,1,tr("Wechsel in den DS-Modus"));
        return launchTitle(DS_BOOTSTRAP_ID,MEDIATYPE_NAND,false);
    }
    u64 hbTarget=0;
    if(!envIsHomebrew()){Result r=native_handoff_target(&hbTarget);if(R_FAILED(r))return r;}
    Result r=emulator_prepare_launch(backendPaths[g->system],g->path,g->system==SYSTEM_GBA||g->system==SYSTEM_GB);
    if(R_SUCCEEDED(r)&&!envIsHomebrew()){aptSetChainloader(hbTarget,MEDIATYPE_SD);app_log("CIA emulator handoff: configured Rosalina title");}
    return r;
}

static void reset(void){newModel=true;modelRC=0;modelQueries=0;expectedMem=0;metadataCalls=0;metadataSize=100;metadataMismatch=false;metadataRC=cardRC=0;uiReleased=false;home=hb=initOK=true;transfer=false;queryRC=prepareRC=jumpRC=nsRC=rebootRC=dsRC=0;stopped=started=prepared=jumped=rebooted=waited=exited=nsOpened=nsClosed=dsCalls=0;bootState=hbPrepRC=hbCalls=chainCalls=configRC=emulatorRC=0;}
static void accepted(Game *g,bool cold){home=!cold;transfer=cold;expectedMem=cold&&(g->id>>32)==0x00040000ULL&&newModel?6:0;int outcome=setjmp(escape);if(!outcome){launch(g);assert(!"Accepted handoff must never return to main cleanup");}assert(stopped==1&&!started);if(cold)assert(outcome==2&&rebooted==1&&!prepared&&!jumped&&!exited&&nsOpened==1&&nsClosed==1);else assert(outcome==1&&prepared==1&&jumped==1&&exited==1&&!nsOpened&&!rebooted);}
int main(void){
 Game g={.system=SYSTEM_DS};expectedId=DS_BOOTSTRAP_ID;expectedMedia=MEDIATYPE_NAND;
 reset();accepted(&g,false);assert(dsCalls==1&&!modelQueries);
 reset();accepted(&g,true);assert(dsCalls==1&&!modelQueries);
 reset();dsRC=-30;assert(launch(&g)==-30&&!stopped&&!prepared&&!nsOpened);
 reset();queryRC=-11;assert(launch(&g)==-11&&!stopped&&!prepared&&!nsOpened);
 reset();prepareRC=-12;assert(launch(&g)==-12&&!stopped&&!jumped);
 reset();jumpRC=-13;assert(launch(&g)==-13&&started==1&&!uiReleased&&!exited);
 reset();jumpRC=-13;initOK=false;assert(launch(&g)==-13&&uiReleased);
 reset();home=false;nsRC=-14;assert(launch(&g)==-14&&!stopped&&!rebooted);
 reset();home=false;rebootRC=-15;assert(launch(&g)==-15&&started==1&&!uiReleased&&!waited);
 reset();home=false;assert(launch(&g)==-105&&waited==100&&started==1&&!uiReleased&&!exited);
 g.system=SYSTEM_3DS;g.id=expectedId=0x000400000012BE00ULL;g.media=expectedMedia=MEDIATYPE_SD;
 reset();accepted(&g,false);assert(!modelQueries);reset();accepted(&g,true);assert(modelQueries==1);
 reset();newModel=false;accepted(&g,true);assert(modelQueries==1&&expectedMem==0);
 reset();home=false;modelRC=-111;assert(launch(&g)==-111&&!stopped&&!nsOpened&&!rebooted);
 reset();metadataSize=0;assert(launch(&g)==-2&&!stopped&&!prepared&&!nsOpened);
 reset();metadataMismatch=true;assert(launch(&g)==-2&&!stopped);
 reset();metadataRC=-21;assert(launch(&g)==-21&&!stopped);
 g.cartridge=true;card=(Cartridge){.present=true,.id=g.id};g.media=expectedMedia=MEDIATYPE_GAME_CARD;
 reset();metadataSize=0;accepted(&g,false);assert(!metadataCalls&&!modelQueries);reset();metadataSize=0;accepted(&g,true);assert(!metadataCalls&&modelQueries==1&&expectedMem==6);
 reset();newModel=false;accepted(&g,true);assert(expectedMem==0);
 reset();home=false;modelRC=-111;assert(launch(&g)==-111&&!stopped&&!nsOpened&&!rebooted);
 reset();home=false;rebootRC=-112;expectedMem=6;assert(launch(&g)==-112&&started==1&&!uiReleased);
 reset();metadataMismatch=true;metadataRC=-90;accepted(&g,false);assert(!metadataCalls);
 reset();card.id++;assert(launch(&g)==-103&&!stopped);card.id--;
 reset();card.present=false;assert(launch(&g)==-103&&!stopped);card.present=true;
 reset();cardRC=-10;assert(launch(&g)==-103&&!stopped);
 g.cartridge=false;g.media=expectedMedia=MEDIATYPE_SD;
 reset();g.homebrew=true;accepted(&g,false);reset();g.homebrew=true;accepted(&g,true);
 reset();g.id=expectedId=0x000400000D921E00ULL;bootState=1;assert(!launch(&g)&&hbCalls==1&&!stopped);hb=false;assert(!launch(&g)&&chainCalls==1);g.homebrew=false;
 reset();g.cartridge=true;g.system=SYSTEM_DS;strcpy(g.key,"cart-ABCE");card=(Cartridge){.present=true,.ds=true};strcpy(card.code,"ABCE");expectedId=0;expectedMedia=MEDIATYPE_GAME_CARD;transfer=true;home=true;
 if(!setjmp(escape)){launch(&g);assert(0);}assert(rebooted==1&&!prepared&&!dsCalls); // DS card must reboot even with HOME
 reset();card.present=false;assert(launch(&g)==-103&&!stopped);
 reset();g.cartridge=false;g.system=SYSTEM_GBA;assert(!launch(&g)&&!stopped&&!rebooted);hb=false;assert(!launch(&g)&&chainCalls==1);
 puts("PASS real launch code: DS CIA and coldboot routes; graphics released before IPC; exactly one handoff; failed prepare preserves UI; rejected/timeout handoff restores UI; native/homebrew/card/emulator regressions; raw N3DS memory type 6 and O3DS type 0; DS unchanged; model query failure prevents launch");
}
