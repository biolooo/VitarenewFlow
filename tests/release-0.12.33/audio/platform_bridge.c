#include <3ds.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
static FILE *code;static int mode,opens;static int (*digest)(const void*,size_t,void*);
void test_digest(int(*f)(const void*,size_t,void*)){digest=f;}void test_mode(int v){mode=v;opens=0;}int test_opens(void){return opens;}
int mbedtls_sha256_ret(const unsigned char *d,size_t n,unsigned char *h,int v){assert(!v&&digest);return digest(d,n,h);}
void app_log(const char *s){assert(s);}
Result FSUSER_OpenFileDirectly(Handle *h,unsigned archive,FS_Path ap,FS_Path fp,unsigned flags,unsigned unused){assert(archive==ARCHIVE_SAVEDATA_AND_CONTENT&&ap.type==PATH_BINARY&&ap.size==16&&fp.size==20&&flags==FS_OPEN_READ&&!unused);const u32 *p=ap.data,*f=fp.data;assert(p[1]==0x40030&&p[2]==MEDIATYPE_NAND&&f[2]==2&&f[3]==0x646f632e&&f[4]==0x65);opens++;if(mode==1||p[0]!=0x9802)return -1;code=fopen("home-code.bin","rb");if(!code)return -1;*h=1;return 0;}
Result FSFILE_GetSize(Handle h,u64 *n){assert(h==1&&code);fseek(code,0,SEEK_END);*n=ftell(code);rewind(code);return 0;}
Result FSFILE_Read(Handle h,u32 *got,u64 off,void *out,u32 n){assert(h==1&&code&&!off);*got=fread(out,1,n,code);if(mode==2&&*got)(*got)--;return 0;}
Result FSFILE_Close(Handle h){assert(h==1&&code);fclose(code);code=NULL;return 0;}
