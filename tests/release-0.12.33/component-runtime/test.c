
#define _POSIX_C_SOURCE 200809L
#include "components.h"
#include "ds_backend.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
static bool ioFailure;
static int probe_stat(const char *p,struct stat *s){if(ioFailure&&strstr(p,".exe")){errno=EIO;return -1;}return stat(p,s);}
#define stat(p,s) probe_stat(p,s)
static bool titleGood=true,wrongTitle,listContains;
static Result titleRC=-1,countRC,listRC;static u32 titleCount=2,readOverride;
static unsigned titleQueries,listQueries,logs,profiles;static ComponentState testState;
Result AM_GetTitleInfo(FS_MediaType m,u32 count,u64 *id,AM_TitleInfo *out){assert(m==MEDIATYPE_NAND&&count==1&&*id==DS_BOOTSTRAP_ID);titleQueries++;out->titleID=wrongTitle?7:*id;return titleGood?0:titleRC;}
Result AM_GetTitleCount(FS_MediaType m,u32 *count){assert(m==MEDIATYPE_NAND);*count=titleCount;return countRC;}
Result AM_GetTitleList(u32 *read,FS_MediaType m,u32 count,u64 *ids){assert(m==MEDIATYPE_NAND&&count==titleCount);listQueries++;for(u32 i=0;i<count;i++)ids[i]=listContains&&i==0?DS_BOOTSTRAP_ID:i;*read=readOverride?readOverride:count;return listRC;}
static bool ds_loader_path(char *out,size_t capacity,const char *rom){return snprintf(out,capacity,"%s",rom)>0;}
static Result ds_prepare_profile(const char *rom){assert(rom);profiles++;return 0;}
static bool exists(const char *path){struct stat s;return !stat(path,&s);}
static void app_log(const char *s){assert(s);logs++;}
ComponentStatus components_status(void){ComponentStatus s={.state=testState};return s;}
DsRuntimeState ds_runtime_check(void){
    u64 id=DS_BOOTSTRAP_ID;AM_TitleInfo title={0};
    Result r=AM_GetTitleInfo(MEDIATYPE_NAND,1,&id,&title);
    if(R_FAILED(r)||title.titleID!=id){
        /* An AM service error must not be reported as an absent title. Only
           a complete, successful title-list query proves that it is missing. */
        u32 count=0,read=0;
        if(R_FAILED(AM_GetTitleCount(MEDIATYPE_NAND,&count))||count>8192)return DS_RUNTIME_UNAVAILABLE;
        if(!count)return DS_RUNTIME_NO_TITLE;
        u64 *ids=malloc((size_t)count*sizeof(*ids));if(!ids)return DS_RUNTIME_UNAVAILABLE;
        r=AM_GetTitleList(&read,MEDIATYPE_NAND,count,ids);bool found=false;
        if(R_SUCCEEDED(r)&&read==count)for(u32 i=0;i<read;i++)if(ids[i]==id)found=true;
        free(ids);
        if(R_FAILED(r)||read!=count||found)return DS_RUNTIME_UNAVAILABLE;
        return DS_RUNTIME_NO_TITLE;
    }
    struct stat file;
    if(stat("sdmc:/_nds/ntr-forwarder/sdcard.nds",&file)||!S_ISREG(file.st_mode)||file.st_size<=0)return DS_RUNTIME_NO_FORWARDER;
    if(stat("sdmc:/_nds/nds-bootstrap-release.nds",&file)||!S_ISREG(file.st_mode)||file.st_size<=0)return DS_RUNTIME_NO_BOOTSTRAP;
    return DS_RUNTIME_READY;
}
Result ds_prepare_launch(const char *rom) {
    DsRuntimeState runtime=ds_runtime_check();
    if(runtime==DS_RUNTIME_NO_TITLE||runtime==DS_RUNTIME_UNAVAILABLE)return (Result)-30;
    if(runtime!=DS_RUNTIME_READY)return (Result)-32;
    struct stat info;
    if (stat(rom,&info) || !S_ISREG(info.st_mode) || info.st_size<0x200) return (Result)-31;
    char path[256];
    if (!ds_loader_path(path,sizeof(path),rom)) return (Result)-33;
    Result result=ds_prepare_profile(rom);
    if(R_FAILED(result))return result;
    /* Same handshake as YANBF's official 3DS-mode forwarder. Saves are untouched. */
    const char *target="sdmc:/_nds/ntr-forwarder/path.txt";
    FILE *file=fopen(target,"wb");
    if (!file) return (Result)-34;
    size_t size=strlen(path);
    bool written=fwrite(path,1,size,file)==size;
    int closed=fclose(file);
    if (!written || closed) return (Result)-34;
    file=fopen(target,"rb");
    char verify[256]={0};
    if (!file) return (Result)-34;
    size_t read=fread(verify,1,sizeof(verify)-1,file); fclose(file);
    if (read!=size || memcmp(path,verify,size)) return (Result)-34;
    return 0;
}
static bool old_probe_present(unsigned index){const ComponentDef *d=component_definition(index);
 if(!d||!d->count)return false;
for(unsigned i=0;i<d->count;i++)if(!exists(d->files[i].target))return false;
 if(index==1){AM_TitleInfo info;
u64 id=DS_BOOTSTRAP_ID;
return R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_NAND,1,&id,&info));
}return true;
}
static const char *presence_detail[COMPONENT_COUNT];
static ComponentAvailability probe_present(unsigned index){const ComponentDef *d=component_definition(index);
 if(!d||!d->count)return COMP_MISSING;
 if(index==1){
  DsRuntimeState ds=ds_runtime_check();
  static const char *details[]={"DS-Starttitel und beide Loader vorhanden.","DS-Starttitel ist nicht installiert.","DS-Forwarder fehlt oder ist leer.","nds-bootstrap fehlt oder ist leer.","DS-Starttitel konnte nicht geprüft werden."};
  presence_detail[index]=details[ds];
  return ds==DS_RUNTIME_READY?COMP_AVAILABLE:ds==DS_RUNTIME_UNAVAILABLE?COMP_UNCHECKED:COMP_MISSING;
 }
 if(index==6){
  /* Only the executable determines presence; sidecars are repaired by the
     verified installer. Keep the documented pre-rename executable supported. */
  const char *paths[]={d->files[0].target,"sdmc:/3DS Library PC.exe"};
  bool unknown=false;
  for(unsigned i=0;i<2;i++){
   struct stat file;
   if(!stat(paths[i],&file)){
    if(S_ISREG(file.st_mode)&&file.st_size>0){
     presence_detail[index]=i?"Bisherige PC-App erkannt. Reparieren ergänzt die aktuelle Version.":"Windows-App auf SD vorhanden. Reparieren lädt das vollständige Paket neu.";
     return COMP_AVAILABLE;
    }
   }else if(errno!=ENOENT&&errno!=ENOTDIR)unknown=true;
  }
  presence_detail[index]=unknown?"SD konnte nicht geprüft werden. Erneut prüfen.":"Keine Windows-App auf der SD gefunden.";
  return unknown?COMP_UNCHECKED:COMP_MISSING;
 }
 struct stat file;bool ready=!stat(d->files[0].target,&file)&&S_ISREG(file.st_mode)&&file.st_size>0;
 presence_detail[index]=ready?"Flow-Startdatei vorhanden.":"Flow-Startdatei fehlt oder ist leer.";
 return ready?COMP_AVAILABLE:COMP_MISSING;
}
static int present_cache[COMPONENT_COUNT]={-1,-1,-1,-1,-1,-1,-1};
ComponentAvailability components_availability(unsigned index){if(index>=COMPONENT_COUNT)return COMP_MISSING;if(present_cache[index]<0)present_cache[index]=probe_present(index);return (ComponentAvailability)present_cache[index];}
bool components_present(unsigned index){return components_availability(index)==COMP_AVAILABLE;}
const char *components_presence_detail(unsigned index){
 if(index>=COMPONENT_COUNT)return "";
 components_availability(index);return presence_detail[index]?presence_detail[index]:"";
}
bool components_recheck(void){
 ComponentState s=components_status().state;if(s==COMP_DOWNLOAD||s==COMP_READY)return false;
 for(unsigned i=0;i<COMPONENT_COUNT;i++){
  present_cache[i]=-1;ComponentAvailability found=components_availability(i);
  char log[180];snprintf(log,sizeof(log),"Runtime check %s: %u; %s",component_definition(i)->name,(unsigned)found,components_presence_detail(i));app_log(log);
 }
 return true;
}

