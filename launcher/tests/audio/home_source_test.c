#include <3ds.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static bool stop;static int mountFail;static unsigned logs;
NS_APPID aptGetMenuAppID(void){return 1;}
Result APT_GetAppletInfo(NS_APPID a,u64 *id,u8 *media,bool *r,bool *l,APT_AppletAttr *t){(void)a;*id=0x0004003000009802ULL;*media=0;*r=*l=true;*t=0;return 0;}
Result romfsMountFromTitle(u64 id,FS_MediaType media,const char *n){assert(id==0x0004003000009802ULL&&!media&&!strcmp(n,"vfhome"));return mountFail?-1:0;}
Result romfsUnmount(const char *n){assert(!strcmp(n,"vfhome"));return 0;}
void app_log(const char *s){assert(s);logs++;}
#define SOURCE "/tmp/vf-home-source-test.bcsar"
#include "home_audio_source.c"
static bool cancelled_test(void){return stop;}
int main(void){
 remove(SOURCE);remove(SOURCE ".part");mkdir("vfhome:",0700);mkdir("vfhome:/sound",0700);
 FILE *f=fopen("vfhome:/sound/test.bcsar","wb");assert(f);unsigned char input[65536];memset(input,42,sizeof(input));memcpy(input,"CSAR",4);assert(fwrite(input,1,sizeof(input),f)==sizeof(input));fclose(f);
 home_audio_prepare_source(cancelled_test);assert(!access(SOURCE,F_OK));f=fopen(SOURCE,"rb");assert(f);unsigned char output[65536];assert(fread(output,1,sizeof(output),f)==sizeof(output)&&!memcmp(input,output,sizeof(input)));fclose(f);
 stop=true;home_audio_prepare_source(cancelled_test);assert(!access(SOURCE,F_OK));stop=false;remove(SOURCE);
 mountFail=1;home_audio_prepare_source(cancelled_test);assert(access(SOURCE,F_OK));mountFail=0;
 f=fopen("vfhome:/sound/test.bcsar","wb");assert(f);fputs("INVALID",f);fclose(f);home_audio_prepare_source(cancelled_test);assert(access(SOURCE,F_OK));
 assert(logs>=3);puts("PASS actual HOME source acquisition: read-only title guard/mount, bounded search, real file copy and CRC verification, existing cache preserved, cancellation, mount error and invalid source");
}
