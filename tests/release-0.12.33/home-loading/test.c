
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
typedef int Result;typedef uint32_t u32;typedef uint64_t u64;typedef uint8_t u8;typedef unsigned NS_APPID;
typedef struct {u64 titleID;} AM_TitleInfo;
#define R_SUCCEEDED(x) ((x)>=0)
#define R_FAILED(x) ((x)<0)
#define MEDIATYPE_NAND 1
#define tr(x) (x)
static bool uiReleased,musicReady,homeUnavailable,settings,autobootPrompt,powerPrompt,powerCancelFailed,installing,preparingGame;
static unsigned twlPage,homeLoading;
static const char *status;static char message[180];
static bool registered,allowPending,jump,closing,rejected,active=true,sleepAllowed=true,homeAllowed;
static unsigned queries,registerAt,frames,drains,jumps,publishes,launches,saves,reboots,waits,logs,shutdowns;
static unsigned phaseFrames[4];static u64 clockMs;static Result queryRC,launchRC,publishRC,powerRC=-5;static u8 region=2;
static bool aptIsActive(void){return active;}static bool aptShouldClose(void){return closing;}
static bool aptShouldJumpToHome(void){return jump;}static void aptHandleSleep(void){}
static bool aptCheckHomePressRejected(void){bool r=rejected;rejected=false;return r;}
static NS_APPID aptGetMenuAppID(void){return 257;}
static Result APT_IsRegistered(NS_APPID id,bool *out){assert(id==257);queries++;if(registerAt&&queries>=registerAt)registered=true;*out=registered;return queryRC;}
static void ui_runtime_allow_pending_home(bool b){allowPending=b;}
static void ui_runtime_reset(void){}
static void draw(bool loading,float progress,const char *stage){assert(loading&&progress==0&&stage&&!uiReleased&&active&&allowPending&&homeLoading>0&&homeLoading<4);frames++;phaseFrames[homeLoading]++;}
static void ui_runtime_drain(void){assert(!uiReleased&&frames);drains++;}
static void svcSleepThread(int64_t ns){assert(ns==20000000||ns==100000000);clockMs+=ns/1000000;waits++;}
static u64 osGetTime(void){return clockMs;}
static void app_log(const char *s){assert(s);logs++;}static void app_handoff_log(const char*s){app_log(s);}
static Result ptmSysmInit(void){return 0;}static void ptmSysmExit(void){}
static bool aptIsSleepAllowed(void){return sleepAllowed;}static void aptSetSleepAllowed(bool b){sleepAllowed=b;}
static void ui_shutdown(void){assert(homeLoading==3&&phaseFrames[3]);uiReleased=true;shutdowns++;}
static bool ui_init(void){return true;}static void aptClearChainloader(void){}
static Result PTMSYSM_RebootAsync(u64 n){assert(n==1000000000ULL&&uiReleased);reboots++;return powerRC;}
static Result PTMSYSM_ShutdownAsync(u64 n){(void)n;assert(0);return -1;}
static void saveView(void){saves++;}static void aptSetHomeAllowed(bool b){homeAllowed=b;}
static Result srvPublishToSubscriber(u32 id,u32 flags){assert(id==0x204&&!flags&&homeAllowed&&phaseFrames[2]&&drains);publishes++;return publishRC;}
static int autoboot_status(void){return 1;}static bool autoboot_home_suspend(void){assert(phaseFrames[1]);return true;}
static void aptHandleJumpToHome(void){assert(homeLoading==2&&phaseFrames[2]&&drains&&allowPending);jumps++;jump=false;}
static Result cfguInit(void){assert(phaseFrames[1]);return 0;}static void cfguExit(void){}
static Result CFGU_SecureInfoGetRegion(u8 *out){*out=region;return 0;}
static Result amInit(void){return 0;}static void amExit(void){}
static Result AM_GetTitleInfo(int media,unsigned count,const u64 *id,AM_TitleInfo *info){assert(media==MEDIATYPE_NAND&&count==1);info->titleID=*id;return 0;}
static Result nsInit(void){return 0;}static void nsExit(void){}
static Result NS_LaunchTitle(u64 id,unsigned flags,u32 *pid){assert(id==0x0004003000009802ULL&&!flags&&phaseFrames[1]&&drains);launches++;*pid=123;return launchRC;}
static void showHomeLoading(unsigned phase){
    bool first=homeLoading==0;homeLoading=phase;
    if(uiReleased||!aptIsActive()||aptShouldClose())return;
    ui_runtime_allow_pending_home(true);
    draw(true,0,"");ui_runtime_drain();
    /* One refresh interval lets the queued buffer reach the LCD before IPC
       blocks. It is not an artificial minimum display duration. */
    if(first)svcSleepThread(20000000);
}
static void finishHomeLoading(void){
    homeLoading=0;ui_runtime_allow_pending_home(powerPrompt);ui_runtime_reset();
}
static bool systemPowerAction(bool reboot){
    Result r=ptmSysmInit();
    if(R_FAILED(r)){app_log("PTM system service unavailable");return false;}
    bool sleep=aptIsSleepAllowed();aptSetSleepAllowed(false);
    app_log(reboot?"PTM: restart requested":"PTM: shutdown requested");
    ui_shutdown();uiReleased=true;aptClearChainloader();
    r=reboot?PTMSYSM_RebootAsync(1000000000ULL):PTMSYSM_ShutdownAsync(1000000000ULL);
    ptmSysmExit();
    char entry[96];snprintf(entry,sizeof(entry),"PTM request result: %08lX",(unsigned long)(u32)r);app_log(entry);
    /* Keep the process alive while PM coordinates the MCU operation. Exiting
       immediately turns an asynchronous request into an ordinary HOME return. */
    if(R_SUCCEEDED(r))for(unsigned i=0;i<50;i++)svcSleepThread(100000000);
    app_log(R_FAILED(r)?"PTM request rejected":"PTM request timed out after 5 seconds");
    if(ui_init()){uiReleased=false;ui_runtime_reset();}
    aptSetSleepAllowed(sleep);return false;
}
static bool openHomeMenu(void){
    showHomeLoading(1);
    bool registered=false;Result r=APT_IsRegistered(aptGetMenuAppID(),&registered);
    saveView();settings=false;autobootPrompt=false;
    if(R_SUCCEEDED(r)&&registered){
        /* Same HOME-press notification as Rosalina's Return To HOME Menu. */
        aptSetHomeAllowed(true);
        showHomeLoading(2);
        r=srvPublishToSubscriber(0x204,0);
        if(R_SUCCEEDED(r)){finishHomeLoading();return true;}
    }else if(R_SUCCEEDED(r)){
        int boot=autoboot_status();
        if(boot==0||(boot==1&&autoboot_home_suspend())){
            showHomeLoading(3);
            app_handoff_log("HOME escape: managed autoboot paused; reboot to HOME");
            systemPowerAction(true);r=(Result)-105;
        }else r=(Result)-106;
    }
    snprintf(message,sizeof(message),tr("HOME nicht geöffnet (%08lX). Erneut versuchen."),(unsigned long)(u32)r);
    status=message;app_handoff_log(message);powerPrompt=true;finishHomeLoading();return false;
}
static bool flowMainLoop(void){
    aptHandleSleep();
    /* HOME is intercepted only in a coldboot session; POWER retains its dialog.
       Do not leave while an install/preflight/repair owns the foreground. */
    if(homeUnavailable&&aptCheckHomePressRejected()&&!installing&&!preparingGame&&twlPage!=3){
        openHomeMenu();
    }
    if(aptShouldJumpToHome()){
        bool registered=false;NS_APPID menu=aptGetMenuAppID();Result r=APT_IsRegistered(menu,&registered);
        char entry[100];snprintf(entry,sizeof(entry),"HOME request: menu=%u registered=%u result=%08lX",(unsigned)menu,registered,(unsigned long)(u32)r);if(!powerPrompt)app_log(entry);
        if(R_FAILED(r)||!registered){
            /* libctru screen-transfer waits indefinitely for an absent HOME.
               Stay active and offer PTM power controls instead. */
            ui_runtime_allow_pending_home(true);
            if(!powerPrompt){powerCancelFailed=false;status=tr("Autostart pausiert bis zum nächsten Öffnen der Library.");}
            powerPrompt=true;return !aptShouldClose();
        }
        showHomeLoading(2);
        if(!uiReleased)ui_runtime_drain();
        app_handoff_log("HOME loading: frame submitted; APT system handoff");
        aptHandleJumpToHome();
        finishHomeLoading();
    }
    return !aptShouldClose();
}
static Result prepareNativeHome(void){
    static bool attempted=false;
    if(aptShouldClose())return (Result)-107;
    bool registered=false;
    Result r=APT_IsRegistered(aptGetMenuAppID(),&registered);
    if(R_FAILED(r))return (Result)-107;
    if(!registered)showHomeLoading(1);
    if(!registered&&!attempted){
        static const u64 ids[]={0x0004003000008202ULL,0x0004003000008F02ULL,
            0x0004003000009802ULL,0,0x000400300000A102ULL,
            0x000400300000A902ULL,0x000400300000B102ULL};
        u8 region=255;char log[160];
        r=cfguInit();if(R_FAILED(r))return (Result)-107;
        r=CFGU_SecureInfoGetRegion(&region);cfguExit();
        if(R_FAILED(r)||region>=sizeof(ids)/sizeof(ids[0])||!ids[region])return (Result)-107;
        u64 menu=ids[region];AM_TitleInfo info={0};
        r=amInit();if(R_FAILED(r))return (Result)-107;
        r=AM_GetTitleInfo(MEDIATYPE_NAND,1,&menu,&info);amExit();
        if(R_FAILED(r)||info.titleID!=menu)return (Result)-107;
        r=nsInit();if(R_FAILED(r))return (Result)-107;
        attempted=true;u32 pid=0;
        app_handoff_log("HOME explicit process launch before APT preparation");
        r=NS_LaunchTitle(menu,0,&pid);nsExit();
        snprintf(log,sizeof(log),"HOME NS launch title=%016llX result=%08lX pid=%lu",
            (unsigned long long)menu,(unsigned long)(u32)r,(unsigned long)pid);
        app_handoff_log(log);
        if(R_FAILED(r))return (Result)-107;
    }
    for(unsigned i=0;;i++){
        r=APT_IsRegistered(aptGetMenuAppID(),&registered);
        if(R_FAILED(r))return (Result)-107;
        if(registered){
            homeUnavailable=false;aptSetHomeAllowed(true);
            if(homeLoading)showHomeLoading(2);
            app_handoff_log("HOME registered; native APT preparation now allowed");
            return 0;
        }
        if(i==100){app_handoff_log("HOME startup timed out; UI retained; no native jump prepared");return (Result)-107;}
        if(aptShouldClose()){app_handoff_log("HOME startup interrupted by system close");return (Result)-107;}
        showHomeLoading(1);
        svcSleepThread(100000000);
    }
}
static Result ensureNativeHome(void){
    u64 started=osGetTime();Result r=prepareNativeHome();
    if(homeLoading){
        char log[120];snprintf(log,sizeof(log),"HOME loading: preparation %s after %lu ms",R_SUCCEEDED(r)?"ready":"failed",(unsigned long)(osGetTime()-started));app_handoff_log(log);
        finishHomeLoading();
    }
    return r;
}
int main(int argc,char **argv){
 assert(argc==2);unsigned test=(unsigned)atoi(argv[1]);
 (void)musicReady;(void)powerCancelFailed;(void)systemPowerAction;
 if(test==1){registered=true;assert(openHomeMenu());assert(publishes==1&&frames==2&&!homeLoading&&!allowPending&&!reboots);jump=true;assert(flowMainLoop());assert(jumps==1&&!homeLoading&&!allowPending);}
 if(test==2){registerAt=6;homeUnavailable=true;assert(!ensureNativeHome());assert(launches==1&&frames>=4&&!homeLoading&&!allowPending&&!homeUnavailable&&homeAllowed&&phaseFrames[2]);}
 if(test==3){assert(ensureNativeHome()<0);assert(launches==1&&clockMs>=10000&&clockMs<11000&&queries<110&&!homeLoading&&!allowPending&&!jumps);}
 if(test==4){launchRC=-7;assert(ensureNativeHome()<0);assert(launches==1&&frames&&!homeLoading&&!allowPending&&!uiReleased);}
 if(test==5){region=3;assert(ensureNativeHome()<0);assert(!launches&&frames&&!homeLoading&&!allowPending);}
 if(test==6){queryRC=-9;assert(ensureNativeHome()<0);assert(!frames&&!homeLoading&&!allowPending);assert(!openHomeMenu());assert(powerPrompt&&!homeLoading&&allowPending&&!publishes&&!reboots);}
 if(test==7){assert(!openHomeMenu());assert(reboots==1&&shutdowns==1&&phaseFrames[3]&&!homeLoading&&powerPrompt&&!uiReleased&&allowPending);}
 if(test==8){homeUnavailable=true;installing=true;rejected=true;assert(flowMainLoop());assert(!frames&&!reboots);installing=false;preparingGame=true;rejected=true;assert(flowMainLoop());assert(!frames);preparingGame=false;twlPage=3;rejected=true;assert(flowMainLoop());assert(!frames);}
 if(test==9){closing=true;assert(ensureNativeHome()<0);assert(!frames&&!homeLoading&&!jumps);}
 if(test==10){registered=true;assert(!ensureNativeHome());assert(!frames&&!launches&&!homeLoading);}
 if(test==11){registered=true;publishRC=-1;assert(!openHomeMenu());assert(publishes==1&&powerPrompt&&!homeLoading&&allowPending&&!jumps);}
 if(test==12){jump=true;assert(flowMainLoop());assert(powerPrompt&&allowPending&&!frames&&!jumps);}
 puts("PASS actual HOME loading ordering and recovery scenario");
}
