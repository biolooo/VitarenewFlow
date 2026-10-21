#define _POSIX_C_SOURCE 200809L
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
Result AM_GetTitleInfo(FS_MediaType m,u32 n,u64 *id,AM_TitleInfo *info){assert(m==MEDIATYPE_NAND&&n==1);if(fault==8)return -8;info->titleID=fault==9?7:*id;return 0;}
Result AM_DeleteTitle(FS_MediaType m,u64 id){assert(m==MEDIATYPE_NAND&&deletions<20);deleted[deletions++]=id;return (int)deletions==deleteFail?-9:0;}
void app_log(const char *s){assert(s);}
#define AUDIT "repair.log"
#define JOURNAL "repair.pending"
#include "twl_repair.c"
static void file(const char *path){FILE *f=fopen(path,"wb");assert(f);assert(fputc(1,f)!=EOF);assert(!fclose(f));}
static void reset(void){file("sdmc:/_nds/ntr-forwarder/sdcard.nds");file("sdmc:/_nds/nds-bootstrap-release.nds");remove(AUDIT);rmdir(AUDIT);liveIntent=false;remove(JOURNAL);rmdir(JOURNAL);newModel=true;region=2;config.env_info=1;fault=0;deleteFail=-1;deletions=0;TwlReport r={.new3ds=true,.region=2};count=targets(&r,installed);installed[count++]=0x0004800546574452ULL;}
static unsigned progressCount,lastDone;
static void progressed(unsigned done,unsigned total,void *context){assert(context==&progressCount&&total==5&&done<=total);if(progressCount)assert(done==lastDone+1);lastDone=done;progressCount++;}

