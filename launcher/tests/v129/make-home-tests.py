from pathlib import Path
s=Path('outputs/3ds-library/source/main.c').read_text();block=s[s.index('static bool systemPowerAction'):s.index('static void onApt')]
prefix=r'''
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
'''
tail=r'''
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
'''
Path('work/v0138/home_test.c').write_text(prefix+block+tail)
# Actual autoboot event branch: verify saving does not initiate reboot/exit.
a=s.index('        }else if(autobootPrompt){')+len('        }else if(autobootPrompt){');b=s.index('        }else if(supportOpen){',a)
Path('work/v0138/autoboot_controls_test.c').write_text(r'''
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
typedef int Result;typedef uint64_t u64;
#define KEY_A 1
#define KEY_B 2
#define R_SUCCEEDED(x) ((x)>=0)
#define R_FAILED(x) ((x)<0)
#define tr(x) (x)
static bool autobootPrompt=true,installFailed,leave,uiReleased,okResult=true;static int autobootState,checkRC,sets,saves;
static char message[180];static const char *status,*errorTitle;
static Result native_handoff_target(u64 *id){*id=123;return checkRC;}static void draw(bool a,float b,const char*c){(void)a;(void)b;(void)c;}
static bool autoboot_set(bool b){sets++;if(okResult)autobootState=b;return okResult;}static int autoboot_status(void){return autobootState;}static const char* autoboot_error(void){return "mock";}
static void app_log(const char*s){(void)s;}static void saveView(void){saves++;}
static void input(unsigned keys){
'''+s[a:b]+r'''
}
int main(void){input(KEY_B);assert(!autobootPrompt&&!sets);input(KEY_A);assert(autobootState==1&&sets==1&&saves==1&&!leave&&!uiReleased);input(KEY_A);assert(autobootState==0&&sets==2&&saves==2&&!leave&&!uiReleased);checkRC=-1;input(KEY_A);assert(sets==2&&installFailed&&errorTitle&&status);puts("PASS actual autoboot UI: enable/disable saves without exit/reboot; cancel and preflight failure safe");}
''')
