#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static int failCommit;
static int fatRename(const char *from,const char *to){
 if(access(to,F_OK)==0){errno=EEXIST;return -1;}
 if(failCommit&&strstr(from,".tmp")){failCommit=0;errno=EIO;return -1;}
 return rename(from,to);
}
#define rename fatRename
#include "l10n.c"
#undef rename
static void writePref(const char *path,const char *text){FILE*f=fopen(path,"wb");assert(f);assert(fputs(text,f)>=0);assert(!fclose(f));}
int main(void){
 mkdir("language-fixture",0777);assert(!chdir("language-fixture"));
 mkdir("sdmc:",0777);mkdir("sdmc:/3ds",0777);mkdir("sdmc:/3ds/3ds-library",0777);
 remove(pref);remove(backup);l10n_load(false);assert(!l10n_english());
 for(int i=0;i<40;i++){bool value=(i%2)==0;assert(l10n_save(value));assert(l10n_english()==value);l10n_load(!value);assert(l10n_english()==value);}
 assert(l10n_save(false));failCommit=1;assert(!l10n_save(true));assert(!l10n_english());l10n_load(true);assert(!l10n_english());
 assert(!rename(pref,backup));l10n_load(true);assert(!l10n_english()&&access(pref,F_OK)==0);
 writePref(backup,"1\n");writePref(pref,"broken");l10n_load(false);assert(l10n_english());
 assert(l10n_save(false));assert(access(backup,F_OK)!=0);
 puts("PASS FAT rename: 40 switches/reloads, commit failure rollback, interrupted/corrupt preference recovery");
}
