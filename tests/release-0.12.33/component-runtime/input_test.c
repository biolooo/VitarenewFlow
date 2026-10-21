
#include "components.h"
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
typedef uint64_t u64;
enum {KEY_A=1,KEY_B=2,KEY_X=4,KEY_L=8,KEY_R=16,KEY_UP=32,KEY_DOWN=64,KEY_TOUCH=128};
static unsigned componentSelected,checks,downloads,clears,cancels,commits,clicks,lastDownload;
static u64 componentCheckedUntil;static bool componentPage=true,installing;static ComponentState state;static ComponentAvailability availability=COMP_AVAILABLE;
ComponentAvailability components_availability(unsigned i){(void)i;return availability;}
static const char *tr(const char*s){return s;}static u64 osGetTime(void){return 100;}
ComponentStatus components_status(void){ComponentStatus s={.state=state};return s;}
void components_clear(void){state=COMP_IDLE;clears++;}bool components_recheck(void){checks++;return true;}
void components_cancel(void){cancels++;}bool components_commit(void){commits++;return true;}
void components_start(unsigned n){downloads++;lastDownload=n;}static void app_update_stop(void){}
static void menu_music_click(void){clicks++;}static bool aptIsHomeAllowed(void){return true;}static bool aptIsSleepAllowed(void){return true;}
static void aptSetHomeAllowed(bool b){(void)b;}static void aptSetSleepAllowed(bool b){(void)b;}
static void draw(bool b,float p,const char*s){(void)b;(void)p;(void)s;}
static void input(unsigned keys,int x,int y){struct{int px,py;}pos={x,y};

            ComponentStatus cs=components_status();bool busy=cs.state==COMP_DOWNLOAD||cs.state==COMP_READY;
            if(cs.state==COMP_READY){bool home=aptIsHomeAllowed(),sleep=aptIsSleepAllowed();installing=true;aptSetHomeAllowed(false);aptSetSleepAllowed(false);draw(true,0,tr("Emulator wird eingerichtet"));components_commit();installing=false;aptSetHomeAllowed(home);aptSetSleepAllowed(sleep);}
            else if(keys&KEY_B || ((keys&KEY_TOUCH)&&((pos.py<44&&pos.px>=276)||(busy&&pos.py>=192)||(componentSelected==6&&pos.py>=201&&pos.py<231&&pos.px<94)))){if(busy)components_cancel();else componentPage=false;}
            else if(!busy){
                if(keys&(KEY_L|KEY_R|KEY_UP|KEY_DOWN|KEY_TOUCH))components_clear();
                bool footer=(keys&KEY_TOUCH)&&pos.py>=201&&pos.py<231;
                bool recheck=(keys&KEY_X)||(footer&&(componentSelected==6?(pos.px>=104&&pos.px<186):(pos.px>=68&&pos.px<184)));
                if(recheck){components_clear();if(components_recheck()){componentCheckedUntil=osGetTime()+1500;menu_music_click();}}
                if(!recheck){
                if(!recheck&&componentSelected!=6&&((keys&KEY_L)||(footer&&pos.px>=12&&pos.px<60)))componentSelected=componentSelected<3?3:0;
                if(componentSelected!=6&&(keys&KEY_R))componentSelected=componentSelected<3?3:0;
                if(componentSelected!=6&&(keys&KEY_UP))componentSelected=componentSelected?componentSelected-1:5;
                if(componentSelected!=6&&(keys&KEY_DOWN))componentSelected=(componentSelected+1)%6;
                if(componentSelected!=6&&(keys&KEY_TOUCH)&&pos.py>=54&&pos.py<174){unsigned row=(pos.py-54)/40;componentSelected=(componentSelected/3)*3+row;}
                if(!recheck&&components_availability(componentSelected)!=COMP_UNCHECKED&&((keys&KEY_A)||(footer&&pos.px>=(componentSelected==6?196:192)&&pos.px<308))){app_update_stop();components_start(componentSelected);}
                }
            }

}
int main(void){
 componentSelected=1;input(KEY_X,0,0);assert(checks==1&&!downloads&&componentSelected==1&&componentCheckedUntil==1600&&clicks==1);
 input(KEY_X|KEY_A|KEY_R,0,0);assert(checks==2&&!downloads&&componentSelected==1);
 input(KEY_TOUCH,126,218);assert(checks==3&&!downloads&&componentSelected==1);
 input(KEY_TOUCH,36,218);assert(componentSelected==3&&checks==3);input(KEY_R,0,0);assert(componentSelected==0);
 input(KEY_TOUCH,180,110);assert(componentSelected==1&&!downloads);
 input(KEY_A,0,0);assert(downloads==1&&lastDownload==1);input(KEY_TOUCH,250,218);assert(downloads==2);
 input(KEY_TOUCH,187,218);input(KEY_TOUCH,126,235);input(KEY_TOUCH,310,218);assert(downloads==2&&checks==3);
 availability=COMP_UNCHECKED;input(KEY_A,0,0);input(KEY_TOUCH,250,218);assert(downloads==2);availability=COMP_AVAILABLE;
 state=COMP_DOWNLOAD;input(KEY_X,0,0);assert(checks==3&&downloads==2);input(KEY_TOUCH,126,218);assert(cancels==1&&checks==3);
 state=COMP_READY;input(KEY_X,0,0);assert(commits==1&&checks==3);state=COMP_IDLE;
 componentSelected=6;input(KEY_X,0,0);assert(checks==4&&downloads==2);
 input(KEY_TOUCH,145,218);assert(checks==5&&downloads==2);
 input(KEY_X|KEY_A|KEY_R,0,0);assert(checks==6&&downloads==2&&componentSelected==6);
 input(KEY_TOUCH,190,218);input(KEY_TOUCH,145,235);assert(checks==6&&downloads==2);
 input(KEY_TOUCH,252,218);assert(downloads==3&&lastDownload==6);
 availability=COMP_UNCHECKED;input(KEY_A,0,0);input(KEY_TOUCH,252,218);assert(downloads==3);availability=COMP_AVAILABLE;
 state=COMP_DOWNLOAD;input(KEY_X,0,0);assert(checks==6);input(KEY_B,0,0);assert(cancels==2);state=COMP_IDLE;
 input(KEY_B,0,0);assert(!componentPage);
 puts("PASS actual component page input: X/touch recheck, no accidental install or page switch, boundaries, L/R, A, busy/commit guards and Desktop controls");
}
