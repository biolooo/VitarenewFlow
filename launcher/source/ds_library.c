// SPDX-License-Identifier: GPL-3.0-only
#include "ds_library.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
static const char *defaults[]={"sdmc:/roms/nds","sdmc:/nds","sdmc:/games/nds"};
bool ds_root_normalize(char *out,size_t cap,const char *path){
 if(!out||!path||strncasecmp(path,"sdmc:/",6)||strlen(path)>=DS_PATH_MAX)return false;
 char normalized[DS_PATH_MAX]="sdmc:";size_t used=5;const char *at=path+6;
 while(*at){while(*at=='/')at++;if(!*at)break;const char *end=strchr(at,'/');size_t n=end?(size_t)(end-at):strlen(at);
  if((n==1&&at[0]=='.')||(n==2&&!strncmp(at,"..",2)))return false;
  for(size_t i=0;i<n;i++)if((unsigned char)at[i]<32||at[i]=='\\'||at[i]==':')return false;
  if(used+n+2>sizeof(normalized))return false;
  normalized[used++]='/';memcpy(normalized+used,at,n);used+=n;normalized[used]=0;at+=n;
 }
 if(used==5)normalized[used++]='/';
 normalized[used]=0;if(used>=cap)return false;memcpy(out,normalized,used+1);return true;
}
bool ds_roots_add(DsRoots *roots,const char *path){char clean[DS_PATH_MAX];if(!roots||!ds_root_normalize(clean,sizeof(clean),path))return false;
 for(unsigned i=0;i<roots->count;i++)if(!strcasecmp(clean,roots->paths[i]))return true;
 if(roots->count>=DS_ROOT_MAX)return false;
 strcpy(roots->paths[roots->count++],clean);return true;
}
static bool read_roots(DsRoots *roots,const char *config){
 memset(roots,0,sizeof(*roots));FILE *f=fopen(config,"rb");if(!f)return false;
 char line[DS_PATH_MAX+2];while(fgets(line,sizeof(line),f)){size_t n=strcspn(line,"\r\n");if(line[n])line[n]=0;else if(n==sizeof(line)-1){int c;while((c=fgetc(f))!=EOF&&c!='\n');continue;}ds_roots_add(roots,line);}
 bool ok=!ferror(f);if(fclose(f))ok=false;return ok;
}
static bool companion(char *out,size_t cap,const char *config,const char *suffix){int n=snprintf(out,cap,"%s%s",config,suffix);return n>=0&&(size_t)n<cap;}
void ds_roots_load(DsRoots *roots,const char *config){
 if(read_roots(roots,config))return;
 char backup[DS_PATH_MAX+8];if(companion(backup,sizeof(backup),config,".bak"))read_roots(roots,backup);
}
static bool roots_equal(const DsRoots *a,const DsRoots *b){if(a->count!=b->count)return false;for(unsigned i=0;i<a->count;i++)if(strcmp(a->paths[i],b->paths[i]))return false;return true;}
bool ds_roots_save(const DsRoots *roots,const char *config){
 char tmp[DS_PATH_MAX+8],backup[DS_PATH_MAX+8];
 if(roots->count>DS_ROOT_MAX||!companion(tmp,sizeof(tmp),config,".tmp")||!companion(backup,sizeof(backup),config,".bak"))return false;
 DsRoots check,old;bool hadOld=read_roots(&old,config);
 /* Recover an interrupted FAT commit before touching its only backup. */
 if(!hadOld&&read_roots(&old,backup)){if(rename(backup,config))return false;hadOld=true;}
 FILE *f=fopen(tmp,"wb");if(!f)return false;bool ok=true;
 for(unsigned i=0;i<roots->count;i++)if(fprintf(f,"%s\n",roots->paths[i])<0)ok=false;
 if(fclose(f))ok=false;
 if(!ok||!read_roots(&check,tmp)||!roots_equal(roots,&check)){remove(tmp);return false;}
 /* FAT rename refuses an existing destination: retain old config until verified. */
 if(hadOld){remove(backup);if(rename(config,backup)){remove(tmp);return false;}}
 if(rename(tmp,config)){if(hadOld)rename(backup,config);remove(tmp);return false;}
 if(!read_roots(&check,config)||!roots_equal(roots,&check)){
  remove(config);if(hadOld)rename(backup,config);return false;
 }
 remove(backup);return true;
}

