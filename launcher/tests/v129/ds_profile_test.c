#include <3ds.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
static bool failRename;static unsigned failClose;
static unsigned logs;
void app_handoff_log(const char *p){assert(p);logs++;}
static int checkedRename(const char *a,const char *b){if(failRename){errno=EIO;return -1;}return rename(a,b);}
static int checkedClose(FILE *f){int r=fclose(f);if(failClose&&!--failClose)return EOF;return r;}
#define rename checkedRename
#define fclose checkedClose
#include "ds_profile.c"
#undef rename
#undef fclose
static void put(const char *p,const void *data,size_t n){FILE*f=fopen(p,"wb");assert(f);assert(fwrite(data,1,n,f)==n);assert(!fclose(f));}
static void text(const char *p,const char *s){put(p,s,strlen(s));}
static void same(const char *p,const char *s){char buf[1024]={0};FILE*f=fopen(p,"rb");assert(f);size_t n=fread(buf,1,sizeof(buf),f);fclose(f);assert(n==strlen(s)&&!memcmp(buf,s,n));}
static void rom(const char *p,unsigned unit,const char *code,bool valid){unsigned char h[0x200]={0};memcpy(h+12,code,4);memcpy(h+16,"01",2);h[0x12]=unit;h[0x1e]=1;unsigned crc=header_crc(h,0x15e)+(valid?0:1);h[0x15e]=crc;h[0x15f]=crc>>8;put(p,h,sizeof(h));}
static void newdir(void){char dir[]="/tmp/vf-profile-v129-XXXXXX";assert(mkdtemp(dir));assert(!chdir(dir));mkdir("sdmc:",0700);mkdir("sdmc:/roms",0700);mkdir("sdmc:/_nds",0700);mkdir("sdmc:/_nds/ntr-forwarder",0700);mkdir("sdmc:/_nds/nds-bootstrap",0700);mkdir("sdmc:/_nds/nds-bootstrap/patchOffsetCache",0700);}
int main(void){
 newdir();const char *r="sdmc:/roms/Test game.nds";
 const char *p="sdmc:/_nds/ntr-forwarder/gamesettings/Test game.nds.ini";
 const char *s="sdmc:/_nds/ntr-forwarder/gamesettings/Test game.nds.ini.vf-stage";
 assert(ds_prepare_profile(r)==-31);text(r,"short");assert(ds_prepare_profile(r)==-31);
 rom(r,0,"ASME",true);unsigned char h[0x160];FILE*f=fopen(r,"rb");fread(h,1,sizeof(h),f);fclose(f);
 char cache[192],backup[224];snprintf(cache,sizeof(cache),"sdmc:/_nds/nds-bootstrap/patchOffsetCache/ASME-%04X.bin",h[0x15e]|h[0x15f]<<8);snprintf(backup,sizeof(backup),"%s.vf-ntr-v1-0.bak",cache);
 text(cache,"old patch cache");text("sdmc:/roms/Test game.sav","SAVE DATA");text("sdmc:/_nds/nds-bootstrap.ini","GLOBAL SETTINGS");
 assert(!ds_prepare_profile(r));same(p,profile);same(backup,"old patch cache");assert(access(cache,F_OK));
 text(cache,"new cache");assert(!ds_prepare_profile(r));same(cache,"new cache");same(backup,"old patch cache");
 text(p,"[GAMESETTINGS]\nSAVE_NUMBER=3\nDSI_MODE=2\n");assert(!ds_prepare_profile(r));same(p,"[GAMESETTINGS]\nSAVE_NUMBER=3\nDSI_MODE=2\n");same(cache,"new cache");
 same("sdmc:/roms/Test game.sav","SAVE DATA");same("sdmc:/_nds/nds-bootstrap.ini","GLOBAL SETTINGS");
 remove(p);rom(r,2,"IRBO",true);assert(!ds_prepare_profile(r)&&access(p,F_OK));rom(r,3,"KAAA",true);assert(!ds_prepare_profile(r)&&access(p,F_OK));
 rom(r,0,"####",true);assert(!ds_prepare_profile(r)&&access(p,F_OK));rom(r,0,"ASME",false);assert(!ds_prepare_profile(r)&&access(p,F_OK));
 rom(r,0,"ASME",true);text(s,"foreign data");assert(ds_prepare_profile(r)==-36);same(s,"foreign data");same(cache,"new cache");remove(s);
 put(s,profile,17);assert(!ds_prepare_profile(r));same(p,profile);same(backup,"old patch cache");assert(access(cache,F_OK));
 remove(p);text(cache,"third cache");text(s,profile);failRename=true;assert(ds_prepare_profile(r)==-36);same(cache,"third cache");assert(access(p,F_OK));failRename=false;
 assert(!ds_prepare_profile(r));same(p,profile);assert(access(s,F_OK));same(backup,"old patch cache");
 remove(p);assert(!mkdir(p,0700));assert(ds_prepare_profile(r)==-36);assert(!rmdir(p));
 text(s,profile);failClose=2;assert(ds_prepare_profile(r)==-36);same(s,profile); // failed readback leaves original stage
 assert(!ds_prepare_profile(r));assert(logs>10);
 puts("PASS real DS profile filesystem: NTR-only defaults, preserve DSi/homebrew/custom settings/global config/saves, one-time exact cache backup, repeated launches reuse cache, unique backup retention, interrupted write recovery, unknown stage protection, rename/readback failures");
}
