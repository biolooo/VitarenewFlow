// SPDX-License-Identifier: GPL-3.0-only
#define _POSIX_C_SOURCE 200809L
#include "rom_library.h"
#include "library_model.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#define FOUND_MAX 1700
static char found[FOUND_MAX][DS_PATH_MAX];
static unsigned count,stopAfter,continued,capacity=FOUND_MAX;
static void directory(const char *path){
 char tmp[DS_PATH_MAX];assert(strlen(path)<sizeof(tmp));strcpy(tmp,path);
 for(char *p=tmp+1;*p;p++)if(*p=='/'){*p=0;mkdir(tmp,0700);*p='/';}
 assert(!mkdir(tmp,0700)||!access(tmp,F_OK));
}
static void file(const char *path){char tmp[DS_PATH_MAX];assert(strlen(path)<sizeof(tmp));strcpy(tmp,path);char *slash=strrchr(tmp,'/');if(slash){*slash=0;directory(tmp);}FILE *f=fopen(path,"wb");assert(f);assert(!fclose(f));}
static bool visit(const char *path,void *unused){
 (void)unused;assert(strlen(path)<DS_PATH_MAX);if(count==capacity)return false;
 for(unsigned i=0;i<count;i++)if(!strcasecmp(found[i],path))return true;
 assert(count<FOUND_MAX);strcpy(found[count++],path);return true;
}
static bool proceed(void *unused){(void)unused;return !stopAfter||++continued<stopAfter;}
static bool has(const char *path){for(unsigned i=0;i<count;i++)if(!strcmp(path,found[i]))return true;return false;}
static DsScan scan(const DsRoots *roots){DsScan s;count=continued=0;rom_library_scan(roots,&s,visit,proceed,NULL);return s;}
static void fresh(void){char tmp[]="/tmp/flow-romscan-XXXXXX";assert(mkdtemp(tmp));assert(!chdir(tmp));directory("sdmc:");count=stopAfter=continued=0;capacity=FOUND_MAX;}
static void layout(void){
 fresh();file("sdmc:/GBA/a.GBA");file("sdmc:/NES/nested/b.NES");file("sdmc:/SNES/deep/c.sfc");file("sdmc:/Virtual Boy/v.VB");
 file("sdmc:/Games/games ds/d.NDS");file("sdmc:/rOmS/gb/region/x.GBC");file("sdmc:/rOmS/nds/one/dup.nds");file("sdmc:/rOmS/nds/two/dup.nds");
 file("sdmc:/3DS/dAeDaLuSx64/rOmS/z.N64");file("sdmc:/3DS/mGBA/games/f.gba");file("sdmc:/3DS/mGBA/application.gba");
 file("sdmc:/_nds/TWiLightMenu/emulators/system.nds");file("sdmc:/Nintendo 3DS/secret/game.gba");file("sdmc:/luma/backups/private.gba");
 file("sdmc:/rOmS/hidden.zip");file("sdmc:/rOmS/save.sav");file("sdmc:/rOmS/a.bin");file("sdmc:/rOmS/._duplicate.gba");file("sdmc:/rOmS/.hidden/h.gba");
 assert(!symlink("../GBA","sdmc:/rOmS/link"));assert(!symlink("GBA","sdmc:/GB"));
 DsScan s=scan(NULL);assert(count==10);assert(s.files==10&&!s.limited&&!s.cancelled);assert(has("sdmc:/Games/games ds/d.NDS"));assert(has("sdmc:/NES/nested/b.NES"));
 assert(has("sdmc:/rOmS/nds/one/dup.nds")&&has("sdmc:/rOmS/nds/two/dup.nds"));assert(has("sdmc:/3DS/dAeDaLuSx64/rOmS/z.N64"));
 printf("PASS: positive roots, nested formats, case, distinct same-name games, no system/archive/hidden/symlink traversal (%u games, %u directories)\n",count,s.directories);
}
static void custom(void){
 fresh();file("sdmc:/custom/deep/extra.nds");file("sdmc:/custom/notDS.gba");file("sdmc:/Games/games ds/d.nds");file("sdmc:/Games/games ds/g.gba");
 DsRoots roots={0};assert(ds_roots_add(&roots,"sdmc:/custom"));assert(ds_roots_add(&roots,"sdmc:/Games/games ds"));DsScan s=scan(&roots);
 assert(count==3&&!s.limited&&!s.cancelled);assert(!has("sdmc:/custom/notDS.gba"));assert(has("sdmc:/Games/games ds/g.gba"));
 assert(!strcmp(found[0],"sdmc:/custom/deep/extra.nds"));
 /* Default-only games/roms tree is visited once, not another DS pass. */
 s=scan(NULL);assert(count==2&&s.files==2&&s.directories==2);
 /* Use one spelling: FAT cannot contain separate Games and games roots. */
 char path[DS_PATH_MAX]="sdmc:/Games";directory(path);for(unsigned i=0;i<9;i++){strcat(path,"/deep");directory(path);}char game[DS_PATH_MAX];int n=snprintf(game,sizeof(game),"%s/x.nds",path);assert(n>0&&n<(int)sizeof(game));file(game);
 s=scan(NULL);assert(s.limited&&!has(game));
 memset(&roots,0,sizeof(roots));assert(ds_roots_add(&roots,path));s=scan(&roots);assert(has(game));/* Other formats below the automatic depth bound remain explicitly incomplete. */assert(s.limited);
 puts("PASS: DS-only extra roots retain priority, mixed overlap is complete, one automatic scan, deep explicit roots retain full reach");
}
static void limits(void){
 fresh();for(unsigned i=0;i<1536;i++){char path[80];snprintf(path,sizeof(path),"sdmc:/NES/g%04u.nes",i);file(path);}DsScan s=scan(NULL);assert(count==1536&&!s.limited);
 file("sdmc:/NES/overflow.nes");capacity=1536;s=scan(NULL);assert(count==1536&&s.limited);
 capacity=FOUND_MAX;stopAfter=20;s=scan(NULL);assert(s.cancelled&&count<1536);
 fresh();for(unsigned i=0;i<260;i++){char path[80];snprintf(path,sizeof(path),"sdmc:/roms/d%03u",i);directory(path);}s=scan(NULL);assert(s.limited&&s.directories<=256);
 fresh();for(unsigned i=0;i<12010;i++){char path[80];snprintf(path,sizeof(path),"sdmc:/roms/f%05u.txt",i);file(path);}s=scan(NULL);assert(s.limited&&s.entries==12001);
 fresh();char path[DS_PATH_MAX]="sdmc:/roms";directory(path);char piece[201];memset(piece,'x',200);piece[200]=0;for(unsigned i=0;i<2;i++){strcat(path,"/");strcat(path,piece);directory(path);}char huge[256];memset(huge,'g',240);strcpy(huge+240,".gba");char actual[800];snprintf(actual,sizeof(actual),"%s/%s",path,huge);FILE *f=fopen(actual,"wb");assert(f);fclose(f);s=scan(NULL);assert(s.limited&&count==0);
 fresh();DsRoots missing={0};assert(ds_roots_add(&missing,"sdmc:/missing"));s=scan(&missing);assert(s.missing==1&&s.limited);
 puts("PASS:1536 entries, explicit overflow/cancel/depth/directory/entry/path/missing limits");
}
static void irrelevant(void){
 fresh();file("sdmc:/GBA/visible.gba");for(unsigned i=0;i<600;i++){char path[120];snprintf(path,sizeof(path),"sdmc:/Nintendo 3DS/nested/%u/irrelevant.gba",i);file(path);snprintf(path,sizeof(path),"sdmc:/_nds/runtime/%u/system.nds",i);file(path);}
 DsScan s=scan(NULL);assert(count==1&&s.directories==1&&s.entries<12&&!s.limited);printf("PASS:1200 irrelevant system files not traversed (entries=%u)\n",s.entries);
}
int main(void){layout();custom();limits();irrelevant();puts("PASS: ROM discovery uses only synthetic file names and never opens game contents");return 0;}
