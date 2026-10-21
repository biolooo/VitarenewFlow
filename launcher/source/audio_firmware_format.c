// SPDX-License-Identifier: GPL-3.0-only
/* Bounded readers for Nintendo's backward-LZ code and DSP1 component formats.
   Format references: DSP1 (zoogie), Luma3DS miscellaneous DSP dump, ctrtool BLZ.
   This implementation is independently authored; no firmware bytes embedded. */
#include "audio_firmware_format.h"
#include <string.h>
static uint32_t le32(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
size_t audio_code_size(const uint8_t *d,size_t n){
 if(!d||n<8||n>16u*1024u*1024u)return 0;
 uint32_t control=le32(d+n-8),extra=le32(d+n-4);unsigned header=control>>24;size_t packed=control&0xffffff;
 if(header<8||header>n||packed<header||packed>n||extra>16u*1024u*1024u-n)return 0;
 return n+extra;
}
bool audio_code_unpack(const uint8_t *d,size_t n,uint8_t *o,size_t cap){
 size_t length=audio_code_size(d,n);if(!length||!o||cap<length)return false;
 uint32_t footer=le32(d+n-8);size_t stop=n-(footer&0xffffff),at=n-(footer>>24),out=length;
 memcpy(o,d,n);
 while(at>stop){
  uint8_t bits=d[--at];
  for(unsigned b=0;b<8&&at>stop;b++,bits<<=1){
   if(bits&0x80){
    if(at-stop<2)return false;
    at-=2;unsigned token=d[at]|(unsigned)d[at+1]<<8,run=(token>>12)+3,offset=(token&4095)+2;
    if(out<stop||run>out-stop)return false;
    while(run--){if(out>=length||offset>=length-out)return false;o[out-1]=o[out+offset];out--;}
   }else{if(out<=stop)return false;o[--out]=d[--at];}
  }
 }
 return at==stop&&out==stop;
}
bool audio_dsp_valid(const uint8_t *d,size_t n,AudioDigest digest){
 if(!d||!digest||n<0x300||n>0x10000||memcmp(d+0x100,"DSP1",4)||le32(d+0x104)!=n)return false;
 unsigned count=d[0x10e];if(!count||count>10)return false;
 for(unsigned i=0;i<count;i++){
  const uint8_t *entry=d+0x120+0x30*i;size_t at=le32(entry),length=le32(entry+8);uint8_t hash[32];
  if(at<0x300||at>n||!length||length>n-at||!digest(d+at,length,hash)||memcmp(hash,entry+0x10,32))return false;
 }
 return true;
}
