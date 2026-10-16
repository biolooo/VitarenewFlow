#include "game_seed.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static int policy,checks;static int mode,network,closed,exited,added;static u32 command[16];static Handle fs=42;
void app_log(const char *text){assert(text&&*text);}
Result cfguInit(void){return 0;}void cfguExit(void){}
Result CFGU_SecureInfoGetRegion(u8 *region){*region=2;return 0;}
Result httpcInit(u32 n){assert(n==0x1000);network++;return mode==6?-106:0;}
void httpcExit(void){exited++;}
Result httpcOpenContext(httpcContext *c,int method,const char *url,int proxy){(void)c;assert(method==HTTPC_METHOD_GET&&proxy==1);assert(!strcmp(url,"https://kagiya-ctr.cdn.nintendo.net/title/0x0004000001234500/ext_key?country=GB"));return 0;}
Result httpcBeginRequest(httpcContext *c){(void)c;return 0;}
Result httpcAddDefaultCert(httpcContext *c,int id){(void)c;assert(id==6);return mode==8?-108:0;}
Result httpcGetResponseStatusCodeTimeout(httpcContext *c,u32 *s,u64 timeout){(void)c;assert(timeout==10000000000ULL);*s=mode==1?404:mode==2?500:200;return 0;}
Result httpcReceiveDataTimeout(httpcContext *c,u8 *out,u32 n,u64 timeout){(void)c;(void)timeout;assert(n==17);memset(out,0xAB,17);return mode==5?-105:0;}
Result httpcGetDownloadSizeState(httpcContext *c,u32 *n,u32 *total){(void)c;*n=*total=mode==3?15:mode==4?17:16;return 0;}
Result httpcCloseContext(httpcContext *c){(void)c;closed++;return 0;}
u32 *getThreadCommandBuffer(void){return command;}Handle *fsGetSessionHandle(void){return &fs;}
Result svcSendSyncRequest(Handle h){assert(h==42);
if(command[0]==0x087F0100){assert(command[1]==1&&command[2]==0x01234500&&command[3]==0x00040000&&command[4]==0);command[1]=policy==5?(u32)-205:0;command[2]=policy!=0;return 0;}
if(command[0]==0x088500C0){assert(command[1]==1&&command[2]==0x01234500&&command[3]==0x00040000);checks++;command[1]=policy==1?0:policy==4?(u32)-204:added&&policy!=3?0:0xC8804464;return 0;}
assert(command[0]==0x087A0180&&command[1]==0x01234500&&command[2]==0x00040000);for(unsigned i=3;i<7;i++)assert(command[i]==0xABABABAB);added++;command[1]=mode==7?(u32)-107:0;return 0;}
static void reset(int m){mode=m;network=closed=exited=added=checks=0;}
static void seed_file(unsigned n){FILE *f=fopen("sdmc:/fbi/seed/0004000001234500.dat","wb");assert(f);for(unsigned i=0;i<n;i++)fputc(0xAB,f);assert(!fclose(f));}
int main(void){
 mkdir("seed-fixture",0777);assert(!chdir("seed-fixture"));mkdir("sdmc:",0777);mkdir("sdmc:/fbi",0777);mkdir("sdmc:/fbi/seed",0777);
 const u64 id=0x0004000001234500ULL;unlink("sdmc:/fbi/seed/0004000001234500.dat");
 reset(0);assert(game_seed_import(0x0004013820000002ULL)==SEED_BAD_DATA&&!network&&!added);
 reset(0);assert(!game_seed_import(id)&&added==1&&network==1&&closed==1&&exited==1);
 for(int m=1;m<=8;m++){reset(m);Result r=game_seed_import(id);assert(m==1?r==SEED_NOT_OFFERED:R_FAILED(r));assert(added==(m==7));assert(closed==(m==6?0:1));assert(exited==(m==6?0:1));}
 seed_file(16);reset(0);assert(!game_seed_import(id)&&added==1&&!network);
 seed_file(15);reset(0);assert(game_seed_import(id)==SEED_BAD_DATA&&!added&&!network);
 seed_file(17);reset(0);assert(game_seed_import(id)==SEED_BAD_DATA&&!added&&!network);
 unlink("sdmc:/fbi/seed/0004000001234500.dat");
 u32 phase=99;
 policy=0;reset(0);assert(!game_seed_ensure(id,MEDIATYPE_SD,&phase)&&!network&&!checks&&phase==0);
 policy=1;reset(0);assert(!game_seed_ensure(id,MEDIATYPE_SD,&phase)&&!network&&checks==1);
 policy=2;reset(0);assert(!game_seed_ensure(id,MEDIATYPE_SD,&phase)&&network==1&&added==1&&checks==2&&phase==2);
 policy=2;reset(6);assert(R_FAILED(game_seed_ensure(id,MEDIATYPE_SD,&phase))&&phase==1&&!added&&checks==1);
 policy=2;reset(1);assert(R_FAILED(game_seed_ensure(id,MEDIATYPE_SD,&phase))&&phase==1&&!added);
 policy=3;reset(0);assert(R_FAILED(game_seed_ensure(id,MEDIATYPE_SD,&phase))&&checks==2&&phase==2);
 policy=4;reset(0);assert(game_seed_ensure(id,MEDIATYPE_SD,&phase)==-204&&!network&&phase==0);
 policy=5;reset(0);assert(game_seed_ensure(id,MEDIATYPE_SD,&phase)==-205&&!network&&!checks);
 puts("PASS preflight: no-seed and valid offline, automatic repair, offline/404 blocked, post-import verification, query and unrelated validation failures");
 puts("PASS seed: exact 16-byte local/online payload, Nintendo URL/region, FS AddSeed IPC, no request for unsupported titles, 404 vs server/network/length/import failures, balanced HTTP cleanup");
}
