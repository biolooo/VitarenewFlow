#include <3ds.h>
#include "menu_music.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
static bool stopping,paused,firmFail,initFail,threadFail,driver;static unsigned tick,attempts,firmCalls,initCalls,owned,active,allocNo;static int failAlloc=-1,source=1,delayed;static unsigned clockOffset;static unsigned musicQueued,clickQueued,pauses,retries;
static ndspWaveBuf *queuedBuffers[2][3];static unsigned next[2];
static void *ownAlloc(size_t n){void *p=malloc(n);if(p)owned++;return p;}static void ownFree(void *p){if(p){assert(owned);owned--;free(p);}}
bool audio_firmware_prepare(bool(*cancel)(void)){assert(!cancel());firmCalls++;return !firmFail;}
unsigned theme_read_music(uint8_t **data,size_t *size){assert(firmCalls);attempts++;*data=NULL;*size=0;if(delayed){delayed--;return 0;}if(!source)return 0;unsigned char b[48]={0};memcpy(b,"RIFF",4);memcpy(b+8,"WAVEfmt ",8);b[16]=16;b[20]=1;b[22]=1;b[24]=0x22;b[25]=0x56;b[32]=2;b[34]=16;memcpy(b+36,"data",4);b[40]=4;b[44]=1;b[46]=2;*data=ownAlloc(48);memcpy(*data,b,48);*size=48;return 1;}
void app_log(const char *m){assert(m);}
Result ndspInit(void){initCalls++;if(initFail)return -1;assert(!driver);driver=true;return 0;}void ndspExit(void){assert(driver);driver=false;}
void ndspSetOutputMode(int n){(void)n;}void ndspChnReset(int n){(void)n;}void ndspChnSetInterp(int n,int i){(void)n;(void)i;}
void ndspChnSetRate(int n,float v){assert(n==1?v==44100:v==(source?22050:44100));}
void ndspChnSetFormat(int n,int f){assert(f==(n?NDSP_FORMAT_MONO_PCM16:NDSP_FORMAT_STEREO_PCM16));}
void ndspChnSetMix(int n,float *m){assert(fabsf(m[0]-(n?.85f:.28f))<.00001f&&m[0]==m[1]);}
void ndspChnSetPaused(int n,bool p){(void)n;if(p)pauses++;}
void ndspChnWaveBufAdd(int n,ndspWaveBuf *b){assert(driver&&b&&b->data_vaddr);assert(b->nsamples==(n?1058:4096));if(n)clickQueued++;else musicQueued++;queuedBuffers[n][next[n]++%3]=b;b->status=1;}
void ndspChnWaveBufClear(int n){memset(queuedBuffers[n],0,sizeof(queuedBuffers[n]));}
void *linearAlloc(size_t n){if((int)allocNo++==failAlloc)return NULL;void *p=malloc(n);if(p)active++;return p;}
void linearFree(void *p){for(unsigned c=0;c<2;c++)for(unsigned i=0;i<3;i++)assert(!queuedBuffers[c][i]||queuedBuffers[c][i]->data_vaddr!=p);assert(active);active--;free(p);}
void DSP_FlushDataCache(void *p,size_t n){assert(p&&(n==1058*2||n==4096*4));}
uint64_t osGetTime(void){return 1000+clockOffset+tick*60;}
void svcSleepThread(uint64_t n){if(n==200000000){retries++;return;}assert(n==5000000);tick++;if(tick==1)paused=true;if(tick==2)paused=false;if(tick==3)menu_music_click();if(tick==6)stopping=true;for(unsigned c=0;c<2;c++)for(unsigned i=0;i<3;i++)if(queuedBuffers[c][i])queuedBuffers[c][i]->status=NDSP_WBUF_DONE;}
Thread threadCreate(void(*f)(void*),void *x,size_t s,int p,int c,bool d){(void)s;(void)p;(void)c;(void)d;if(threadFail)return NULL;tick=allocNo=0;menu_music_click();f(x);return (Thread)1;}
void threadJoin(Thread t,uint64_t n){assert(t&&n==UINT64_MAX);}void threadFree(Thread t){assert(t);}
#define malloc ownAlloc
#define free ownFree
#include "menu_music.c"
#undef malloc
#undef free
static void clean(void){menu_music_stop();assert(!driver&&!owned&&!active);}
int main(void){for(unsigned i=0;i<100;i++){menu_music_reload();clean();}assert(musicQueued&&clickQueued&&pauses);unsigned before=clickQueued;source=0;attempts=retries=0;menu_music_reload();clean();assert(clickQueued>before&&attempts==3&&retries==2);unsigned reads=attempts;clockOffset+=2500;menu_music_reload();clockOffset+=2500;menu_music_start_if_needed();clockOffset+=5000;menu_music_start_if_needed();reads=attempts;clockOffset+=100000;menu_music_start_if_needed();assert(attempts==reads);clean();source=1;delayed=2;attempts=0;menu_music_reload();clean();assert(attempts==3);initFail=true;menu_music_reload();clean();assert(!strcmp(menu_music_status(),"DSP-Audio nicht verfügbar"));initFail=false;firmFail=true;unsigned old=initCalls;menu_music_reload();clean();assert(initCalls==old);firmFail=false;for(int i=0;i<4;i++){failAlloc=i;menu_music_reload();clean();}failAlloc=-1;threadFail=true;menu_music_reload();clean();puts("PASS actual audio worker: 100 cycles, built-in clicks with/without theme, firmware preparation ordering/failure, source retries, mix levels, sleep/wake, DSP/thread/allocation failures and owned-buffer cleanup");}
