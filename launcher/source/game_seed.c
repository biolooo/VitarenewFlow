// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
/* Independent implementation of the seed workflow documented by FBI's
   source/core/http.c: local 16-byte seed, Nintendo ext_key endpoint, FS IPC. */
#include "game_seed.h"
#include "catalog_format.h"
#include "app_log.h"
#include <stdio.h>
#include <string.h>
static Result local_seed(u64 id,u8 seed[16]){
    const char *folders[]={"sdmc:/3ds/3ds-library/seeds","sdmc:/fbi/seed"};
    for(unsigned i=0;i<2;i++){
        char path[96];snprintf(path,sizeof(path),"%s/%016llX.dat",folders[i],(unsigned long long)id);
        FILE *f=fopen(path,"rb");if(!f)continue;
        u8 bytes[17];size_t n=fread(bytes,1,sizeof(bytes),f);bool bad=ferror(f)!=0;fclose(f);
        if(bad||n!=16)return SEED_BAD_DATA;
        memcpy(seed,bytes,16);return 0;
    }
    return (Result)-1;
}
static Result online_seed(u64 id,u8 seed[16]){
    u8 region=0;Result r=cfguInit();if(R_FAILED(r))return r;
    r=CFGU_SecureInfoGetRegion(&region);cfguExit();if(R_FAILED(r))return r;
    static const char *countries[]={"JP","US","GB","GB","HK","KR","TW"};
    if(region>=sizeof(countries)/sizeof(countries[0]))return SEED_BAD_DATA;
    char url[160];snprintf(url,sizeof(url),"https://kagiya-ctr.cdn.nintendo.net/title/0x%016llX/ext_key?country=%s",(unsigned long long)id,countries[region]);
    r=httpcInit(0x1000);if(R_FAILED(r))return r;
    httpcContext context; r=httpcOpenContext(&context,HTTPC_METHOD_GET,url,1);
    if(R_SUCCEEDED(r)){
        /* Nintendo's CDN uses its own CA, already available in the 3DS trust store. */
        r=httpcAddDefaultCert(&context,SSLC_DefaultRootCert_Nintendo_Class2_CA_G3);
        if(R_SUCCEEDED(r))r=httpcBeginRequest(&context);
        u32 status=0,downloaded=0,total=0;
        if(R_SUCCEEDED(r))r=httpcGetResponseStatusCodeTimeout(&context,&status,10000000000ULL);
        if(R_SUCCEEDED(r)&&status==404)r=SEED_NOT_OFFERED;
        else if(R_SUCCEEDED(r)&&status!=200)r=SEED_HTTP_ERROR;
        if(r==0){
            u8 bytes[17]={0};r=httpcReceiveDataTimeout(&context,bytes,sizeof(bytes),10000000000ULL);
            if(R_SUCCEEDED(r))r=httpcGetDownloadSizeState(&context,&downloaded,&total);
            if(R_SUCCEEDED(r)&&(downloaded!=16||(total&&total!=16)))r=SEED_BAD_DATA;
            if(R_SUCCEEDED(r))memcpy(seed,bytes,16);
        }
        httpcCloseContext(&context);
    }
    httpcExit();return r;
}
Result game_seed_import(u64 id){
    if(!catalog_is_application(id))return SEED_BAD_DATA;
    u8 seed[16];Result r=local_seed(id,seed);
    if(r==(Result)-1)r=online_seed(id,seed);
    if(r==0){
        u32 *command=getThreadCommandBuffer();command[0]=IPC_MakeHeader(0x87A,6,0);
        command[1]=(u32)id;command[2]=(u32)(id>>32);memcpy(command+3,seed,16);
        r=svcSendSyncRequest(*fsGetSessionHandle());if(R_SUCCEEDED(r))r=(Result)command[1];
    }
    char log[96];snprintf(log,sizeof(log),"Seed %016llX: %08lX",(unsigned long long)id,(unsigned long)(u32)r);app_log(log);
    return r;
}

/* FS commands documented by 3dbrew: TitleContentHasSeed and CheckTitleSeed.
   No network request for titles without seed crypto or with a valid seed. */
static Result seed_check(u64 id,FS_MediaType media){
    u32 *c=getThreadCommandBuffer();c[0]=IPC_MakeHeader(0x885,3,0);
    c[1]=media;c[2]=(u32)id;c[3]=(u32)(id>>32);
    Result r=svcSendSyncRequest(*fsGetSessionHandle());return R_FAILED(r)?r:(Result)c[1];
}
Result game_seed_ensure(u64 id,FS_MediaType media,u32 *phase){
    if(!catalog_is_application(id))return SEED_BAD_DATA;
    __atomic_store_n(phase,0,__ATOMIC_RELEASE);
    u32 *c=getThreadCommandBuffer();c[0]=IPC_MakeHeader(0x87F,4,0);
    c[1]=media;c[2]=(u32)id;c[3]=(u32)(id>>32);c[4]=0;
    Result r=svcSendSyncRequest(*fsGetSessionHandle());
    if(R_SUCCEEDED(r))r=(Result)c[1];
    bool required=R_SUCCEEDED(r)&&c[2]!=0;
    char log[128];snprintf(log,sizeof(log),"Seed preflight %016llX required=%u query=%08lX",(unsigned long long)id,required,(unsigned long)(u32)r);app_log(log);
    if(R_FAILED(r)||!required)return r;
    r=seed_check(id,media);
    snprintf(log,sizeof(log),"Seed validation %016llX: %08lX",(unsigned long long)id,(unsigned long)(u32)r);app_log(log);
    if(R_SUCCEEDED(r))return 0;
    /* Only documented missing-seed / invalid-seed errors trigger a repair. */
    if((u32)r!=0xC8804464 && (u32)r!=0xD900458C)return r;
    __atomic_store_n(phase,1,__ATOMIC_RELEASE);
    r=game_seed_import(id);
    if(r==SEED_NOT_OFFERED)return SEED_HTTP_ERROR;
    if(R_FAILED(r))return r;
    __atomic_store_n(phase,2,__ATOMIC_RELEASE);
    r=seed_check(id,media);
    snprintf(log,sizeof(log),"Seed repaired verification %016llX: %08lX",(unsigned long long)id,(unsigned long)(u32)r);app_log(log);
    return r;
}
