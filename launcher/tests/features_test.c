#include "l10n.h"
#include "native_handoff.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static int mode;
Result AM_GetTitleInfo(FS_MediaType media,u32 n,u64 *ids,AM_TitleInfo *out){assert(media==MEDIATYPE_SD&&n==1&&*ids==0x000400000D921E00ULL);out->titleID=mode==2?*ids+1:*ids;out->size=mode==3?0:42;return mode==1?-1:0;}
static void write_file(const char *path,const char *text){FILE *f=fopen(path,"wb");assert(f);assert(fwrite(text,1,strlen(text),f)==strlen(text));assert(!fclose(f));}
int main(void){
 mkdir("flow-fixture",0777);assert(!chdir("flow-fixture"));mkdir("sdmc:",0777);mkdir("sdmc:/3ds",0777);mkdir("sdmc:/3ds/3ds-library",0777);mkdir("sdmc:/luma",0777);
 const char *lang="sdmc:/3ds/3ds-library/language.txt";unlink(lang);l10n_load(false);assert(!l10n_english()&&!strcmp(tr("Öffnen"),"Öffnen"));
 assert(l10n_save(true));assert(l10n_english()&&!strcmp(tr("Öffnen"),"Open"));l10n_load(false);assert(l10n_english());assert(!strcmp(tr("Unbekannter Spielname"),"Unbekannter Spielname"));
 assert(!strcmp(tr("Spiel-Seed wird repariert..."),"Repairing game seed..."));assert(!strcmp(tr("ca. %u Sek."),"about %u sec"));assert(!strcmp(tr("Projekt unterstützen"),"Support the project"));
 write_file(lang,"7\n");l10n_load(false);assert(!l10n_english());write_file(lang,"1\nextra");l10n_load(false);assert(!l10n_english());
 assert(l10n_save(false));assert(!l10n_english());
 rename("sdmc:/3ds/3ds-library","sdmc:/3ds/hidden");assert(!l10n_save(true)&&!l10n_english());rename("sdmc:/3ds/hidden","sdmc:/3ds/3ds-library");
 const char *config="sdmc:/luma/config.ini";u64 id=0;unlink(config);assert(native_handoff_target(&id)==FLOW_ERROR_HBL_CONFIG);
 write_file(config,"[rosalina]\nhbldr_3dsx_titleid = 000400000d921e00\n");assert(!native_handoff_target(&id)&&id==0x000400000D921E00ULL);
 for(mode=1;mode<=3;mode++)assert(native_handoff_target(&id)==FLOW_ERROR_HBL_MISSING);mode=0;
 write_file(config,"[other]\nhbldr_3dsx_titleid = 000400000d921e00\n");assert(native_handoff_target(&id)==FLOW_ERROR_HBL_CONFIG);
 write_file(config,"[rosalina]\nhbldr_3dsx_titleid = 000400000F104200\n");assert(native_handoff_target(&id)==FLOW_ERROR_HBL_CONFIG);
 write_file(config,"[rosalina]\nhbldr_3dsx_titleid = 000400000d921e00\nhbldr_3dsx_titleid=000400000d921e00\n");assert(native_handoff_target(&id)==FLOW_ERROR_HBL_CONFIG);
 write_file(config,"[rosalina]\nhbldr_3dsx_titleid = 000400000d921e00BAD\n");assert(native_handoff_target(&id)==FLOW_ERROR_HBL_CONFIG);
 write_file(config,"[rosalina]\nhbldr_3dsx_titleid = 000400000d921e00 ; comment\n");assert(!native_handoff_target(&id));
 puts("PASS languages: DE/EN, persisted reload, damaged preferences, failed writes preserve active language, unknown game names unchanged");
 puts("PASS native CIA routing: configured Rosalina title, missing config/title, wrong title, zero size, self-loop, wrong section, duplicate/malformed key, inline comments");
}
