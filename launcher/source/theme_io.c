// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "theme_io.h"
#include "theme_format.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "app_log.h"
#include "theme_music_plan.h"
static bool read_ext_at(FS_Archive a,const char *name,u64 offset,void *buffer,unsigned bytes){
    Handle f;
    u32 read=0;
    Result r=FSUSER_OpenFile(&f,a,fsMakePath(PATH_ASCII,name),FS_OPEN_READ,0);
    if(R_FAILED(r))return false;
    r=FSFILE_Read(f,&read,offset,buffer,bytes);
    FSFILE_Close(f);
    return R_SUCCEEDED(r)&&read==bytes;
}
static bool read_ext(FS_Archive a,const char *name,void *buffer,unsigned bytes){return read_ext_at(a,name,0,buffer,bytes);}
bool theme_read_active(uint8_t **body,size_t *size,const char **source){
    *body=NULL;
    *size=0;
    *source="Standard";
    u8 region=0xff;
    bool ok=false;
    if(R_SUCCEEDED(cfguInit())){CFGU_SecureInfoGetRegion(&region);
    cfguExit();}
    u32 id=region==0?0x2cc:region==1?0x2cd:region==2?0x2ce:region==5?0x2cf:0;
    if(id){
        u32 homeId=region==0?0x82:region==1?0x8f:region==2?0x98:0xa9;
        u32 homePath[]={MEDIATYPE_SD,homeId,0};
        FS_Path hp={PATH_BINARY,sizeof(homePath),homePath};FS_Archive home;
        if(R_SUCCEEDED(FSUSER_OpenArchive(&home,ARCHIVE_EXTDATA,hp))){
            uint8_t savedata[0x141c];
            bool known=read_ext(home,"/SaveData.dat",savedata,sizeof(savedata));
            FSUSER_CloseArchive(home);
            if(known && (savedata[0x13bd]!=3 || savedata[0x141b])){
                /* A stale BodyCache must not override a basic theme or shuffle. */
                *source=savedata[0x141b]?"Theme-Shuffle":"HOME-Standard";
                return false;
            }
        }
        u32 path[]={MEDIATYPE_SD,id,0};
        FS_Path p={PATH_BINARY,sizeof(path),path};
        FS_Archive a;
        if(R_SUCCEEDED(FSUSER_OpenArchive(&a,ARCHIVE_EXTDATA,p))){
            u32 manage[0x800/4];
            if(read_ext(a,"/ThemeManage.bin",manage,sizeof(manage)) && manage[7] && manage[2]>=4 && manage[2]<=0x150000){
                uint8_t *compressed=malloc(manage[2]);
                if(compressed){if(read_ext(a,"/BodyCache.bin",compressed,manage[2]))ok=theme_decompress(compressed,manage[2],body,size);
                free(compressed);}
            }
            FSUSER_CloseArchive(a);
        }
    }
    if(ok){*source="HOME-Theme";
    return true;}
    /* Explicitly labelled copy of the user's SD theme; no writes to extdata. */
    FILE *f=fopen("sdmc:/3ds/3ds-library/theme/body_LZ.bin","rb");
    if(!f)return false;
    if(fseek(f,0,SEEK_END)){fclose(f);
    return false;}long n=ftell(f);
    rewind(f);
    if(n<4||n>0x150000){fclose(f);
    return false;}uint8_t *buffer=malloc(n);
    if(buffer){if(fread(buffer,1,n,f)==(size_t)n)ok=theme_decompress(buffer,n,body,size);
    free(buffer);}
    fclose(f);
    if(ok)*source="SD-Theme (Ersatz)";
    return ok;
}
#include <string.h>
unsigned theme_read_music(uint8_t **data,size_t *size){
 *data=NULL;*size=0;u8 region=255;
 if(R_SUCCEEDED(cfguInit())){CFGU_SecureInfoGetRegion(&region);cfguExit();}
 u32 themeId=region==0?0x2cc:region==1?0x2cd:region==2?0x2ce:region==5?0x2cf:0;
 u32 homeId=region==0?0x82:region==1?0x8f:region==2?0x98:region==5?0xa9:0;
 if(!homeId||!themeId){app_log("Theme BGM: unsupported region");return 0;}
 uint8_t savedata[0x141c]={0},manage[0x800]={0};bool known=false;FS_Archive a;
 u32 path[]={MEDIATYPE_SD,homeId,0};FS_Path ap={PATH_BINARY,sizeof(path),path};
 Result r=FSUSER_OpenArchive(&a,ARCHIVE_EXTDATA,ap);
 if(R_SUCCEEDED(r)){known=read_ext(a,"/SaveData.dat",savedata,sizeof(savedata));FSUSER_CloseArchive(a);}
 path[1]=themeId;r=FSUSER_OpenArchive(&a,ARCHIVE_EXTDATA,ap);
 if(R_FAILED(r)){char log[96];snprintf(log,sizeof(log),"Theme BGM: open extdata failed %08lX",(unsigned long)(u32)r);app_log(log);return 0;}
 ThemeMusicPlan plan;bool ok=read_ext(a,"/ThemeManage.bin",manage,sizeof(manage))&&theme_music_plan(&plan,known?savedata:NULL,known?sizeof(savedata):0,manage,sizeof(manage));
 uint8_t *compressed=NULL,*body=NULL;size_t length=0;
 if(ok){compressed=malloc(plan.bodyBytes);ok=compressed&&read_ext_at(a,plan.shuffle?"/BodyCache_rd.bin":"/BodyCache.bin",plan.shuffle?(u64)plan.slot*0x150000:0,compressed,plan.bodyBytes)&&theme_decompress(compressed,plan.bodyBytes,&body,&length)&&length>5&&body[5]==1;}
 free(compressed);free(body);
 if(ok){
  uint8_t *music=malloc(plan.musicBytes);char filename[32];
  if(plan.shuffle)snprintf(filename,sizeof(filename),"/BgmCache_%02u.bin",plan.slot);else snprintf(filename,sizeof(filename),"/BgmCache.bin");
  ok=music&&read_ext(a,filename,music,plan.musicBytes);
  if(ok){*data=music;*size=plan.musicBytes;}else free(music);
 }
 FSUSER_CloseArchive(a);char log[128];snprintf(log,sizeof(log),"Theme BGM: settings=%u type=%u shuffle=%u result=%u bytes=%lu",known,savedata[0x13bd],savedata[0x141b],ok,(unsigned long)*size);app_log(log);
 return ok?1:0;
}
