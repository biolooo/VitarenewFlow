// SPDX-License-Identifier: GPL-3.0-only
// VitarenewFlow HOME shortcut. Independently authored IPC workflow.
// Shared-config layout: LumaTeam/Luma3DS v13.4 (notice in accompanying header).
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include "luma_shared_config.h"
#define FLOW_ID 0x000400000F104200ULL
#define FLOW_PATH "sdmc:/3ds/3ds-library/3ds-library.3dsx"
#define RESTORE "vitarenewflow-forwarder:restore="
static char target[0x400];static u32 arguments[0x100];
static Result command(Handle service,u32 id,u32 slot,const void *data,u32 bytes){
 u32 *c=getThreadCommandBuffer();c[0]=IPC_MakeHeader(id,0,2);c[1]=IPC_Desc_StaticBuffer(bytes,slot);c[2]=(u32)data;
 Result r=svcSendSyncRequest(service);return R_FAILED(r)?r:(Result)c[1];
}
static Result prepare(void){
 FILE *f=fopen(FLOW_PATH,"rb");char magic[4];bool valid=f&&fread(magic,1,4,f)==4&&!memcmp(magic,"3DSX",4);if(f)fclose(f);if(!valid)return (Result)-1;
 s64 luma=0;Result r=svcGetSystemInfo(&luma,0x10000,0);if(R_FAILED(r))return r;
 if(!Luma_SharedConfig->use_hbldr)return (Result)-2;
 u64 self=0;r=APT_GetProgramID(&self);if(R_FAILED(r)||self!=FLOW_ID)return (Result)-3;
 u64 previous=Luma_SharedConfig->selected_hbldr_3dsx_tid;
 if(previous==FLOW_ID||((previous>>32)!=0x00040000ULL&&(previous>>32)!=0x00040010ULL))return (Result)-4; /* Never enter a self-restart loop. */
 Handle service;r=svcConnectToPort(&service,"hb:ldr");if(R_FAILED(r))return r;
 arguments[0]=2;char *arg=(char*)&arguments[1];strcpy(arg,FLOW_PATH);arg+=strlen(arg)+1;
 snprintf(arg,80,RESTORE "%016llX",(unsigned long long)previous);
 strcpy(target,FLOW_PATH+5);
 r=command(service,3,1,arguments,sizeof(arguments));
 if(R_SUCCEEDED(r))r=command(service,2,0,target,sizeof(target));
 svcCloseHandle(service);if(R_FAILED(r))return r;
 /* Temporary RAM state only: config.ini and boot files are never edited. */
 Luma_SharedConfig->selected_hbldr_3dsx_tid=self;aptSetChainloader(self,MEDIATYPE_SD);return 0;
}
int main(void){
 Result r=prepare();if(R_SUCCEEDED(r))return 0;
 gfxInitDefault();consoleInit(GFX_TOP,NULL);
 printf("\n VitarenewFlow\n\n Start nicht moeglich / Cannot start\n Error: %08lX\n\n",(unsigned long)(u32)r);
 if(r==(Result)-1)printf(" App-Datei fehlt / App file missing.\n Universal-Updater: erneut installieren.\n Reinstall from Universal-Updater.\n");
 else printf(" Luma3DS / HOME shortcut needs checking.\n Try launching Flow from Homebrew Launcher.\n");
 printf("\n START: HOME\n");while(aptMainLoop()){hidScanInput();if(hidKeysDown()&KEY_START)break;gfxFlushBuffers();gfxSwapBuffers();gspWaitForVBlank();}
 aptClearChainloader();gfxExit();return 0;
}