#undef stat
static void makefile(const char *path,unsigned bytes){
 char b[256];snprintf(b,sizeof(b),"%s",path);for(char *q=b+1;*q;q++)if(*q=='/'){*q=0;mkdir(b,0700);*q='/';}
 FILE *f=fopen(path,"wb");assert(f);for(unsigned i=0;i<bytes;i++)assert(fputc(0x42,f)!=EOF);assert(!fclose(f));
}
static void runtimes(void){for(unsigned i=0;i<6;i++){const ComponentDef *d=component_definition(i);makefile(d->files[0].target,16);if(i==1)makefile(d->files[1].target,16);}}
int main(void){
 char temp[]="/tmp/flow-runtime-XXXXXX";assert(mkdtemp(temp)&&!chdir(temp));runtimes();
 assert(ds_runtime_check()==DS_RUNTIME_READY);assert(!old_probe_present(1));assert(!old_probe_present(0));
 assert(components_recheck());for(unsigned i=0;i<6;i++)assert(components_present(i));
 assert(strstr(components_presence_detail(1),"beide Loader"));assert(access("sdmc:/3ds/3ds-library/components/ds-bootstrap.cia",F_OK));
 unsigned queries=titleQueries;for(unsigned n=0;n<1000;n++)assert(components_present(1));assert(titleQueries==queries); // no NAND IPC per frame
 const char *forwarder=component_definition(1)->files[0].target,*bootstrap=component_definition(1)->files[1].target;
 assert(!unlink(forwarder));assert(components_present(1));assert(components_recheck());assert(!components_present(1)&&strstr(components_presence_detail(1),"Forwarder"));
 makefile(forwarder,16);assert(!unlink(bootstrap));assert(components_recheck());assert(!components_present(1)&&strstr(components_presence_detail(1),"nds-bootstrap"));
 makefile(bootstrap,0);assert(ds_runtime_check()==DS_RUNTIME_NO_BOOTSTRAP);assert(!unlink(bootstrap));assert(!mkdir(bootstrap,0700));assert(ds_runtime_check()==DS_RUNTIME_NO_BOOTSTRAP);assert(!rmdir(bootstrap));makefile(bootstrap,16);
 titleGood=false;assert(components_recheck());assert(components_availability(1)==COMP_MISSING&&strstr(components_presence_detail(1),"nicht installiert"));
 listContains=true;assert(components_recheck());assert(components_availability(1)==COMP_UNCHECKED&&!components_present(1));
 listContains=false;countRC=-1;assert(ds_runtime_check()==DS_RUNTIME_UNAVAILABLE);countRC=0;listRC=-1;assert(ds_runtime_check()==DS_RUNTIME_UNAVAILABLE);listRC=0;
 titleCount=0;assert(ds_runtime_check()==DS_RUNTIME_NO_TITLE);titleCount=8193;assert(ds_runtime_check()==DS_RUNTIME_UNAVAILABLE);titleCount=2;readOverride=1;assert(ds_runtime_check()==DS_RUNTIME_UNAVAILABLE);readOverride=0;
 titleGood=true;wrongTitle=true;listContains=true;assert(ds_runtime_check()==DS_RUNTIME_UNAVAILABLE);wrongTitle=false;listContains=false;
 assert(components_recheck()&&components_present(1));
 const char *gba=component_definition(0)->files[0].target;assert(!unlink(gba));makefile(gba,0);assert(components_recheck()&&!components_present(0));
 assert(!unlink(gba));assert(!mkdir(gba,0700));assert(components_recheck()&&!components_present(0));assert(!rmdir(gba));makefile(gba,16);assert(components_recheck()&&components_present(0));
 testState=COMP_DOWNLOAD;queries=titleQueries;assert(!components_recheck()&&queries==titleQueries);testState=COMP_READY;assert(!components_recheck());testState=COMP_IDLE;
 assert(!components_present(999)&&!strcmp(components_presence_detail(999),""));
 assert(!profiles&&access("sdmc:/_nds/ntr-forwarder/path.txt",F_OK)); // all probes read-only
 makefile("sdmc:/roms/nds/test.nds",512);assert(!ds_prepare_launch("sdmc:/roms/nds/test.nds")&&profiles==1);
 assert(!unlink(forwarder));assert(ds_prepare_launch("sdmc:/roms/nds/test.nds")==-32&&profiles==1);makefile(forwarder,16);
 titleGood=false;assert(ds_prepare_launch("sdmc:/roms/nds/test.nds")==-30&&profiles==1);
 const char *desktop=component_definition(6)->files[0].target,*legacy="sdmc:/3DS Library PC.exe";
 assert(components_recheck()&&!components_present(6));
 makefile(desktop,16);assert(!components_present(6));assert(components_recheck()&&components_present(6));
 assert(strstr(components_presence_detail(6),"vollständige Paket"));
 for(unsigned i=1;i<component_definition(6)->count;i++)assert(access(component_definition(6)->files[i].target,F_OK));
 assert(!unlink(desktop));makefile(legacy,16);assert(components_recheck()&&components_present(6));assert(strstr(components_presence_detail(6),"Bisherige"));
 makefile(desktop,0);assert(components_recheck()&&components_present(6));assert(!unlink(legacy));assert(components_recheck()&&!components_present(6));
 assert(!unlink(desktop));assert(!mkdir(desktop,0700));assert(components_recheck()&&!components_present(6));assert(!rmdir(desktop));
 makefile(desktop,32);ioFailure=true;assert(components_recheck()&&components_availability(6)==COMP_UNCHECKED);ioFailure=false;assert(components_recheck()&&components_present(6));
 queries=titleQueries;for(unsigned i=0;i<1000;i++)assert(components_present(6));assert(queries==titleQueries);
 testState=COMP_DOWNLOAD;assert(!unlink(desktop));assert(!components_recheck()&&components_present(6));testState=COMP_IDLE;assert(components_recheck()&&!components_present(6));
 assert(logs&&listQueries);puts("PASS actual DS runtime and component probes: old false-missing reproduced, no cached CIA/sidecars needed, cache/refresh, precise missing loader/title, unknown AM errors, bounds, empty/directory files, busy guard, read-only checks and shared launch preflight");
}
