// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "bgm_format.h"
#include <string.h>
static uint16_t u16(const uint8_t *p){return p[0]|(uint16_t)p[1]<<8;}
static uint32_t u32(const uint8_t *p){return u16(p)|(uint32_t)u16(p+2)<<16;}
static bool span(size_t n,size_t p,size_t len){return p<=n&&len<=n-p;}
/* A DSP frame holds up to 14 samples: one header byte and packed nibbles.
   The final frame may omit unused sample bytes; channel padding is separate. */
static uint64_t adpcm_bytes(unsigned samples){
 unsigned tail=samples%14;
 return (uint64_t)(samples/14)*8+(tail?1+(tail+1)/2:0);
}
static bool wav(BgmStream *s){
 const uint8_t *d=s->data;size_t n=s->size,at=12;unsigned bits=0;bool format=false,body=false;
 if(n<12||memcmp(d,"RIFF",4)||memcmp(d+8,"WAVE",4))return false;
 while(span(n,at,8)){
  uint32_t len=u32(d+at+4);size_t payload=at+8;if(!span(n,payload,len))return false;
  if(!memcmp(d+at,"fmt ",4)){
   if(format||len<16||u16(d+payload)!=1)return false;
   s->channels=u16(d+payload+2);s->rate=u32(d+payload+4);bits=u16(d+payload+14);
   if(s->channels<1||s->channels>2||(bits!=8&&bits!=16)||u16(d+payload+12)!=s->channels*(bits/8))return false;
   format=true;
  }else if(!memcmp(d+at,"data",4)){if(body)return false;s->offset=payload;s->lastBytes=len;body=true;}
  size_t step=(size_t)len+8+(len&1u);if(step>n-at)break;at+=step;
 }
 if(!format||!body||s->rate<8000||s->rate>96000)return false;
 unsigned stride=s->channels*(bits/8);if(!s->lastBytes||s->lastBytes%stride)return false;
 s->samples=s->lastBytes/stride;s->codec=bits==16?1:0;s->wav=true;s->loop=true;return true;
}
bool bgm_parse(BgmStream *s,const void *data,size_t size){
 if(!s)return false;
 memset(s,0,sizeof(*s));if(!data||size<12||size>32u*1024u*1024u)return false;
 s->data=data;s->size=size;const uint8_t *d=data;
 if(!memcmp(d,"RIFF",4))return wav(s);
 if(size<0x40||memcmp(d,"CSTM",4)||u16(d+4)!=0xfeff)return false;
 uint32_t declared=u32(d+12);if(declared<0x40||declared>size)return false;size=declared;s->size=size;
 unsigned sections=u16(d+0x10);if(sections<2||sections>5||!span(size,0x14,(size_t)sections*12))return false;
 size_t info=0,infoSize=0,body=0,bodySize=0;
 for(unsigned i=0;i<sections;i++){
  const uint8_t *entry=d+0x14+12*i;size_t off=u32(entry+4),len=u32(entry+8);
  if(!span(size,off,len))return false;
  if(u16(entry)==0x4000){if(info)return false;info=off;infoSize=len;}
  if(u16(entry)==0x4002){if(body)return false;body=off;bodySize=len;}
 }
 if(infoSize<0x48||bodySize<0x20||memcmp(d+info,"INFO",4)||memcmp(d+body,"DATA",4))return false;
 s->codec=d[info+0x20];s->loop=d[info+0x21]!=0;s->channels=d[info+0x22];s->rate=u32(d+info+0x24);
 s->loopStart=u32(d+info+0x28);s->samples=u32(d+info+0x2c);s->blocks=u32(d+info+0x30);
 s->blockBytes=u32(d+info+0x34);s->blockSamples=u32(d+info+0x38);
 /* INFO+0x3c: payload bytes, +0x40: samples, +0x44: padded channel stride.
  * +0x48 is SEEK-entry size, never an audio stride (VGAudio StreamInfo).
  * Keeping payload and stride separate avoids playing padding as audio. */
 s->lastBytes=u32(d+info+0x3c);s->lastSamples=u32(d+info+0x40);s->lastStride=u32(d+info+0x44);
 if(s->lastStride<s->lastBytes||s->lastStride>s->blockBytes)return false;
 s->offset=body+0x20;
 if(s->codec>2||s->channels<1||s->channels>2||s->rate<8000||s->rate>96000||!s->samples||!s->blocks||!s->blockBytes||!s->blockSamples||!s->lastSamples||!s->lastBytes||s->lastSamples>s->blockSamples||s->lastBytes>s->blockBytes)return false;
 if((uint64_t)(s->blocks-1)*s->blockSamples+s->lastSamples!=s->samples||(s->loop&&s->loopStart>=s->samples))return false;
 uint64_t bytes=(uint64_t)(s->blocks-1)*s->blockBytes*s->channels+(uint64_t)s->lastStride*(s->channels-1)+s->lastBytes;
 if(bytes>bodySize-0x20)return false;
 uint64_t regular=s->codec==2?adpcm_bytes(s->blockSamples):(uint64_t)s->blockSamples*(s->codec+1);
 uint64_t last=s->codec==2?adpcm_bytes(s->lastSamples):(uint64_t)s->lastSamples*(s->codec+1);
 if(regular>s->blockBytes||last>s->lastBytes||(s->codec==2&&s->blockSamples%14))return false;
 if(s->codec==2){
  uint32_t relative=u32(d+info+0x1c);if(relative>infoSize-8)return false;
  size_t table=8u+relative;if(!span(infoSize,table,4+8*s->channels))return false;
  if(u32(d+info+table)!=s->channels)return false;
  for(unsigned c=0;c<s->channels;c++){
   size_t entry=table+4+8*c;if(u16(d+info+entry)!=0x4102)return false;
   relative=u32(d+info+entry+4);if(relative>infoSize-table)return false;
   size_t channel=table+relative;if(!span(infoSize,channel,8)||u16(d+info+channel)!=0x300)return false;
   relative=u32(d+info+channel+4);if(relative>infoSize-channel)return false;
   size_t coeff=channel+relative;if(!span(infoSize,coeff,38))return false;
   for(unsigned k=0;k<16;k++)s->coefficients[c][k]=(int16_t)u16(d+info+coeff+2*k);
   s->history[c][0]=(int16_t)u16(d+info+coeff+34);s->history[c][1]=(int16_t)u16(d+info+coeff+36);
  }
 }
 return true;
}
static bool sample(BgmStream *s,unsigned channel,int16_t *value){
 size_t at;unsigned index;
 if(s->wav){at=s->offset+(size_t)s->position*s->channels*(s->codec+1)+channel*(s->codec+1);index=0;}
 else{
  unsigned block=s->position/s->blockSamples;index=s->position%s->blockSamples;
  unsigned stride=block==s->blocks-1?s->lastStride:s->blockBytes;
  at=s->offset+(size_t)block*s->blockBytes*s->channels+(size_t)channel*stride;
 }
 if(s->codec<2){at+=(size_t)index*(s->codec+1);if(!span(s->size,at,s->codec+1))return false;*value=s->codec?(int16_t)u16(s->data+at):(int16_t)((s->wav?(int)s->data[at]-128:(int)(int8_t)s->data[at])*256);return true;}
 unsigned nibble=index%14;
 at+=(size_t)(index/14)*8;if(!span(s->size,at,2+nibble/2))return false;
 unsigned header=s->data[at],predictor=header>>4;if(predictor>7)return false;
 int v=(s->data[at+1+nibble/2]>>(nibble%2?0:4))&15;if(v>=8)v-=16;
 int64_t decoded=((int64_t)v*(1u<<(header&15))*2048+(int64_t)s->coefficients[channel][2*predictor]*s->history[channel][0]+(int64_t)s->coefficients[channel][2*predictor+1]*s->history[channel][1]+1024)>>11;
 if(decoded>32767)decoded=32767;
 if(decoded< -32768)decoded=-32768;
 s->history[channel][1]=s->history[channel][0];s->history[channel][0]=(int16_t)decoded;*value=(int16_t)decoded;return true;
}
size_t bgm_render(BgmStream *s,int16_t *pcm,size_t frames){
 if(!s||!pcm||!s->samples)return 0;
 size_t done=0;
 while(done<frames){
  if(s->position==s->samples){if(!s->loop)break;s->position=s->loopStart;if(s->codec==2){if(!s->loopSaved)break;memcpy(s->history,s->loopHistory,sizeof(s->history));}}
  if(s->position==s->loopStart&&!s->loopSaved){memcpy(s->loopHistory,s->history,sizeof(s->history));s->loopSaved=true;}
  int16_t left,right;if(!sample(s,0,&left)|| (s->channels==2&&!sample(s,1,&right)))break;
  pcm[2*done]=left;pcm[2*done+1]=s->channels==2?right:left;done++;s->position++;
 }
 return done;
}
