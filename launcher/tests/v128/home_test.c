
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
typedef int Result;typedef uint32_t u32;typedef int NS_APPID;
#define R_SUCCEEDED(r) ((r)>=0)
#define R_FAILED(r) ((r)<0)
#define tr(x) (x)
static bool uiReleased,powerPrompt,settings=true,autobootPrompt=true,homeUnavailable,installing,preparingGame;static unsigned twlPage;
static char message[180];static const char *status;
static bool registered=true,jump,closing,initOK=true,sleepAllowed=true,allowPending,homeAllowed=true,rejected,hardwareReset;
static int connectRC,requestRC,menuRC,publishRC,bootState=1,pauseCalls,powerCalls,homeCalls,drains,waits,stops,starts,saves,publishes;
static bool pauseOK=true,requestedReboot;static jmp_buf resetJump;
static void app_log(const char*p){(void)p;}static void app_handoff_log(const char*p){(void)p;}
static Result ptmSysmInit(void){return connectRC;}static void ptmSysmExit(void){}
static bool aptIsSleepAllowed(void){return sleepAllowed;}static void aptSetSleepAllowed(bool b){sleepAllowed=b;}
static void aptSetHomeAllowed(bool b){homeAllowed=b;}
static bool aptCheckHomePressRejected(void){bool b=rejected;rejected=false;return b;}
static void ui_shutdown(void){stops++;}static void aptClearChainloader(void){}
static Result PTMSYSM_RebootAsync(uint64_t ns){assert(ns==1000000000ULL&&uiReleased);powerCalls++;requestedReboot=true;return requestRC;}
static Result PTMSYSM_ShutdownAsync(uint64_t ns){assert(ns==1000000000ULL&&uiReleased);powerCalls++;requestedReboot=false;return requestRC;}
static void svcSleepThread(int64_t ns){assert(ns==100000000);waits++;if(hardwareReset)longjmp(resetJump,1);}
static bool ui_init(void){starts++;return initOK;}static void ui_runtime_reset(void){}
static void aptHandleSleep(void){}static bool aptShouldJumpToHome(void){return jump;}static bool aptShouldClose(void){return closing;}
static int aptGetMenuAppID(void){return 257;}static Result APT_IsRegistered(int id,bool*b){assert(id==257);*b=registered;return menuRC;}
static void ui_runtime_drain(void){drains++;}static void aptHandleJumpToHome(void){assert(drains>0);homeCalls++;}
static void ui_runtime_allow_pending_home(bool b){allowPending=b;}
static int autoboot_status(void){return bootState;}static bool autoboot_home_suspend(void){pauseCalls++;if(pauseOK)bootState=0;return pauseOK;}
static Result srvPublishToSubscriber(u32 id,u32 flags){assert(id==0x204&&!flags&&homeAllowed);publishes++;return publishRC;}
static void saveView(void){saves++;}
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
static bool restartAfterBootChange(void){return systemPowerAction(true);}
static void saveView(void);
static bool openHomeMenu(void){
    bool registered=false;Result r=APT_IsRegistered(aptGetMenuAppID(),&registered);
    saveView();settings=false;autobootPrompt=false;
    if(R_SUCCEEDED(r)&&registered){
        /* Same HOME-press notification as Rosalina's Return To HOME Menu. */
        aptSetHomeAllowed(true);
        r=srvPublishToSubscriber(0x204,0);
        if(R_SUCCEEDED(r))return true;
    }else if(R_SUCCEEDED(r)){
        int boot=autoboot_status();
        if(boot==0||(boot==1&&autoboot_home_suspend())){
            app_handoff_log("HOME escape: managed autoboot paused; reboot to HOME");
            systemPowerAction(true);r=(Result)-105;
        }else r=(Result)-106;
    }
    snprintf(message,sizeof(message),tr("HOME nicht geöffnet (%08lX). Erneut versuchen."),(unsigned long)(u32)r);
    status=message;app_handoff_log(message);powerPrompt=true;return false;
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
            if(!powerPrompt)status=tr("Autostart pausiert bis zum nächsten Öffnen der Library.");
            powerPrompt=true;return !aptShouldClose();
        }
        ui_runtime_allow_pending_home(false);
        if(!uiReleased)ui_runtime_drain();
        aptHandleJumpToHome();
    }
    return !aptShouldClose();
}

int main(void){
 assert(openHomeMenu()&&publishes==1&&!powerCalls&&!pauseCalls&&!settings&&!autobootPrompt);
 jump=true;assert(flowMainLoop()&&homeCalls==1);jump=false;
 registered=false;homeUnavailable=true;homeAllowed=false;hardwareReset=true;rejected=true;
 if(!setjmp(resetJump)){flowMainLoop();assert(0);}assert(powerCalls==1&&pauseCalls==1&&bootState==0&&requestedReboot&&uiReleased);
 hardwareReset=false;uiReleased=false;rejected=false;powerPrompt=false;jump=true;
 assert(flowMainLoop()&&powerPrompt&&allowPending&&powerCalls==1); // POWER: dialog, no surprise reboot
 jump=false;powerPrompt=false;bootState=1;pauseOK=false;assert(!openHomeMenu()&&powerCalls==1&&bootState==1&&powerPrompt);
 pauseOK=true;bootState=-1;assert(!openHomeMenu()&&powerCalls==1);
 bootState=0;assert(!openHomeMenu()&&powerCalls==2&&waits==51&&starts==1&&!uiReleased); // timeout bounded
 connectRC=-1;assert(!restartAfterBootChange()&&powerCalls==2);connectRC=0;
 assert(!systemPowerAction(false)&&!requestedReboot&&powerCalls==3);
 registered=true;menuRC=-9;assert(!openHomeMenu()&&publishes==1);menuRC=0;publishRC=-10;assert(!openHomeMenu()&&publishes==2);
 registered=false;installing=true;rejected=true;assert(flowMainLoop()&&!rejected&&powerCalls==3);installing=false;preparingGame=true;rejected=true;assert(flowMainLoop()&&powerCalls==3);preparingGame=false;twlPage=3;rejected=true;assert(flowMainLoop()&&powerCalls==3);twlPage=0;
 puts("PASS real HOME controls: normal HOME notification, coldboot HOME key pauses and reboots, POWER remains a dialog, blocked during install/preflight/repair, failures preserve recovery, bounded reboot and shutdown");
}
