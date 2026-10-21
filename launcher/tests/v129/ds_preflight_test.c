
#include <3ds.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
void app_handoff_log(const char *p){assert(p);}
static int queryError;
Result AM_GetTitleInfo(FS_MediaType m,u32 n,u64 *id,AM_TitleInfo *out){assert(m==MEDIATYPE_NAND&&n==1&&*id==0x0004800546574452ULL);out->titleID=*id;return queryError;}

#include "ds_profile.c"
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
Result ds_prepare_launch(const char *rom) {
    const u64 bootstrapId=DS_BOOTSTRAP_ID;
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
    result=ds_prepare_profile(rom);
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

static void writeBytes(const char *p,unsigned n){FILE*f=fopen(p,"wb");assert(f);for(unsigned i=0;i<n;i++)fputc(i&255,f);assert(!fclose(f));}
int main(void){
 char dir[]="/tmp/vf-ds-v129-XXXXXX";assert(mkdtemp(dir));assert(!chdir(dir));mkdir("sdmc:",0700);mkdir("sdmc:/roms",0700);mkdir("sdmc:/roms/nds",0700);mkdir("sdmc:/_nds",0700);mkdir("sdmc:/_nds/ntr-forwarder",0700);
 const char*rom="sdmc:/roms/nds/Test game (Europe).nds";
 queryError=-10;assert(ds_prepare_launch(rom)==-30);queryError=0;assert(ds_prepare_launch(rom)==-31);writeBytes(rom,0x100);assert(ds_prepare_launch(rom)==-31);writeBytes(rom,0x400);assert(ds_prepare_launch(rom)==-32);
 writeBytes("sdmc:/_nds/ntr-forwarder/sdcard.nds",100);writeBytes("sdmc:/_nds/nds-bootstrap-release.nds",100);assert(!ds_prepare_launch(rom));
 char buf[256]={0};FILE*f=fopen("sdmc:/_nds/ntr-forwarder/path.txt","rb");assert(f);fread(buf,1,sizeof(buf)-1,f);fclose(f);assert(!strcmp(buf,"sd:/roms/nds/Test game (Europe).nds"));
 unsigned char h[0x400]={0};memcpy(h+12,"ASME01",6);unsigned crc=header_crc(h,0x15e);h[0x15e]=crc;h[0x15f]=crc>>8;
 f=fopen(rom,"wb");assert(f&&fwrite(h,1,sizeof(h),f)==sizeof(h));fclose(f);assert(!ds_prepare_launch(rom));
 const char *profilePath="sdmc:/_nds/ntr-forwarder/gamesettings/Test game (Europe).nds.ini";assert(exact_profile(profilePath));
 remove(profilePath);assert(!mkdir(profilePath,0700));assert(ds_prepare_launch(rom)==-36);assert(!rmdir(profilePath));
 remove("sdmc:/_nds/ntr-forwarder/path.txt");assert(!mkdir("sdmc:/_nds/ntr-forwarder/path.txt",0700));assert(ds_prepare_launch(rom)==-34);
 puts("PASS actual DS preflight: installed bootstrap check, missing/truncated ROM, missing runtime, spaced path encoding and verified handoff file, write failure");
}
