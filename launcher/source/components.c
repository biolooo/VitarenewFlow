// SPDX-License-Identifier: GPL-3.0-only
#include "components.h"
#include "app_log.h"
#include "update_ca_bin.h"
#include "ds_backend.h"
#include <3ds.h>
#include <curl/curl.h>
#include <mbedtls/sha256.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#define RUNNING_VERSION "0.12.37"
static Thread worker;
static int diagnostic_code;
int components_error_code(void){return __atomic_load_n(&diagnostic_code,__ATOMIC_ACQUIRE);}
static unsigned state,received,total,selected,base;
static bool cancelled;
static char message[128];
static bool stop_requested(void){return __atomic_load_n(&cancelled,__ATOMIC_ACQUIRE);
}
static void publish(ComponentState s,const char *text){if(s==COMP_ERROR&&!components_error_code())__atomic_store_n(&diagnostic_code,-1,__ATOMIC_RELEASE);snprintf(message,sizeof(message),"%s",text);
__atomic_store_n(&state,s,__ATOMIC_RELEASE);
}
void components_cancel(void){__atomic_store_n(&cancelled,true,__ATOMIC_RELEASE);
}
void components_stop(void){if(worker){components_cancel();
threadJoin(worker,U64_MAX);
threadFree(worker);
worker=NULL;
}}
ComponentStatus components_status(void){ComponentStatus s={0};
s.state=__atomic_load_n(&state,__ATOMIC_ACQUIRE);
s.received=__atomic_load_n(&received,__ATOMIC_RELAXED);
s.total=__atomic_load_n(&total,__ATOMIC_RELAXED);
 if(s.state!=COMP_DOWNLOAD)snprintf(s.message,sizeof(s.message),"%s",message);
return s;
}
static void suffix(char *out,const char *path,const char *suffix){snprintf(out,256,"%s%s",path,suffix);
}
static bool exists(const char *path){struct stat s;
return stat(path,&s)==0;
}
static bool parents(const char *path){char b[256];
snprintf(b,sizeof(b),"%s",path);
for(char *q=b+6;
*q;
q++)if(*q=='/'){*q=0;
 if(mkdir(b,0777)&&!exists(b))return false;
*q='/';
}return true;
}
typedef struct {FILE *file;
char *memory;
size_t limit,used;
} Sink;
static size_t receive(void *bytes,size_t a,size_t b,void *context){
 Sink *sink=context;
 if(a&&b>SIZE_MAX/a)return 0;
size_t n=a*b;
 if(stop_requested()||n>sink->limit-sink->used)return 0;
 if(sink->file){if(fwrite(bytes,1,n,sink->file)!=n)return 0;
}else memcpy(sink->memory+sink->used,bytes,n);
 sink->used+=n;
__atomic_store_n(&received,base+(unsigned)sink->used,__ATOMIC_RELAXED);
return n;
}
static int progress(void *unused,curl_off_t a,curl_off_t b,curl_off_t c,curl_off_t d){(void)unused;
(void)a;
(void)b;
(void)c;
(void)d;
return stop_requested()?1:0;
}
static int fetch(const char *url,Sink *sink){
 CURL *curl=curl_easy_init();
 if(!curl)return -1;
 struct curl_blob ca={(void*)update_ca_bin,update_ca_bin_size,CURL_BLOB_NOCOPY};
 curl_easy_setopt(curl,CURLOPT_URL,url);
 curl_easy_setopt(curl,CURLOPT_CAINFO_BLOB,&ca);
 curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);
curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,2L);
 curl_easy_setopt(curl,CURLOPT_PROTOCOLS_STR,"https");
 curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,0L);
 /* Exact publisher URL;
 redirects fail closed. */
 curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,12L);
curl_easy_setopt(curl,CURLOPT_TIMEOUT,180L);
 curl_easy_setopt(curl,CURLOPT_LOW_SPEED_LIMIT,256L);
curl_easy_setopt(curl,CURLOPT_LOW_SPEED_TIME,15L);
 curl_easy_setopt(curl,CURLOPT_NOSIGNAL,1L);
curl_easy_setopt(curl,CURLOPT_PROXY,"");
 curl_easy_setopt(curl,CURLOPT_USERAGENT,"VitarenewFlow/" RUNNING_VERSION);
 curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,receive);
curl_easy_setopt(curl,CURLOPT_WRITEDATA,sink);
 curl_easy_setopt(curl,CURLOPT_NOPROGRESS,0L);
curl_easy_setopt(curl,CURLOPT_XFERINFOFUNCTION,progress);
 CURLcode code=curl_easy_perform(curl);
long http=0;
curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&http);
curl_easy_cleanup(curl);
 char log[120];
