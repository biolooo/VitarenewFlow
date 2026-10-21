
#include "twl_repair.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
enum {KEY_A=1,KEY_B=2,KEY_X=4,KEY_Y=8,KEY_START=16,KEY_TOUCH=32};
static unsigned twlPage,componentSelected,held,diagnoses,repairs,clears,checks,settingsLaunches,restarts,finishes;
static bool twlNeedsReboot,componentPage,uiReleased,leave,settings,infoOpen,pending;
static Result twlError,lastError,repairResult,launchResult,finishResult;
static TwlReport twlReport,nextReport;static const char *status;static char message[180];
typedef struct {u64 id;FS_MediaType media;unsigned system;} Game;
#define SYSTEM_3DS 0
static const char *tr(const char *s){return s;}
void twl_diagnose(TwlReport *r){*r=nextReport;r->confirmedAllFail=r->confirmedDownloadPlayFail=false;diagnoses++;}
bool twl_repair_pending(void){return pending;}
static void components_clear(void){clears++;}static void components_recheck(void){checks++;}
static void saveView(void){}static void menu_music_reload(void){}static void app_log(const char *s){assert(s);}
static unsigned hidKeysHeld(void){return held;}
static Result performTwlRepair(void){assert(twl_recommendation(&twlReport)==TWL_CHECK_REPAIR);repairs++;if(!repairResult){pending=true;nextReport.missing=4;}return repairResult;}
static Result launch(Game *g){assert(g->id==0x0004001000022000ULL&&g->media==MEDIATYPE_NAND);settingsLaunches++;return launchResult;}
static void restartAfterBootChange(void){restarts++;}
Result twl_repair_finish(void){finishes++;if(!finishResult)pending=false;return finishResult;}
u64 twl_settings_title(const TwlReport *r){return r->known?0x0004001000022000ULL:0;}
TwlAdvice twl_recommendation(const TwlReport *r){
 return r?twl_advice(r->known,r->supported,r->missing,r->loader,r->files,r->confirmedAllFail,r->confirmedDownloadPlayFail):TWL_CHECK_UNKNOWN;
}
static void openTwl(void){twl_diagnose(&twlReport);twlError=twlReport.error;twlPage=twl_repair_pending()?4:1;settings=false;infoOpen=false;}

