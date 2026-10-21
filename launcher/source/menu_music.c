// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "menu_music.h"
#include "bgm_format.h"
#include "theme_io.h"
#include "app_log.h"
#include "audio_firmware.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define CLICK_SAMPLES 1058
static unsigned clickPending;
void menu_music_click(void){__atomic_store_n(&clickPending,1,__ATOMIC_RELEASE);}
#define FRAMES 4096
#define BUFFERS 3
static Thread worker;
static bool stopping,paused;
static unsigned state,retryCount;
static u64 retryAt;
/* State is published with release/acquire; the audio thread owns DSP and buffers. */
const char *menu_music_status(void){
 static const char *names[]={"Musik wird geladen","Theme-Musik","HOME-Musik noch nicht verfügbar","HOME-Musik noch nicht verfügbar","Musikformat nicht unterstützt","DSP-Audio nicht verfügbar"};
 unsigned n=__atomic_load_n(&state,__ATOMIC_ACQUIRE);return n<6?names[n]:names[3];
}
static void publish(unsigned n){__atomic_store_n(&state,n,__ATOMIC_RELEASE);}
static bool cancelled(void){return __atomic_load_n(&stopping,__ATOMIC_ACQUIRE);}
static void play(void *unused){
 (void)unused;
 if(!audio_firmware_prepare(cancelled)){publish(5);return;}
 if(cancelled())return;
 uint8_t *data=NULL;size_t size=0;unsigned source=0;
 for(unsigned attempt=0;attempt<3&&!cancelled();attempt++){
  source=theme_read_music(&data,&size);if(data)break;
  if(attempt<2)svcSleepThread(200000000);
 }
 if(cancelled()){free(data);return;}
 if(!data){publish(3);app_log("Audio: no active theme BGM; built-in selection clicks stay enabled");}
 BgmStream stream={0};bool music=data&&bgm_parse(&stream,data,size);
 if(data&&!music){publish(4);char log[240];snprintf(log,sizeof(log),"Menu music: invalid source bytes=%lu codec=%u channels=%u rate=%u blocks=%u blockBytes=%u blockSamples=%u lastBytes=%u lastSamples=%u lastStride=%u; clicks remain enabled",(unsigned long)size,stream.codec,stream.channels,stream.rate,stream.blocks,stream.blockBytes,stream.blockSamples,stream.lastBytes,stream.lastSamples,stream.lastStride);app_log(log);}
 if(__atomic_load_n(&stopping,__ATOMIC_ACQUIRE)){free(data);return;}
 Result result=ndspInit();if(R_FAILED(result)){char log[96];snprintf(log,sizeof(log),"Audio: ndspInit failed %08lX",(unsigned long)(u32)result);app_log(log);free(data);publish(5);return;}
 ndspWaveBuf buffers[BUFFERS];memset(buffers,0,sizeof(buffers));bool allocated=true;
 for(unsigned i=0;i<BUFFERS;i++){buffers[i].data_vaddr=linearAlloc(FRAMES*4);if(!buffers[i].data_vaddr)allocated=false;}
 if(!allocated){for(unsigned i=0;i<BUFFERS;i++)if(buffers[i].data_vaddr)linearFree((void*)buffers[i].data_vaddr);ndspExit();free(data);publish(5);return;}
 ndspSetOutputMode(NDSP_OUTPUT_STEREO);ndspChnReset(0);ndspChnSetInterp(0,NDSP_INTERP_LINEAR);
 ndspChnSetRate(0,(float)(music?stream.rate:44100));ndspChnSetFormat(0,NDSP_FORMAT_STEREO_PCM16);
 float mix[12]={0};mix[0]=mix[1]=.28f;ndspChnSetMix(0,mix);
 ndspWaveBuf click={0};click.data_vaddr=linearAlloc(CLICK_SAMPLES*2);
 if(click.data_vaddr){
  int16_t *pcm=(int16_t*)click.data_vaddr;
  /* Original soft mechanical tick: short attack and fast decay, no sample I/O. */
  for(unsigned i=0;i<CLICK_SAMPLES;i++){
   float t=(float)i/44100.f,envelope=(1.f-expf(-t*5500.f))*expf(-t*240.f);
   pcm[i]=(int16_t)(9000.f*envelope*(sinf(t*2.f*3.14159265f*1450.f)+.3f*sinf(t*2.f*3.14159265f*2900.f)));
  }
  click.nsamples=CLICK_SAMPLES;DSP_FlushDataCache(click.data_vaddr,CLICK_SAMPLES*2);
  ndspChnReset(1);ndspChnSetRate(1,44100.f);ndspChnSetFormat(1,NDSP_FORMAT_MONO_PCM16);
  ndspChnSetInterp(1,NDSP_INTERP_LINEAR);float clickMix[12]={0};clickMix[0]=clickMix[1]=.85f;ndspChnSetMix(1,clickMix);
 }

 u64 lastClick=0;
 bool wasPaused=false,ended=!music;if(music)publish(source==1?1:2);if(music)app_log(source==1?"Menu music: active theme":"Menu music: bundled original VitaRenewFlow fallback");
 while(!__atomic_load_n(&stopping,__ATOMIC_ACQUIRE)){
  bool now=__atomic_load_n(&paused,__ATOMIC_ACQUIRE);
  if(now!=wasPaused){ndspChnSetPaused(0,now);ndspChnSetPaused(1,now);wasPaused=now;}
  if(__atomic_exchange_n(&clickPending,0,__ATOMIC_ACQ_REL)&&!now&&click.data_vaddr&&osGetTime()-lastClick>=45){
   if(click.status==NDSP_WBUF_FREE||click.status==NDSP_WBUF_DONE){ndspChnWaveBufAdd(1,&click);lastClick=osGetTime();}
  }
  if(!now&&!ended)for(unsigned i=0;i<BUFFERS;i++){
   if(buffers[i].status!=NDSP_WBUF_FREE&&buffers[i].status!=NDSP_WBUF_DONE)continue;
   size_t rendered=bgm_render(&stream,(int16_t*)buffers[i].data_vaddr,FRAMES);
   if(!rendered){ended=true;break;}
   buffers[i].nsamples=rendered;DSP_FlushDataCache(buffers[i].data_vaddr,rendered*4);ndspChnWaveBufAdd(0,&buffers[i]);
  }
  svcSleepThread(5000000); /* no busy-spin; about 250 ms of queued audio at 48 kHz */
 }
 ndspChnWaveBufClear(1);ndspChnReset(1);
 if(click.data_vaddr)linearFree((void*)click.data_vaddr);
 ndspChnWaveBufClear(0);ndspChnReset(0);ndspExit();
 for(unsigned i=0;i<BUFFERS;i++)linearFree((void*)buffers[i].data_vaddr);
 free(data);
}
void menu_music_stop(void){
 if(!worker)return;
 __atomic_store_n(&stopping,true,__ATOMIC_RELEASE);threadJoin(worker,U64_MAX);threadFree(worker);worker=NULL;
}
void menu_music_reload(void){
 menu_music_stop();retryCount=0;retryAt=osGetTime()+2000;publish(0);__atomic_store_n(&stopping,false,__ATOMIC_RELEASE);__atomic_store_n(&paused,false,__ATOMIC_RELEASE);
 worker=threadCreate(play,NULL,0x10000,0x38,-2,false);if(!worker)publish(5);
}
void menu_music_pause(bool value){__atomic_store_n(&paused,value,__ATOMIC_RELEASE);}

void menu_music_start_if_needed(void){
 if(!worker){if(retryCount>=2)return;unsigned tries=retryCount;menu_music_reload();retryCount=tries+1;return;}
 unsigned value=__atomic_load_n(&state,__ATOMIC_ACQUIRE);
 if((value==3||value==5)&&retryCount<2&&osGetTime()>=retryAt){unsigned tries=retryCount+1;menu_music_reload();retryCount=tries;retryAt=osGetTime()+4000;}
}
