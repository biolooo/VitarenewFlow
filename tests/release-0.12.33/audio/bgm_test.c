#include "bgm_format.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static unsigned char original[0x180],data[0x180];
static void w16(unsigned char *p,unsigned n){p[0]=n;p[1]=n>>8;}
static void w32(unsigned char *p,unsigned n){w16(p,n);w16(p+2,n>>16);}
static void fixture(void){
 memset(original,0,sizeof(original));memcpy(original,"CSTM",4);w16(original+4,0xfeff);w16(original+6,0x40);w32(original+12,sizeof(original));w16(original+0x10,2);
 w16(original+0x14,0x4000);w32(original+0x18,0x40);w32(original+0x1c,0x100);w16(original+0x20,0x4002);w32(original+0x24,0x140);w32(original+0x28,0x40);
 unsigned char *i=original+0x40;memcpy(i,"INFO",4);i[0x20]=2;i[0x21]=1;i[0x22]=2;w32(i+0x24,32000);w32(i+0x28,17);w32(i+0x2c,21);w32(i+0x30,2);w32(i+0x34,8);w32(i+0x38,14);w32(i+0x3c,8);w32(i+0x40,7);w32(i+0x44,8);w32(i+0x48,4);w32(i+0x4c,14336);
 w32(i+0x1c,0x68);w32(i+0x70,2);w16(i+0x74,0x4102);w32(i+0x78,0x20);w16(i+0x7c,0x4102);w32(i+0x80,0x50);
 w16(i+0x90,0x300);w32(i+0x94,8);w16(i+0xc0,0x300);w32(i+0xc4,8);w16(i+0x98,2048);w16(i+0x98+34,10);
 memcpy(original+0x140,"DATA",4);memset(original+0x161,0x11,7);memset(original+0x169,0xff,7);memset(original+0x171,0x11,7);memset(original+0x179,0xff,7);
}
int main(void){
 fixture();BgmStream s;int16_t pcm[512];assert(bgm_parse(&s,original,sizeof(original)));assert(s.channels==2&&s.rate==32000);assert(bgm_render(&s,pcm,30)==30);
 for(int j=0;j<21;j++){assert(pcm[2*j]==11+j&&pcm[2*j+1]==-1);}for(int j=21;j<30;j++)assert(pcm[2*j]==28+(j-21)%4);
 assert(!bgm_parse(&s,NULL,12));for(unsigned n=0;n<sizeof(original);n++)assert(!bgm_parse(&s,original,n));
 memcpy(data,original,sizeof(data));data[4]=0xfe;data[5]=0xff;assert(!bgm_parse(&s,data,sizeof(data)));
 memcpy(data,original,sizeof(data));w32(data+0x40+0x1c,0xfffffffcu);assert(!bgm_parse(&s,data,sizeof(data)));
 memcpy(data,original,sizeof(data));data[0x160]=0xf0;assert(bgm_parse(&s,data,sizeof(data)));assert(!bgm_render(&s,pcm,20));
 memcpy(data,original,sizeof(data));unsigned char *i=data+0x40;i[0x20]=1;w32(i+0x28,1);w32(i+0x2c,3);w32(i+0x34,4);w32(i+0x38,2);w32(i+0x3c,2);w32(i+0x40,1);w32(i+0x44,2);
 w16(data+0x160,(unsigned)(uint16_t)-1000);w16(data+0x162,1000);w16(data+0x164,(unsigned)(uint16_t)-2000);w16(data+0x166,2000);w16(data+0x168,3000);w16(data+0x16a,4000);
 assert(bgm_parse(&s,data,sizeof(data))&&bgm_render(&s,pcm,5)==5);assert(pcm[0]==-1000&&pcm[1]==-2000&&pcm[4]==3000&&pcm[5]==4000&&pcm[6]==1000&&pcm[7]==2000);
 unsigned char wave[48]={0};memcpy(wave,"RIFF",4);w32(wave+4,40);memcpy(wave+8,"WAVEfmt ",8);w32(wave+16,16);w16(wave+20,1);w16(wave+22,1);w32(wave+24,22050);w16(wave+32,2);w16(wave+34,16);memcpy(wave+36,"data",4);w32(wave+40,4);w16(wave+44,1234);w16(wave+46,(unsigned)(uint16_t)-2345);
 assert(bgm_parse(&s,wave,sizeof(wave))&&bgm_render(&s,pcm,3)==3);assert(pcm[0]==1234&&pcm[1]==1234&&pcm[2]==-2345&&pcm[3]==-2345&&pcm[4]==1234);
 srand(4);for(int n=0;n<20000;n++){memcpy(data,original,sizeof(data));for(int j=0;j<1+n%8;j++)data[rand()%sizeof(data)]=(unsigned char)rand();if(bgm_parse(&s,data,sizeof(data)))bgm_render(&s,pcm,256);}
 unsigned char padded[0x1c0]={0};memcpy(padded,original,sizeof(original));w32(padded+12,sizeof(padded));w32(padded+0x28,0x80);unsigned char *pi=padded+0x40;pi[0x20]=1;pi[0x21]=0;w32(pi+0x28,0);w32(pi+0x2c,1);w32(pi+0x30,1);w32(pi+0x34,64);w32(pi+0x38,32);w32(pi+0x3c,2);w32(pi+0x40,1);w32(pi+0x44,32);w32(pi+0x48,4);w16(padded+0x160,1234);w16(padded+0x180,5678);assert(bgm_parse(&s,padded,sizeof(padded))&&bgm_render(&s,pcm,1)==1&&pcm[0]==1234&&pcm[1]==5678);
 /* SEEK metadata must not influence audio channel layout. */
 for(unsigned seek=0;seek<65536;seek=seek?seek*2:1){memcpy(data,original,sizeof(data));w32(data+0x40+0x48,seek);assert(bgm_parse(&s,data,sizeof(data)));assert(bgm_render(&s,pcm,30)==30);assert(pcm[0]==11&&pcm[1]==-1&&pcm[42]==28);}
 memcpy(data,original,sizeof(data));w32(data+0x40+0x3c,0);assert(!bgm_parse(&s,data,sizeof(data)));
 memcpy(data,original,sizeof(data));w32(data+0x40+0x44,4);assert(!bgm_parse(&s,data,sizeof(data)));
 memcpy(data,original,sizeof(data));w32(data+0x40+0x44,9);assert(!bgm_parse(&s,data,sizeof(data)));
 /* Packed tail: every partial DSP frame size, with and without padding.
    Right channel ends exactly at EOF; no unused full-frame read is allowed. */
 for(unsigned channels=1;channels<=2;channels++)for(unsigned tail=1;tail<=14;tail++)for(unsigned pad=0;pad<2;pad++){
  unsigned payload=1+(tail+1)/2,stride=pad?8:payload;
  size_t length=0x160+(channels-1)*stride+payload;
  memcpy(data,original,sizeof(data));w32(data+12,length);w32(data+0x28,length-0x140);
  unsigned char *h=data+0x40;h[0x21]=0;h[0x22]=(unsigned char)channels;
  w32(h+0x28,0);w32(h+0x2c,tail);w32(h+0x30,1);w32(h+0x34,8);w32(h+0x38,14);
  w32(h+0x3c,payload);w32(h+0x40,tail);w32(h+0x44,stride);w32(h+0x70,channels);
  memset(data+0x160,0x11,length-0x160);data[0x160]=0;if(channels==2){data[0x160+stride]=0;memset(data+0x160+stride+1,0xff,payload-1);}
  assert(bgm_parse(&s,data,length));assert(bgm_render(&s,pcm,tail+1)==tail);
  for(unsigned j=0;j<tail;j++){assert(pcm[2*j]==11+(int)j);assert(pcm[2*j+1]==(channels==2?-1:11+(int)j));}
  assert(!bgm_parse(&s,data,length-1));
  w32(h+0x3c,payload-1);assert(!bgm_parse(&s,data,length));
 }
 puts("PASS actual audio parser/decoder: 56 mono/stereo packed-tail cases, DSP ADPCM stereo and histories, exact mid-frame loop, blocked PCM16, mono WAV duplication, malformed/truncated/overflow inputs and 20000 mutations");
}
