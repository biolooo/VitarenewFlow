// SPDX-License-Identifier: GPL-3.0-only
#include "report_transport.h"
#include "update_ca_bin.h"
#include <3ds.h>
#include <curl/curl.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
static Thread worker;
static unsigned state;
static bool cancelled;
static char payload[ERROR_REPORT_JSON_CAPACITY];
static size_t payload_size;
bool report_transport_configured(void){
 const char *url=FLOW_REPORT_ENDPOINT;
 return !strncmp(url,"https://",8)&&url[8]&&strlen(url)<256&&!strchr(url,'@')&&!strchr(url,'?')&&!strchr(url,'#');
}
ReportSendState report_transport_state(void){return (ReportSendState)__atomic_load_n(&state,__ATOMIC_ACQUIRE);}
static bool stopped(void){return __atomic_load_n(&cancelled,__ATOMIC_ACQUIRE);}
void report_transport_cancel(void){__atomic_store_n(&cancelled,true,__ATOMIC_RELEASE);}
void report_transport_stop(void){if(worker){report_transport_cancel();threadJoin(worker,U64_MAX);threadFree(worker);worker=NULL;}}
void report_transport_reset(void){report_transport_stop();__atomic_store_n(&state,REPORT_IDLE,__ATOMIC_RELEASE);}
static int progress(void *unused,curl_off_t a,curl_off_t b,curl_off_t c,curl_off_t d){(void)unused;(void)a;(void)b;(void)c;(void)d;return stopped()?1:0;}
static size_t receive(void *data,size_t a,size_t b,void *context){
 (void)data;size_t *used=context;
 if(stopped()||(a&&b>SIZE_MAX/a))return 0;
 size_t n=a*b;if(n>1024-*used)return 0;*used+=n;return n;
}
static void run(void *unused){
 (void)unused;bool soc_ready=false,global_ready=false;bool success=false;
 void *memory=memalign(0x1000,0x100000);CURL *curl=NULL;struct curl_slist *headers=NULL;
 if(!memory||stopped())goto done;
 if(R_FAILED(socInit(memory,0x100000)))goto done;
 soc_ready=true;if(curl_global_init(CURL_GLOBAL_DEFAULT)!=CURLE_OK)goto done;
 global_ready=true;curl=curl_easy_init();if(!curl)goto done;
 headers=curl_slist_append(NULL,"Content-Type: application/json");if(!headers)goto done;
 struct curl_blob ca={(void*)update_ca_bin,update_ca_bin_size,CURL_BLOB_NOCOPY};size_t received=0;
 curl_easy_setopt(curl,CURLOPT_URL,FLOW_REPORT_ENDPOINT);
 curl_easy_setopt(curl,CURLOPT_CAINFO_BLOB,&ca);
 curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,2L);
 curl_easy_setopt(curl,CURLOPT_PROTOCOLS_STR,"https");curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,0L);
 curl_easy_setopt(curl,CURLOPT_PROXY,"");curl_easy_setopt(curl,CURLOPT_NOSIGNAL,1L);
 curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,8L);curl_easy_setopt(curl,CURLOPT_TIMEOUT,20L);
 curl_easy_setopt(curl,CURLOPT_POST,1L);curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
 curl_easy_setopt(curl,CURLOPT_POSTFIELDS,payload);curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE,(long)payload_size);
 curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,receive);curl_easy_setopt(curl,CURLOPT_WRITEDATA,&received);
 curl_easy_setopt(curl,CURLOPT_NOPROGRESS,0L);curl_easy_setopt(curl,CURLOPT_XFERINFOFUNCTION,progress);
 CURLcode result=curl_easy_perform(curl);long http=0;curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&http);
 /* Receiver contract: 201 only after durable storage. Never treat a redirect as success. */
 success=result==CURLE_OK&&http==201;
done:
 if(curl)curl_easy_cleanup(curl);
 if(headers)curl_slist_free_all(headers);
 if(global_ready)curl_global_cleanup();
 if(soc_ready)socExit();
 free(memory);memset(payload,0,sizeof(payload));
 __atomic_store_n(&state,stopped()?REPORT_CANCELLED:success?REPORT_SENT:REPORT_FAILED,__ATOMIC_RELEASE);
}
bool report_transport_start(const ErrorReport *report,bool consent){
 if(!consent||!report_transport_configured()||report_transport_state()==REPORT_SENDING)return false;
 if(worker){threadJoin(worker,U64_MAX);threadFree(worker);worker=NULL;}
 if(!error_report_json(report,payload,sizeof(payload),&payload_size))return false;
 __atomic_store_n(&cancelled,false,__ATOMIC_RELEASE);__atomic_store_n(&state,REPORT_SENDING,__ATOMIC_RELEASE);
 worker=threadCreate(run,NULL,0x10000,0x30,-2,false);
 if(!worker){memset(payload,0,sizeof(payload));__atomic_store_n(&state,REPORT_FAILED,__ATOMIC_RELEASE);return false;}
 return true;
}
