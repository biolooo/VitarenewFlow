// SPDX-License-Identifier: GPL-3.0-only
#include "rom_library.h"
#include "library_model.h"
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#define WALK_DEPTH 8
#define WALK_DIRS 256
#define WALK_ENTRIES 12000
#define MASK_DS 1u
#define MASK_OTHER 2u
#define MASK_ALL (MASK_DS|MASK_OTHER)
typedef struct {char path[DS_PATH_MAX];unsigned depth[2];} Seen;
typedef struct {DIR *dir;char path[DS_PATH_MAX];unsigned depth,mask;} Frame;
typedef struct {
 DsScan *result;DsVisit visit;DsContinue proceed;void *user;
 Seen seen[WALK_DIRS];unsigned count;Frame frames[WALK_DEPTH+1];unsigned active;bool stopped;
} Walker;
static bool progress(Walker *w){
 if(w->stopped||w->result->cancelled)return false;
 if(w->proceed&&!w->proceed(w->user)){w->result->cancelled=true;return false;}
 return true;
}
static bool entry_budget(Walker *w){
 if(++w->result->entries>WALK_ENTRIES){w->result->limited=true;w->stopped=true;return false;}
 return progress(w);
}
static struct dirent *next_entry(Walker *w,DIR *dir){
 errno=0;struct dirent *entry=readdir(dir);
 if(!entry&&errno){w->result->missing++;w->result->limited=true;}
 return entry;
}
static bool child_path(char *out,const char *parent,const char *name){
 int n=snprintf(out,DS_PATH_MAX,"%s%s%s",parent,parent[strlen(parent)-1]=='/'?"":"/",name);
 return n>=0&&n<DS_PATH_MAX;
}
static unsigned mark(Walker *w,const char *path,unsigned depth,unsigned mask){
 unsigned i;for(i=0;i<w->count;i++)if(!strcasecmp(w->seen[i].path,path))break;
 if(i==w->count){if(i==WALK_DIRS){w->result->limited=true;w->stopped=true;return 0;}
  strcpy(w->seen[i].path,path);w->seen[i].depth[0]=w->seen[i].depth[1]=UINT_MAX;w->count++;
 }
 for(unsigned bit=0;bit<2;bit++)if(mask&(1u<<bit)){
  if(w->seen[i].depth[bit]<=depth)mask&=~(1u<<bit);else w->seen[i].depth[bit]=depth;
 }
 return mask;
}
static void push(Walker *w,const char *path,unsigned depth,unsigned mask){
 mask=mark(w,path,depth,mask);if(!mask)return;
 struct stat st;if(lstat(path,&st)||!S_ISDIR(st.st_mode)){w->result->missing++;w->result->limited=true;return;}
 DIR *dir=opendir(path);if(!dir){w->result->missing++;w->result->limited=true;return;}
 Frame *f=&w->frames[w->active++];f->dir=dir;strcpy(f->path,path);f->depth=depth;f->mask=mask;w->result->directories++;
}
static void walk(Walker *w,const char *root,unsigned mask){
 if(!progress(w))return;push(w,root,0,mask);
 while(w->active&&progress(w)){
  Frame *f=&w->frames[w->active-1];struct dirent *e=next_entry(w,f->dir);
  if(!e){closedir(f->dir);w->active--;continue;}
  if(!strcmp(e->d_name,".")||!strcmp(e->d_name,".."))continue;
  if(!entry_budget(w))break;if(e->d_name[0]=='.')continue;
  char path[DS_PATH_MAX];if(!child_path(path,f->path,e->d_name)){w->result->limited=true;continue;}
  struct stat st;if(lstat(path,&st)){w->result->missing++;w->result->limited=true;continue;}
  if(S_ISDIR(st.st_mode)){
   if(f->depth==WALK_DEPTH)w->result->limited=true;else push(w,path,f->depth+1,f->mask);
  }else if(S_ISREG(st.st_mode)){
   int system=library_file_system(e->d_name);unsigned kind=system==SYSTEM_DS?MASK_DS:MASK_OTHER;
   if(system<0||!(f->mask&kind))continue;
   w->result->files++;if(!w->visit(path,w->user)){w->result->limited=true;w->stopped=true;}
  }
 }
 while(w->active)closedir(w->frames[--w->active].dir);
}
static bool root_name(const char *name){
 static const char *names[]={"roms","games","nds","ds","gba","gb","gbc","gameboy","game boy","game boy advance","gameboy advance","game boy color","n64","nintendo 64","nes","snes","super nintendo","vb","virtual boy","virtualboy"};
 for(unsigned i=0;i<sizeof(names)/sizeof(names[0]);i++)if(!strcasecmp(name,names[i]))return true;
 return false;
}
/* Resolve just known emulator ROM subfolders. Never recurse through /3ds,
   /_nds, /Nintendo 3DS or other top-level application/runtime trees. */
