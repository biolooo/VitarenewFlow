// SPDX-License-Identifier: GPL-3.0-only
#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#include "title_cache.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
static unsigned accepted;
static unsigned versionOffset;
static int cancelRead(const TitleCacheEntry *entry,void *context){(void)entry;(void)context;return -1;}
static int accept(const TitleCacheEntry *entry,void *context){
    unsigned count=*(unsigned*)context;
    assert(entry->medium==(entry->id&1));
    assert(entry->homebrew==((entry->id%3)==0));
    assert(entry->icon[0]==(uint16_t)entry->id);
    assert(entry->icon[TITLE_CACHE_ICON_PIXELS-1]==(uint16_t)(entry->id+7));
    char name[160];snprintf(name,sizeof(name),"Game %llu",(unsigned long long)entry->id);assert(!strcmp(entry->name,name));
    if(entry->id>count||entry->version!=7+versionOffset)return false;
    ++accepted;return true;
}
static TitleCacheEntry makeEntry(unsigned id){
    TitleCacheEntry entry={0};entry.id=id;entry.version=7;entry.medium=id&1;entry.homebrew=(id%3)==0;
    snprintf(entry.name,sizeof(entry.name),"Game %u",id);entry.icon[0]=id;entry.icon[TITLE_CACHE_ICON_PIXELS-1]=id+7;return entry;
}
static void writeCache(const char *path,unsigned count){
    TitleCacheWriter writer={0};assert(title_cache_begin(&writer,path,2,count));
    for(unsigned i=1;i<=count;++i){TitleCacheEntry entry=makeEntry(i);assert(title_cache_append(&writer,&entry));}
    assert(title_cache_finish(&writer));
}
static void flipByte(const char *path,long offset){FILE *f=fopen(path,"r+b");assert(f);assert(!fseek(f,offset,SEEK_SET));int c=fgetc(f);assert(c!=EOF);assert(!fseek(f,offset,SEEK_SET));assert(fputc(c^0x80,f)!=EOF);assert(!fclose(f));}
int main(void){
    char temp[]="/tmp/flow-title-cache-XXXXXX";assert(mkdtemp(temp));char path[512],previous[524];snprintf(path,sizeof(path),"%s/cache",temp);snprintf(previous,sizeof(previous),"%s.previous",path);
    unsigned count=300;TitleCacheStats stats;accepted=0;
    assert(!title_cache_read(path,2,accept,&count,&stats));
    writeCache(path,count);assert(!title_cache_read(path,2,cancelRead,NULL,&stats)&&stats.invalid==count&&!stats.hits);assert(title_cache_read(path,2,accept,&count,&stats));assert(stats.records==count&&stats.hits==count&&!stats.invalid&&accepted==count);
    accepted=0;assert(!title_cache_read(path,1,accept,&count,&stats)&&!accepted); // language
    versionOffset=1;assert(title_cache_read(path,2,accept,&count,&stats)&&stats.hits==0);versionOffset=0;
    count=299;accepted=0;assert(title_cache_read(path,2,accept,&count,&stats)&&stats.hits==299&&accepted==299); // removed title not resurrected
    count=301;accepted=0;assert(title_cache_read(path,2,accept,&count,&stats)&&stats.hits==300&&accepted==300); // new title remains miss
    // Corrupt icon bytes: the entire damaged metadata record is rejected.
    count=300;flipByte(path,24+178);accepted=0;assert(title_cache_read(path,2,accept,&count,&stats)&&stats.invalid==1&&stats.hits==299&&accepted==299);
    // Truncated files are refused before callbacks, never a partial catalog.
    struct stat st;assert(!stat(path,&st));assert(!truncate(path,st.st_size-1));accepted=0;assert(!title_cache_read(path,2,accept,&count,&stats)&&!accepted);
    writeCache(path,count);flipByte(path,16);assert(!title_cache_read(path,2,accept,&count,&stats)); // corrupt header count/checksum
    writeCache(path,count); // abort, incomplete and rename failure retain the prior committed cache
    TitleCacheWriter writer={0};assert(title_cache_begin(&writer,path,2,301));TitleCacheEntry entry=makeEntry(1);assert(title_cache_append(&writer,&entry));title_cache_abort(&writer);
    accepted=0;assert(title_cache_read(path,2,accept,&count,&stats)&&stats.hits==300);
    assert(title_cache_begin(&writer,path,2,301));assert(!title_cache_finish(&writer));accepted=0;assert(title_cache_read(path,2,accept,&count,&stats)&&stats.hits==300);
    assert(title_cache_begin(&writer,path,2,1));assert(title_cache_append(&writer,&entry));assert(!mkdir(previous,0700));char blocked[540];snprintf(blocked,sizeof(blocked),"%s/keep",previous);FILE *block=fopen(blocked,"wb");assert(block);assert(!fclose(block));assert(!title_cache_finish(&writer));assert(!remove(blocked));assert(!rmdir(previous));accepted=0;assert(title_cache_read(path,2,accept,&count,&stats)&&stats.hits==300);
    assert(!title_cache_begin(&writer,path,2,TITLE_CACHE_LIMIT+1));
    // Bounded maximum library, one record of scratch storage regardless of N.
    count=TITLE_CACHE_LIMIT;clock_t before=clock();writeCache(path,count);accepted=0;assert(title_cache_read(path,2,accept,&count,&stats)&&accepted==count);double ms=1000.0*(clock()-before)/CLOCKS_PER_SEC;
    printf("PASS: title cache 0/300/1024, language/version/new/removed, corruption/truncation, abort and rename-failure preservation; maximum roundtrip CPU %.2f ms (HOST, not console)\n",ms);
    writeCache(path,0);accepted=0;assert(title_cache_read(path,2,accept,&count,&stats)&&stats.records==0&&!accepted);
    assert(!remove(path));assert(!rmdir(temp));return 0;
}
