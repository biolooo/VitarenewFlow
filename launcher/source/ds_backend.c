// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "ds_backend.h"
#include "ds_path.h"
#include "ds_profile.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
DsRuntimeState ds_runtime_check(void){
    u64 id=DS_BOOTSTRAP_ID;AM_TitleInfo title={0};
    Result r=AM_GetTitleInfo(MEDIATYPE_NAND,1,&id,&title);
    if(R_FAILED(r)||title.titleID!=id){
        /* An AM service error must not be reported as an absent title. Only
           a complete, successful title-list query proves that it is missing. */
        u32 count=0,read=0;
        if(R_FAILED(AM_GetTitleCount(MEDIATYPE_NAND,&count))||count>8192)return DS_RUNTIME_UNAVAILABLE;
        if(!count)return DS_RUNTIME_NO_TITLE;
        u64 *ids=malloc((size_t)count*sizeof(*ids));if(!ids)return DS_RUNTIME_UNAVAILABLE;
        r=AM_GetTitleList(&read,MEDIATYPE_NAND,count,ids);bool found=false;
        if(R_SUCCEEDED(r)&&read==count)for(u32 i=0;i<read;i++)if(ids[i]==id)found=true;
        free(ids);
        if(R_FAILED(r)||read!=count||found)return DS_RUNTIME_UNAVAILABLE;
        return DS_RUNTIME_NO_TITLE;
    }
    struct stat file;
    if(stat("sdmc:/_nds/ntr-forwarder/sdcard.nds",&file)||!S_ISREG(file.st_mode)||file.st_size<=0)return DS_RUNTIME_NO_FORWARDER;
    if(stat("sdmc:/_nds/nds-bootstrap-release.nds",&file)||!S_ISREG(file.st_mode)||file.st_size<=0)return DS_RUNTIME_NO_BOOTSTRAP;
    return DS_RUNTIME_READY;
}
Result ds_prepare_launch(const char *rom) {
    DsRuntimeState runtime=ds_runtime_check();
    if(runtime==DS_RUNTIME_NO_TITLE||runtime==DS_RUNTIME_UNAVAILABLE)return (Result)-30;
    if(runtime!=DS_RUNTIME_READY)return (Result)-32;
    struct stat info;
    if (stat(rom,&info) || !S_ISREG(info.st_mode) || info.st_size<0x200) return (Result)-31;
    char path[256];
    if (!ds_loader_path(path,sizeof(path),rom)) return (Result)-33;
    Result result=ds_prepare_profile(rom);
    if(R_FAILED(result))return result;
    /* Same handshake as YANBF's official 3DS-mode forwarder. Saves are untouched. */
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
Result ds_install_bootstrap_file(const char *path,void (*progress)(float)) {
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

Result ds_install_bootstrap(void (*progress)(float)){return ds_install_bootstrap_file("/cias/launcher-test-updates/bootstrap.cia",progress);}
