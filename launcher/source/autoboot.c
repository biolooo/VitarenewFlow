// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "autoboot.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#define BASE "sdmc:/3ds/3ds-library/"
static const char *root="sdmc:/boot.3dsx",*loader=BASE "boot-dispatcher.3dsx",*original=BASE "autoboot-hbmenu.3dsx";
static const char *installed=BASE "autoboot-installed.3dsx";
static const char *config="sdmc:/luma/config.ini",*configBackup=BASE "autoboot-config.original.ini";
static const char *error="";
const char *autoboot_error(void){return error;}
static bool exists(const char *p){struct stat st;return !stat(p,&st)&&S_ISREG(st.st_mode);}
static bool valid3dsx(const char *p){FILE*f=fopen(p,"rb");if(!f)return false;char magic[4];bool ok=fread(magic,1,4,f)==4&&!memcmp(magic,"3DSX",4);fclose(f);return ok;}
static bool same(const char *a,const char *b){
 FILE *x=fopen(a,"rb"),*y=fopen(b,"rb");if(!x||!y){if(x)fclose(x);if(y)fclose(y);return false;}
 char ax[4096],by[4096];bool ok=true;
 for(;;){size_t n=fread(ax,1,sizeof(ax),x),m=fread(by,1,sizeof(by),y);if(n!=m||memcmp(ax,by,n)){ok=false;break;}if(!n)break;}
 if(ferror(x)||ferror(y))ok=false;
 fclose(x);fclose(y);return ok;
}
static bool copy(const char *from,const char *to){
 FILE *in=fopen(from,"rb");if(!in)return false;
 FILE *out=fopen(to,"wb");if(!out){fclose(in);return false;}
 char buf[8192];size_t n;bool ok=true;
 while((n=fread(buf,1,sizeof(buf),in)))if(fwrite(buf,1,n,out)!=n){ok=false;break;}
 if(ferror(in))ok=false;
 fclose(in);if(fclose(out))ok=false;
 return ok&&same(from,to);
}
/* Stage and verify before a FAT-compatible swap. Preserve interrupted swaps. */
static bool replace(const char *from,const char *to){
 char temp[256],old[256];snprintf(temp,sizeof(temp),"%s.vrflow-new",to);snprintf(old,sizeof(old),"%s.vrflow-old",to);
 if(exists(old)){error="Interrupted swap: restore .vrflow-old first";return false;}
 if(!copy(from,temp)){remove(temp);return false;}
 bool had=exists(to);
 if(had&&rename(to,old)){remove(temp);return false;}
 if(rename(temp,to)){if(had)rename(old,to);remove(temp);return false;}
 if(!same(from,to)){remove(to);if(had)rename(old,to);return false;}
 if(had)remove(old);
 return true;
}
/* Return exact span of a unique INI value, leaving comments/other keys untouched. */
static bool field(char *buf,const char *section,const char *key,char **start,char **end){
 bool inside=false,found=false;
 for(char *p=buf;*p;){char *next=strchr(p,'\n');if(!next)next=p+strlen(p);char *q=p;while(q<next&&isspace((unsigned char)*q))q++;
  if(q<next&&*q=='['){char *e=memchr(q,']',(size_t)(next-q));inside=e&&(size_t)(e-q-1)==strlen(section)&&!memcmp(q+1,section,strlen(section));}
  else if(inside&&q<next&&*q!=';'&&*q!='#'){
   char *eq=memchr(q,'=',(size_t)(next-q));if(eq){char *k=eq;while(k>q&&isspace((unsigned char)k[-1]))k--;
    if((size_t)(k-q)==strlen(key)&&!memcmp(q,key,strlen(key))){
     if(found)return false;
     char *v=eq+1;while(v<next&&isspace((unsigned char)*v))v++;
     char *e=v;while(e<next&&!isspace((unsigned char)*e)&&*e!=';'&&*e!='#')e++;
     *start=v;*end=e;found=true;
    }
   }
  }
  p=*next?next+1:next;
 }
 return found;
}
static bool matches(char *buf,const char *section,const char *key,const char *value){char *a,*b;return field(buf,section,key,&a,&b)&&(size_t)(b-a)==strlen(value)&&!memcmp(a,value,strlen(value));}
static char *readConfig(void){
 FILE*f=fopen(config,"rb");if(!f)return NULL;
 char *buf=malloc(65537);if(!buf){fclose(f);return NULL;}
 size_t n=fread(buf,1,65537,f);bool ok=!ferror(f)&&n<65537;fclose(f);
 if(!ok||memchr(buf,0,n)){free(buf);return NULL;}buf[n]=0;return buf;
}
static bool supported(char *buf){return matches(buf,"meta","config_version_major","3")&&matches(buf,"meta","config_version_minor","13")&&matches(buf,"autoboot","autoboot_3ds_app_mem_type","0");}
static bool managedBoot(void){return same(root,loader)||(valid3dsx(installed)&&same(root,installed));}
int autoboot_status(void){
 char *buf=readConfig();if(!buf)return -2;
 bool ok=supported(buf),on=matches(buf,"boot","autoboot_mode","3ds"),off=matches(buf,"boot","autoboot_mode","off");free(buf);
 if(!ok||(!on&&!off))return -2;
 if(on)return managedBoot()&&valid3dsx(original)?1:-1;
 return managedBoot()?-1:0;
}
bool autoboot_set(bool enabled){
 error="Autostart file operation failed";
 char *buf=readConfig();if(!buf)return false;
 if(!supported(buf)){error="Requires Luma config 3.13 and normal memory layout";free(buf);return false;}
 bool on=matches(buf,"boot","autoboot_mode","3ds"),off=matches(buf,"boot","autoboot_mode","off");
 if((enabled&&!off)||(!enabled&&!on&&!off)){error="Existing autoboot configuration conflicts";free(buf);return false;}
 if(!valid3dsx(loader)||!valid3dsx(root)){error="Missing or damaged boot files";free(buf);return false;}
 if(enabled){
  if(managedBoot()){error="Boot recovery required before enabling";free(buf);return false;}
  if(exists(original)&&!same(original,root)){error="Saved Homebrew Launcher differs; restore it first";free(buf);return false;}
  if(!exists(original)&&!copy(root,original)){free(buf);return false;}
  if(!exists(configBackup)&&!copy(config,configBackup)){free(buf);return false;}
  if(!copy(loader,installed)){free(buf);return false;}
 }else if(!valid3dsx(original)||(!managedBoot()&&!same(root,original))){error="Boot file changed externally; automatic restore refused";free(buf);return false;}
 char *a,*b;if(!field(buf,"boot","autoboot_mode",&a,&b)){free(buf);return false;}
 const char *value=enabled?"3ds":"off",*temp=BASE "autoboot-config.next.ini";
 FILE*f=fopen(temp,"wb");if(!f){free(buf);return false;}
 size_t head=(size_t)(a-buf),tail=strlen(b);bool ok=fwrite(buf,1,head,f)==head&&fwrite(value,1,strlen(value),f)==strlen(value)&&fwrite(b,1,tail,f)==tail;
 if(fclose(f))ok=false;
 free(buf);if(!ok){remove(temp);return false;}
 if(enabled){
  if(!replace(loader,root)){remove(temp);return false;}
  if(!replace(temp,config)){replace(original,root);remove(temp);return false;}
 }else{
  if(!replace(temp,config)){remove(temp);return false;}
  if(!replace(original,root)){remove(temp);return false;}
 }
 remove(temp);error="";return true;
}

