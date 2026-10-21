#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static bool failConfig,failBoot;
static int fatRename(const char *from,const char *to){
 if(access(to,F_OK)==0){errno=EEXIST;return -1;}
 if((failConfig&&!strcmp(to,"sdmc:/luma/config.ini")&&strstr(from,".vrflow-new"))||(failBoot&&!strcmp(to,"sdmc:/boot.3dsx")&&strstr(from,".vrflow-new"))){failConfig=failBoot=false;errno=EIO;return -1;}
 return rename(from,to);
}
#define rename fatRename
#include "autoboot.c"
#undef rename
static const char *ini="; untouched comment\r\n[meta]\r\nconfig_version_major = 3\r\nconfig_version_minor = 13\r\n[boot]\r\nautoboot_mode = off ; keep this\r\n[rosalina]\r\nhbldr_3dsx_titleid = 000400000d921e00\r\n[autoboot]\r\nautoboot_3ds_app_mem_type = 0\r\n[other]\r\nfoo = 57\r\n";
static void writeFile(const char *path,const char *text){FILE*f=fopen(path,"wb");assert(f);assert(fputs(text,f)>=0);assert(!fclose(f));}
int main(void){
 mkdir("autoboot-fixture",0777);assert(!chdir("autoboot-fixture"));mkdir("sdmc:",0777);mkdir("sdmc:/3ds",0777);mkdir("sdmc:/3ds/3ds-library",0777);mkdir("sdmc:/luma",0777);
 writeFile(root,"3DSX ORIGINAL HBL");writeFile(loader,"3DSX VF DISPATCHER");writeFile(config,ini);remove(original);remove(configBackup);
 assert(autoboot_status()==0);
 for(int i=0;i<10;i++){
  assert(autoboot_set(true)&&autoboot_status()==1);assert(same(root,loader));assert(!autoboot_set(true));
  char *buf=readConfig();assert(matches(buf,"rosalina","hbldr_3dsx_titleid","000400000d921e00")&&matches(buf,"other","foo","57"));free(buf);
  assert(autoboot_set(false)&&autoboot_status()==0);assert(same(root,original)&&same(config,configBackup));
 }
 assert(autoboot_set(true));writeFile(loader,"3DSX UPDATED HELPER");assert(autoboot_status()==1);assert(autoboot_set(false)&&same(root,original));assert(autoboot_set(true)&&same(root,loader)&&same(root,installed));assert(autoboot_set(false));
 failConfig=true;assert(!autoboot_set(true));assert(autoboot_status()==0&&same(root,original)&&same(config,configBackup));
 failBoot=true;assert(!autoboot_set(true));assert(autoboot_status()==0&&same(root,original));
 assert(autoboot_set(true));FILE*f=fopen(config,"ab");assert(f);fputs("bar = 99\r\n",f);fclose(f);assert(autoboot_set(false));char*buf=readConfig();assert(matches(buf,"other","bar","99"));free(buf);
 assert(autoboot_set(true));writeFile(root,"3DSX EXTERNAL BOOT");assert(!autoboot_set(false)&&autoboot_status()==-1);assert(!same(root,original));assert(copy(original,root));assert(autoboot_set(false));
 writeFile(config,"[meta]\nconfig_version_major = 4\n");assert(autoboot_status()==-2&&!autoboot_set(true)&&same(root,original));
 writeFile(config,ini);writeFile(loader,"BAD!");assert(!autoboot_set(true)&&same(root,original));
 writeFile(loader,"3DSX VF DISPATCHER");writeFile("sdmc:/boot.3dsx.vrflow-old","unfinished");assert(!autoboot_set(true)&&same(root,original));remove("sdmc:/boot.3dsx.vrflow-old");
 puts("PASS autoboot: FAT swaps, 10 enable/disable cycles, exact original restore, config/boot commit failure, preserve unrelated edits, external boot conflict, unsupported config, invalid loader, interrupted swap");
}
