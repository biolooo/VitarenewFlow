#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include "components.h"
#define R_FAILED(x) ((x)<0)
static unsigned selected=0;static int present_cache[7];static ComponentState state;static int fail_at,operations,ds_fail;static char message[128];
static ComponentFile files[4];static ComponentDef def={"Test","1",files,4};
const ComponentDef *component_definition(unsigned i){(void)i;return &def;}
static bool exists(const char *p){return access(p,F_OK)==0;}
static void suffix(char *o,const char *p,const char *s){snprintf(o,256,"%s%s",p,s);}
static unsigned parent_calls;
static bool parents(const char *p){(void)p;parent_calls++;return true;}
static bool valid(const char *p,const ComponentFile *d){(void)d;char b[4]={0};FILE *f=fopen(p,"rb");if(!f)return false;int n=fread(b,1,4,f);fclose(f);return n==3&&!strcmp(b,"NEW");}
void components_stop(void){}
ComponentStatus components_status(void){ComponentStatus s={.state=state};return s;}
static void publish(ComponentState s,const char *m){state=s;snprintf(message,sizeof(message),"%s",m);}
static int ds_install_bootstrap_file(const char *p,void *cb){(void)p;(void)cb;return ds_fail?-1:0;}
static void app_log(const char *p){(void)p;}
static int frename(const char *a,const char *b){if(++operations==fail_at)return -1;return rename(a,b);}
static int fremove(const char *a){if(++operations==fail_at)return -1;return remove(a);}
#define rename frename
#define remove fremove
bool components_commit(void){
 if(components_status().state!=COMP_READY)return false;
 components_stop();
const ComponentDef *d=component_definition(selected);
 bool backed[COMPONENT_MAX_FILES]={0},moved[COMPONENT_MAX_FILES]={0};
unsigned i;
 for(i=0;
i<d->count;
i++){char stage[256];
suffix(stage,d->files[i].target,".flow-download");
 if(!valid(stage,&d->files[i]))goto rollback;
}
 for(i=0;
i<d->count;
i++){
  char stage[256],backup[256];
const char *path=d->files[i].target;
suffix(stage,path,".flow-download");
suffix(backup,path,".flow-backup");
  if(exists(path)){if(exists(backup)&&remove(backup))goto rollback;
 if(rename(path,backup))goto rollback;
backed[i]=true;
}
  if(rename(stage,path))goto rollback;
 moved[i]=true;
 if(!valid(path,&d->files[i]))goto rollback;
 }
 if(selected==1&&R_FAILED(ds_install_bootstrap_file("/3ds/3ds-library/components/ds-bootstrap.cia",NULL)))goto rollback;
 if(selected!=6){const char *dirs[]={"sdmc:/roms/gba/placeholder","sdmc:/roms/gb/placeholder","sdmc:/roms/gbc/placeholder","sdmc:/roms/nds/placeholder","sdmc:/roms/n64/placeholder","sdmc:/roms/snes/placeholder","sdmc:/roms/nes/placeholder","sdmc:/roms/vb/placeholder","sdmc:/3ds/DaedalusX64/SaveGames/placeholder","sdmc:/3ds/DaedalusX64/SaveStates/placeholder","sdmc:/_nds/nds-bootstrap/placeholder"};
for(unsigned j=0;j<sizeof(dirs)/sizeof(*dirs);j++)if(!parents(dirs[j])){publish(COMP_ERROR,"Emulator installiert. Spieleordner manuell prüfen.");
return false;
}}
 present_cache[selected]=-1;
publish(COMP_DONE,selected==6?"Auf SD gespeichert. Am PC VitarenewFlow Desktop.exe öffnen.":"Eingerichtet. Spiele neu einlesen und starten.");
app_log("Components: verified install completed");
return true;
rollback:
 {bool restored=true;
for(unsigned j=d->count;
j>0;
j--){unsigned k=j-1;
char backup[256];
suffix(backup,d->files[k].target,".flow-backup");
 if(moved[k]&&remove(d->files[k].target))restored=false;
 if(backed[k]&&rename(backup,d->files[k].target))restored=false;
}
 publish(COMP_ERROR,restored?"Einrichtung fehlgeschlagen. Alte Dateien erhalten.":"SD-Fehler. .flow-backup auf SD wiederherstellen.");
app_log("Components: installation failed, rollback attempted");
return false;
}
}
#undef rename
#undef remove
static void writefile(const char *p,const char *s){FILE *f=fopen(p,"wb");assert(f);fputs(s,f);fclose(f);}
static void reset(bool old){system("rm -rf fixture; mkdir fixture");for(unsigned i=0;i<4;i++){static char paths[4][40];snprintf(paths[i],40,"fixture/file %u",i);files[i].target=paths[i];char s[256];suffix(s,paths[i],".flow-download");writefile(s,"NEW");if(old)writefile(paths[i],"OLD");}state=COMP_READY;operations=0;ds_fail=0;selected=0;message[0]=0;}
static void oldfiles(bool old){for(unsigned i=0;i<4;i++){if(!old){assert(!exists(files[i].target));continue;}char b[4]={0};FILE *f=fopen(files[i].target,"rb");assert(f);assert(fread(b,1,4,f)==3);fclose(f);assert(!strcmp(b,"OLD"));}}
int main(void){
 for(int old=0;old<2;old++){reset(old);fail_at=0;assert(components_commit());int max=operations;for(unsigned i=0;i<4;i++)assert(valid(files[i].target,&files[i]));for(int fault=1;fault<=max;fault++){reset(old);fail_at=fault;assert(!components_commit());assert(state==COMP_ERROR);oldfiles(old);}}
 reset(true);fail_at=0;char stage[256];suffix(stage,files[2].target,".flow-download");writefile(stage,"BAD");assert(!components_commit());assert(operations==0);oldfiles(true);
 reset(true);selected=1;ds_fail=1;assert(!components_commit());oldfiles(true);
 reset(true);state=COMP_CANCELLED;assert(!components_commit());assert(!operations);oldfiles(true);
 reset(true);selected=6;ds_fail=1;parent_calls=0;assert(components_commit());assert(state==COMP_DONE&&parent_calls==0);
 puts("PASS: actual batch commit, every rename failure (fresh/existing), corrupt staging, DS failure, cancelled-state guard");return 0;
}
