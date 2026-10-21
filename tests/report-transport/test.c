#include "report_transport.h"
#include <3ds.h>
#include <curl/curl.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
static int jobs,sockets,globals,joins;static long http=201;static int network_error,cancel_request,thread_failure;
static CURL handle;static struct curl_slist headers;
static size_t (*sink)(void*,size_t,size_t,void*);static void *sink_context;
int socInit(void*p,unsigned n){assert(p&&n==0x100000);sockets++;return 0;}void socExit(void){sockets--;}
Thread threadCreate(void(*fn)(void*),void*p,size_t n,int priority,int cpu,bool detached){(void)n;(void)priority;(void)cpu;(void)detached;jobs++;if(thread_failure)return NULL;fn(p);return (Thread)1;}
int threadJoin(Thread t,uint64_t timeout){assert(t&&timeout==U64_MAX);joins++;return 0;}void threadFree(Thread t){assert(t);}
int curl_global_init(long f){(void)f;globals++;return 0;}void curl_global_cleanup(void){globals--;}
CURL *curl_easy_init(void){return &handle;}void curl_easy_cleanup(CURL*p){assert(p==&handle);}
struct curl_slist *curl_slist_append(struct curl_slist*p,const char*s){assert(!p&&!strcmp(s,"Content-Type: application/json"));return &headers;}void curl_slist_free_all(struct curl_slist*p){assert(p==&headers);}
int curl_easy_setopt(CURL*p,int opt,...){assert(p==&handle);va_list args;va_start(args,opt);
switch(opt){
case CURLOPT_SSL_VERIFYPEER:assert(va_arg(args,long)==1);break;
case CURLOPT_SSL_VERIFYHOST:assert(va_arg(args,long)==2);break;
case CURLOPT_FOLLOWLOCATION:assert(va_arg(args,long)==0);break;
case CURLOPT_CONNECTTIMEOUT:assert(va_arg(args,long)<=8);break;
case CURLOPT_TIMEOUT:assert(va_arg(args,long)<=20);break;
case CURLOPT_PROTOCOLS_STR:assert(!strcmp(va_arg(args,char*),"https"));break;
case CURLOPT_PROXY:assert(!strcmp(va_arg(args,char*),""));break;
case CURLOPT_POSTFIELDS:assert(strstr(va_arg(args,char*),"description"));break;
case CURLOPT_POSTFIELDSIZE:assert(va_arg(args,long)<4096);break;
case CURLOPT_WRITEFUNCTION:sink=va_arg(args,size_t(*)(void*,size_t,size_t,void*));break;
case CURLOPT_WRITEDATA:sink_context=va_arg(args,void*);break;
default:break;}va_end(args);return 0;}
int curl_easy_perform(CURL*p){assert(p==&handle);if(cancel_request)report_transport_cancel();assert(sink);if(!cancel_request)assert(sink(NULL,1,2,sink_context)==2);return network_error;}
int curl_easy_getinfo(CURL*p,int info,long*out){assert(p==&handle&&info==CURLINFO_RESPONSE_CODE);*out=http;return 0;}
int main(void){ErrorReport report={.version="0.12.34",.description="test"};
assert(!report_transport_start(&report,false)&&jobs==0);
if(!strcmp(FLOW_REPORT_ENDPOINT,"")){
assert(!report_transport_configured());assert(!report_transport_start(&report,true)&&jobs==0);puts("PASS: no request without endpoint or consent");
return 0;
}
assert(report_transport_configured());
for(unsigned i=0;i<5;i++){long responses[]={201,200,302,429,500};http=responses[i];assert(report_transport_start(&report,true));assert(report_transport_state()==(http==201?REPORT_SENT:REPORT_FAILED));assert(!sockets&&!globals);report_transport_reset();assert(report_transport_state()==REPORT_IDLE);}
network_error=60;assert(report_transport_start(&report,true));assert(report_transport_state()==REPORT_FAILED);network_error=0;
cancel_request=1;assert(report_transport_start(&report,true));assert(report_transport_state()==REPORT_CANCELLED);cancel_request=0;
thread_failure=1;assert(!report_transport_start(&report,true));assert(report_transport_state()==REPORT_FAILED);
assert(!sockets&&!globals&&joins>0);puts("PASS: actual HTTPS transport consent, TLS policy, 201/200/redirect/429/500, failure, cancellation, cleanup and reopen");

return 0;}
