// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "cartridge.h"
#include "catalog_format.h"
#include <stdio.h>
#include <string.h>
static u32 le32(const u8 *p){return (u32)p[0]|(u32)p[1]<<8|(u32)p[2]<<16|(u32)p[3]<<24;}
static unsigned morton(unsigned x,unsigned y){unsigned v=0;for(unsigned i=0;i<3;i++){v|=((x>>i)&1)<<(2*i);v|=((y>>i)&1)<<(2*i+1);}return v;}
Result cartridge_read(Cartridge *c,unsigned language){
 memset(c,0,sizeof(*c));bool inserted=false;Result r=FSUSER_CardSlotIsInserted(&inserted);if(R_FAILED(r)||!inserted)return r;
 FS_CardType type;r=FSUSER_GetCardType(&type);if(R_FAILED(r))return r;
 c->ds=type==CARD_TWL;if(type!=CARD_CTR&&type!=CARD_TWL)return (Result)-100;
 if(!c->ds){
  u32 read=0;r=AM_GetTitleList(&read,MEDIATYPE_GAME_CARD,1,&c->id);if(R_FAILED(r)||read!=1||!c->id)return R_FAILED(r)?r:(Result)-101;
  AM_TitleInfo info={0};if(R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_GAME_CARD,1,&c->id,&info)))c->size=info.size;
  snprintf(c->name,sizeof(c->name),"3DS %016llX",(unsigned long long)c->id);
 }else{
  u8 header[0x3b4]={0};r=FSUSER_GetLegacyRomHeader(MEDIATYPE_GAME_CARD,0,header);if(R_FAILED(r))return r;
  for(unsigned i=0;i<4;i++){if(header[12+i]<'0'||header[12+i]>'Z')return (Result)-102;c->code[i]=(char)header[12+i];}
  for(unsigned i=0;i<12&&header[i];i++)c->name[i]=header[i]>=32&&header[i]<127?(char)header[i]:' ';
  c->size=le32(header+0x80);if(c->size<0x200||c->size>0x80000000ULL)c->size=0;
  u8 banner[0x23c0]={0};
  if(R_SUCCEEDED(FSUSER_GetLegacyBannerData(MEDIATYPE_GAME_CARD,0,banner))){
   unsigned locale=language<6?language:1;catalog_text(c->name,sizeof(c->name),banner+0x240+locale*0x100,128);
   for(unsigned y=0;y<48;y++)for(unsigned x=0;x<48;x++){
    unsigned sx=x*32/48,sy=y*32/48,src=((sy/8)*4+sx/8)*64+(sy%8)*8+sx%8;
    unsigned pal=(banner[0x20+src/2]>>((src%2)*4))&15;u16 color=(u16)banner[0x220+pal*2]|(u16)banner[0x221+pal*2]<<8;
    c->icon[((y/8)*6+x/8)*64+morton(x%8,y%8)]=(u16)(((color&31)<<11)|(((color>>5)&31)<<6)|((color>>10)&31));
   }c->hasIcon=true;
  }
  if(!c->name[0])snprintf(c->name,sizeof(c->name),"DS %.4s",c->code);
 }
 c->present=true;return 0;
}
