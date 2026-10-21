// SPDX-License-Identifier: GPL-3.0-only
#include "home_audio_source.h"
#include "app_log.h"
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <zlib.h>
#ifndef SOURCE
#define SOURCE "sdmc:/3ds/3ds-library/music/home-source.bcsar"
#endif
static unsigned visited;static bool (*cancelled)(void);
static bool copy_archive(const char *path){
 FILE *in=fopen(path,"rb");if(!in)return false;
 unsigned char chunk[8192];bool ok=fread(chunk,1,8,in)==8&&!memcmp(chunk,"CSAR",4);
 if(fseek(in,0,SEEK_END))ok=false;
 long size=ftell(in);if(size<32||size>32*1024*1024)ok=false;
 if(!ok){fclose(in);return false;}rewind(in);
 FILE *out=fopen(SOURCE ".part","wb");if(!out){fclose(in);return false;}
 uLong hash=crc32(0,NULL,0);long total=0;size_t got;
 while(!cancelled()&&(got=fread(chunk,1,sizeof(chunk),in))){hash=crc32(hash,chunk,got);if(fwrite(chunk,1,got,out)!=got){ok=false;break;}total+=(long)got;}
 if(ferror(in)||total!=size)ok=false;
 fclose(in);if(fclose(out))ok=false;
 FILE *check=fopen(SOURCE ".part","rb");uLong verify=crc32(0,NULL,0);long read=0;
 if(check){while((got=fread(chunk,1,sizeof(chunk),check))){verify=crc32(verify,chunk,got);read+=(long)got;}if(ferror(check))ok=false;fclose(check);}else ok=false;
 if(!ok||verify!=hash||read!=size)return false;
 struct stat st;if(!stat(SOURCE,&st))return false;
 return rename(SOURCE ".part",SOURCE)==0;
}
static bool search(const char *path,unsigned depth){
 if(cancelled()||depth>4||visited>=128)return false;
 DIR *dir=opendir(path);if(!dir)return false;bool found=false;struct dirent *entry;
 while(!cancelled()&&!found&&visited<128&&(entry=readdir(dir))){
  if(entry->d_name[0]=='.')continue;
  visited++;char name[512];int n=snprintf(name,sizeof(name),"%s/%s",path,entry->d_name);if(n<0||(size_t)n>=sizeof(name))continue;
  struct stat st;if(stat(name,&st))continue;
  if(S_ISDIR(st.st_mode))found=search(name,depth+1);
  else if(S_ISREG(st.st_mode)){const char *ext=strrchr(name,'.');if(ext&&!strcmp(ext,".bcsar"))found=copy_archive(name);}
 }
 closedir(dir);return found;
}
void home_audio_prepare_source(bool (*shouldCancel)(void)){
 cancelled=shouldCancel;if(!cancelled||cancelled())return;
 struct stat st;if(!stat(SOURCE,&st))return;
 u64 title=0;u8 media=0;bool registered=false,loaded=false;APT_AppletAttr attributes;
 Result r=APT_GetAppletInfo(aptGetMenuAppID(),&title,&media,&registered,&loaded,&attributes);
 if(R_FAILED(r)||!title||(u32)(title>>32)!=0x00040030u){app_log("HOME audio source: HOME title unavailable; launch via HOME Menu for diagnosis");return;}
 r=romfsMountFromTitle(title,(FS_MediaType)media,"vfhome");
 if(R_FAILED(r)){char error[120];snprintf(error,sizeof(error),"HOME audio mount: title=%016llX media=%u error=%08lX",(unsigned long long)title,media,(unsigned long)(u32)r);app_log(error);return;}
 mkdir("sdmc:/3ds/3ds-library/music",0777);visited=0;
 bool found=search("vfhome:/sound",0);if(!found){visited=0;found=search("vfhome:",0);}
 romfsUnmount("vfhome");
 app_log(found?"HOME audio source: private source cached for native sequence-player development":"HOME audio source: no readable sound archive found");
}
