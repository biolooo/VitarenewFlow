#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#define KEY_A 1u
#define KEY_X 2u
#define KEY_Y 4u
#define KEY_SELECT 8u
static bool powerPrompt=true,exitToHomebrew,leave,uiReleased,pauseOK=true;static int bootState=1,pauseCalls,powerCalls;static bool reboot;
static const char *status;static const char *tr(const char *s){return s;}static int autoboot_status(void){return bootState;}
static bool autoboot_home_suspend(void){pauseCalls++;if(pauseOK)bootState=0;return pauseOK;}
static void saveView(void){}static bool systemPowerAction(bool b){powerCalls++;reboot=b;return false;}
static void input(unsigned keys){
        if(powerPrompt){
            if(keys&KEY_A){
                int boot=autoboot_status();
                if(boot==0||(boot==1&&autoboot_home_suspend())){
                    saveView();systemPowerAction(true);
                    status=tr("Neustart fehlgeschlagen; Autostart ist pausiert");
                }else status=tr("HOME-Wechsel fehlgeschlagen; Autostart-Einstellungen prüfen");
                if(uiReleased)leave=true;
            }
            if(keys&(KEY_X|KEY_SELECT)){
                systemPowerAction((keys&KEY_X)!=0);
                status=tr("Systemanfrage fehlgeschlagen; erneut versuchen");if(uiReleased)leave=true;
            }
            if(keys&KEY_Y){powerPrompt=false;exitToHomebrew=true;leave=true;}

        }
}
int main(void){
 input(KEY_A);assert(pauseCalls==1&&bootState==0&&powerCalls==1&&reboot&&!leave);
 bootState=1;pauseOK=false;input(KEY_A);assert(pauseCalls==2&&powerCalls==1&&bootState==1);
 bootState=-1;input(KEY_A);assert(pauseCalls==2&&powerCalls==1);
 bootState=0;input(KEY_A);assert(powerCalls==2&&reboot);
 input(KEY_X);assert(powerCalls==3&&reboot);input(KEY_SELECT);assert(powerCalls==4&&!reboot);
 input(KEY_Y);assert(exitToHomebrew&&leave&&!powerPrompt);
 puts("PASS actual HOME escape controls: managed pause/reboot, failed pause and conflicts block reboot, off mode, restart, power off and HBL exit");
}
