"""Compile the CURRENT scan title phase and cache scheduling, with service mocks.
No copied runtime fragments: extraction is performed again for every test run.
The real filesystem/cache implementation is used in an isolated temp directory.
"""
from pathlib import Path
import os, subprocess, tempfile
root=Path(__file__).resolve().parents[2]
source=(root/'launcher/source/main.c').read_text()
start=source.index('#define TITLE_CACHE_PATH ')
stop=source.index('    Cartridge cart;',start)
runtime=source[start:stop]+'}\n'
# The rest of scan (CIA/ROM/DS/cartridge) is unchanged, outside this 3DS-cache test.
game=source[source.index('#define MAX_GAMES '):source.index('static Game games[')]
prefix=r'''
#define _POSIX_C_SOURCE 200809L
#include "title_cache.h"
#include "catalog_format.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
typedef uint64_t u64;typedef uint32_t u32;typedef uint16_t u16;typedef uint8_t u8;
typedef int Result;typedef unsigned FS_MediaType;typedef struct {unsigned dummy;} CiaPackage;
typedef struct {u64 titleID;u64 size;u16 version;} AM_TitleInfo;
#define R_SUCCEEDED(r) ((r)>=0)
#define R_FAILED(r) ((r)<0)
#define MEDIATYPE_NAND 0
#define MEDIATYPE_SD 1
#define SYSTEM_3DS 0
static bool failCacheAllocation;
void *title_cache_test_malloc(size_t size){return failCacheAllocation?NULL:malloc(size);}
static unsigned liveCount=300,nandCount,baseId,changedVersion;
static bool infoFails;
static unsigned productReads,metadataReads,batchCalls,draws;
static u64 now;
static u64 osGetTime(void){return now;}
static bool flowMainLoop(void){return true;}
static void ui_clear_covers(void){}
static const char *tr(const char *p){return p;}
static void draw(bool loading,float progress,const char *stage){(void)loading;(void)progress;(void)stage;++draws;now+=17;}
static void app_perf_log(const char *p){(void)p;}
static Result AM_GetTitleCount(unsigned media,u32 *count){*count=media==MEDIATYPE_SD?liveCount:nandCount;return 0;}
static Result AM_GetTitleList(u32 *read,unsigned media,u32 count,u64 *ids){(void)media;*read=count;for(unsigned i=0;i<count;++i)ids[i]=i+baseId+1;return 0;}
static Result AM_GetTitleInfo(unsigned media,unsigned count,u64 *ids,AM_TitleInfo *info){(void)media;assert(count<=32);++batchCalls;if(infoFails)return -1;for(unsigned i=0;i<count;++i){info[i].titleID=ids[i];info[i].version=ids[i]==changedVersion?8:7;}return 0;}
static Result AM_GetTitleProductCode(unsigned media,u64 id,char *product){(void)media;(void)id;strcpy(product,"CTR-P-X");++productReads;return 0;}
static bool catalog_is_application_mock(u64 id){return id!=0;}
#define catalog_is_application catalog_is_application_mock
static bool library_is_homebrew(u64 id,const char *product){(void)id;return product&&!strcmp(product,"CTR-H-X");}
static bool library_cover_path(char *path,size_t capacity,int system,const char *key){(void)system;size_t length=strlen(key);if(length+7>capacity)return false;memmove(path+6,key,length+1);memcpy(path,"cover/",6);return true;}
static u8 smdh[SMDH_SIZE];
static Result metadata(u64 id,unsigned media){++metadataReads;now+=3;memset(smdh,0,sizeof(smdh));smdh[0]=id&255;smdh[1]=media;return 0;}
static void catalog_name_mock(char *name,size_t capacity,const u8 *data,unsigned language,bool publisher){(void)publisher;snprintf(name,capacity,"Game %u medium %u lang %u",data[0],data[1],language);}
#define catalog_name catalog_name_mock
'''
state=r'''
static Game games[MAX_GAMES];
static unsigned gameCount,visibleCount,selected,language=2;
static bool amReady=true;
static Result lastError;
static const char *status;
'''
tests=r'''
static void counters(void){productReads=metadataReads=batchCalls=draws=0;}
static void completeCache(void){now+=1001;unsigned ticks=0;while(titleCachePending&&ticks++<2000)titleCacheIdle(true);assert(!titleCachePending);}
int main(void){
 assert(!mkdir("sdmc:",0700));assert(!mkdir("sdmc:/3ds",0700));assert(!mkdir("sdmc:/3ds/3ds-library",0700));
 // Cold 300-title catalog: zero banner operations; no forced VBlank per title.
 scan(false);assert(gameCount==300&&metadataReads==300&&productReads==300&&scanCacheHits==0&&batchCalls==10);assert(draws<20);unsigned coldDraws=draws;assert(titleCachePending);completeCache();
 counters();scan(false);assert(gameCount==300&&scanCacheHits==300&&metadataReads==0&&productReads==0&&batchCalls==10&&draws==0&&!titleCachePending);
 printf("PASS actual 300-title scan: cold metadata/product=%u/%u frames=%u; warm metadata/product/banner=0/0/0 frames=%u version_batches=%u\n",300u,300u,coldDraws,draws,batchCalls);
 // A cache scratch allocation failure falls back to live title contents.
 failCacheAllocation=true;counters();scan(false);assert(scanCacheHits==0&&metadataReads==300&&productReads==300);failCacheAllocation=false;
 // Removed ID 1 cannot return from the cache, new ID301 is loaded live.
 baseId=1;counters();scan(false);assert(gameCount==300&&games[0].id==2&&games[299].id==301&&scanCacheHits==299&&metadataReads==1);completeCache();
 // Installed version change invalidates just that title.
 changedVersion=2;counters();scan(false);assert(scanCacheHits==299&&metadataReads==1&&games[0].titleVersion==8);completeCache();
 // Failed version IPC never accepts stale metadata as version zero.
 infoFails=true;counters();scan(false);assert(scanCacheHits==0&&metadataReads==300);infoFails=false;
 // Explicit refresh always reads current name/icon/product metadata.
 counters();scan(true);assert(scanCacheHits==0&&metadataReads==300&&productReads==300);completeCache();
 // SD and NAND with same title ID are different cache keys.
 nandCount=1;counters();scan(false);assert(gameCount==301&&scanCacheHits==300&&metadataReads==1);completeCache();
 counters();scan(false);assert(scanCacheHits==301&&metadataReads==0);assert(games[300].media==MEDIATYPE_NAND);
 // Language invalidation: old names never become valid for another language.
 language=1;counters();scan(false);assert(scanCacheHits==0&&metadataReads==301);assert(strstr(games[0].name,"lang 1"));
 // If settings change language before idle save, the scan language stays bound.
 language=2;completeCache();TitleCacheStats stats;assert(title_cache_read(TITLE_CACHE_PATH,1,NULL,NULL,&stats));assert(!title_cache_read(TITLE_CACHE_PATH,2,NULL,NULL,&stats));
 // Quantity ceiling includes the actual scan path and never grows unbounded.
 nandCount=0;liveCount=1024;language=2;counters();scan(false);assert(gameCount==1024&&metadataReads==1024&&batchCalls==32&&draws<50);completeCache();
 counters();scan(false);assert(scanCacheHits==1024&&metadataReads==0&&productReads==0&&batchCalls==32&&draws==0&&!titleCachePending);
 printf("PASS actual 1024-title warm scan: metadata/product/banner=0/0/0 frames=0 version_batches=32; new/removed/version/failure/refresh/medium/language/save cases\n");
 // Rescan aborts an in-progress write, then still reads the committed cache.
 counters();scan(true);now+=1001;titleCacheIdle(true);assert(titleCacheWriter.file);counters();scan(false);assert(!titleCacheWriter.file&&scanCacheHits==1024&&metadataReads==0);
 titleCacheCancel();for(unsigned i=0;i<gameCount;++i)free(games[i].icon);
 return 0;
}
'''
with tempfile.TemporaryDirectory(prefix='flow-scan-test-') as temporary:
    folder=Path(temporary);test=folder/'test.c';test.write_text(prefix+game+state+runtime+tests)
    flags=[os.environ.get('CC','cc'),'-std=c11','-O2','-Wall','-Wextra','-Werror','-fsanitize=undefined','-I'+str(root/'launcher/source')]
    subprocess.run(flags+['-Dmalloc=title_cache_test_malloc','-c',str(root/'launcher/source/title_cache.c'),'-o',str(folder/'cache.o')],check=True)
    subprocess.run(flags+[str(test),str(folder/'cache.o'),'-o',str(folder/'test')],check=True)
    subprocess.run([str(folder/'test')],cwd=folder,check=True)
# The settings handler must cancel the old pending save, not relabel old names.
assert 'if(saved){titleCacheCancel();status=tr("Sprache gespeichert");language=' in source
print('PASS: language setting cancels old pending write; runtime fragments extracted from current main.c')
