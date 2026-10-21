#include <3ds.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "theme_io.h"
static unsigned char save[0x141c],body[0xc4],bgm[64];static u32 manage[0x800/4];static bool homeFail,bgmFail;static unsigned reads;
FS_Path fsMakePath(unsigned n,const char *s){return(FS_Path){n,0,s};}
Result cfguInit(void){return 0;}Result CFGU_SecureInfoGetRegion(u8 *r){*r=2;return 0;}void cfguExit(void){}
Result FSUSER_OpenArchive(FS_Archive *a,int t,FS_Path p){assert(t==1&&p.type==PATH_BINARY);const u32 *v=p.data;if(v[1]==0x98){if(homeFail)return -1;*a=1;}else {assert(v[1]==0x2ce);*a=2;}return 0;}
Result FSUSER_CloseArchive(FS_Archive a){(void)a;return 0;}
Result FSUSER_OpenFile(Handle *f,FS_Archive a,FS_Path p,unsigned flags,unsigned attrib){(void)a;(void)attrib;assert(flags==FS_OPEN_READ);const char *s=p.data;if(!strcmp(s,"/SaveData.dat"))*f=1;else if(!strcmp(s,"/ThemeManage.bin"))*f=2;else if(!strcmp(s,"/BodyCache.bin"))*f=3;else if(!strcmp(s,"/BgmCache.bin")){if(bgmFail)return -1;*f=4;}else return -1;return 0;}
Result FSFILE_Read(Handle h,u32 *n,u64 off,void *out,u32 bytes){assert(!off);const void *src=h==1?(const void*)save:h==2?(const void*)manage:h==3?(const void*)body:(const void*)bgm;unsigned len=h==1?sizeof(save):h==2?sizeof(manage):h==3?sizeof(body):sizeof(bgm);assert(bytes<=len);if(h==4)reads++;memcpy(out,src,bytes);*n=bytes;return 0;}
Result FSFILE_Close(Handle h){(void)h;return 0;}
int main(void){uint8_t *data;size_t size;save[0x13bd]=3;body[5]=1;manage[2]=sizeof(body);manage[3]=sizeof(bgm);manage[7]=1;memcpy(bgm,"CSTM",4);
 assert(theme_read_music(&data,&size)==1&&data&&size==64&&reads==1);free(data);
 save[0x13bd]=0;assert(!theme_read_music(&data,&size)&&!data&&reads==1);save[0x13bd]=3;
 save[0x141b]=1;assert(!theme_read_music(&data,&size)&&reads==1);save[0x141b]=0;
 homeFail=true;assert(!theme_read_music(&data,&size)&&reads==1);homeFail=false;
 body[5]=0;assert(!theme_read_music(&data,&size)&&reads==1);body[5]=1;
 manage[3]=3371009;assert(!theme_read_music(&data,&size)&&reads==1);manage[3]=sizeof(bgm);
 bgmFail=true;assert(!theme_read_music(&data,&size)&&!data);bgmFail=false;
 assert(theme_read_music(&data,&size)==1&&reads==2);free(data);
 puts("PASS actual theme BGM selection: enabled active cache, basic theme, shuffle/unknown state never uses stale cache, silent theme, oversized/unreadable audio and recovery; extdata read-only");}
