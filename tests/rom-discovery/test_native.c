// SPDX-License-Identifier: GPL-3.0-only
#define _POSIX_C_SOURCE 200809L
#include "rom_library.h"
#include "library_model.h"
#include <sys/dir.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
struct DIR_ITER {DIR *host;char path[DS_PATH_MAX];};
static unsigned active,opens,closes,statCalls,fileStatCalls,visits,progressCalls,stopAt;
static bool allocationFailure,addedSeen,removedSeen;
static const char *readFailure,*openFailure;
DIR_ITER *diropen(const char *path){
 if(openFailure&&!strcmp(path,openFailure)){errno=EACCES;return NULL;}
 DIR *host=opendir(path);if(!host)return NULL;
 DIR_ITER *dir=malloc(sizeof(*dir));assert(dir);dir->host=host;assert(strlen(path)<sizeof(dir->path));strcpy(dir->path,path);active++;opens++;return dir;
}
int dirnext(DIR_ITER *dir,char *name,struct stat *metadata){
 if(readFailure&&!strcmp(dir->path,readFailure)){errno=EIO;return -1;}
 struct dirent *entry=readdir(dir->host);if(!entry){errno=ENOENT;return -1;}
 assert(strlen(entry->d_name)<=NAME_MAX);strcpy(name,entry->d_name);
 /* SDK fills only st_mode. Poison all other fields to catch accidental use. */
 memset(metadata,0xa5,sizeof(*metadata));
 switch(entry->d_type){
 case DT_REG:metadata->st_mode=S_IFREG;break;
 case DT_DIR:metadata->st_mode=S_IFDIR;break;
 case DT_LNK:metadata->st_mode=S_IFLNK;break;
 default:assert(!"synthetic filesystem must provide directory type");
 }
 return 0;
}
int dirclose(DIR_ITER *dir){int rc=closedir(dir->host);free(dir);assert(active);active--;closes++;return rc;}
int rom_native_lstat(const char *path,struct stat *metadata){statCalls++;if(library_file_system(path)>=0)fileStatCalls++;return lstat(path,metadata);}
void *rom_native_calloc(size_t count,size_t size){return allocationFailure?NULL:calloc(count,size);}
static void directory(const char *path){assert(!mkdir(path,0700));}
static void file(const char *path){FILE *f=fopen(path,"wb");assert(f);assert(!fclose(f));}
static void files(const char *root,const char *extension,unsigned count){for(unsigned i=0;i<count;i++){char path[DS_PATH_MAX];int n=snprintf(path,sizeof(path),"%s/f%03u.%s",root,i,extension);assert(n>0&&n<(int)sizeof(path));file(path);}}
static bool visit(const char *path,void *unused){(void)unused;visits++;if(strstr(path,"added.gba"))addedSeen=true;if(strstr(path,"f000.gba"))removedSeen=true;return !stopAt||visits<stopAt;}
static bool proceed(void *unused){(void)unused;return ++progressCalls<20;}
static DsScan scan(const DsRoots *roots,DsContinue progress){
 DsScan result;assert(!active);opens=closes=statCalls=fileStatCalls=visits=progressCalls=0;addedSeen=removedSeen=false;
 rom_library_scan(roots,&result,visit,progress,NULL);assert(!active&&opens==closes);assert(!fileStatCalls);return result;
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
 assert(stats.metadata_entries>=846&&!stats.stat_fallbacks&&!stats.root_checks&&!statCalls);
 printf("PASS native actual scanner: 846 live ROMs, %u iterator metadata entries, 0 per-file stat/open lookups, %u balanced directory handles, SDK ENOENT exhaustion complete\n",stats.metadata_entries,opens);
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
