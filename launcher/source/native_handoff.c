#include "native_handoff.h"
#include "catalog_format.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
/* Read only. Respect the user's configured Rosalina takeover application. */
Result native_handoff_target(u64 *id){
 FILE *f=fopen("sdmc:/luma/config.ini","rb");if(!f)return FLOW_ERROR_HBL_CONFIG;
 char line[512];bool rosalina=false,found=false,bad=false;u64 target=0;
 while(fgets(line,sizeof(line),f)){
  if(!strchr(line,'\n')&&!feof(f)){bad=true;break;}
  char *p=line;while(isspace((unsigned char)*p))p++;
  if(*p==';'||*p=='#')continue;
  if(*p=='['){char *end=strchr(p,']');if(!end){bad=true;break;}*end=0;rosalina=!strcmp(p+1,"rosalina");continue;}
  if(!rosalina)continue;
  char *eq=strchr(p,'=');if(!eq)continue;
  char *end=eq;while(end>p&&isspace((unsigned char)end[-1]))end--;*end=0;
  if(strcmp(p,"hbldr_3dsx_titleid"))continue;
  if(found){bad=true;break;}
  p=eq+1;while(isspace((unsigned char)*p))p++;
  if(strlen(p)<16){bad=true;break;}
  for(unsigned i=0;i<16;i++)if(!isxdigit((unsigned char)p[i]))bad=true;
  if(bad)break;
  char digits[17];memcpy(digits,p,16);digits[16]=0;target=strtoull(digits,NULL,16);p+=16;
  while(isspace((unsigned char)*p))p++;
  if(*p&&*p!=';'&&*p!='#'){bad=true;break;}
  found=true;
 }
 if(ferror(f))bad=true;
 fclose(f);
 if(bad||!found||target==FLOW_TITLE_ID||!catalog_is_application(target))return FLOW_ERROR_HBL_CONFIG;
 AM_TitleInfo title;Result r=AM_GetTitleInfo(MEDIATYPE_SD,1,&target,&title);
 if(R_FAILED(r)||title.titleID!=target||!title.size)return FLOW_ERROR_HBL_MISSING;
 *id=target;return 0;
}
