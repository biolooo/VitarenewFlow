// SPDX-License-Identifier: GPL-3.0-only
#include "audio_firmware.h"
#include "audio_firmware_format.h"
#include "app_log.h"
#include <3ds.h>
#include <mbedtls/sha256.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#define FIRM "sdmc:/3ds/dspfirm.cdc"
#define STAGE "sdmc:/3ds/dspfirm.cdc.flow-download"
static bool sha(const void *data,size_t size,uint8_t hash[32]){return mbedtls_sha256_ret(data,size,hash,0)==0;}
static bool valid_file(const char *path){
 FILE *f=fopen(path,"rb");if(!f)return false;
 bool ok=false;if(!fseek(f,0,SEEK_END)){long n=ftell(f);if(n>=0x300&&n<=0x10000&&!fseek(f,0,SEEK_SET)){uint8_t *d=malloc(n);if(d){ok=fread(d,1,n,f)==(size_t)n&&audio_dsp_valid(d,n,sha);free(d);}}}
 if(fclose(f))ok=false;
 return ok;
}
static bool store(const uint8_t *data,size_t size){
 if(!audio_dsp_valid(data,size,sha))return false;
 mkdir("sdmc:/3ds",0777);FILE *f=fopen(STAGE,"wb");if(!f)return false;
 bool ok=fwrite(data,1,size,f)==size;if(fflush(f)||ferror(f))ok=false;if(fclose(f))ok=false;
 if(!ok||!valid_file(STAGE)){remove(STAGE);return false;}
 struct stat st;char backup[128]={0};
 int existing=stat(FIRM,&st);if(existing&&errno!=ENOENT){remove(STAGE);return false;}
 if(!existing){
  if(!S_ISREG(st.st_mode)){remove(STAGE);return false;}
  unsigned i;for(i=0;i<100;i++){snprintf(backup,sizeof(backup),FIRM ".flow-invalid-%u",i);if(stat(backup,&st)){if(errno!=ENOENT){remove(STAGE);return false;}break;}}
  if(i==100||rename(FIRM,backup)){remove(STAGE);return false;}
 }
 if(rename(STAGE,FIRM)){if(*backup)rename(backup,FIRM);remove(STAGE);return false;}
 if(!valid_file(FIRM)){remove(FIRM);if(*backup)rename(backup,FIRM);return false;}
 app_log("Audio: local DSP component extracted, section hashes and SD readback verified");return true;
}
static bool find(const uint8_t *data,size_t size,bool (*cancelled)(void)){
 for(size_t at=0x100;at+0x200<=size;at+=4){
  if(!(at&0xffff)&&cancelled())return false;
  if(memcmp(data+at,"DSP1",4))continue;
  const uint8_t *d=data+at-0x100;size_t left=size-(at-0x100);if(left<0x300)continue;
  size_t n=d[0x104]|(uint32_t)d[0x105]<<8|(uint32_t)d[0x106]<<16|(uint32_t)d[0x107]<<24;
  if(n>=0x300&&n<=left&&n<=0x10000&&audio_dsp_valid(d,n,sha))return !cancelled()&&store(d,n);
 }
 return false;
}
bool audio_firmware_prepare(bool (*cancelled)(void)){
 if(valid_file(FIRM))return true;
 /* Read-only HOME .code; the component stays private to this console's SD. */
 const u32 lows[]={0x9802,0x8f02,0x8202,0xa102,0xa902,0xb102};
 for(unsigned i=0;i<sizeof(lows)/sizeof(*lows)&&!cancelled();i++){
  u32 archive[]={lows[i],0x00040030,MEDIATYPE_NAND,0},path[]={0,0,2,0x646f632e,0x65};
  Handle file;Result r=FSUSER_OpenFileDirectly(&file,ARCHIVE_SAVEDATA_AND_CONTENT,(FS_Path){PATH_BINARY,sizeof(archive),archive},(FS_Path){PATH_BINARY,sizeof(path),path},FS_OPEN_READ,0);
  if(R_FAILED(r))continue;
  u64 size=0;r=FSFILE_GetSize(file,&size);uint8_t *data=NULL;u32 got=0;
  if(R_SUCCEEDED(r)&&size>=8&&size<=16*1024*1024){data=malloc(size);if(data)r=FSFILE_Read(file,&got,0,data,(u32)size);}
  FSFILE_Close(file);bool ok=false;
  if(data&&R_SUCCEEDED(r)&&got==size&&!cancelled()){
   ok=find(data,size,cancelled);
   if(!ok&&!cancelled()){
    size_t capacity=audio_code_size(data,size);uint8_t *out=capacity?malloc(capacity):NULL;
    if(out){if(audio_code_unpack(data,size,out,capacity))ok=find(out,capacity,cancelled);free(out);}
   }
  }
  free(data);if(ok)return true;
 }
 app_log("Audio: DSP extraction unavailable; existing files preserved. Rosalina > Miscellaneous > Dump DSP firmware can diagnose permissions.");return false;
}
