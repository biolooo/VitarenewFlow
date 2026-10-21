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
