// SPDX-License-Identifier: GPL-3.0-only
#define _POSIX_C_SOURCE 200809L
#include "app_update.h"
#include <3ds.h>
#include <curl/curl.h>
#include <assert.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct TestThread {pthread_t thread;void (*fn)(void*);void *arg;};
struct TestCurl {const char *url;size_t (*write)(void*,size_t,size_t,void*);void *context;long http,connect_timeout,timeout;};
static pthread_mutex_t snapshot_mutex=PTHREAD_MUTEX_INITIALIZER;
static atomic_int sockets,globals,notes_requests,snapshots;
static atomic_bool read_status;
static int notes_http=200,notes_error,notes_oversize,cancel_notes,invalid_manifest,thread_failure;
static char target[24]="0.12.40",notes_version[24]="0.12.40";
void LightLock_Lock(LightLock *lock){(void)lock;assert(!pthread_mutex_lock(&snapshot_mutex));}
void LightLock_Unlock(LightLock *lock){(void)lock;assert(!pthread_mutex_unlock(&snapshot_mutex));}
int socInit(void *p,unsigned n){assert(p&&n==0x100000);atomic_fetch_add(&sockets,1);return 0;}
void socExit(void){atomic_fetch_sub(&sockets,1);}
void app_log(const char *text){assert(text);}
static void *thread_entry(void *arg){struct TestThread *job=arg;job->fn(job->arg);return NULL;}
Thread threadCreate(void (*fn)(void*),void *arg,size_t stack,int priority,int cpu,bool detached){
 (void)stack;(void)priority;(void)cpu;assert(!detached);if(thread_failure)return NULL;
 struct TestThread *t=calloc(1,sizeof(*t));assert(t);t->fn=fn;t->arg=arg;
 assert(!pthread_create(&t->thread,NULL,thread_entry,t));return t;
}
int threadJoin(Thread t,uint64_t timeout){assert(timeout==U64_MAX);return pthread_join(t->thread,NULL);}
void threadFree(Thread t){free(t);}
int FSUSER_GetArchiveResource(FS_ArchiveResource *r,int media){assert(media==SYSTEM_MEDIATYPE_SD);memset(r,0,sizeof(*r));return -1;}
int curl_global_init(long flags){assert(flags==CURL_GLOBAL_DEFAULT);atomic_fetch_add(&globals,1);return 0;}
void curl_global_cleanup(void){atomic_fetch_sub(&globals,1);}
CURL *curl_easy_init(void){CURL *c=calloc(1,sizeof(*c));assert(c);return c;}
void curl_easy_cleanup(CURL *c){free(c);}
int curl_easy_setopt(CURL *c,int option,...){
 va_list args;va_start(args,option);
 switch(option){
 case CURLOPT_URL:c->url=va_arg(args,const char*);break;
 case CURLOPT_WRITEFUNCTION:c->write=va_arg(args,size_t(*)(void*,size_t,size_t,void*));break;
 case CURLOPT_WRITEDATA:c->context=va_arg(args,void*);break;
 case CURLOPT_SSL_VERIFYPEER:case CURLOPT_SSL_VERIFYHOST:assert(va_arg(args,long)==(option==CURLOPT_SSL_VERIFYPEER?1:2));break;
 case CURLOPT_FOLLOWLOCATION:assert(va_arg(args,long)==0);break;
 case CURLOPT_PROTOCOLS_STR:assert(!strcmp(va_arg(args,const char*),"https"));break;
 case CURLOPT_PROXY:assert(!strcmp(va_arg(args,const char*),""));break;
 case CURLOPT_CONNECTTIMEOUT:c->connect_timeout=va_arg(args,long);break;
 case CURLOPT_TIMEOUT:c->timeout=va_arg(args,long);break;
 default:break;
 }
 va_end(args);return 0;
}
int curl_easy_perform(CURL *c){
 assert(c->url&&c->write);char body[1200];size_t size;
 if(!strcmp(c->url,UPDATE_MANIFEST_URL)){
  assert(c->connect_timeout==12&&c->timeout==180);c->http=200;
  int n=snprintf(body,sizeof(body),"VFUPDATE1\nversion=%s\nsize=32\nsha256=0000000000000000000000000000000000000000000000000000000000000000\nurl=" UPDATE_URL_ROOT "%s/VitarenewFlow.3dsx\n",target,target);
  assert(n>0);size=(size_t)n;if(invalid_manifest)body[0]='X';
 }else{
  char expected[240];assert(update_notes_url(target,expected,sizeof(expected))&&!strcmp(c->url,expected));
  assert(c->connect_timeout<=4&&c->timeout<=8);atomic_fetch_add(&notes_requests,1);
  UpdateStatus snapshot=app_update_status();assert(snapshot.state==UPDATE_CHECKING&&snapshot.notes_state==UPDATE_NOTES_MISSING&&!snapshot.available[0]);
  c->http=notes_http;if(cancel_notes)app_update_cancel();if(notes_error)return notes_error;
  int n=snprintf(body,sizeof(body),"VFNOTES1\nversion=%s\nde=Neue Hinweise\nen=New highlights\n",notes_version);assert(n>0);size=(size_t)n;
  if(notes_oversize){memset(body,'A',1025);size=1025;}
 }
 return c->write(body,1,size,c->context)==size?0:CURLE_WRITE_ERROR;
}
int curl_easy_getinfo(CURL *c,int info,long *out){assert(info==CURLINFO_RESPONSE_CODE);*out=c->http;return 0;}
static UpdateStatus checked(void){
 app_update_check();UpdateStatus s;
 do{s=app_update_status();struct timespec delay={0,100000};nanosleep(&delay,NULL);}while(s.state==UPDATE_CHECKING);
 app_update_stop();assert(!atomic_load(&sockets)&&!atomic_load(&globals));return s;
}
static void *observe(void *unused){
 (void)unused;
 while(atomic_load(&read_status)){
  UpdateStatus s=app_update_status();
  if(s.notes_state==UPDATE_NOTES_AVAILABLE){assert(s.notes.count>=1&&s.notes.count<=3);assert(!strcmp(s.available,s.notes.version));}
  else assert(!s.notes.count&&!s.notes.version[0]);
  if(s.state==UPDATE_CHECKING)assert(!s.available[0]&&s.notes_state==UPDATE_NOTES_MISSING);
  atomic_fetch_add(&snapshots,1);
 }
 return NULL;
}
int main(void){
 UpdateStatus s=checked();assert(s.state==UPDATE_AVAILABLE&&s.notes_state==UPDATE_NOTES_AVAILABLE&&s.notes.count==1);
 assert(!strcmp(s.available,"0.12.40")&&!strcmp(s.notes.de[0],"Neue Hinweise")&&!strcmp(s.notes.en[0],"New highlights"));
 notes_http=404;s=checked();assert(s.state==UPDATE_AVAILABLE&&s.notes_state==UPDATE_NOTES_MISSING&&!s.notes.count&&!app_update_error_code());
 notes_http=200;notes_error=CURLE_PEER_FAILED_VERIFICATION;s=checked();assert(s.state==UPDATE_AVAILABLE&&s.notes_state==UPDATE_NOTES_MISSING&&!app_update_error_code());
 notes_error=1;s=checked();assert(s.state==UPDATE_AVAILABLE&&s.notes_state==UPDATE_NOTES_MISSING);notes_error=0;
 notes_oversize=1;s=checked();assert(s.state==UPDATE_AVAILABLE&&s.notes_state==UPDATE_NOTES_MISSING);notes_oversize=0;
 strcpy(notes_version,"0.12.41");s=checked();assert(s.state==UPDATE_AVAILABLE&&s.notes_state==UPDATE_NOTES_MISSING);strcpy(notes_version,target);
 cancel_notes=1;s=checked();assert(s.state==UPDATE_CANCELLED&&s.notes_state==UPDATE_NOTES_MISSING);cancel_notes=0;
 invalid_manifest=1;int before=atomic_load(&notes_requests);s=checked();assert(s.state==UPDATE_ERROR&&atomic_load(&notes_requests)==before);invalid_manifest=0;
 strcpy(target,"0.12.36");strcpy(notes_version,target);s=checked();assert(s.state==UPDATE_CURRENT&&s.notes_state==UPDATE_NOTES_AVAILABLE);
 thread_failure=1;s=checked();assert(s.state==UPDATE_ERROR&&s.notes_state==UPDATE_NOTES_MISSING);thread_failure=0;
 pthread_t reader;atomic_store(&read_status,true);assert(!pthread_create(&reader,NULL,observe,NULL));
 for(unsigned i=0;i<80;i++){
  snprintf(target,sizeof(target),"0.12.%u",40+i);strcpy(notes_version,target);notes_http=i%2?404:200;
  s=checked();assert(s.state==UPDATE_AVAILABLE);assert(s.notes_state==(notes_http==200?UPDATE_NOTES_AVAILABLE:UPDATE_NOTES_MISSING));
 }
 atomic_store(&read_status,false);assert(!pthread_join(reader,NULL));assert(atomic_load(&snapshots)>0);
 puts("PASS: actual updater optional HTTPS notes, missing/TLS/oversize/version mismatch, cancellation, unchanged OTA availability and race-free version-bound snapshots");
 return 0;
}
