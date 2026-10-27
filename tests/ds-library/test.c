// SPDX-License-Identifier: GPL-3.0-only
#define _POSIX_C_SOURCE 200809L
#include "ds_library.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
/* Match libfat semantics and simulate interruption during commit/rollback. */
static bool fail_commit,fail_restore;
int __real_rename(const char *,const char *);
int __wrap_rename(const char *from,const char *to){
 if(fail_commit&&strstr(from,".tmp")){fail_commit=false;errno=EIO;return -1;}
 if(fail_restore&&strstr(from,".bak")){fail_restore=false;errno=EIO;return -1;}
 if(access(to,F_OK)==0){errno=EEXIST;return -1;}return __real_rename(from,to);
}
static unsigned found,ticks,stop;static char first[512];
static bool visit(const char *p,void *u){(void)u;assert(strstr(p,".nds")||strstr(p,".NDS"));if(!found)snprintf(first,sizeof(first),"%s",p);found++;return found<stop;}
static bool proceed(void *u){(void)u;return ++ticks<10000;}
static bool cancel(void *u){(void)u;return false;}
static void file(const char *p){FILE *f=fopen(p,"wb");assert(f);fputs("fixture",f);fclose(f);}
int main(void){char tmp[]="/tmp/flow-ds-XXXXXX";assert(mkdtemp(tmp));assert(!chdir(tmp));mkdir("sdmc:",0700);mkdir("sdmc:/roms",0700);mkdir("sdmc:/roms/nds",0700);mkdir("sdmc:/roms/nds/series",0700);mkdir("sdmc:/other",0700);mkdir("sdmc:/other/nested",0700);
 file("sdmc:/roms/nds/first.nds");file("sdmc:/roms/nds/series/second.NDS");file("sdmc:/other/nested/third.nds");file("sdmc:/other/zip.zip");assert(!symlink("../../other","sdmc:/roms/nds/link"));
 DsRoots roots={0};char path[512];assert(ds_root_normalize(path,sizeof(path),"SDMC://other//nested/"));assert(!strcmp(path,"sdmc:/other/nested"));assert(!ds_root_normalize(path,sizeof(path),"sdmc:/../private"));assert(!ds_root_normalize(path,sizeof(path),"/host"));assert(!ds_root_normalize(path,sizeof(path),"sdmc:/other\\escape"));
 assert(ds_roots_add(&roots,"sdmc:/other"));assert(ds_roots_add(&roots,"sdmc:/OTHER/"));assert(roots.count==1);assert(ds_roots_add(&roots,"sdmc:/roms/nds/series"));assert(ds_roots_save(&roots,"config"));DsRoots loaded;ds_roots_load(&loaded,"config");assert(loaded.count==2);DsRoots removed=roots;removed.count=1;assert(ds_roots_save(&removed,"config"));ds_roots_load(&loaded,"config");assert(loaded.count==1);removed.count=0;assert(ds_roots_save(&removed,"config"));ds_roots_load(&loaded,"config");assert(!loaded.count);assert(!ds_roots_save(&roots,"missing/config"));FILE *f=fopen("lastline","wb");fputs("sdmc:/other",f);fclose(f);ds_roots_load(&loaded,"lastline");assert(loaded.count==1);
 /* Existing config survives failed commits, including failed immediate rollback. */
 assert(ds_roots_save(&roots,"durable"));DsRoots altered=roots;altered.count=1;
 fail_commit=true;assert(!ds_roots_save(&altered,"durable"));ds_roots_load(&loaded,"durable");assert(loaded.count==roots.count);
 fail_commit=true;fail_restore=true;assert(!ds_roots_save(&altered,"durable"));assert(access("durable",F_OK));assert(!access("durable.bak",F_OK));ds_roots_load(&loaded,"durable");assert(loaded.count==roots.count);
 assert(ds_roots_save(&altered,"durable"));ds_roots_load(&loaded,"durable");assert(loaded.count==1);assert(access("durable.bak",F_OK));
 DsScan s;stop=100;ds_library_scan(&roots,&s,visit,proceed,NULL);assert(found==3&&s.files==3&&!s.limited&&!s.cancelled);found=0;ticks=0;ds_library_scan(&roots,&s,visit,cancel,NULL);assert(s.cancelled&&found==0);found=0;stop=1;ds_library_scan(&roots,&s,visit,proceed,NULL);assert(s.limited&&found==1);

 /* A skipped deep branch must not suppress games in later roots. */
 char deep[512]="sdmc:/roms/nds";for(unsigned i=0;i<10;i++){strcat(deep,"/deep");assert(!mkdir(deep,0700));}
 found=ticks=0;stop=100;ds_library_scan(&roots,&s,visit,proceed,NULL);assert(s.limited&&found==3);

 /* Explicit nested roots get their own depth allowance after a parent scan. */
 char deepfile[600];snprintf(deepfile,sizeof(deepfile),"%s/deep.nds",deep);file(deepfile);assert(ds_roots_add(&roots,deep));found=ticks=0;ds_library_scan(&roots,&s,visit,proceed,NULL);assert(s.limited&&found==4);
 DsRoots overlap={0};assert(ds_roots_add(&overlap,"sdmc:/roms/nds"));char atlimit[512]="sdmc:/roms/nds";for(unsigned i=0;i<8;i++)strcat(atlimit,"/deep");assert(ds_roots_add(&overlap,atlimit));found=ticks=0;ds_library_scan(&overlap,&s,visit,proceed,NULL);assert(s.limited&&found==3);

 /* Explicit roots take priority even if a default tree exhausts its budget. */
 for(unsigned i=0;i<260;i++){char folder[100];snprintf(folder,sizeof(folder),"sdmc:/roms/nds/folder-%03u",i);assert(!mkdir(folder,0700));}
 found=ticks=0;ds_library_scan(&roots,&s,visit,proceed,NULL);assert(s.limited&&found>=1&&!strcmp(first,"sdmc:/other/nested/third.nds")); /* first callback is custom third.nds */
 DsBrowser b={0};strcpy(b.path,"sdmc:/");ds_browser_read(&b);assert(b.count==2);assert(ds_browser_enter(&b,0));assert(ds_browser_parent(&b));assert(!strcmp(b.path,"sdmc:/"));assert(!ds_browser_parent(&b));assert(!ds_browser_enter(&b,99));assert(ds_empty_hit(16,115)==1&&ds_empty_hit(303,148)==1);assert(ds_empty_hit(16,155)==2&&ds_empty_hit(16,193)==3);assert(!ds_empty_hit(15,115)&&!ds_empty_hit(304,115)&&!ds_empty_hit(16,219));
 puts("PASS: actual DS scanner recursion, overlapping roots, extension case, symlink skip, cancellation, budget, persistence, validation, folder navigation and empty-screen touch boundaries");return 0;}
