#include <3ds.h>
#include <assert.h>
#include <stdio.h>
#include "twl_repair.h"
#define KEY_A 1u
#define KEY_B 2u
#define KEY_X 4u
#define KEY_START 8u
#define KEY_TOUCH 16u
#define KEY_Y 32u
static unsigned twlPage,held,calls,reboots;static TwlReport twlReport,nextReport;
static bool pending,uiReleased,leave,twlNeedsReboot;
typedef struct {u64 id;FS_MediaType media;int system;} Game;
#define SYSTEM_3DS 0
static unsigned settingsCalls;
static Result launch(Game *g){assert(g->id==0x0004001000022000ULL&&g->media==MEDIATYPE_NAND);settingsCalls++;return -1;}
u64 twl_settings_title(const TwlReport *r){(void)r;return 0x0004001000022000ULL;}static Result lastError,beginResult,finishResult,twlError;static const char *status;static char message[180];
static unsigned hidKeysHeld(void){return held;}
static const char *tr(const char *x){return x;}static void app_log(const char *x){(void)x;}
static void saveView(void){}static void restartAfterBootChange(void){reboots++;}
void twl_diagnose(TwlReport *r){*r=nextReport;}bool twl_repair_pending(void){return pending;}
Result twl_repair_begin(const TwlReport *r){assert(r);calls++;if(!beginResult)pending=true;return beginResult;}
static Result performTwlRepair(void){return twl_repair_begin(&twlReport);}
Result twl_repair_finish(void){if(!finishResult)pending=false;return finishResult;}
static void dispatch(unsigned keys,int x,int y){struct {int px,py;}pos={x,y};
        if(twlPage){
            if((keys&KEY_TOUCH)&&pos.py>=214)keys|=pos.px<107?KEY_A:pos.px<214?KEY_X:KEY_B;
            if(keys&KEY_B){if(twlPage==2||twlPage==3)twlPage--;else twlPage=0;}
            else if((keys&KEY_Y)&&twlPage==4&&!twlNeedsReboot){
                u64 id=twl_settings_title(&twlReport);
                if(id){Game target={0};target.id=id;target.media=MEDIATYPE_NAND;target.system=SYSTEM_3DS;saveView();lastError=launch(&target);if(uiReleased)leave=true;}
                else lastError=(Result)-121;
                if(R_FAILED(lastError)){twlError=lastError;snprintf(message,sizeof(message),tr("Einstellungen nicht geöffnet: %08lX"),(unsigned long)(u32)lastError);status=message;app_log(message);}
            }
            else if(keys&KEY_X){twl_diagnose(&twlReport);if(twlPage==4&&twlReport.known&&!twlReport.missing)twlPage=5;}
            else if(keys&KEY_A){
                if(twlPage==1&&twlReport.known&&twlReport.supported&&(twlReport.missing||(twlReport.loader&&twlReport.files)))twlPage=2;
                else if(twlPage==2)twlPage=3;
                else if(twlPage==3&&(hidKeysHeld()&KEY_START)){
                    lastError=performTwlRepair();twlError=lastError;twl_diagnose(&twlReport);
                    if(twl_repair_pending()){twlNeedsReboot=true;twlPage=4;status=tr("Reparatur begonnen; Systemupdate erforderlich");}
                    else {twlPage=1;status=tr("Reparatur nicht gestartet; SD und Diagnose prüfen");}
                    if(R_FAILED(lastError))app_log("TWL repair incomplete: inspect session log; do not repeat deletion");
                }else if(twlPage==4){saveView();restartAfterBootChange();if(uiReleased)leave=true;else status=tr("Neustart fehlgeschlagen; Konsole manuell neu starten");}
                else if(twlPage==5){lastError=twl_repair_finish();if(R_SUCCEEDED(lastError)){twlPage=0;status=tr("DS-Reparatur abgeschlossen");}else {twlPage=4;twl_diagnose(&twlReport);}}
            }

        }
}
int main(void){
 twlPage=1;dispatch(KEY_A,0,0);assert(twlPage==1&&calls==0);
 twlReport.known=twlReport.supported=true;dispatch(KEY_A,0,0);assert(twlPage==1);
 twlReport.loader=twlReport.files=true;dispatch(KEY_A,0,0);assert(twlPage==2);
 dispatch(KEY_A,0,0);assert(twlPage==3);dispatch(KEY_A,0,0);assert(calls==0&&twlPage==3);
 dispatch(KEY_B,0,0);assert(twlPage==2);dispatch(KEY_A,0,0);held=KEY_START;dispatch(KEY_A,0,0);assert(calls==1&&twlPage==4&&pending);
 dispatch(KEY_Y,0,0);assert(settingsCalls==0);twlNeedsReboot=false;dispatch(KEY_Y,0,0);assert(settingsCalls==1);
 dispatch(KEY_A,0,0);assert(calls==1&&reboots==1&&twlPage==4);
 nextReport.known=true;nextReport.missing=1;dispatch(KEY_X,0,0);assert(twlPage==4);
 nextReport.missing=0;dispatch(KEY_X,0,0);assert(twlPage==5);
 finishResult=-1;dispatch(KEY_A,0,0);assert(twlPage==4&&pending);
 dispatch(KEY_X,0,0);finishResult=0;dispatch(KEY_A,0,0);assert(twlPage==0&&!pending);
 twlPage=3;beginResult=-1;dispatch(KEY_A,0,0);assert(twlPage==1&&calls==2&&!pending);
 twlPage=4;pending=true;dispatch(KEY_B,0,0);assert(!twlPage&&pending);
 twlPage=3;held=0;dispatch(KEY_TOUCH,30,222);assert(calls==2&&twlPage==3);
 twlPage=1;twlReport.known=twlReport.supported=true;twlReport.missing=1;twlReport.loader=twlReport.files=false;dispatch(KEY_A,0,0);assert(twlPage==2);
 puts("PASS actual assistant dispatch: prerequisites, explicit hold confirmation, touch guard, failure recovery, deferred recovery, no deletion retry, update recheck and manual DS test acknowledgement");
}
