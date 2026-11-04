// SPDX-License-Identifier: GPL-3.0-only
#include "rom_library.h"
#include "library_model.h"
#include <dirent.h>
#ifdef __3DS__
#include <sys/dir.h>
#endif
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
#ifdef __3DS__
typedef DIR_ITER RomDir;
#define rom_open diropen
#define rom_close dirclose
#else
typedef DIR RomDir;
#define rom_open opendir
#define rom_close closedir
#endif
typedef struct {char name[NAME_MAX+1];mode_t mode;bool known;} RomEntry;
typedef struct {RomDir *dir;char path[DS_PATH_MAX];unsigned depth,mask;} Frame;
typedef struct {
 DsScan *result;DsVisit visit;DsContinue proceed;void *user;
 Seen seen[WALK_DIRS];unsigned count;Frame frames[WALK_DEPTH+1];unsigned active;bool stopped;RomEntry entry;RomScanStats stats;
} Walker;
static RomScanStats last_stats;
RomScanStats rom_library_last_stats(void){return last_stats;}
static bool progress(Walker *w){
 if(w->stopped||w->result->cancelled)return false;
 if(w->proceed&&!w->proceed(w->user)){w->result->cancelled=true;return false;}
 return true;
}
static bool entry_budget(Walker *w){
 if(++w->result->entries>WALK_ENTRIES){w->result->limited=true;w->stopped=true;return false;}
 return progress(w);
}
/* The SDK iterator already returns file type with each batched directory entry.
   In particular, archive_dirnext uses ENOENT to signal normal exhaustion. Never
   turn that into an incomplete scan, or reopen every returned file via stat. */
