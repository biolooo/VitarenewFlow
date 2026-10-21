#include <3ds.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
static bool homeRegistered;static int homeError;
static int aptGetMenuAppID(void){return 257;}static Result APT_IsRegistered(int id,bool *b){assert(id==257);*b=homeRegistered;return homeError;}
static u32 ipc[16],held;static int calls,closed,cleared,failCommand,aptCalls,initCalls,exitCalls;
void gfxInitDefault(void){initCalls++;}bool aptMainLoop(void){return aptCalls++==0;}void hidScanInput(void){}u32 hidKeysHeld(void){return held;}u32 hidKeysDown(void){return 0;}
u32 *getThreadCommandBuffer(void){return ipc;}u32 IPC_MakeHeader(u32 id,u32 a,u32 b){(void)a;(void)b;return id;}u32 IPC_Desc_StaticBuffer(u32 a,u32 b){(void)a;(void)b;return 0;}
Result svcSendSyncRequest(Handle h){assert(h==7);calls++;assert(ipc[0]==(calls==1?3:2));if((int)ipc[0]==failCommand)return -5;ipc[1]=0;return 0;}
Result svcConnectToPort(Handle*h,const char*n){assert(!strcmp(n,"hb:ldr"));*h=7;return 0;}void svcCloseHandle(Handle h){assert(h==7);closed++;}
void consoleInit(int s,void*p){(void)s;(void)p;}void gfxFlushBuffers(void){}void gfxSwapBuffers(void){}void gspWaitForVBlank(void){}void aptClearChainloader(void){cleared++;}void gfxExit(void){exitCalls++;}
static char lastFile[200];
static FILE *recordOpen(const char*p,const char*m){snprintf(lastFile,sizeof(lastFile),"%s",p);return fopen(p,m);}
#define fopen recordOpen
#define main dispatcherMain
#include "dispatcher-main.c"
#undef main
#undef fopen
static void writeFile(const char*p,const char*t){FILE*f=fopen(p,"wb");assert(f);fputs(t,f);assert(!fclose(f));}
static void run(void){calls=closed=cleared=aptCalls=initCalls=exitCalls=0;assert(dispatcherMain()==0);assert(initCalls==1&&exitCalls==1);}
int main(void){
 mkdir("dispatcher-home-fixture",0777);assert(!chdir("dispatcher-home-fixture"));mkdir("sdmc:",0777);mkdir("sdmc:/3ds",0777);mkdir("sdmc:/3ds/3ds-library",0777);
 const char *flow="sdmc:/3ds/3ds-library/3ds-library.3dsx",*hbl="sdmc:/3ds/3ds-library/autoboot-hbmenu.3dsx";
 writeFile(flow,"3DSX FLOW");writeFile(hbl,"3DSX HBL");run();assert(calls==2&&closed==1&&!cleared&&!strcmp(lastFile,flow));
 homeRegistered=true;run();assert(calls==2&&!cleared&&!strcmp(lastFile,hbl));homeRegistered=false;homeError=-1;run();assert(calls==2&&!strcmp(lastFile,hbl));homeError=0;
 held=KEY_B;run();assert(calls==2&&!cleared&&!strcmp(lastFile,hbl));held=0;
 remove(flow);run();assert(calls==2&&!cleared&&!strcmp(lastFile,hbl));writeFile(flow,"BAD!");run();assert(calls==2&&!cleared&&!strcmp(lastFile,hbl));
 writeFile(flow,"3DSX FLOW");failCommand=3;run();assert(calls==1&&closed==1&&cleared==1);failCommand=2;run();assert(calls==2&&closed==1&&cleared==1);failCommand=0;
 remove(flow);remove(hbl);run();assert(calls==0&&cleared==1);
 puts("PASS actual boot dispatcher: launcher, B recovery, missing/corrupt launcher fallback, argument failure blocks target, target failure, missing recovery files");
}
