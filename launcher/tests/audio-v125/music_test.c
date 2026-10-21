#include <3ds.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bgm_format.h"
static unsigned allocations,owned,queued,clears,joins,ticks,exported;static unsigned sourceReads,retries;static int delayed;static int source=1,initError,allocationError=-1;static bool threadError,driver,startedPaused;
static void *owned_alloc(size_t n){void *p=malloc(n);if(p)owned++;return p;}
static void owned_free(void *p){if(p){assert(owned);owned--;free(p);}}
static bool stopping,paused;
static ndspWaveBuf *pending[3];
unsigned theme_read_music(uint8_t **data,size_t *size){*data=NULL;*size=0;sourceReads++;if(delayed>0){delayed--;return 0;}if(!source)return 0;unsigned char wave[48]={0};memcpy(wave,"RIFF",4);memcpy(wave+8,"WAVEfmt ",8);wave[16]=16;wave[20]=1;wave[22]=1;wave[24]=0x22;wave[25]=0x56;wave[32]=2;wave[34]=16;memcpy(wave+36,"data",4);wave[40]=4;wave[44]=1;wave[46]=2;*data=owned_alloc(sizeof(wave));memcpy(*data,wave,sizeof(wave));*size=sizeof(wave);return (unsigned)source;}
void home_audio_prepare_source(bool(*cancel)(void)){assert(!cancel());exported++;}void app_log(const char *s){assert(s);}
Result ndspInit(void){if(initError)return -1;assert(!driver);driver=true;return 0;}void ndspExit(void){assert(driver&&clears);driver=false;}
void ndspSetOutputMode(int n){(void)n;}void ndspChnReset(int n){(void)n;}void ndspChnSetInterp(int n,int v){(void)n;(void)v;}void ndspChnSetRate(int n,float v){(void)n;assert(v==22050);}
void ndspChnSetFormat(int n,int v){(void)n;(void)v;}void ndspChnSetMix(int n,float *m){(void)n;assert(m[0]==1.f&&m[1]==1.f);}
void ndspChnSetPaused(int n,bool v){(void)n;if(v)startedPaused=true;}
void ndspChnWaveBufAdd(int n,ndspWaveBuf *b){assert(n==0&&b->nsamples==4096&&driver);pending[queued%3]=b;queued++;b->status=1;}
void ndspChnWaveBufClear(int n){(void)n;clears++;memset(pending,0,sizeof(pending));}
void *linearAlloc(size_t n){if((int)allocations==allocationError){allocationError=-1;return NULL;}void *p=malloc(n);if(p)allocations++;return p;}
void linearFree(void *p){assert(allocations&&(!driver||(!pending[0]&&!pending[1]&&!pending[2])));allocations--;free(p);}void DSP_FlushDataCache(void *p,size_t n){assert(p&&n==4096*4);}
void svcSleepThread(uint64_t n){if(n==200000000){retries++;return;}assert(n==5000000);ticks++;if(ticks==1)paused=true;if(ticks==2)paused=false;if(ticks==5)stopping=true;for(unsigned i=0;i<3;i++)if(pending[i])pending[i]->status=3;}
Thread threadCreate(void(*f)(void*),void *c,size_t s,int p,int a,bool d){(void)s;(void)p;(void)a;(void)d;if(threadError)return NULL;ticks=0;f(c);return (Thread)1;}
void threadJoin(Thread t,uint64_t n){assert(t&&n==UINT64_MAX);joins++;}void threadFree(Thread t){assert(t);}
#define malloc owned_alloc
#define free owned_free
#include "menu_music.c"
#undef malloc
#undef free
int main(void){
 for(unsigned i=0;i<100;i++){menu_music_reload();assert(queued&&startedPaused&&!driver&&!allocations&&!owned);menu_music_stop();assert(!worker);}
 assert(joins==100);sourceReads=retries=0;source=0;menu_music_reload();assert(sourceReads==3&&retries==2&&exported==0&&!strcmp(menu_music_status(),"VitaRenewFlow-Musik"));menu_music_stop();
 source=1;delayed=2;sourceReads=retries=0;menu_music_reload();assert(sourceReads==3&&retries==2&&!strcmp(menu_music_status(),"Theme-Musik"));menu_music_stop();
 source=1;initError=1;menu_music_reload();assert(!driver&&!owned&&!allocations&&!strcmp(menu_music_status(),"DSP-Audio nicht verfügbar"));menu_music_stop();
 initError=0;allocationError=1;menu_music_reload();assert(!driver&&!owned&&!allocations);menu_music_stop();
 threadError=true;menu_music_reload();assert(!worker&&!owned&&!allocations);menu_music_stop();
 puts("PASS actual audio worker: 100 start/stop cycles, decoder/queues, pause/wake, owned memory release, DSP/linear allocation/thread failures, bundled fallback asset without SD/PC, bounded startup retry and no Nintendo source acquisition");
}