static RomEntry *next_entry(Walker *w,RomDir *dir){
 RomEntry *entry=&w->entry;errno=0;
#ifdef __3DS__
 struct stat metadata={0};
 if(dirnext(dir,entry->name,&metadata)<0){
  if(errno&&errno!=ENOENT){w->result->missing++;w->result->limited=true;}
  return NULL;
 }
 entry->name[NAME_MAX]=0;entry->mode=metadata.st_mode;entry->known=true;
#else
 struct dirent *found=readdir(dir);
 if(!found){if(errno){w->result->missing++;w->result->limited=true;}return NULL;}
 size_t length=strnlen(found->d_name,sizeof(entry->name));
 if(length==sizeof(entry->name)){w->result->missing++;w->result->limited=true;return NULL;}
 memcpy(entry->name,found->d_name,length+1);entry->known=false;entry->mode=0;
#if defined(DT_UNKNOWN) && defined(DT_REG) && defined(DT_DIR) && defined(DT_LNK)
 if(found->d_type!=DT_UNKNOWN){
  entry->known=true;
  if(found->d_type==DT_REG)entry->mode=S_IFREG;
  else if(found->d_type==DT_DIR)entry->mode=S_IFDIR;
  else if(found->d_type==DT_LNK)entry->mode=S_IFLNK;
 }
#endif
#endif
 if(entry->known)w->stats.metadata_entries++;
 return entry;
}
static mode_t entry_mode(Walker *w,const RomEntry *entry,const char *path){
 if(entry->known)return entry->mode;
 struct stat metadata;w->stats.stat_fallbacks++;
 if(lstat(path,&metadata)){w->result->missing++;w->result->limited=true;return 0;}
 return metadata.st_mode;
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
static void push(Walker *w,const char *path,unsigned depth,unsigned mask,bool checked){
 mask=mark(w,path,depth,mask);if(!mask)return;
 if(!checked){w->stats.root_checks++;struct stat st;if(lstat(path,&st)||!S_ISDIR(st.st_mode)){w->result->missing++;w->result->limited=true;return;}}
 RomDir *dir=rom_open(path);if(!dir){w->result->missing++;w->result->limited=true;return;}
 Frame *f=&w->frames[w->active++];f->dir=dir;strcpy(f->path,path);f->depth=depth;f->mask=mask;w->result->directories++;
}
static void walk(Walker *w,const char *root,unsigned mask,bool checked){
 if(!progress(w)){return;}
 push(w,root,0,mask,checked);
 while(w->active&&progress(w)){
  Frame *f=&w->frames[w->active-1];RomEntry *e=next_entry(w,f->dir);
  if(!e){rom_close(f->dir);w->active--;continue;}
  if(!strcmp(e->name,".")||!strcmp(e->name,"..")){continue;}
  if(!entry_budget(w)){break;}
  if(e->name[0]=='.'){continue;}
  char path[DS_PATH_MAX];if(!child_path(path,f->path,e->name)){w->result->limited=true;continue;}
  mode_t mode=entry_mode(w,e,path);
  if(S_ISDIR(mode)){
   if(f->depth==WALK_DEPTH)w->result->limited=true;else push(w,path,f->depth+1,f->mask,true);
  }else if(S_ISREG(mode)){
   int system=library_file_system(e->name);unsigned kind=system==SYSTEM_DS?MASK_DS:MASK_OTHER;
   if(system<0||!(f->mask&kind))continue;
   w->result->files++;if(!w->visit(path,w->user)){w->result->limited=true;w->stopped=true;}
  }
 }
 while(w->active)rom_close(w->frames[--w->active].dir);
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
 RomDir *dir=rom_open(apps);if(!dir){w->result->missing++;w->result->limited=true;return;}RomEntry *e;
 while(progress(w)&&(e=next_entry(w,dir))){
  if(!strcmp(e->name,".")||!strcmp(e->name,"..")){continue;}
  if(!entry_budget(w)){break;}
  if(e->name[0]=='.'){continue;}
  bool known=false;for(unsigned i=0;i<sizeof(names)/sizeof(names[0]);i++)if(!strcasecmp(e->name,names[i]))known=true;
  if(!known){continue;}
  char emulator[DS_PATH_MAX];
  if(!child_path(emulator,apps,e->name)){w->result->limited=true;continue;}
  if(!S_ISDIR(entry_mode(w,e,emulator)))continue;
  RomDir *sub=rom_open(emulator);if(!sub){w->result->missing++;w->result->limited=true;continue;}RomEntry *folder;
  while(progress(w)&&(folder=next_entry(w,sub))){
   if(!strcmp(folder->name,".")||!strcmp(folder->name,"..")){continue;}
  if(!entry_budget(w)){break;}
   if(folder->name[0]=='.'){continue;}
   if(strcasecmp(folder->name,"roms")&&strcasecmp(folder->name,"games"))continue;
   char root[DS_PATH_MAX];if(!child_path(root,emulator,folder->name)){w->result->limited=true;continue;}
   mode_t mode=entry_mode(w,folder,root);
   if(!mode)continue;
   if(!S_ISDIR(mode)){w->result->missing++;w->result->limited=true;continue;}
   walk(w,root,MASK_ALL,true);
  }rom_close(sub);
 }rom_close(dir);
}
void rom_library_scan(const DsRoots *extraDsRoots,DsScan *result,DsVisit visit,DsContinue proceed,void *user){
 memset(&last_stats,0,sizeof(last_stats));memset(result,0,sizeof(*result));if(!visit){result->limited=true;return;}
 Walker *w=calloc(1,sizeof(*w));if(!w){result->limited=true;return;}w->result=result;w->visit=visit;w->proceed=proceed;w->user=user;
 if(extraDsRoots)for(unsigned i=0;i<extraDsRoots->count&&i<DS_ROOT_MAX&&progress(w);i++){
  char clean[DS_PATH_MAX];if(ds_root_normalize(clean,sizeof(clean),extraDsRoots->paths[i]))walk(w,clean,MASK_DS,false);else result->limited=true;
 }
 RomDir *dir=progress(w)?rom_open("sdmc:/"):NULL;
 if(!dir){if(!result->cancelled){result->missing++;result->limited=true;}}
 else{RomEntry *e;while(progress(w)&&(e=next_entry(w,dir))){
  if(!strcmp(e->name,".")||!strcmp(e->name,"..")){continue;}
  if(!entry_budget(w)){break;}
  if(e->name[0]=='.'){continue;}
  if(!root_name(e->name)&&strcasecmp(e->name,"3ds"))continue;
  char path[DS_PATH_MAX];
  if(!child_path(path,"sdmc:/",e->name)){result->limited=true;continue;}
  if(!S_ISDIR(entry_mode(w,e,path)))continue;
  if(!strcasecmp(e->name,"3ds"))emulator_roots(w,path);else walk(w,path,MASK_ALL,true);
 }rom_close(dir);}
 last_stats=w->stats;free(w);
}
