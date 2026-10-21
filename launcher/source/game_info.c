// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "game_info.h"
#include "library_model.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
void game_display_title(char *out,size_t capacity,const char *name){
 if(!capacity)return;
 snprintf(out,capacity,"%s",name?name:"");
 size_t n=strlen(out);
 while(n&&out[n-1]==' '){out[--n]=0;}
 for(;;){
  if(!n||(out[n-1]!=')'&&out[n-1]!=']'))break;
  char *start=strrchr(out,out[n-1]==')'?'(':'[');
  if(!start)break;
  static const char *tags[]={"Europe","USA","World","Japan","Germany","France","En,","En)","Rev ","Beta","Wii U","Virtual Console","Proto","!","b","h","T+"};
  int metadata=out[n-1]==']';
  for(unsigned i=0;i<sizeof(tags)/sizeof(tags[0]);++i)if(!strncmp(start+1,tags[i],strlen(tags[i])))metadata=1;
  if(!metadata)break;
  *start=0;
  n=strlen(out);
  while(n&&out[n-1]==' ')out[--n]=0;
 }
}
void game_info_begin(GameInfo *info,const char *name){unsigned revision=info->revision+1;
memset(info,0,sizeof(*info));
info->revision=revision;
game_display_title(info->title,sizeof(info->title),name);}
void game_info_add(GameInfo *info,const char *label,const char *value){
 if(info->count>=GAME_INFO_FIELDS||!value||!*value)return;
 GameInfoField *f=&info->fields[info->count++];
 snprintf(f->label,sizeof(f->label),"%s",label);
 snprintf(f->value,sizeof(f->value),"%s",value);
}
void game_info_size(GameInfo *info,const char *label,uint64_t bytes){
 char text[80];
 snprintf(text,sizeof(text),"%.2f %s (%llu Bytes)",bytes>=1024*1024?bytes/(1024.*1024.):bytes/1024.,bytes>=1024*1024?"MiB":"KiB",(unsigned long long)bytes);
 game_info_add(info,label,text);
}
static void ascii_field(GameInfo *info,const char *label,const unsigned char *data,size_t count){
 char text[64];
 if(count>=sizeof(text))count=sizeof(text)-1;
 for(size_t i=0;i<count;++i){if(data[i]&&data[i]<32)return;
 text[i]=data[i]>=32&&data[i]<127?data[i]:' ';}text[count]=0;
 while(count&&text[count-1]==' ')text[--count]=0;
 if(count)game_info_add(info,label,text);
}
static int game_code(const unsigned char *p){for(unsigned i=0;i<4;++i)if(!((p[i]>='A'&&p[i]<='Z')||(p[i]>='0'&&p[i]<='9')))return 0;
return 1;}
void game_info_rom(GameInfo *info,int system,const char *path){
 FILE *f=fopen(path,"rb");
 if(!f)return;
 unsigned char h[512];
 size_t size=fread(h,1,sizeof(h),f);
 fclose(f);
 char number[40];
 if(system==SYSTEM_DS&&size>=0x160&&game_code(h+12)){ascii_field(info,"ROM-Titel",h,12);
 ascii_field(info,"Spielcode",h+12,4);
 ascii_field(info,"Herstellercode",h+16,2);
 snprintf(number,sizeof(number),"%u",h[30]);
 game_info_add(info,"ROM-Revision",number);}
 if(system==SYSTEM_GBA&&size>=0xc0&&h[0xb2]==0x96&&game_code(h+0xac)){ascii_field(info,"ROM-Titel",h+0xa0,12);
 ascii_field(info,"Spielcode",h+0xac,4);
 ascii_field(info,"Herstellercode",h+0xb0,2);
 snprintf(number,sizeof(number),"%u",h[0xbc]);
 game_info_add(info,"ROM-Revision",number);}
 if(system==SYSTEM_GB&&size>=0x150&&h[0x104]==0xce&&h[0x105]==0xed){ascii_field(info,"ROM-Titel",h+0x134,(h[0x143]&0x80)?11:15);
 game_info_add(info,"Hardware",(h[0x143]&0xc0)==0xc0?"Game Boy Color":(h[0x143]&0x80)?"Game Boy / Game Boy Color":"Game Boy");}
 if(system==SYSTEM_N64&&size>=0x40){
  const char *order=NULL;
  if(!memcmp(h,"\x80\x37\x12\x40",4))order="Big Endian (.z64)";
  else if(!memcmp(h,"\x37\x80\x40\x12",4)){order="Byte-swapped (.v64)";
  for(unsigned i=0;i<0x40;i+=2){unsigned char t=h[i];
  h[i]=h[i+1];
  h[i+1]=t;}}
  else if(!memcmp(h,"\x40\x12\x37\x80",4)){order="Little Endian (.n64)";
  for(unsigned i=0;i<0x40;i+=4){unsigned char a=h[i],b=h[i+1];
  h[i]=h[i+3];
  h[i+1]=h[i+2];
  h[i+2]=b;
  h[i+3]=a;}}
  if(order){ascii_field(info,"ROM-Titel",h+0x20,20);
  game_info_add(info,"ROM-Format",order);}
 }
 if(system==SYSTEM_NES&&size>=16&&!memcmp(h,"NES\x1a",4)){
  int newer=(h[7]&12)==8;
  game_info_add(info,"ROM-Format",newer?"NES 2.0":"iNES");
  unsigned mapper=(h[6]>>4)|(h[7]&0xf0)|(newer?(h[8]&15)<<8:0);
  snprintf(number,sizeof(number),"%u",mapper);
  game_info_add(info,"Mapper",number);
  if(!newer){game_info_size(info,"PRG-ROM",h[4]*16384u);
  game_info_size(info,"CHR-ROM",h[5]*8192u);}
 }
}
