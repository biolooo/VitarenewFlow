// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "ds_profile.h"
#include "app_log.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

static const char profile[] =
    "; VitarenewFlow NTR compatibility defaults v1\n"
    "; Existing user profiles are never replaced. Hold Y during the DS loader to edit.\n"
    "[GAMESETTINGS]\nDSI_MODE = 0\nBOOST_CPU = 0\nBOOST_VRAM = 0\n"
    "CARD_READ_DMA = 0\nASYNC_CARD_READ = 0\nSAVE_RELOCATION = 1\nBOOTSTRAP_FILE = 0\n";

static unsigned header_crc(const unsigned char *p,size_t size){
    unsigned crc=0xffff;
    while(size--){crc^=*p++;for(unsigned i=0;i<8;i++)crc=(crc>>1)^((crc&1)?0xa001:0);}
    return crc;
}
static bool code_char(unsigned char c){return (c>='A'&&c<='Z')||(c>='0'&&c<='9');}
static bool exact_profile(const char *path){
    char data[sizeof(profile)];FILE *f=fopen(path,"rb");if(!f)return false;
    size_t n=fread(data,1,sizeof(data),f);bool ok=!ferror(f)&&n==sizeof(profile)-1&&!memcmp(data,profile,n);
    if(fclose(f))ok=false;
    return ok;
}
static bool own_partial_stage(const char *path){
    char data[sizeof(profile)];FILE *f=fopen(path,"rb");if(!f)return false;
    size_t n=fread(data,1,sizeof(data),f);bool ok=!ferror(f)&&n<sizeof(profile)-1&&!memcmp(data,profile,n);
    if(fclose(f))ok=false;
    return ok;
}
/* The cache is disposable, but retain it for diagnosis/rollback. No saves or
   global bootstrap state are touched. Only this ROM's code + header CRC match. */
static bool retire_cache(const char *code,unsigned crc){
    char path[160],backup[192],log[240];struct stat st;
    snprintf(path,sizeof(path),"sdmc:/_nds/nds-bootstrap/patchOffsetCache/%s-%04X.bin",code,crc);
    if(stat(path,&st))return errno==ENOENT;
    if(!S_ISREG(st.st_mode))return false;
    for(unsigned i=0;i<100;i++){
        snprintf(backup,sizeof(backup),"%s.vf-ntr-v1-%u.bak",path,i);
        if(!stat(backup,&st))continue;
        if(errno!=ENOENT)return false;
        if(rename(path,backup))return false;
        snprintf(log,sizeof(log),"DS cache retained: %s",backup);app_handoff_log(log);
        return true;
    }
    return false;
}
Result ds_prepare_profile(const char *rom){
    unsigned char h[0x160];FILE *f=fopen(rom,"rb");if(!f)return (Result)-31;
    size_t n=fread(h,1,sizeof(h),f);bool readOK=n==sizeof(h)&&!ferror(f);fclose(f);
    if(!readOK)return (Result)-31;
    unsigned crc=h[0x15e]|(h[0x15f]<<8);char code[5]={0},log[192];memcpy(code,h+12,4);
    bool retail=true;for(unsigned i=12;i<18;i++)if(!code_char(h[i]))retail=false;
    if(!retail||header_crc(h,0x15e)!=crc){app_handoff_log("DS profile: unrecognised/homebrew header; loader defaults retained");return 0;}
    snprintf(log,sizeof(log),"DS ROM code=%s revision=%u unit=%u headerCRC=%04X",code,h[0x1e],h[0x12],crc);app_handoff_log(log);
    if(h[0x12]!=0){app_handoff_log("DS profile: DSi title; loader defaults retained");return 0;}
    const char *name=strrchr(rom,'/');if(!name||!name[1]||strpbrk(name,"\r\n\\"))return (Result)-33;
    const char *directory="sdmc:/_nds/ntr-forwarder/gamesettings";
    char path[512],stage[544];struct stat st;
    int len=snprintf(path,sizeof(path),"%s/%s.ini",directory,name+1);
    if(len<0||(size_t)len>=sizeof(path))return (Result)-33;
    if(!stat(path,&st)){
        if(!S_ISREG(st.st_mode))return (Result)-36;
        app_handoff_log(exact_profile(path)?"DS profile: NTR 67MHz, synchronous reads (v1)":"DS profile: existing per-game settings preserved");return 0;
    }
    if(errno!=ENOENT)return (Result)-36;
    if(mkdir(directory,0777)&&(errno!=EEXIST||stat(directory,&st)||!S_ISDIR(st.st_mode)))return (Result)-36;
    snprintf(stage,sizeof(stage),"%s.vf-stage",path);
    /* Resume our own interrupted write; leave unknown staging files untouched. */
    bool staged=false;
    if(!stat(stage,&st)){
        if(!S_ISREG(st.st_mode))return (Result)-36;
        staged=exact_profile(stage);
        if(!staged&&!own_partial_stage(stage))return (Result)-36;
    }
    else if(errno!=ENOENT)return (Result)-36;
    if(!staged){
        f=fopen(stage,"wb");if(!f)return (Result)-36;
        bool ok=fwrite(profile,1,sizeof(profile)-1,f)==sizeof(profile)-1;
        if(fclose(f))ok=false;
        if(!ok||!exact_profile(stage)){remove(stage);return (Result)-36;}
    }
    if(!retire_cache(code,crc))return (Result)-36;
    if(rename(stage,path)||!exact_profile(path))return (Result)-36;
    app_handoff_log("DS profile created: NTR 67MHz, synchronous reads; matching patch cache rebuilt once");
    return 0;
}
