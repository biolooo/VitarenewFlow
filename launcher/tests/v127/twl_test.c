#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static KernelConfig config={1};KernelConfig *OS_KernelConfig=&config;
static bool newModel=true;static u8 region=2;static int fault,deleteFail=-1;
static u64 installed[20],deleted[20];static unsigned count,deletions;
Result APT_CheckNew3DS(bool *n){*n=newModel;return fault==1?-1:0;}
Result cfguInit(void){return fault==2?-2:0;}void cfguExit(void){}
Result CFGU_SecureInfoGetRegion(u8 *r){*r=region;return fault==3?-3:0;}
Result AM_GetTitleCount(FS_MediaType m,u32 *n){assert(m==MEDIATYPE_NAND);*n=fault==5?5000:count;return fault==4?-4:0;}
Result AM_GetTitleList(u32 *n,FS_MediaType m,u32 c,u64 *ids){assert(m==MEDIATYPE_NAND&&c==count);*n=fault==7?0:count;memcpy(ids,installed,count*sizeof(u64));return fault==6?-6:0;}
Result AM_GetTitleInfo(FS_MediaType m,u32 n,u64 *id,AM_TitleInfo *info){assert(m==MEDIATYPE_NAND&&n==1);if(fault==8)return -8;info->titleID=*id;return 0;}
Result AM_DeleteTitle(FS_MediaType m,u64 id){assert(m==MEDIATYPE_NAND&&deletions<20);deleted[deletions++]=id;return (int)deletions==deleteFail?-9:0;}
void app_log(const char *s){assert(s);}
#define AUDIT "/tmp/vf-twl-repair-test.log"
#define JOURNAL "/tmp/vf-twl-repair-test.pending"
#include "twl_repair.c"
static void reset(void){liveIntent=false;remove(JOURNAL);rmdir(JOURNAL);newModel=true;region=2;config.env_info=1;fault=0;deleteFail=-1;deletions=0;TwlReport r={.new3ds=true,.region=2};count=targets(&r,installed);installed[count++]=0x0004800546574452ULL;}
static unsigned progressCount,lastDone;
static void progressed(unsigned done,unsigned total,void *context){assert(context==&progressCount&&total==5&&done<=total);if(progressCount)assert(done==lastDone+1);lastDone=done;progressCount++;}
int main(void){
 TwlReport r;reset();installed[1]=0;twl_diagnose(&r);assert(r.known&&!r.missing&&r.present==0x1D);assert(!twl_repair_begin(&r)&&deletions==4);assert(!twl_repair_finish()&&!twl_repair_pending());
 reset();FILE *stale=fopen(JOURNAL,"wb");assert(stale);fputs("VF-TWL1 PENDING\n",stale);fclose(stale);installed[1]=0;assert(twl_repair_pending()&&!twl_repair_finish()&&!twl_repair_pending()&&deletions==0);
 reset();stale=fopen(JOURNAL,"wb");assert(stale);fputs("corrupt",stale);fclose(stale);assert(twl_repair_finish()<0&&twl_repair_pending()&&deletions==0);
 reset();twl_diagnose(&r);assert(r.known&&r.supported&&r.missing==0&&r.loader&&r.total==5);
 assert(twl_settings_title(&r)==0x0004001000022000ULL);
 assert(!twl_repair_pending());assert(!twl_repair_begin(&r));assert(deletions==5&&twl_repair_pending());
 for(unsigned i=0;i<5;i++)assert(deleted[i]==installed[i]);
 assert(twl_repair_begin(&r)<0&&deletions==5);assert(!twl_repair_finish()&&!twl_repair_pending());
 reset();twl_diagnose(&r);progressCount=0;assert(!twl_repair_begin_progress(&r,progressed,&progressCount));assert(progressCount==6&&lastDone==5);remove(JOURNAL);assert(twl_repair_pending()&&twl_repair_begin(&r)<0);
 reset();newModel=false;installed[4]=0x0004013800000102ULL;twl_diagnose(&r);assert(!r.missing&&!twl_repair_begin(&r)&&deleted[4]==installed[4]);
 for(unsigned rg=4;rg<=5;rg++){reset();region=rg;TwlReport t={.new3ds=true,.region=(u8)rg};count=targets(&t,installed);twl_diagnose(&r);assert(r.total==6&&!r.missing);assert(!twl_repair_begin(&r)&&deletions==6);assert(deleted[5]==(rg==4?0x00048005484E4443ULL:0x00048005484E444BULL));}
 reset();region=6;twl_diagnose(&r);assert(r.known&&!r.supported&&twl_repair_begin(&r)<0&&deletions==0);
 reset();config.env_info=0;twl_diagnose(&r);assert(!r.supported&&twl_repair_begin(&r)<0&&deletions==0);
 for(int f=1;f<=7;f++){reset();fault=f;twl_diagnose(&r);assert(!r.known&&twl_repair_begin(&r)<0&&deletions==0);}
 reset();count=0;twl_diagnose(&r);assert(r.known&&r.missing==4);assert(!twl_repair_begin(&r)&&deletions==0);assert(twl_repair_finish()<0&&twl_repair_pending());
 reset();deleteFail=2;twl_diagnose(&r);assert(twl_repair_begin(&r)<0&&deletions==2&&twl_repair_pending());assert(twl_repair_begin(&r)<0&&deletions==2);
 reset();twl_diagnose(&r);region=1;assert(twl_repair_begin(&r)<0&&deletions==0&&!twl_repair_pending());
 reset();twl_diagnose(&r);fault=8;assert(twl_repair_begin(&r)<0&&deletions==0&&twl_repair_pending());
 reset();FILE *f=fopen(JOURNAL,"wb");assert(f);fputs("corrupt",f);fclose(f);assert(twl_repair_pending());twl_diagnose(&r);assert(twl_repair_begin(&r)<0&&deletions==0);
 reset();assert(!mkdir(JOURNAL,0700));twl_diagnose(&r);assert(twl_repair_pending()&&twl_repair_begin(&r)<0&&deletions==0);
 reset();remove(AUDIT);assert(!mkdir(AUDIT,0700));twl_diagnose(&r);assert(twl_repair_begin(&r)<0&&deletions==0&&twl_repair_pending());rmdir(AUDIT);
 reset();puts("PASS actual TWL repair: exact NAND whitelist, old/new model, EUR/CHN/KOR, unsupported region/devkit, missing titles, query/list errors, model/region changes, partial deletion stops, persistent recovery and no destructive auto-resume");
}