static const char *homeResume=BASE "autoboot-home.resume";
bool autoboot_home_suspend(void){
 if(autoboot_status()!=1){error="Autostart not managed";return false;}
 FILE *f=fopen(homeResume,"wb");if(!f)return false;
 bool ok=fputs("VF-HOME1\n",f)>=0;if(fclose(f))ok=false;
 char buf[16]={0};f=fopen(homeResume,"rb");if(!f)return false;
 size_t n=fread(buf,1,sizeof(buf),f);bool bad=ferror(f);fclose(f);
 if(!ok||bad||n!=9||memcmp(buf,"VF-HOME1\n",9)){error="HOME resume marker verification failed";return false;}
 return autoboot_set(false);
}
int autoboot_home_resume(void){
 if(!exists(homeResume))return 0;
 char buf[16]={0};FILE *f=fopen(homeResume,"rb");if(!f)return -1;
 size_t n=fread(buf,1,sizeof(buf),f);bool bad=ferror(f);fclose(f);
 if(bad||n!=9||memcmp(buf,"VF-HOME1\n",9)){error="Invalid HOME resume marker";return -1;}
 int state=autoboot_status();
 if(state<0){error="HOME resume requires boot recovery";return -1;}
 if(state==0&&!autoboot_set(true))return -1;
 if(remove(homeResume)){error="Cannot acknowledge HOME resume";return -1;}
 return 1;
}
