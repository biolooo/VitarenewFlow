
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

            if(keys&KEY_B)autobootPrompt=false;
            else if(keys&KEY_A){
                bool enable=autobootState==0;
                u64 target=0;Result check=enable?native_handoff_target(&target):0;
                draw(true,0,tr("Autostart wird eingerichtet"));
                bool ok=R_SUCCEEDED(check)&&autoboot_set(enable);
                app_log(ok?(enable?"Autoboot enabled and verified":"Autoboot disabled and restored"):"Autoboot preference failed");
                autobootPrompt=false;autobootState=autoboot_status();
                if(ok){
                    saveView();status=tr("Autostart gespeichert; gilt ab nächstem Einschalten");
                }
                else {snprintf(message,sizeof(message),"%s: %.100s",tr("Autostart fehlgeschlagen"),R_FAILED(check)?"Rosalina target unavailable":autoboot_error());status=message;installFailed=true;errorTitle=tr("Autostart");}
            }

}
int main(void){input(KEY_B);assert(!autobootPrompt&&!sets);input(KEY_A);assert(autobootState==1&&sets==1&&saves==1&&!leave&&!uiReleased);input(KEY_A);assert(autobootState==0&&sets==2&&saves==2&&!leave&&!uiReleased);checkRC=-1;input(KEY_A);assert(sets==2&&installFailed&&errorTitle&&status);puts("PASS actual autoboot UI: enable/disable saves without exit/reboot; cancel and preflight failure safe");}
