#include "banner_export.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
static bool cached_banner(const char *path){
    struct stat st;
    if(stat(path,&st)||!S_ISREG(st.st_mode)||st.st_size<0x88||st.st_size>4*1024*1024)return false;
    FILE *f=fopen(path,"rb");
    if(!f)return false;
    char magic[4];bool valid=fread(magic,1,4,f)==4&&!memcmp(magic,"CBMD",4);
    fclose(f);return valid;
}
Result banner_export_title(u64 title,FS_MediaType media){
    char path[160],temp[168],backup[176];
    snprintf(path,sizeof(path),"sdmc:/3ds/3ds-library/banners/raw/%016llX.bnr",(unsigned long long)title);
    if(cached_banner(path))return 0;
    u32 aData[]={(u32)title,(u32)(title>>32),media,0},fData[]={0,0,2,0x6e6e6162,0x7265};
    FS_Path a={PATH_BINARY,sizeof(aData),aData},f={PATH_BINARY,sizeof(fData),fData};
    Handle in;
    Result r=FSUSER_OpenFileDirectly(&in,ARCHIVE_SAVEDATA_AND_CONTENT,a,f,FS_OPEN_READ,0);
    if(R_FAILED(r))return r;
    u64 size=0;
    r=FSFILE_GetSize(in,&size);
    if(R_FAILED(r)){FSFILE_Close(in);return r;}
    if(size<0x88||size>4*1024*1024){FSFILE_Close(in);return (Result)-60;}
    unsigned char *data=malloc((size_t)size);
    if(!data){FSFILE_Close(in);return (Result)-66;}
    /* NCCH ExeFS system-menu files require offset=0, size=file_size.
       Both the old four-byte probe and subsequent chunked reads are invalid.
       Writes to our ordinary SD cache have no such ExeFS restriction. */
    u32 got=0;
    r=FSFILE_Read(in,&got,0,data,(u32)size);
    FSFILE_Close(in);
    if(R_FAILED(r)){free(data);return r;}
    if(got!=size){free(data);return (Result)-67;}
    if(memcmp(data,"CBMD",4)){free(data);return (Result)-61;}
    mkdir("sdmc:/3ds/3ds-library/banners",0777);
    mkdir("sdmc:/3ds/3ds-library/banners/raw",0777);
    snprintf(temp,sizeof(temp),"%s.tmp",path);
    FILE *out=fopen(temp,"wb");
    if(!out){free(data);return (Result)-62;}
    bool written=fwrite(data,1,(size_t)size,out)==size;
    free(data);
    if(fclose(out))written=false;
    if(!written){remove(temp);return (Result)-64;}
    struct stat st;bool preserved=!stat(path,&st);
    snprintf(backup,sizeof(backup),"%s.previous",path);
    if(preserved){
        /* Keep an invalid prior cache, never overwrite an existing backup. */
        if(!S_ISREG(st.st_mode)||!stat(backup,&st)||rename(path,backup)){remove(temp);return (Result)-68;}
    }
    if(rename(temp,path)){if(preserved)rename(backup,path);remove(temp);return (Result)-65;}
    return 0;
}