static void input(unsigned keys,int x,int y){struct{int px,py;}pos={x,y};

            bool closeTwl=false;
            if(keys&KEY_TOUCH){
                if(pos.px>=276&&pos.px<312&&pos.py>=8&&pos.py<44){keys|=KEY_B;closeTwl=true;}
                if(pos.py>=201&&pos.py<231){
                    if((twlPage==1&&twl_recommendation(&twlReport)>=TWL_CHECK_UPDATE)||twlPage==4){
                        if(pos.px>=12&&pos.px<94)keys|=KEY_B;
                        else if(pos.px>=104&&pos.px<186)keys|=KEY_X;
                        else if(pos.px>=196&&pos.px<308)keys|=KEY_A;
                    }else{
                        if(pos.px>=12&&pos.px<112)keys|=KEY_B;
                        /* NAND changes require physical START + A, never Touch. */
                        else if(twlPage!=3&&pos.px>=120&&pos.px<308)keys|=KEY_A;
                    }
                }
            }
            if(keys&KEY_B){
                if(closeTwl)twlPage=0;
                else if(twlPage==3)twlPage=2;
                else if(twlPage==2||twlPage==6)twlPage=1;
                else twlPage=0;
                twlReport.confirmedAllFail=twlReport.confirmedDownloadPlayFail=false;
            }else if(keys&KEY_X){
                twl_diagnose(&twlReport);twlError=twlReport.error;
                twlPage=twl_repair_pending()?(twlReport.known&&!twlReport.missing?5:4):1;
            }else if((keys&(KEY_A|KEY_Y))&&(twlPage==6||(twlPage==4&&!twlNeedsReboot))){
                u64 id=twl_settings_title(&twlReport);
                if(id){Game target={0};target.id=id;target.media=MEDIATYPE_NAND;target.system=SYSTEM_3DS;saveView();lastError=launch(&target);if(uiReleased)leave=true;}
                else lastError=(Result)-121;
                if(R_FAILED(lastError)){menu_music_reload();twlError=lastError;snprintf(message,sizeof(message),tr("Einstellungen nicht geöffnet: %08lX"),(unsigned long)(u32)lastError);status=message;app_log(message);}
            }else if(keys&KEY_A){
                if(twlPage==1){
                    TwlAdvice advice=twl_recommendation(&twlReport);
                    if(advice==TWL_CHECK_UNKNOWN){twl_diagnose(&twlReport);twlError=twlReport.error;}
                    else if(advice==TWL_CHECK_UNSUPPORTED){twl_diagnose(&twlReport);twlError=twlReport.error;}
                    else if(advice==TWL_CHECK_UPDATE)twlPage=6;
                    else if(advice==TWL_CHECK_LOADER){components_clear();components_recheck();componentSelected=1;componentPage=true;twlPage=0;}
                    else twlPage=2;
                }else if(twlPage==2){
                    /* Revalidate before accepting the user's report that ALL DS
                       games and HOME's DS Download Play fail outside Flow. */
                    twl_diagnose(&twlReport);twlError=twlReport.error;
                    if(twl_repair_pending())twlPage=4;
                    else if(twl_recommendation(&twlReport)==TWL_CHECK_CLEAR){
                        twlReport.confirmedAllFail=twlReport.confirmedDownloadPlayFail=true;twlPage=3;
                    }else twlPage=1;
                }else if(twlPage==3&&!(keys&KEY_TOUCH)&&(hidKeysHeld()&KEY_START)&&twl_recommendation(&twlReport)==TWL_CHECK_REPAIR){
                    lastError=performTwlRepair();twlError=lastError;twl_diagnose(&twlReport);
                    if(twl_repair_pending()){twlNeedsReboot=true;twlPage=4;status=tr("Reparatur begonnen; Systemupdate erforderlich");}
                    else {twlPage=1;status=tr("Reparatur nicht gestartet; SD und Diagnose prüfen");}
                    if(R_FAILED(lastError))app_log("TWL repair incomplete: inspect session log; do not repeat deletion");
                }else if(twlPage==4){saveView();restartAfterBootChange();if(uiReleased)leave=true;else status=tr("Neustart fehlgeschlagen; Konsole manuell neu starten");}
                else if(twlPage==5){lastError=twl_repair_finish();if(R_SUCCEEDED(lastError)){twlPage=0;status=tr("DS-Reparatur abgeschlossen");}else {twlPage=4;twlError=lastError;twl_diagnose(&twlReport);}}
            }

}
static void reset(void){twlPage=0;componentPage=false;held=0;pending=false;repairResult=launchResult=finishResult=0;twlNeedsReboot=false;nextReport=(TwlReport){.known=true,.supported=true,.loader=true,.files=true,.region=2};openTwl();}
int main(void){
 reset();assert(twlPage==1&&!repairs);input(KEY_A,0,0);assert(twlPage==2&&!repairs);input(KEY_B,0,0);assert(twlPage==1&&!twlReport.confirmedAllFail);
 input(KEY_A,0,0);input(KEY_A,0,0);assert(twlPage==3&&twlReport.confirmedAllFail&&twlReport.confirmedDownloadPlayFail&&!repairs);
 held=KEY_START;input(KEY_TOUCH,220,218);assert(!repairs&&twlPage==3);held=0;input(KEY_A,0,0);assert(!repairs);
 held=KEY_START;input(KEY_A|KEY_TOUCH,220,218);assert(!repairs);input(KEY_A,0,0);assert(repairs==1&&pending&&twlPage==4&&twlNeedsReboot);
 input(KEY_A,0,0);assert(restarts==1);nextReport.missing=0;input(KEY_X,0,0);assert(twlPage==5);input(KEY_A,0,0);assert(!twlPage&&!pending&&finishes==1);
 reset();input(KEY_A,0,0);input(KEY_A,0,0);input(KEY_B,0,0);assert(twlPage==2&&!twlReport.confirmedAllFail);input(KEY_A,0,0);assert(twlPage==3);input(KEY_X,0,0);assert(twlPage==1&&!twlReport.confirmedDownloadPlayFail);
 input(KEY_A,0,0);input(KEY_A,0,0);input(KEY_TOUCH,292,20);assert(!twlPage&&!twlReport.confirmedAllFail&&repairs==1);
 reset();nextReport.loader=false;openTwl();input(KEY_TOUCH,250,218);assert(!twlPage&&componentPage&&componentSelected==1&&clears==1&&checks==1&&repairs==1);
 reset();nextReport.missing=1;openTwl();input(KEY_A,0,0);assert(twlPage==6&&repairs==1&&!pending);launchResult=-9;input(KEY_A,0,0);assert(settingsLaunches==1&&twlError==-9&&twlPage==6);input(KEY_B,0,0);assert(twlPage==1);
 reset();nextReport.known=false;openTwl();unsigned before=diagnoses;input(KEY_A,0,0);assert(diagnoses==before+1&&twlPage==1&&repairs==1);
 reset();nextReport.supported=false;openTwl();input(KEY_A,0,0);assert(twlPage==1&&repairs==1);
 reset();input(KEY_A,0,0);nextReport.files=false;input(KEY_A,0,0);assert(twlPage==1&&!twlReport.confirmedAllFail&&repairs==1);
 reset();input(KEY_A,0,0);pending=true;input(KEY_A,0,0);assert(twlPage==4&&repairs==1);
 reset();input(KEY_A,0,0);input(KEY_A,0,0);twlReport.confirmedAllFail=false;held=KEY_START;input(KEY_A,0,0);assert(repairs==1);
 reset();input(KEY_A,0,0);input(KEY_A,0,0);repairResult=-122;held=KEY_START;input(KEY_A,0,0);assert(repairs==2&&twlPage==1&&!pending&&twlError==-122);
 reset();input(KEY_TOUCH,5,235);assert(twlPage==1);input(KEY_TOUCH,53,218);assert(!twlPage);
 puts("PASS actual DS dialog input: advice-specific actions, read-only recheck, symptom reconfirmation, loader setup without install, system update without deletion, physical START+A only, Touch excluded, back/close clear consent, stale diagnosis, pending recovery and failure handling");
}
