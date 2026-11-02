// SPDX-License-Identifier: GPL-3.0-only
#include "title_cache.h"
#include <string.h>
#include <sys/stat.h>
/* Explicit LE layout, no persisted pointers/padding, bounded to < 5 MiB. */
#define HEADER_SIZE 24
#define RECORD_SIZE (8+2+1+1+160+TITLE_CACHE_ICON_PIXELS*2+4)
static uint32_t checksum(const unsigned char *p,size_t n){
    uint32_t hash=2166136261u;
    for(size_t i=0;i<n;++i)hash=(hash^p[i])*16777619u;
    return hash;
}
static uint32_t get32(const unsigned char *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void put32(unsigned char *p,uint32_t n){for(unsigned i=0;i<4;++i)p[i]=(unsigned char)(n>>(8*i));}
static bool decode(TitleCacheEntry *entry,const unsigned char *record){
    if(checksum(record,RECORD_SIZE-4)!=get32(record+RECORD_SIZE-4)||record[10]>1||record[11]>1||!record[12]||!memchr(record+12,0,160))return false;
    entry->id=0;for(unsigned i=0;i<8;++i)entry->id|=(uint64_t)record[i]<<(8*i);
    if(!entry->id)return false;
    entry->version=record[8]|(uint16_t)record[9]<<8;entry->medium=record[10];entry->homebrew=record[11]!=0;
    memcpy(entry->name,record+12,160);
    for(unsigned i=0;i<TITLE_CACHE_ICON_PIXELS;++i)entry->icon[i]=record[172+2*i]|(uint16_t)record[173+2*i]<<8;
    return true;
}
bool title_cache_read(const char *path,unsigned language,TitleCacheAccept accept,void *context,TitleCacheStats *stats){
    TitleCacheStats result={0};bool valid=false;
    FILE *file=fopen(path,"rb");if(!file){if(stats)*stats=result;return false;}
    unsigned char header[HEADER_SIZE],record[RECORD_SIZE];
    if(fread(header,1,sizeof(header),file)!=sizeof(header)||memcmp(header,"VFTCACHE",8)||get32(header+8)!=1||get32(header+12)!=language||get32(header+20)!=checksum(header,20))goto done;
    unsigned count=get32(header+16);if(count>TITLE_CACHE_LIMIT)goto done;
    if(fseek(file,0,SEEK_END))goto done;
    long size=ftell(file);if(size<0||(unsigned long)size!=HEADER_SIZE+(unsigned long)count*RECORD_SIZE||fseek(file,HEADER_SIZE,SEEK_SET))goto done;
    result.records=count;
    for(unsigned i=0;i<count;++i){
        TitleCacheEntry entry;
        if(fread(record,1,sizeof(record),file)!=sizeof(record)){result.invalid+=count-i;goto done;}
        if(!decode(&entry,record)){++result.invalid;continue;}
        int accepted=accept?accept(&entry,context):0;
        if(accepted<0){result.invalid+=count-i;goto done;}
        if(accepted>0)++result.hits;
    }
    valid=true;
done:
    fclose(file);if(stats)*stats=result;return valid;
}
void title_cache_abort(TitleCacheWriter *writer){
    if(writer->file){fclose(writer->file);writer->file=NULL;}
    if(writer->temporary[0])remove(writer->temporary);
    memset(writer,0,sizeof(*writer));
}
bool title_cache_begin(TitleCacheWriter *writer,const char *path,unsigned language,unsigned count){
    title_cache_abort(writer);
    if(!path||strlen(path)>=sizeof(writer->path)||count>TITLE_CACHE_LIMIT)return false;
    snprintf(writer->path,sizeof(writer->path),"%s",path);
    snprintf(writer->temporary,sizeof(writer->temporary),"%s.tmp",path);
    snprintf(writer->previous,sizeof(writer->previous),"%s.previous",path);
    writer->expected=count;writer->file=fopen(writer->temporary,"wb");if(!writer->file)return false;
    unsigned char header[HEADER_SIZE]={0};memcpy(header,"VFTCACHE",8);
    put32(header+8,1);put32(header+12,language);put32(header+16,count);put32(header+20,checksum(header,20));
    if(fwrite(header,1,sizeof(header),writer->file)!=sizeof(header)){title_cache_abort(writer);return false;}
    return true;
}
bool title_cache_append(TitleCacheWriter *writer,const TitleCacheEntry *entry){
    if(!writer->file||writer->written>=writer->expected||!entry||!entry->id||entry->medium>1||!entry->name[0]||!memchr(entry->name,0,sizeof(entry->name)))return false;
    unsigned char record[RECORD_SIZE]={0};
    for(unsigned i=0;i<8;++i)record[i]=(unsigned char)(entry->id>>(8*i));
    record[8]=(unsigned char)entry->version;record[9]=(unsigned char)(entry->version>>8);record[10]=entry->medium;record[11]=entry->homebrew;
    memcpy(record+12,entry->name,sizeof(entry->name));
    for(unsigned i=0;i<TITLE_CACHE_ICON_PIXELS;++i){record[172+2*i]=(unsigned char)entry->icon[i];record[173+2*i]=(unsigned char)(entry->icon[i]>>8);}
    put32(record+RECORD_SIZE-4,checksum(record,RECORD_SIZE-4));
    if(fwrite(record,1,sizeof(record),writer->file)!=sizeof(record)){title_cache_abort(writer);return false;}
    ++writer->written;return true;
}
bool title_cache_finish(TitleCacheWriter *writer){
    if(!writer->file||writer->written!=writer->expected){title_cache_abort(writer);return false;}
    bool closed=fclose(writer->file)==0;writer->file=NULL;
    if(!closed){title_cache_abort(writer);return false;}
    struct stat st;bool old=stat(writer->path,&st)==0;
    /* FAT rename cannot replace. Preserve the committed cache until new bytes
       have closed successfully; a crash leaves a complete .previous fallback. */
    if(old){
        if(!S_ISREG(st.st_mode)){title_cache_abort(writer);return false;}
        remove(writer->previous);
        if(rename(writer->path,writer->previous)){title_cache_abort(writer);return false;}
    }
    if(rename(writer->temporary,writer->path)){
        if(old)rename(writer->previous,writer->path);
        title_cache_abort(writer);return false;
    }
    if(old)remove(writer->previous);
    memset(writer,0,sizeof(*writer));return true;
}
