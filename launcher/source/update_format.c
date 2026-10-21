// SPDX-License-Identifier: GPL-3.0-only
#include "update_format.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
static bool version(const char *s,unsigned v[3]){
 if(!s||!*s)return false;
 for(unsigned i=0;i<3;i++){
  if(!isdigit((unsigned char)*s))return false;
  unsigned n=0;do{n=n*10+(*s++-'0');if(n>999)return false;}while(isdigit((unsigned char)*s));
  v[i]=n;if(i<2){if(*s++!='.')return false;}else if(*s)return false;
 }return true;
}
int update_version_compare(const char *a,const char *b){unsigned x[3],y[3];if(!version(a,x)||!version(b,y))return -2;for(unsigned i=0;i<3;i++){if(x[i]>y[i])return 1;if(x[i]<y[i])return -1;}return 0;}
bool update_manifest_parse(const char *text,size_t size,UpdateManifest *out){
 if(!text||!out||size==0||size>=1024||memchr(text,0,size))return false;
 char buf[1024];memcpy(buf,text,size);buf[size]=0;UpdateManifest m={0};
 char *p=buf,*lines[5];for(unsigned i=0;i<5;i++){lines[i]=p;char *end=strchr(p,'\n');if(!end)return false;*end=0;p=end+1;}
 if(*p||strcmp(lines[0],"VFUPDATE1")||strncmp(lines[1],"version=",8)||strncmp(lines[2],"size=",5)||strncmp(lines[3],"sha256=",7)||strncmp(lines[4],"url=",4))return false;
 const char *v=lines[1]+8,*n=lines[2]+5,*hash=lines[3]+7,*url=lines[4]+4;
 if(strlen(v)>=sizeof(m.version)||update_version_compare(v,v)!=0||!*n)return false;
 unsigned long bytes=0;for(const char *q=n;*q;q++){if(!isdigit((unsigned char)*q))return false;bytes=bytes*10+(*q-'0');if(bytes>UPDATE_MAX_BYTES)return false;}
 if(bytes<32||strlen(hash)!=64)return false;
 for(unsigned i=0;i<64;i++)if(!isdigit((unsigned char)hash[i])&&(hash[i]<'a'||hash[i]>'f'))return false;
 char expected[240];snprintf(expected,sizeof(expected),UPDATE_URL_ROOT "%s/VitarenewFlow.3dsx",v);
 if(strcmp(url,expected))return false;
 strcpy(m.version,v);strcpy(m.url,url);strcpy(m.sha,hash);m.bytes=(uint32_t)bytes;*out=m;return true;
}

static bool notes_version_valid(const char *text){
 if(!text)return false;
 for(size_t i=0;i<sizeof(((UpdateNotes*)0)->version);i++){
  if(!text[i])return update_version_compare(text,text)==0;
 }
 return false;
}
bool update_notes_url(const char *v,char *out,size_t capacity){
 if(!out||!capacity)return false;
 out[0]=0;if(!notes_version_valid(v))return false;
 int n=snprintf(out,capacity,UPDATE_URL_ROOT "%s/notes.txt",v);
 if(n<0||(size_t)n>=capacity){out[0]=0;return false;}
 return true;
}
static bool notes_text_valid(const char *text){
 size_t n=strlen(text);if(!n||n>UPDATE_NOTES_LINE_BYTES)return false;
 bool content=false;const unsigned char *p=(const unsigned char*)text;
 for(size_t i=0;i<n;){
  uint32_t code=p[i++],minimum=0;unsigned next=0;
  if(code<0x80){}else if(code>=0xc2&&code<=0xdf){next=1;code&=0x1f;minimum=0x80;}
  else if(code>=0xe0&&code<=0xef){next=2;code&=0x0f;minimum=0x800;}
  else if(code>=0xf0&&code<=0xf4){next=3;code&=7;minimum=0x10000;}
  else return false;
  if(next>n-i)return false;
  while(next--){unsigned char b=p[i++];if((b&0xc0)!=0x80)return false;code=(code<<6)|(b&0x3f);}
  if(code<minimum||code>0x10ffff||(code>=0xd800&&code<=0xdfff)||code<32||
     (code>=127&&code<=159)||code==0x2028||code==0x2029||code==0xad||code==0x61c||
     code==0x180e||(code>=0x200b&&code<=0x200f)||(code>=0x202a&&code<=0x202e)||
     (code>=0x2060&&code<=0x206f)||code==0xfeff)return false;
  if(code!=32&&code!=0xa0&&!(code>=0x2000&&code<=0x200a)&&code!=0x202f&&code!=0x205f&&code!=0x3000)content=true;
 }
 return content;
}
static char *notes_line(char **cursor){
 char *line=*cursor,*end=strchr(line,'\n');if(!end)return NULL;
 *end=0;*cursor=end+1;return line;
}
bool update_notes_parse(const char *text,size_t size,const char *expected_version,UpdateNotes *out){
 if(!out)return false;
 memset(out,0,sizeof(*out));
 if(!text||!size||size>UPDATE_NOTES_MAX_BYTES||memchr(text,0,size)||!notes_version_valid(expected_version))return false;
 char buffer[UPDATE_NOTES_MAX_BYTES+1];memcpy(buffer,text,size);buffer[size]=0;
 char *cursor=buffer,*magic=notes_line(&cursor);if(!magic||strcmp(magic,"VFNOTES1"))return false;
 char *version_line=notes_line(&cursor);
 if(!version_line||strncmp(version_line,"version=",8)||strcmp(version_line+8,expected_version))return false;
 UpdateNotes notes={0};strcpy(notes.version,expected_version);
 while(*cursor){
  if(notes.count>=UPDATE_NOTES_MAX_LINES)return false;
  char *de=notes_line(&cursor),*en=notes_line(&cursor);
  if(!de||!en||strncmp(de,"de=",3)||strncmp(en,"en=",3)||!notes_text_valid(de+3)||!notes_text_valid(en+3))return false;
  strcpy(notes.de[notes.count],de+3);strcpy(notes.en[notes.count],en+3);notes.count++;
 }
 if(!notes.count)return false;
 *out=notes;return true;
}
