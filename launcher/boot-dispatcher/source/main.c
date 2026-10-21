// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include <3ds.h>
#include <stdio.h>
#include <string.h>
static bool valid(const char *path){FILE*f=fopen(path,"rb");if(!f)return false;char magic[4];bool ok=fread(magic,1,4,f)==4&&!memcmp(magic,"3DSX",4);fclose(f);return ok;}
static Result command(Handle port,u32 id,u32 slot,const void *data,u32 size){u32 *ipc=getThreadCommandBuffer();ipc[0]=IPC_MakeHeader(id,0,2);ipc[1]=IPC_Desc_StaticBuffer(size,slot);ipc[2]=(u32)data;Result r=svcSendSyncRequest(port);return R_FAILED(r)?r:(Result)ipc[1];}
int main(void){
 gfxInitDefault();aptMainLoop();hidScanInput();
 const char *flow="/3ds/3ds-library/3ds-library.3dsx",*fallback="/3ds/3ds-library/autoboot-hbmenu.3dsx";
 bool registered=false;Result menu=APT_IsRegistered(aptGetMenuAppID(),&registered);
 bool rescue=(hidKeysHeld()&KEY_B)!=0||R_FAILED(menu)||registered;
 const char *target=(!rescue&&valid("sdmc:/3ds/3ds-library/3ds-library.3dsx"))?flow:fallback;
 char path[160];snprintf(path,sizeof(path),"sdmc:%s",target);
 Result r=-1;
 if(valid(path)){
  Handle port;r=svcConnectToPort(&port,"hb:ldr");
  if(R_SUCCEEDED(r)){u32 args[0x100]={0};args[0]=1;strcpy((char*)&args[1],path);
   r=command(port,3,1,args,sizeof(args));if(R_SUCCEEDED(r))r=command(port,2,0,target,strlen(target)+1);svcCloseHandle(port);
  }
 }
 if(R_FAILED(r)){
  consoleInit(GFX_TOP,NULL);printf("VitarenewFlow boot recovery\n\nCannot load app / Homebrew Launcher.\nRestore boot.3dsx from:\n/3ds/3ds-library/autoboot-hbmenu.3dsx\n\nHold SELECT at power-on and disable\nHbmenu autoboot in Luma.\n\nSTART: exit to HOME\n");
  while(aptMainLoop()){hidScanInput();if(hidKeysDown()&KEY_START)break;gfxFlushBuffers();gfxSwapBuffers();gspWaitForVBlank();}
  aptClearChainloader();
 }
 gfxExit();return 0;
}
