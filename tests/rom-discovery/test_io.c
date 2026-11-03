// SPDX-License-Identifier: GPL-3.0-only
#define _POSIX_C_SOURCE 200809L
#include "rom_library.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static const char *failReadPath,*failOpenPath,*failStatPath;static DIR *failReadDir;
static bool failAllocation;static unsigned count;
DIR *rom_test_opendir(const char *path){
 if(failOpenPath&&!strcmp(path,failOpenPath)){errno=EACCES;return NULL;}
 DIR *dir=opendir(path);if(failReadPath&&!strcmp(path,failReadPath))failReadDir=dir;return dir;
}
int rom_test_lstat(const char *path,struct stat *st){if(failStatPath&&!strcmp(path,failStatPath)){errno=EIO;return -1;}return lstat(path,st);}
struct dirent *rom_test_readdir(DIR *dir){if(dir&&dir==failReadDir){failReadDir=NULL;errno=EIO;return NULL;}return readdir(dir);}
void *rom_test_calloc(size_t count,size_t size){return failAllocation?NULL:calloc(count,size);}
static void directory(const char *p){assert(!mkdir(p,0700));}
static void file(const char *p){FILE *f=fopen(p,"wb");assert(f);assert(!fclose(f));}
static bool visit(const char *path,void *unused){(void)unused;assert(path);count++;return true;}
static DsScan scan(void){DsScan s;count=0;failReadDir=NULL;rom_library_scan(NULL,&s,visit,NULL,NULL);return s;}
int main(void){
 char tmp[]="/tmp/flow-romio-XXXXXX";assert(mkdtemp(tmp));assert(!chdir(tmp));directory("sdmc:");directory("sdmc:/GBA");file("sdmc:/GBA/game.gba");directory("sdmc:/3ds");directory("sdmc:/3ds/DaedalusX64");directory("sdmc:/3ds/DaedalusX64/Roms");file("sdmc:/3ds/DaedalusX64/Roms/game.n64");
 DsScan s=scan();assert(count==2&&!s.limited);
 const char *paths[]={"sdmc:/","sdmc:/3ds","sdmc:/3ds/DaedalusX64","sdmc:/GBA","sdmc:/3ds/DaedalusX64/Roms"};
 for(unsigned i=0;i<sizeof(paths)/sizeof(paths[0]);i++){failReadPath=paths[i];s=scan();assert(s.limited&&s.missing>=1);}
 failReadPath=NULL;
 const char *opens[]={"sdmc:/","sdmc:/3ds","sdmc:/3ds/DaedalusX64","sdmc:/GBA","sdmc:/3ds/DaedalusX64/Roms"};
 for(unsigned i=0;i<sizeof(opens)/sizeof(opens[0]);i++){failOpenPath=opens[i];s=scan();assert(s.limited&&s.missing>=1);}
 failOpenPath=NULL;
 const char *stats[]={"sdmc:/GBA","sdmc:/GBA/game.gba","sdmc:/3ds","sdmc:/3ds/DaedalusX64","sdmc:/3ds/DaedalusX64/Roms"};
 for(unsigned i=0;i<sizeof(stats)/sizeof(stats[0]);i++){failStatPath=stats[i];s=scan();assert(s.limited&&s.missing>=1);}
 failStatPath=NULL;failAllocation=true;s=scan();assert(s.limited&&count==0);failAllocation=false;
 DsRoots invalid={.count=1};strcpy(invalid.paths[0],"sdmc:/../escape");rom_library_scan(&invalid,&s,visit,NULL,NULL);assert(s.limited);
 rom_library_scan(NULL,&s,NULL,NULL,NULL);assert(s.limited);
 puts("PASS: actual scanner allocationfailure, root/emulator/game-directory readdir EIO and opendir EACCES/lstat EIO, invalid roots, all explicitly incomplete");return 0;
}
