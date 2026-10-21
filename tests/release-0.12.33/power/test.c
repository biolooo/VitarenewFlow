#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
typedef int Result;typedef unsigned NS_APPID;typedef uint32_t u32;typedef unsigned char u8;
#define R_FAILED(r) ((r)<0)
#define R_SUCCEEDED(r) ((r)>=0)
enum {FLAG_ACTIVE=1,FLAG_ALLOWSLEEP=4,FLAG_SHOULDHOME=32,FLAG_POWERBUTTON=128,FLAG_ORDERTOCLOSE=512,FLAG_CANCELLED=1024};
enum {KEY_A=1,KEY_B=2,KEY_X=4,KEY_Y=8,KEY_SELECT=16,KEY_TOUCH=32};
static unsigned aptFlags;static int aptHomeButtonState;
static bool powerPrompt,powerCancelFailed,homeUnavailable,installing,preparingGame,uiReleased,leave,exitToHomebrew,allowPending;
static unsigned twlPage,selected,flow;static const char *status;
static struct {int px,py;}pos;
static int unlocks,sleeps,homeCalls,reboots,shutdowns,musicResumes,resets,homeJumps;
static bool registered,nsPowerLatched;static Result osResult;static unsigned osCalls,notifications;
static Result APT_AppletUtility(int id,void *out,size_t outSize,const void *in,size_t inSize){
 assert(id==0&&out&&in&&outSize==1&&inSize==1&&*(const u8*)in==0);
 osCalls++;if(osResult<0)return osResult;nsPowerLatched=false;*(u8*)out=0;return 0;
}
static void physicalPower(void){if(!nsPowerLatched){nsPowerLatched=true;aptFlags|=FLAG_POWERBUTTON;notifications++;}}
static void APT_UnlockTransition(unsigned n){assert(n==1);unlocks++;}
static void APT_SleepIfShellClosed(void){sleeps++;}
static const char *tr(const char *s){return s;}
static void aptHandleSleep(void){}
static bool aptCheckHomePressRejected(void){return false;}
static NS_APPID aptGetMenuAppID(void){return 0x101;}
static Result APT_IsRegistered(NS_APPID menu,bool *value){assert(menu==0x101);*value=registered;return 0;}
static void app_log(const char*s){(void)s;}
static void ui_runtime_allow_pending_home(bool b){allowPending=b;}
static void ui_runtime_drain(void){}
static void ui_runtime_reset(void){resets++;}
static void aptHandleJumpToHome(void){homeJumps++;aptFlags&=~(FLAG_SHOULDHOME|FLAG_POWERBUTTON);aptHomeButtonState=0;}
static void openHomeMenu(void){homeCalls++;}
static void systemPowerAction(bool reboot){if(reboot)reboots++;else shutdowns++;}
static void flow_reset(unsigned *f,unsigned n){*f=n;}
static void menu_music_pause(bool p){assert(!p);musicResumes++;}
bool aptShouldClose(void)
{
	return (aptFlags & (FLAG_ORDERTOCLOSE|FLAG_CANCELLED)) != 0;
}
bool aptShouldJumpToHome(void)
{
	return aptHomeButtonState || (aptFlags & (FLAG_SHOULDHOME|FLAG_POWERBUTTON)) != 0;
}
static void aptClearJumpToHome(void)
{
	aptHomeButtonState = 0;
	APT_UnlockTransition(0x01);
	APT_SleepIfShellClosed();
}
Result flow_apt_cancel_home_request(void)
{
    if (aptShouldClose()) return (Result)-1;
    /* APT utility 0 = ClearPowerButtonState. Clearing only aptFlags leaves
       NS's power latch set, so later physical presses are not delivered.
       Match the one-byte void utility buffers used by SleepIfShellClosed. */
    u8 input=0,output=0;
    Result result=APT_AppletUtility(0,&output,sizeof(output),&input,sizeof(input));
    if (R_FAILED(result)) return result;
    __atomic_fetch_and(&aptFlags, ~(FLAG_SHOULDHOME | FLAG_POWERBUTTON), __ATOMIC_ACQ_REL);
    aptClearJumpToHome();
    return 0;
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
        ui_runtime_allow_pending_home(false);
        if(!uiReleased)ui_runtime_drain();
        aptHandleJumpToHome();
    }
    return !aptShouldClose();
}
static void input(unsigned keys){        if(powerPrompt){
            if(keys&KEY_TOUCH){
                if(pos.px>=12&&pos.px<308){
                    if(pos.py>=54&&pos.py<85)keys|=KEY_A;
                    else if(pos.py>=91&&pos.py<122)keys|=KEY_X;
                    else if(pos.py>=128&&pos.py<159)keys|=KEY_Y;
                    else if(pos.py>=165&&pos.py<196)keys|=KEY_SELECT;
                    else if(pos.py>=205&&pos.py<235)keys|=KEY_B;
                }
            }
            if(keys&KEY_B){
                Result cancelled=flow_apt_cancel_home_request();
                char entry[100];snprintf(entry,sizeof(entry),"Power cancel: OS ClearPowerButtonState=%08lX",(unsigned long)(u32)cancelled);app_log(entry);
                if(R_SUCCEEDED(cancelled)){
                    powerCancelFailed=false;powerPrompt=false;ui_runtime_allow_pending_home(false);ui_runtime_reset();
                    flow_reset(&flow,selected);status=tr("Bereit");
                    menu_music_pause(false);app_log("Power dialog cancelled; OS and local HOME/POWER cleared");
                }else powerCancelFailed=true;
            }else if(keys&KEY_A){openHomeMenu();if(uiReleased)leave=true;}
            else if(keys&(KEY_X|KEY_SELECT)){
                systemPowerAction((keys&KEY_X)!=0);
                status=tr("Systemanfrage fehlgeschlagen; erneut versuchen");if(uiReleased)leave=true;
            }else if(keys&KEY_Y){powerPrompt=false;exitToHomebrew=true;leave=true;}
}}
void old_cancel(void)
{
    if (aptShouldClose()) return;
    __atomic_fetch_and(&aptFlags, ~(FLAG_SHOULDHOME | FLAG_POWERBUTTON), __ATOMIC_ACQ_REL);
    aptClearJumpToHome();
}

