// SPDX-License-Identifier: GPL-3.0-only
#define _POSIX_C_SOURCE 200809L
#include "rom_library.h"
#include "library_model.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
typedef struct {DIR *host;char path[DS_PATH_MAX];} Handle;
static Handle handles[16];
static unsigned active,opens,closes,statCalls,fileStatCalls,fileOpenCalls,visits,progressCalls,stopAt,exhaustions;
static bool allocationFailure,addedSeen,removedSeen;
static const char *readFailure,*openFailure;
static Handle *handle(DIR *dir){for(unsigned i=0;i<active;i++)if(handles[i].host==dir)return &handles[i];assert(!"untracked directory");return NULL;}
DIR *rom_native_opendir(const char *path){
 if(openFailure&&!strcmp(path,openFailure)){errno=EACCES;return NULL;}
 DIR *host=opendir(path);if(!host)return NULL;
 assert(active<sizeof(handles)/sizeof(handles[0]));Handle *h=&handles[active++];h->host=host;assert(strlen(path)<sizeof(h->path));strcpy(h->path,path);opens++;return host;
}
struct dirent *rom_native_readdir(DIR *dir){
 Handle *h=handle(dir);
 if(readFailure&&!strcmp(h->path,readFailure)){errno=EIO;return NULL;}
 struct dirent *entry=readdir(dir);
 /* Public SDK readdir maps archive_dirnext's ENOENT to errno zero, while
    exposing IFTODT(st_mode) in d_type. Host provides these same entry types. */
 if(!entry){errno=0;exhaustions++;}
 else assert(entry->d_type!=DT_UNKNOWN);
 return entry;
}
int rom_native_closedir(DIR *dir){unsigned i=(unsigned)(handle(dir)-handles);int rc=closedir(dir);assert(active);handles[i]=handles[--active];closes++;return rc;}
FILE *rom_native_fopen(const char *path,const char *mode){(void)path;(void)mode;fileOpenCalls++;errno=EACCES;return NULL;}
int rom_native_open(const char *path,int flags,...){(void)path;(void)flags;fileOpenCalls++;errno=EACCES;return -1;}
int rom_native_lstat(const char *path,struct stat *metadata){statCalls++;if(library_file_system(path)>=0)fileStatCalls++;return lstat(path,metadata);}
void *rom_native_calloc(size_t count,size_t size){return allocationFailure?NULL:calloc(count,size);}
static void directory(const char *path){assert(!mkdir(path,0700));}
static void file(const char *path){FILE *f=fopen(path,"wb");assert(f);assert(!fclose(f));}
static void files(const char *root,const char *extension,unsigned count){for(unsigned i=0;i<count;i++){char path[DS_PATH_MAX];int n=snprintf(path,sizeof(path),"%s/f%03u.%s",root,i,extension);assert(n>0&&n<(int)sizeof(path));file(path);}}
static bool visit(const char *path,void *unused){(void)unused;visits++;if(strstr(path,"added.gba"))addedSeen=true;if(strstr(path,"f000.gba"))removedSeen=true;return !stopAt||visits<stopAt;}
static bool proceed(void *unused){(void)unused;return ++progressCalls<20;}
static DsScan scan(const DsRoots *roots,DsContinue progress){
 DsScan result;assert(!active);opens=closes=statCalls=fileStatCalls=fileOpenCalls=visits=progressCalls=exhaustions=0;addedSeen=removedSeen=false;
 rom_library_scan(roots,&result,visit,progress,NULL);assert(!active&&opens==closes);assert(!fileStatCalls&&!fileOpenCalls);return result;
}
int main(void){
 char temp[]="/tmp/flow-native-dir-XXXXXX";assert(mkdtemp(temp));assert(!chdir(temp));directory("sdmc:");
 directory("sdmc:/GBA");files("sdmc:/GBA","gba",234);directory("sdmc:/NES");files("sdmc:/NES","nes",222);
 directory("sdmc:/SNES");files("sdmc:/SNES","sfc",214);directory("sdmc:/Virtual Boy");files("sdmc:/Virtual Boy","vb",24);
 directory("sdmc:/Games");directory("sdmc:/Games/games ds");files("sdmc:/Games/games ds","nds",131);
 directory("sdmc:/3ds");directory("sdmc:/3ds/DaedalusX64");directory("sdmc:/3ds/DaedalusX64/Roms");files("sdmc:/3ds/DaedalusX64/Roms","n64",21);
 assert(!symlink("f000.gba","sdmc:/GBA/link.gba"));
 DsScan result=scan(NULL,NULL);RomScanStats stats=rom_library_last_stats();
 assert(result.files==846&&visits==846&&!result.limited&&!result.cancelled&&!result.missing);
 assert(stats.metadata_entries>=846&&!stats.stat_fallbacks&&!stats.root_checks&&!statCalls&&exhaustions==opens);
 printf("PASS native actual scanner: 846 live ROMs, %u iterator metadata entries, 0 per-file stat/open lookups, %u balanced directory handles, SDK public EOF complete\n",stats.metadata_entries,opens);
 assert(!unlink("sdmc:/GBA/f000.gba"));file("sdmc:/GBA/added.gba");result=scan(NULL,NULL);
 assert(visits==846&&addedSeen&&!removedSeen&&!result.limited&&!statCalls);
 puts("PASS native live rescan: added/removed files reflected without cache or per-file stat");
 const char *failurePaths[]={"sdmc:/","sdmc:/3ds","sdmc:/3ds/DaedalusX64","sdmc:/GBA","sdmc:/3ds/DaedalusX64/Roms"};
 for(unsigned i=0;i<sizeof(failurePaths)/sizeof(failurePaths[0]);i++){
  readFailure=failurePaths[i];result=scan(NULL,NULL);assert(result.limited&&result.missing);readFailure=NULL;
  openFailure=failurePaths[i];result=scan(NULL,NULL);assert(result.limited&&result.missing);openFailure=NULL;
 }
 result=scan(NULL,proceed);assert(result.cancelled&&visits<846);
 stopAt=30;result=scan(NULL,NULL);assert(result.limited&&!result.cancelled&&visits==30);stopAt=0;
 allocationFailure=true;result=scan(NULL,NULL);assert(result.limited&&!visits&&!opens);allocationFailure=false;
 DsRoots roots={.count=1};strcpy(roots.paths[0],"sdmc:/Games/games ds");result=scan(&roots,NULL);stats=rom_library_last_stats();
 assert(visits==846&&!result.limited&&statCalls==1&&stats.root_checks==1&&!stats.stat_fallbacks);
 puts("PASS native EIO/EACCES stay incomplete, cancel/callback stop/allocation failure close all handles; explicit DS root keeps one validation stat");
 return 0;
}