typedef struct {DsScan *s;DsVisit visit;DsContinue proceed;void *user;char visited[256][DS_PATH_MAX];unsigned seen;unsigned depths[256];bool stopped;} Walker;
static void walk(Walker *w,const char *path,unsigned depth){
 if(w->s->cancelled||w->stopped)return;
 int slot=-1;for(unsigned i=0;i<w->seen;i++)if(!strcasecmp(path,w->visited[i])){slot=(int)i;break;}
 if(slot>=0){if(w->depths[slot]<=depth)return;w->depths[slot]=depth;}
 else{if(w->seen>=256){w->s->limited=true;w->stopped=true;return;}strcpy(w->visited[w->seen],path);w->depths[w->seen++]=depth;}
 struct stat root;if(lstat(path,&root)||!S_ISDIR(root.st_mode)){w->s->missing++;return;}
 DIR *d=opendir(path);if(!d){w->s->missing++;return;}w->s->directories++;struct dirent *e;
 while((e=readdir(d))){if(e->d_name[0]=='.')continue;if(++w->s->entries>12000){w->s->limited=true;w->stopped=true;break;}
  if(w->proceed&&!w->proceed(w->user)){w->s->cancelled=true;break;}
  char child[DS_PATH_MAX];int n=snprintf(child,sizeof(child),"%s%s%s",path,path[strlen(path)-1]=='/'?"":"/",e->d_name);if(n<0||n>=(int)sizeof(child)){w->s->limited=true;continue;}
  struct stat st;if(lstat(child,&st))continue;
  if(S_ISDIR(st.st_mode)){if(depth>=8)w->s->limited=true;else walk(w,child,depth+1);}
  else if(S_ISREG(st.st_mode)){const char *ext=strrchr(e->d_name,'.');if(ext&&!strcasecmp(ext,".nds")){w->s->files++;if(!w->visit(child,w->user)){w->s->limited=true;w->stopped=true;break;}}}
  if(w->s->cancelled||w->stopped)break;
 }closedir(d);
}
void ds_library_scan(const DsRoots *roots,DsScan *result,DsVisit visit,DsContinue proceed,void *user){memset(result,0,sizeof(*result));Walker *w=calloc(1,sizeof(*w));if(!w){result->limited=true;return;}w->s=result;w->visit=visit;w->proceed=proceed;w->user=user;
 for(unsigned i=0;i<roots->count;i++)walk(w,roots->paths[i],0);
 for(unsigned i=0;i<sizeof(defaults)/sizeof(defaults[0]);i++)walk(w,defaults[i],0);
 free(w);
}
static int namecmp(const void *a,const void *b){return strcasecmp(a,b);}
void ds_browser_read(DsBrowser *b){b->count=b->selected=0;b->limited=b->unreadable=false;char clean[DS_PATH_MAX];if(!ds_root_normalize(clean,sizeof(clean),b->path)){strcpy(b->path,"sdmc:/");}DIR *d=opendir(b->path);if(!d){b->unreadable=true;return;}struct dirent *e;unsigned entries=0;
 while((e=readdir(d))){if(++entries>12000){b->limited=true;break;}if(e->d_name[0]=='.')continue;char child[DS_PATH_MAX];int n=snprintf(child,sizeof(child),"%s%s%s",b->path,b->path[strlen(b->path)-1]=='/'?"":"/",e->d_name);struct stat st;if(n<0||n>=(int)sizeof(child)||lstat(child,&st)||!S_ISDIR(st.st_mode))continue;if(b->count==DS_FOLDER_MAX){b->limited=true;break;}snprintf(b->names[b->count++],256,"%s",e->d_name);
 }closedir(d);qsort(b->names,b->count,sizeof(b->names[0]),namecmp);
}
bool ds_browser_enter(DsBrowser *b,unsigned i){if(i>=b->count)return false;char path[DS_PATH_MAX];int n=snprintf(path,sizeof(path),"%s%s%s",b->path,b->path[strlen(b->path)-1]=='/'?"":"/",b->names[i]);if(n<0||n>=(int)sizeof(path))return false;strcpy(b->path,path);ds_browser_read(b);return true;}
bool ds_browser_parent(DsBrowser *b){if(!strcmp(b->path,"sdmc:/"))return false;char *slash=strrchr(b->path,'/');if(!slash)return false;if(slash==b->path+5)slash[1]=0;else *slash=0;ds_browser_read(b);return true;}
int ds_empty_hit(int x,int y){if(x<16||x>=304)return 0;if(y>=115&&y<149)return 1;if(y>=155&&y<187)return 2;if(y>=193&&y<219)return 3;return 0;}