static void consent(TwlReport *r){r->confirmedAllFail=r->confirmedDownloadPlayFail=true;}
static void blocked(TwlReport *r){assert(twl_repair_begin(r)<0&&deletions==0&&!twl_repair_pending());}
int main(void){
 char temp[]="/tmp/flow-twl-XXXXXX";assert(mkdtemp(temp)&&!chdir(temp));
 assert(!mkdir("sdmc:",0700));assert(!mkdir("sdmc:/_nds",0700));assert(!mkdir("sdmc:/_nds/ntr-forwarder",0700));
 TwlReport r;reset();twl_diagnose(&r);assert(r.known&&r.supported&&r.loader&&r.files&&!r.missing&&twl_recommendation(&r)==TWL_CHECK_CLEAR);blocked(&r);
 r.confirmedAllFail=true;blocked(&r);r.confirmedAllFail=false;r.confirmedDownloadPlayFail=true;blocked(&r);
 consent(&r);assert(twl_recommendation(&r)==TWL_CHECK_REPAIR);progressCount=lastDone=0;assert(!twl_repair_begin_progress(&r,progressed,&progressCount));assert(progressCount==6&&lastDone==5&&deletions==5&&twl_repair_pending());
 for(unsigned i=0;i<5;i++)assert(deleted[i]==installed[i]);assert(twl_repair_begin(&r)<0&&deletions==5);assert(!twl_repair_finish()&&!twl_repair_pending());
 reset();installed[1]=0;twl_diagnose(&r);assert(r.present==0x1D&&!r.missing&&twl_recommendation(&r)==TWL_CHECK_CLEAR);blocked(&r);consent(&r);assert(!twl_repair_begin(&r)&&deletions==4);assert(!twl_repair_finish());
 for(unsigned i=0;i<5;i++)if(i!=1){reset();installed[i]=0;twl_diagnose(&r);assert(r.missing==1&&twl_recommendation(&r)==TWL_CHECK_UPDATE);consent(&r);blocked(&r);}
 reset();count=0;twl_diagnose(&r);assert(r.missing==4&&twl_recommendation(&r)==TWL_CHECK_UPDATE);consent(&r);blocked(&r);
 reset();count--;twl_diagnose(&r);assert(!r.loader&&!r.missing&&twl_recommendation(&r)==TWL_CHECK_LOADER);consent(&r);blocked(&r);
 reset();assert(!unlink("sdmc:/_nds/nds-bootstrap-release.nds"));twl_diagnose(&r);assert(!r.files&&twl_recommendation(&r)==TWL_CHECK_LOADER);consent(&r);blocked(&r);
 reset();FILE *empty=fopen("sdmc:/_nds/ntr-forwarder/sdcard.nds","wb");assert(empty);fclose(empty);twl_diagnose(&r);assert(!r.files);consent(&r);blocked(&r);
 reset();assert(!unlink("sdmc:/_nds/ntr-forwarder/sdcard.nds"));assert(!mkdir("sdmc:/_nds/ntr-forwarder/sdcard.nds",0700));twl_diagnose(&r);assert(!r.files);consent(&r);blocked(&r);assert(!rmdir("sdmc:/_nds/ntr-forwarder/sdcard.nds"));
 for(int f=1;f<=7;f++){reset();fault=f;twl_diagnose(&r);assert(!r.known&&twl_recommendation(&r)==TWL_CHECK_UNKNOWN);consent(&r);blocked(&r);}
 reset();region=6;twl_diagnose(&r);assert(twl_recommendation(&r)==TWL_CHECK_UNSUPPORTED);consent(&r);blocked(&r);
 reset();config.env_info=0;twl_diagnose(&r);assert(twl_recommendation(&r)==TWL_CHECK_UNSUPPORTED);consent(&r);blocked(&r);
 for(unsigned model=0;model<2;model++)for(unsigned rg=0;rg<6;rg++){
  reset();newModel=model;region=(u8)rg;TwlReport t={.new3ds=model,.region=(u8)rg};count=targets(&t,installed);unsigned n=count;installed[count++]=0x0004800546574452ULL;installed[count++]=0x0004000001234000ULL;
  twl_diagnose(&r);assert(twl_recommendation(&r)==TWL_CHECK_CLEAR);consent(&r);assert(!twl_repair_begin(&r)&&deletions==n);for(unsigned i=0;i<n;i++)assert(deleted[i]==installed[i]);assert(!twl_repair_finish());
 }
 reset();twl_diagnose(&r);consent(&r);region=1;blocked(&r);
 reset();twl_diagnose(&r);consent(&r);newModel=false;blocked(&r);
 reset();twl_diagnose(&r);consent(&r);installed[0]=0;blocked(&r);
 reset();twl_diagnose(&r);consent(&r);assert(!unlink("sdmc:/_nds/nds-bootstrap-release.nds"));blocked(&r);
 reset();twl_diagnose(&r);consent(&r);fault=4;blocked(&r);
 reset();twl_diagnose(&r);consent(&r);deleteFail=2;assert(twl_repair_begin(&r)<0&&deletions==2&&twl_repair_pending());assert(twl_repair_begin(&r)<0&&deletions==2);
 reset();twl_diagnose(&r);consent(&r);fault=8;assert(twl_repair_begin(&r)<0&&!deletions&&twl_repair_pending());
 reset();twl_diagnose(&r);consent(&r);fault=9;assert(twl_repair_begin(&r)<0&&!deletions&&twl_repair_pending());
 reset();file(JOURNAL);assert(twl_repair_pending());twl_diagnose(&r);consent(&r);assert(twl_repair_begin(&r)<0&&!deletions);assert(twl_repair_finish()<0&&twl_repair_pending());
 reset();assert(!mkdir(JOURNAL,0700));twl_diagnose(&r);consent(&r);assert(twl_repair_pending()&&twl_repair_begin(&r)<0&&!deletions);
 reset();assert(!mkdir(AUDIT,0700));twl_diagnose(&r);consent(&r);assert(twl_repair_begin(&r)<0&&!deletions&&twl_repair_pending());assert(!rmdir(AUDIT));
 reset();FILE *stale=fopen(JOURNAL,"wb");assert(stale);assert(fputs("VF-TWL1 PENDING\n",stale)>=0);fclose(stale);installed[1]=0;assert(twl_repair_pending()&&!twl_repair_finish()&&!twl_repair_pending()&&!deletions);
 for(unsigned bits=0;bits<64;bits++)for(unsigned missing=0;missing<2;missing++){
  bool known=bits&1,supported=bits&2,loader=bits&4,files=bits&8,all=bits&16,dlp=bits&32;
  TwlAdvice a=twl_advice(known,supported,missing,loader,files,all,dlp);
  assert((a==TWL_CHECK_REPAIR)==(known&&supported&&!missing&&loader&&files&&all&&dlp));
 }
 reset();puts("PASS actual TWL code: consent required, healthy/version-data absence not damage, loader vs core update guidance, stale-report revalidation, exact regional/model deletion whitelist, persistent journal, no destructive auto-resume, IO failures and 128 recommendation combinations");
}
