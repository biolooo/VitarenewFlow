// SPDX-License-Identifier: GPL-3.0-only
#include "settings_menu.h"
#include <string.h>
static const MenuItem root[]={
 {"Anpassen","Sprache und Theme",MENU_PERSONAL_OPEN},
 {"Spiele verwalten","Bibliothek aktualisieren, DS prüfen",MENU_GAMES_OPEN},
 {"Konsole & Start","Autostart und HOME-Menü",MENU_SYSTEM_OPEN},
 {"Über & Unterstützung","VitarenewFlow und VitaRenew",MENU_SUPPORT}};
static const MenuItem personal[]={
 {"Sprache","Deutsch / English",MENU_LANGUAGE},
 {"Theme neu laden","Hintergrund und Musik aktualisieren",MENU_THEME}};
static const MenuItem games[]={
 {"Spiele neu einlesen","Neue Spiele auf der SD-Karte suchen",MENU_SCAN},
 {"DS-Diagnose","DS-System und Loader prüfen",MENU_DS},
 {"Emulatoren","Herunterladen und einrichten",MENU_COMPONENTS},
 {"Desktop-Verwaltung","Windows-App auf die SD laden",MENU_DESKTOP}};
static const MenuItem system[]={
 {"Autostart","Flow beim Einschalten starten",MENU_AUTOBOOT},
 {"HOME-Menü öffnen","Zum Nintendo HOME-Menü wechseln",MENU_HOME},
 {"Updates","VitarenewFlow aktualisieren",MENU_UPDATE}};
unsigned settings_menu_count(unsigned p){return p==MENU_ROOT?4:p==MENU_PERSONAL?2:p==MENU_SYSTEM?3:p==MENU_GAMES?4:0;}
const char *settings_menu_title(unsigned p){return p==MENU_PERSONAL?"Anpassen":p==MENU_GAMES?"Spiele verwalten":p==MENU_SYSTEM?"Konsole & Start":"Menü";}
const MenuItem *settings_menu_item(unsigned p,unsigned r){if(r>=settings_menu_count(p))return 0;return &(p==MENU_PERSONAL?personal:p==MENU_GAMES?games:p==MENU_SYSTEM?system:root)[r];}
int settings_menu_hit(int x,int y,unsigned p){if(x<12||x>=308||y<54)return -1;unsigned r=(unsigned)(y-54)/46;if(r>=settings_menu_count(p)||(y-54)%46>=40)return -1;return (int)r;}
unsigned settings_menu_parent_row(unsigned p){return p==MENU_PERSONAL?0:p==MENU_GAMES?1:p==MENU_SYSTEM?2:0;}

static const char *releaseNotes33[]={"- Desktop erkennen und reparieren","- DS-Laufzeit erneut prüfen","- Suche mit Wortvorschlägen"};
static const char *releaseNotes34[]={"- Spielstart ohne Startanimation","- Über & Unterstützung erneuert","- Neuerungen ausklappbar"};
static const char *releaseNotes35[]={"- Probleme bewusst an VitaRenew melden","- Diagnose ohne Spiele oder Rohlogs","- Beschreibung vor dem Senden prüfen"};
unsigned settings_release_note_count(const char *version){return version&&(!strcmp(version,"0.12.33")||!strcmp(version,"0.12.34")||!strcmp(version,"0.12.35"))?3:0;}
const char *settings_release_note(const char *version,unsigned row){return row<settings_release_note_count(version)?(!strcmp(version,"0.12.35")?releaseNotes35:!strcmp(version,"0.12.34")?releaseNotes34:releaseNotes33)[row]:NULL;}
bool settings_update_notes_hit(int x,int y){return x>=20&&x<300&&y>=172&&y<187;}
