#include "theme_io.h"
#include "theme_format.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
static bool read_ext(FS_Archive a,const char *name,void *buffer,unsigned bytes){
    Handle f;
    u32 read=0;
    Result r=FSUSER_OpenFile(&f,a,fsMakePath(PATH_ASCII,name),FS_OPEN_READ,0);
    if(R_FAILED(r))return false;
    r=FSFILE_Read(f,&read,0,buffer,bytes);
    FSFILE_Close(f);
    return R_SUCCEEDED(r)&&read==bytes;
}
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
