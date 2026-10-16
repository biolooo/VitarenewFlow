#include "ds_backend.h"
#include "ds_path.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
Result ds_prepare_launch(const char *rom) {
    const u64 bootstrapId=0x0004800546574452ULL;
    u64 id=bootstrapId;
    AM_TitleInfo title;
    Result result=AM_GetTitleInfo(MEDIATYPE_NAND,1,&id,&title);
    if (R_FAILED(result) || title.titleID!=id) return (Result)-30;
    struct stat info;
    if (stat(rom,&info) || !S_ISREG(info.st_mode) || info.st_size<0x200) return (Result)-31;
    const char *runtime[]={"sdmc:/_nds/ntr-forwarder/sdcard.nds","sdmc:/_nds/nds-bootstrap-release.nds"};
    for (unsigned i=0;i<2;++i) if (stat(runtime[i],&info) || !S_ISREG(info.st_mode) || !info.st_size) return (Result)-32;
    char path[256];
    if (!ds_loader_path(path,sizeof(path),rom)) return (Result)-33;
    /* Same handshake as YANBF's official 3DS-mode forwarder. No INI/save edits. */
    const char *target="sdmc:/_nds/ntr-forwarder/path.txt";
    FILE *file=fopen(target,"wb");
    if (!file) return (Result)-34;
    size_t size=strlen(path);
    bool written=fwrite(path,1,size,file)==size;
    int closed=fclose(file);
    if (!written || closed) return (Result)-34;
    file=fopen(target,"rb");
    char verify[256]={0};
    if (!file) return (Result)-34;
    size_t read=fread(verify,1,sizeof(verify)-1,file); fclose(file);
    if (read!=size || memcmp(path,verify,size)) return (Result)-34;
    return 0;
}
Result ds_commit_launch(void) {
    /* Direct jump matches YANBF. A 3DSX return trampoline can bypass aptExit's deferred chainloader. */
    Result result=APT_PrepareToDoApplicationJump(0,0x0004800546574452ULL,MEDIATYPE_NAND);
    if(R_FAILED(result))return result;
    u8 parameters[0x300]={0},hmac[0x20]={0};
    return APT_DoApplicationJump(parameters,sizeof(parameters),hmac);
}
Result ds_install_bootstrap(void (*progress)(float)) {
    const char *path="/cias/launcher-test-updates/bootstrap.cia";
    FS_Path archive={PATH_EMPTY,1,""};Handle input;
    Result result=FSUSER_OpenFileDirectly(&input,ARCHIVE_SDMC,archive,fsMakePath(PATH_ASCII,path),FS_OPEN_READ,0);
    if(R_FAILED(result))return result;
    AM_TitleInfo info;memset(&info,0,sizeof(info));
    result=AM_GetCiaFileInfo(MEDIATYPE_NAND,&info,input);
    if(R_SUCCEEDED(result)&&info.titleID!=0x0004800546574452ULL)result=(Result)-35;
    u64 size=0;if(R_SUCCEEDED(result))result=FSFILE_GetSize(input,&size);
    if(R_FAILED(result)||!size||size>4*1024*1024){FSFILE_Close(input);return R_FAILED(result)?result:(Result)-35;}
    Handle install;result=AM_StartCiaInstall(MEDIATYPE_NAND,&install);
    if(R_FAILED(result)){FSFILE_Close(input);return result;}
    u8 *buffer=malloc(16384);
    if(!buffer){AM_CancelCIAInstall(install);FSFILE_Close(input);return (Result)-3;}
    u64 offset=0;
    while(offset<size && R_SUCCEEDED(result)) {
        u32 read=0,written=0;u32 amount=size-offset>16384?16384:(u32)(size-offset);
        result=FSFILE_Read(input,&read,offset,buffer,amount);
        if(R_FAILED(result)||read!=amount){if(R_SUCCEEDED(result))result=(Result)-34;break;}
        result=FSFILE_Write(install,&written,offset,buffer,read,0);
        if(R_SUCCEEDED(result)&&written!=read)result=(Result)-34;
        offset+=written;if(progress)progress((float)offset/size);
    }
    free(buffer);FSFILE_Close(input);
    if(R_FAILED(result)){AM_CancelCIAInstall(install);return result;}
    result=AM_FinishCiaInstall(install);
    if(R_FAILED(result))AM_CancelCIAInstall(install);
    return result;
}
