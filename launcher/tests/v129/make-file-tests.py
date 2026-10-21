from pathlib import Path
s=Path('outputs/3ds-library/source/ds_backend.c').read_text();s=s[:s.index('Result ds_install_bootstrap')]
pre=r'''
#include <3ds.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
void app_handoff_log(const char *p){assert(p);}
static int queryError;
Result AM_GetTitleInfo(FS_MediaType m,u32 n,u64 *id,AM_TitleInfo *out){assert(m==MEDIATYPE_NAND&&n==1&&*id==0x0004800546574452ULL);out->titleID=*id;return queryError;}
'''
tail=r'''
static void writeBytes(const char *p,unsigned n){FILE*f=fopen(p,"wb");assert(f);for(unsigned i=0;i<n;i++)fputc(i&255,f);assert(!fclose(f));}
int main(void){
 char dir[]="/tmp/vf-ds-v129-XXXXXX";assert(mkdtemp(dir));assert(!chdir(dir));mkdir("sdmc:",0700);mkdir("sdmc:/roms",0700);mkdir("sdmc:/roms/nds",0700);mkdir("sdmc:/_nds",0700);mkdir("sdmc:/_nds/ntr-forwarder",0700);
 const char*rom="sdmc:/roms/nds/Test game (Europe).nds";
 queryError=-10;assert(ds_prepare_launch(rom)==-30);queryError=0;assert(ds_prepare_launch(rom)==-31);writeBytes(rom,0x100);assert(ds_prepare_launch(rom)==-31);writeBytes(rom,0x400);assert(ds_prepare_launch(rom)==-32);
 writeBytes("sdmc:/_nds/ntr-forwarder/sdcard.nds",100);writeBytes("sdmc:/_nds/nds-bootstrap-release.nds",100);assert(!ds_prepare_launch(rom));
 char buf[256]={0};FILE*f=fopen("sdmc:/_nds/ntr-forwarder/path.txt","rb");assert(f);fread(buf,1,sizeof(buf)-1,f);fclose(f);assert(!strcmp(buf,"sd:/roms/nds/Test game (Europe).nds"));
 unsigned char h[0x400]={0};memcpy(h+12,"ASME01",6);unsigned crc=header_crc(h,0x15e);h[0x15e]=crc;h[0x15f]=crc>>8;
 f=fopen(rom,"wb");assert(f&&fwrite(h,1,sizeof(h),f)==sizeof(h));fclose(f);assert(!ds_prepare_launch(rom));
 const char *profilePath="sdmc:/_nds/ntr-forwarder/gamesettings/Test game (Europe).nds.ini";assert(exact_profile(profilePath));
 remove(profilePath);assert(!mkdir(profilePath,0700));assert(ds_prepare_launch(rom)==-36);assert(!rmdir(profilePath));
 remove("sdmc:/_nds/ntr-forwarder/path.txt");assert(!mkdir("sdmc:/_nds/ntr-forwarder/path.txt",0700));assert(ds_prepare_launch(rom)==-34);
 puts("PASS actual DS preflight: installed bootstrap check, missing/truncated ROM, missing runtime, spaced path encoding and verified handoff file, write failure");
}
'''
Path('work/v0138/ds_preflight_test.c').write_text(pre+'\n#include "ds_profile.c"\n'+s+tail)
Path('work/v0138/log_test.c').write_text(r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "app_log.c"
int main(void){char dir[]="/tmp/vf-logs-v129-XXXXXX";assert(mkdtemp(dir)&&!chdir(dir));mkdir("sdmc:",0700);mkdir("sdmc:/3ds",0700);mkdir("sdmc:/3ds/3ds-library",0700);app_log_begin();app_handoff_log("Handoff failure 1234");app_log_begin();FILE*f=fopen("sdmc:/3ds/3ds-library/handoff.log","rb");assert(f);char buf[2048]={0};fread(buf,1,sizeof(buf)-1,f);fclose(f);assert(strstr(buf,"Handoff failure 1234"));f=fopen("sdmc:/3ds/3ds-library/handoff.log","ab");assert(f);for(unsigned i=0;i<262145;i++)fputc('x',f);fclose(f);app_handoff_log("after rotation");struct stat st;assert(!stat("sdmc:/3ds/3ds-library/handoff.previous.log",&st)&&st.st_size>262144);assert(!stat("sdmc:/3ds/3ds-library/handoff.log",&st)&&st.st_size<100);puts("PASS persistent handoff log: failure survives new session, bounded rotation retains previous log");}
''')