static void emulator_roots(Walker *w,const char *apps){
 static const char *names[]={"DaedalusX64","mGBA","snes9x_3ds","virtuanes_3ds","red-viper"};
 DIR *dir=opendir(apps);if(!dir){w->result->missing++;w->result->limited=true;return;}struct dirent *e;
 while(progress(w)&&(e=next_entry(w,dir))){
  if(!strcmp(e->d_name,".")||!strcmp(e->d_name,".."))continue;if(!entry_budget(w))break;if(e->d_name[0]=='.')continue;
  bool known=false;for(unsigned i=0;i<sizeof(names)/sizeof(names[0]);i++)if(!strcasecmp(e->d_name,names[i]))known=true;
  if(!known)continue;char emulator[DS_PATH_MAX];struct stat st;
  if(!child_path(emulator,apps,e->d_name)){w->result->limited=true;continue;}
  if(lstat(emulator,&st)){w->result->missing++;w->result->limited=true;continue;}if(!S_ISDIR(st.st_mode))continue;
  DIR *sub=opendir(emulator);if(!sub){w->result->missing++;w->result->limited=true;continue;}struct dirent *folder;
  while(progress(w)&&(folder=next_entry(w,sub))){
   if(!strcmp(folder->d_name,".")||!strcmp(folder->d_name,".."))continue;if(!entry_budget(w))break;if(folder->d_name[0]=='.')continue;
   if(strcasecmp(folder->d_name,"roms")&&strcasecmp(folder->d_name,"games"))continue;
   char root[DS_PATH_MAX];if(!child_path(root,emulator,folder->d_name)){w->result->limited=true;continue;}
   walk(w,root,MASK_ALL);
  }closedir(sub);
 }closedir(dir);
}
void rom_library_scan(const DsRoots *extraDsRoots,DsScan *result,DsVisit visit,DsContinue proceed,void *user){
 memset(result,0,sizeof(*result));if(!visit){result->limited=true;return;}
 Walker *w=calloc(1,sizeof(*w));if(!w){result->limited=true;return;}w->result=result;w->visit=visit;w->proceed=proceed;w->user=user;
 if(extraDsRoots)for(unsigned i=0;i<extraDsRoots->count&&i<DS_ROOT_MAX&&progress(w);i++){
  char clean[DS_PATH_MAX];if(ds_root_normalize(clean,sizeof(clean),extraDsRoots->paths[i]))walk(w,clean,MASK_DS);else result->limited=true;
 }
 DIR *dir=progress(w)?opendir("sdmc:/"):NULL;
 if(!dir){if(!result->cancelled){result->missing++;result->limited=true;}}
 else{struct dirent *e;while(progress(w)&&(e=next_entry(w,dir))){
  if(!strcmp(e->d_name,".")||!strcmp(e->d_name,".."))continue;if(!entry_budget(w))break;if(e->d_name[0]=='.')continue;
  if(!root_name(e->d_name)&&strcasecmp(e->d_name,"3ds"))continue;
  char path[DS_PATH_MAX];struct stat st;
  if(!child_path(path,"sdmc:/",e->d_name)){result->limited=true;continue;}
  if(lstat(path,&st)){result->missing++;result->limited=true;continue;}if(!S_ISDIR(st.st_mode))continue;
  if(!strcasecmp(e->d_name,"3ds"))emulator_roots(w,path);else walk(w,path,MASK_ALL);
 }closedir(dir);}
 free(w);
}
