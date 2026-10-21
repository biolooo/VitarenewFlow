// SPDX-License-Identifier: GPL-3.0-only
#include "app_update.h"
#include "update_store.h"
#include "app_log.h"
#include "update_ca_bin.h"
#include <3ds.h>
#include <curl/curl.h>
#include <mbedtls/sha256.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#define RUNNING_VERSION "0.12.34"
static Thread worker;
static unsigned state,received,total;
static bool cancelled;
static UpdateManifest manifest;
static char message[128];
static bool stop_requested(void){return __atomic_load_n(&cancelled,__ATOMIC_ACQUIRE);}
static void publish(UpdateState s,const char *text){snprintf(message,sizeof(message),"%s",text);__atomic_store_n(&state,s,__ATOMIC_RELEASE);}
void app_update_cancel(void){__atomic_store_n(&cancelled,true,__ATOMIC_RELEASE);}
void app_update_stop(void){if(worker){app_update_cancel();threadJoin(worker,U64_MAX);threadFree(worker);worker=NULL;}}
UpdateStatus app_update_status(void){
 UpdateStatus s={0};s.state=__atomic_load_n(&state,__ATOMIC_ACQUIRE);s.received=__atomic_load_n(&received,__ATOMIC_RELAXED);s.total=__atomic_load_n(&total,__ATOMIC_RELAXED);
 if(s.state!=UPDATE_CHECKING&&s.state!=UPDATE_DOWNLOADING){snprintf(s.message,sizeof(s.message),"%s",message);snprintf(s.available,sizeof(s.available),"%s",manifest.version);}return s;
}
typedef struct {FILE *file;char *memory;size_t limit,used;} Sink;
static size_t receive(void *bytes,size_t a,size_t b,void *context){
 Sink *sink=context;if(a&&b>SIZE_MAX/a)return 0;size_t n=a*b;
 if(stop_requested()||n>sink->limit-sink->used)return 0;
 if(sink->file){if(fwrite(bytes,1,n,sink->file)!=n)return 0;}else memcpy(sink->memory+sink->used,bytes,n);
 sink->used+=n;__atomic_store_n(&received,(unsigned)sink->used,__ATOMIC_RELAXED);return n;
}
static int progress(void *unused,curl_off_t a,curl_off_t b,curl_off_t c,curl_off_t d){(void)unused;(void)a;(void)b;(void)c;(void)d;return stop_requested()?1:0;}
static int fetch(const char *url,Sink *sink){
 CURL *curl=curl_easy_init();if(!curl)return -1;
 struct curl_blob ca={(void*)update_ca_bin,update_ca_bin_size,CURL_BLOB_NOCOPY};
 curl_easy_setopt(curl,CURLOPT_URL,url);
 curl_easy_setopt(curl,CURLOPT_CAINFO_BLOB,&ca);
 curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,2L);
 curl_easy_setopt(curl,CURLOPT_PROTOCOLS_STR,"https");
 curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,0L); /* Exact publisher URL; redirects fail closed. */
 curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,12L);curl_easy_setopt(curl,CURLOPT_TIMEOUT,180L);
 curl_easy_setopt(curl,CURLOPT_LOW_SPEED_LIMIT,256L);curl_easy_setopt(curl,CURLOPT_LOW_SPEED_TIME,15L);
 curl_easy_setopt(curl,CURLOPT_NOSIGNAL,1L);curl_easy_setopt(curl,CURLOPT_PROXY,"");
 curl_easy_setopt(curl,CURLOPT_USERAGENT,"VitarenewFlow/" RUNNING_VERSION);
 curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,receive);curl_easy_setopt(curl,CURLOPT_WRITEDATA,sink);
 curl_easy_setopt(curl,CURLOPT_NOPROGRESS,0L);curl_easy_setopt(curl,CURLOPT_XFERINFOFUNCTION,progress);
 CURLcode code=curl_easy_perform(curl);long http=0;curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&http);curl_easy_cleanup(curl);
 char log[120];snprintf(log,sizeof(log),"Update: curl=%d HTTP=%ld bytes=%lu",(int)code,http,(unsigned long)sink->used);app_log(log);
 if(stop_requested())return -3;
 if(code==CURLE_PEER_FAILED_VERIFICATION)return -4;
 if(code!=CURLE_OK)return -1;
 return http==200?0:http==404?-2:-1;
}
static bool file_valid(const char *path){
 FILE *file=fopen(path,"rb");if(!file)return false;
 mbedtls_sha256_context sha;mbedtls_sha256_init(&sha);bool ok=mbedtls_sha256_starts_ret(&sha,0)==0;
 unsigned char block[8192],digest[32]={0};size_t count=0,n;
 while(ok&&(n=fread(block,1,sizeof(block),file))){
  if(!count&&(n<32||memcmp(block,"3DSX",4)))ok=false;
  count+=n;if(count>manifest.bytes)ok=false;
  if(mbedtls_sha256_update_ret(&sha,block,n))ok=false;
 }
 if(ferror(file)||count!=manifest.bytes)ok=false;
 if(fclose(file))ok=false;
 if(mbedtls_sha256_finish_ret(&sha,digest))ok=false;
 mbedtls_sha256_free(&sha);
 char hex[65];for(unsigned i=0;i<32;i++)snprintf(hex+i*2,3,"%02x",digest[i]);
 return ok&&!strcmp(hex,manifest.sha);
}
static void run(void *job){
 bool download=job!=NULL;void *soc=memalign(0x1000,0x100000);bool connected=false,global=false;int result=-1;
 if(!soc){publish(UPDATE_ERROR,"Nicht genug Arbeitsspeicher.");return;}
 if(R_FAILED(socInit(soc,0x100000)))goto finish;
 connected=true;if(curl_global_init(CURL_GLOBAL_DEFAULT)!=CURLE_OK)goto finish;global=true;
 if(!download){
  char text[1024]={0};Sink sink={.memory=text,.limit=sizeof(text)-1};result=fetch(UPDATE_MANIFEST_URL,&sink);
  if(result)goto finish;
  if(!update_manifest_parse(text,sink.used,&manifest)){result=-5;goto finish;}
  int cmp=update_version_compare(manifest.version,RUNNING_VERSION);
  if(cmp>0)publish(UPDATE_AVAILABLE,"Update verfügbar.");
  else publish(UPDATE_CURRENT,"Du hast die aktuelle Version.");
 }else{
  FS_ArchiveResource resource={0};
  if(R_FAILED(FSUSER_GetArchiveResource(&resource,SYSTEM_MEDIATYPE_SD))||(u64)resource.clusterSize*resource.freeClusters<(u64)manifest.bytes+65536){result=-6;goto finish;}
  FILE *file=fopen(UPDATE_STAGE,"wb");if(!file){result=-7;goto finish;}
  Sink sink={.file=file,.limit=manifest.bytes};result=fetch(manifest.url,&sink);
  if(fflush(file)||ferror(file))result=-7;
  if(fclose(file))result=-7;
  if(!result&&!file_valid(UPDATE_STAGE))result=-5;
  if(!result&&stop_requested())result=-3;
  if(result)remove(UPDATE_STAGE);else publish(UPDATE_READY,"Download geprüft. Bereit zum Installieren.");
 }
finish:
 if(global)curl_global_cleanup();
 if(connected)socExit();
 free(soc);
 if(result){
  if(result==-3)publish(UPDATE_CANCELLED,"Update abgebrochen.");
  else publish(UPDATE_ERROR,result==-2?"Noch kein Update veröffentlicht.":result==-4?"HTTPS-Prüfung fehlgeschlagen. Datum prüfen.":result==-5?"Update-Datei ungültig. Nichts geändert.":result==-6?"Nicht genug Platz auf der SD-Karte.":result==-7?"SD-Schreibfehler. Alte Version bleibt erhalten.":"Keine Verbindung. WLAN prüfen.");
 }
}
static void start(bool download){
 app_update_stop();__atomic_store_n(&cancelled,false,__ATOMIC_RELEASE);__atomic_store_n(&received,0,__ATOMIC_RELAXED);__atomic_store_n(&total,download?manifest.bytes:0,__ATOMIC_RELAXED);
 if(!download)memset(&manifest,0,sizeof(manifest));
 __atomic_store_n(&state,download?UPDATE_DOWNLOADING:UPDATE_CHECKING,__ATOMIC_RELEASE);
 worker=threadCreate(run,download?(void*)1:NULL,0x18000,0x38,-2,false);
 if(!worker)publish(UPDATE_ERROR,"Update konnte nicht gestartet werden.");
}
void app_update_check(void){start(false);}
void app_update_download(void){if(__atomic_load_n(&state,__ATOMIC_ACQUIRE)==UPDATE_AVAILABLE)start(true);}
static bool exists(const char *path){struct stat st;return stat(path,&st)==0;}
bool app_update_install(void){
 if(__atomic_load_n(&state,__ATOMIC_ACQUIRE)!=UPDATE_READY)return false;
 app_update_stop();
 if(!file_valid(UPDATE_STAGE)){publish(UPDATE_ERROR,"Update-Datei ungültig. Nichts geändert.");return false;}
 UpdateFs fs={exists,rename,remove};int result=update_store_commit(&fs);
 if(!result&&!file_valid(UPDATE_APP)){
  if(rename(UPDATE_APP,UPDATE_STAGE)||rename(UPDATE_BACKUP,UPDATE_APP))result=-2;else result=-1;
 }
 if(result){publish(UPDATE_ERROR,result==-2?"Wiederherstellung nötig: Sicherung auf SD vorhanden.":"Update fehlgeschlagen. Alte Version erhalten.");return false;}
 publish(UPDATE_INSTALLED,"Update installiert. Jetzt neu öffnen.");app_log("Update: installed and hash verified; previous binary retained");return true;
}