int main(void){
 /* Model NS refusing a new click until utility 0 acknowledges the old one. */
 aptFlags=FLAG_ACTIVE|FLAG_ALLOWSLEEP;physicalPower();assert(flowMainLoop()&&powerPrompt);
 old_cancel();powerPrompt=false;physicalPower();assert(!aptShouldJumpToHome()&&nsPowerLatched&&notifications==1&&!osCalls);
 /* Same physical sequence, now through the fixed actual handler. */
 nsPowerLatched=false;notifications=0;aptFlags=FLAG_ACTIVE|FLAG_ALLOWSLEEP;
 for(unsigned cycle=0;cycle<20;cycle++){
  physicalPower();assert(flowMainLoop()&&powerPrompt&&allowPending);
  if(cycle%2){pos.px=150;pos.py=218;input(KEY_TOUCH);}else input(KEY_B);
  assert(!powerPrompt&&!powerCancelFailed&&!allowPending&&!aptShouldJumpToHome()&&!nsPowerLatched);
  assert(aptFlags==(FLAG_ACTIVE|FLAG_ALLOWSLEEP));
  for(int frame=0;frame<100;frame++){assert(flowMainLoop());assert(!powerPrompt);}
 }
 assert(notifications==20&&osCalls==20&&!homeCalls&&!reboots&&!shutdowns&&!leave&&!exitToHomebrew);
 /* Failed OS acknowledgement must retain the dialog and system request. */
 physicalPower();assert(flowMainLoop()&&powerPrompt);osResult=-9;input(KEY_B);
 assert(powerPrompt&&powerCancelFailed&&nsPowerLatched&&aptShouldJumpToHome()&&allowPending);
 assert(!homeCalls&&!reboots&&!shutdowns&&!leave);
 osResult=0;input(KEY_B|KEY_A|KEY_X|KEY_Y|KEY_SELECT);
 assert(!powerPrompt&&!powerCancelFailed&&!nsPowerLatched&&!leave&&!exitToHomebrew&&!homeCalls&&!reboots&&!shutdowns);
 physicalPower();assert(flowMainLoop()&&powerPrompt);input(KEY_B);
 unsigned beforeClose=osCalls;aptFlags=FLAG_ACTIVE|FLAG_POWERBUTTON|FLAG_ORDERTOCLOSE;
 assert(flow_apt_cancel_home_request()<0&&osCalls==beforeClose&&(aptFlags&FLAG_ORDERTOCLOSE)&&(aptFlags&FLAG_POWERBUTTON));
 aptFlags=FLAG_ACTIVE|FLAG_POWERBUTTON;registered=true;assert(flowMainLoop());assert(homeJumps==1&&!allowPending);
 puts("PASS: old 0.12.26 helper reproduces lost second click; fixed handler: 20 OS-gated power/cancel cycles, 2000 follow-up frames, touch, retry after OS failure, cancel priority, true close and normal HOME preserved");
}