snprintf(log,sizeof(log),"Component: curl=%d HTTP=%ld bytes=%lu",(int)code,http,(unsigned long)sink->used);
app_log(log);
 if(stop_requested())return -3;
 if(code==CURLE_PEER_FAILED_VERIFICATION)return -4;
 if(code!=CURLE_OK)return -1;
 return http==200?0:http==404?-2:-1;
}
static bool valid(const char *path,const ComponentFile *def){
 FILE *f=fopen(path,"rb");
 if(!f)return false;
mbedtls_sha256_context sha;
mbedtls_sha256_init(&sha);
 bool ok=mbedtls_sha256_starts_ret(&sha,0)==0;
unsigned char block[8192],digest[32]={0};
size_t count=0,n;
 while(ok&&(n=fread(block,1,sizeof(block),f))){count+=n;
 if(count>def->bytes||mbedtls_sha256_update_ret(&sha,block,n))ok=false;
}
 if(ferror(f)||count!=def->bytes)ok=false;
 if(fclose(f))ok=false;
 if(mbedtls_sha256_finish_ret(&sha,digest))ok=false;
 mbedtls_sha256_free(&sha);
 char hex[65];
for(unsigned i=0;i<32;i++)snprintf(hex+2*i,3,"%02x",digest[i]);
return ok&&!strcmp(hex,def->sha);
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
static void run(void *unused){
 (void)unused;
const ComponentDef *d=component_definition(selected);
void *soc=memalign(0x1000,0x100000);
bool connected=false,global=false;
int result=-1;
unsigned completed=0;
 if(!soc)goto finish;
 if(R_FAILED(socInit(soc,0x100000)))goto finish;
 connected=true;
 if(curl_global_init(CURL_GLOBAL_DEFAULT)!=CURLE_OK)goto finish;
 global=true;
 FS_ArchiveResource resource={0};
 if(R_FAILED(FSUSER_GetArchiveResource(&resource,SYSTEM_MEDIATYPE_SD))||(u64)resource.clusterSize*resource.freeClusters<(u64)total+65536){result=-6;
goto finish;
}
 for(unsigned i=0;i<d->count;i++){
  const ComponentFile *f=&d->files[i];
char stage[256];
suffix(stage,f->target,".flow-download");
  if(!parents(f->target)){result=-7;
goto finish;
}FILE *file=fopen(stage,"wb");
 if(!file){result=-7;
goto finish;
}
  base=completed;
Sink sink={.file=file,.limit=f->bytes};
result=fetch(f->url,&sink);
 if(fflush(file)||ferror(file))result=-7;
 if(fclose(file))result=-7;
  if(!result&&!valid(stage,f))result=-5;
 if(result)goto finish;
 completed+=f->bytes;
__atomic_store_n(&received,completed,__ATOMIC_RELAXED);
 }
 result=stop_requested()?-3:0;
finish:
 if(global)curl_global_cleanup();
 if(connected)socExit();
 free(soc);
 if(result){
  __atomic_store_n(&diagnostic_code,result,__ATOMIC_RELEASE);for(unsigned i=0;i<d->count;i++){char stage[256];
suffix(stage,d->files[i].target,".flow-download");
remove(stage);
}
 publish(result==-3?COMP_CANCELLED:COMP_ERROR,result==-3?"Abgebrochen. Nichts verändert.":result==-5?"Dateiprüfung fehlgeschlagen. Nichts verändert.":result==-6?"Nicht genug freier SD-Speicher.":result==-7?"SD-Schreibfehler. Nichts verändert.":result==-4?"HTTPS fehlgeschlagen. Datum prüfen.":"Download fehlgeschlagen. WLAN prüfen.");
 }else publish(COMP_READY,"Dateien geprüft. Einrichtung läuft...");
}
void components_start(unsigned index){const ComponentDef *d=component_definition(index);
 if(!d||!d->count)return;
 components_stop();
__atomic_store_n(&diagnostic_code,0,__ATOMIC_RELEASE);
selected=index;
present_cache[index]=-1;
unsigned bytes=0;
for(unsigned i=0;i<d->count;i++)bytes+=d->files[i].bytes;
__atomic_store_n(&total,bytes,__ATOMIC_RELAXED);
__atomic_store_n(&received,0,__ATOMIC_RELAXED);
__atomic_store_n(&cancelled,false,__ATOMIC_RELEASE);
publish(COMP_DOWNLOAD,"");
worker=threadCreate(run,NULL,0x18000,0x38,-2,false);
 if(!worker)publish(COMP_ERROR,"Download konnte nicht gestartet werden.");
}
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
void components_clear(void){components_stop();publish(COMP_IDLE,"");}
