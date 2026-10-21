// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "twl_repair.h"
#include "app_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#ifndef JOURNAL
#define JOURNAL "sdmc:/3ds/3ds-library/twl-repair.pending"
#endif
#ifndef AUDIT
#define AUDIT "sdmc:/3ds/3ds-library/twl-repair.log"
#endif
static bool audit(const char *line){FILE *f=fopen(AUDIT,"ab");if(!f)return false;bool ok=fprintf(f,"%s\n",line)>0;if(fclose(f))ok=false;return ok;}
static bool liveIntent;
static unsigned targets(const TwlReport *r,u64 *ids){
 ids[0]=0x0004800F484E4841ULL;ids[1]=0x0004800F484E4C41ULL;
 ids[2]=0x0004800542383841ULL;ids[3]=0x00048005484E4441ULL;
 ids[4]=r->new3ds?0x0004013820000102ULL:0x0004013800000102ULL;
 unsigned n=5;if(r->region==4)ids[n++]=0x00048005484E4443ULL;
 if(r->region==5)ids[n++]=0x00048005484E444BULL;
 return n;
}
static bool regular(const char *path){struct stat s;return !stat(path,&s)&&S_ISREG(s.st_mode)&&s.st_size>0;}
TwlAdvice twl_recommendation(const TwlReport *r){
 return r?twl_advice(r->known,r->supported,r->missing,r->loader,r->files,r->confirmedAllFail,r->confirmedDownloadPlayFail):TWL_CHECK_UNKNOWN;
}
void twl_diagnose(TwlReport *r){
 memset(r,0,sizeof(*r));r->region=255;
 r->files=regular("sdmc:/_nds/ntr-forwarder/sdcard.nds")&&regular("sdmc:/_nds/nds-bootstrap-release.nds");
 Result result=APT_CheckNew3DS(&r->new3ds);
 if(R_SUCCEEDED(result))result=cfguInit();
 if(R_SUCCEEDED(result)){result=CFGU_SecureInfoGetRegion(&r->region);cfguExit();}
 if(R_FAILED(result)){r->error=result;return;}
 /* Refuse development hardware and unhandled regions, including TWN. */
 r->supported=(OS_KernelConfig->env_info&1)!=0&&r->region<=5;
 u32 count=0,read=0;result=AM_GetTitleCount(MEDIATYPE_NAND,&count);
 if(R_FAILED(result)||count>4096){r->error=R_FAILED(result)?result:(Result)-120;return;}
 u64 *installed=calloc(count?count:1,sizeof(u64));if(!installed){r->error=(Result)-3;return;}
 result=AM_GetTitleList(&read,MEDIATYPE_NAND,count,installed);
 if(R_FAILED(result)||read!=count){free(installed);r->error=R_FAILED(result)?result:(Result)-120;return;}
 u64 ids[6];r->total=targets(r,ids);
 for(unsigned i=0;i<r->total;i++){bool found=false;for(u32 j=0;j<count;j++)if(installed[j]==ids[i])found=true;/* TWLFix lists deletion targets, not a launch prerequisite list.
    Version Data is not required to launch DS software; retain its presence
    for an explicitly requested repair, but never treat its absence as damage. */
  if(!found){if(i!=1)r->missing++;}else r->present|=1u<<i;}
 for(u32 j=0;j<count;j++)if(installed[j]==0x0004800546574452ULL)r->loader=true;
 free(installed);r->known=true;
 char line[112];snprintf(line,sizeof(line),"TWL diagnosis: new3ds=%u region=%u present=%02X missingCore=%u",r->new3ds,r->region,r->present,r->missing);app_log(line);
}
bool twl_repair_pending(void){
 if(liveIntent)return true;
 struct stat s;if(!stat(JOURNAL,&s))return true;
 return errno!=ENOENT;
}
Result twl_repair_begin_progress(const TwlReport *approved,void (*progress)(unsigned,unsigned,void*),void *context){
 if(twl_recommendation(approved)!=TWL_CHECK_REPAIR||twl_repair_pending())return (Result)-121;
 TwlReport now;twl_diagnose(&now);
 if(!now.known||!now.supported||now.missing||!now.loader||!now.files||now.region!=approved->region||now.new3ds!=approved->new3ds)return (Result)-121;
 /* Persist intent before the first NAND change. No destructive auto-resume. */
 FILE *f=fopen(JOURNAL,"wb");if(!f)return (Result)-122;
 bool ok=fputs("VF-TWL1 PENDING\n",f)>=0;if(fclose(f))ok=false;
 char data[32]={0};f=fopen(JOURNAL,"rb");if(!f)return (Result)-122;
 size_t size=fread(data,1,sizeof(data)-1,f);bool bad=ferror(f)!=0;fclose(f);
 if(!ok||bad||size!=16||strcmp(data,"VF-TWL1 PENDING\n"))return (Result)-122;
 liveIntent=true;
 char start[96];snprintf(start,sizeof(start),"TWL repair started: new3ds=%u region=%u present=%02X",now.new3ds,now.region,now.present);
 if(!audit(start))return (Result)-122;
 u64 ids[6];unsigned count=targets(&now,ids);Result first=0;
 if(progress)progress(0,count,context);
 for(unsigned i=0;i<count;i++){
  if(!(now.present&(1u<<i))){if(progress)progress(i+1,count,context);continue;}
  char attempt[64];snprintf(attempt,sizeof(attempt),"Attempt %016llX",(unsigned long long)ids[i]);if(!audit(attempt))return (Result)-122;
  AM_TitleInfo info;Result r=AM_GetTitleInfo(MEDIATYPE_NAND,1,&ids[i],&info);
  /* Missing titles need the update, too. Never delete arbitrary IDs/tickets. */
  if(R_SUCCEEDED(r))r=info.titleID==ids[i]?AM_DeleteTitle(MEDIATYPE_NAND,ids[i]):(Result)-120;
  char line[96];snprintf(line,sizeof(line),"TWL repair %016llX: %08lX",(unsigned long long)ids[i],(unsigned long)(u32)r);app_log(line);
  if(!audit(line))return (Result)-122;
  if(R_FAILED(r)){first=r;break;}
  if(progress)progress(i+1,count,context);
 }
 return first;
}
Result twl_repair_finish(void){
 TwlReport r;twl_diagnose(&r);if(!r.known||r.missing)return (Result)-123;
 /* Do not silently discard malformed or inaccessible recovery records. */
 char marker[32]={0};FILE *f=fopen(JOURNAL,"rb");if(!f)return (Result)-122;
 size_t n=fread(marker,1,sizeof(marker)-1,f);bool bad=ferror(f)!=0;fclose(f);
 if(bad||n!=16||strcmp(marker,"VF-TWL1 PENDING\n"))return (Result)-122;
 if(!audit("Core TWL titles present; repair marker acknowledged"))return (Result)-122;
 if(remove(JOURNAL))return (Result)-122;
 liveIntent=false;return 0;
}

u64 twl_settings_title(const TwlReport *r){
 if(!r||!r->known)return 0;
 const u64 ids[]={0x0004001000020000ULL,0x0004001000021000ULL,0x0004001000022000ULL,0x0004001000022000ULL,0x0004001000026000ULL,0x0004001000027000ULL};
 return r->region<6?ids[r->region]:0;
}

Result twl_repair_begin(const TwlReport *approved){return twl_repair_begin_progress(approved,NULL,NULL);}
