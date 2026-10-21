from pathlib import Path
s=Path('outputs/3ds-library/source/main.c').read_text(encoding='utf8')
rebuild=s[s.index('static void rebuild(void)'):s.index('static void draw(')]
a=s.index('}else if(searchOpen){');b=s.index('}else if(confirm){',a)
body=s[a+6:b]+'}'
head='''#include "ui.h"
#include "game_search.h"
#include "library_model.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define MAX_GAMES 1024
typedef struct {int system;char name[160],displayName[160],cover[512],banner[512];uint16_t *icon;bool pending,homebrew,cartridge;} Game;
static Game games[MAX_GAMES];static UiCard cards[MAX_GAMES],neighborCards[32];
static unsigned visible[MAX_GAMES],visibleCount,selected,systemView,gameCount,neighborCount,remembered[VIEW_COUNT];
static unsigned searchReturn,searchReturnView;static bool searchOpen,searchNumeric;
static char searchQuery[SEARCH_QUERY_SIZE];static char searchSuggestions[3][SEARCH_QUERY_SIZE];static int searchRanks[MAX_GAMES];static const char *searchSystems[MAX_GAMES];
static const char *backendNames[SYSTEM_COUNT]={"3ds","ds","gba","n64","snes","nes","vb","gb"};
static const char *tr(const char *s){return s;}
void game_display_title(char *out,size_t n,const char *in){memmove(out,in,strlen(in)+1);(void)n;}
static int flow,started=-1,clicks;static struct {int px,py;} pos;
enum {KEY_A=1,KEY_B=2,KEY_X=4,KEY_Y=8,KEY_TOUCH=16,KEY_UP=32,KEY_DOWN=64};
static void flow_reset(int *f,unsigned index){*f=index;}
static void menu_music_click(void){clicks++;}
static bool startGame(void){started=visible[selected];return true;}
'''
tail='''
int main(void){
 gameCount=5;games[0]=(Game){.system=SYSTEM_3DS,.name="Mario Kart 7"};
 games[1]=(Game){.system=SYSTEM_GBA,.name="Mario Kart Super Circuit"};
 games[2]=(Game){.system=SYSTEM_DS,.name="Mario Kart DS"};
 games[3]=(Game){.system=SYSTEM_N64,.name="The Legend of Zelda"};
 games[4]=(Game){.system=SYSTEM_3DS,.name="FBI",.homebrew=true};
 systemView=VIEW_3DS;rebuild();assert(visibleCount==1&&visible[0]==0);
 searchOpen=true;searchReturnView=VIEW_3DS;searchReturn=0;strcpy(searchQuery,"mario");rebuild();
 assert(visibleCount==3&&visible[0]==0&&visible[1]==1&&visible[2]==2);
 strcpy(searchQuery,"mario gba");selected=0;rebuild();assert(visibleCount==1&&visible[0]==1);
 assert(input(KEY_A)&&started==1);started=-1;
 strcpy(searchQuery,"zzzz");rebuild();assert(!visibleCount);assert(!input(KEY_A)&&started==-1);
 input(KEY_Y);assert(visibleCount==5&&!searchQuery[0]&&clicks==1);
 pos.px=13;pos.py=63;input(KEY_TOUCH);assert(!strcmp(searchQuery,"q")&&!visibleCount);
 input(KEY_X);assert(!searchQuery[0]&&visibleCount==5);
 input(KEY_DOWN);assert(selected==1);input(KEY_UP);assert(!selected);
 input(KEY_DOWN);pos.px=290;pos.py=220;assert(input(KEY_TOUCH)&&started==1);

 strcpy(searchQuery,"mraio");rebuild();assert(!strcmp(searchSuggestions[0],"mario"));
 pos.px=18;pos.py=20;input(KEY_TOUCH);assert(!strcmp(searchQuery,"mario "));
 assert(visibleCount==3&&!strcmp(searchSuggestions[0],"kart"));
 input(KEY_B);assert(!searchOpen&&systemView==VIEW_3DS&&visibleCount==1&&selected==0);
 searchOpen=true;strcpy(searchQuery,"mraio");rebuild();assert(visibleCount==3);
 strcpy(searchQuery,"homebrew");rebuild();assert(visibleCount==1&&visible[0]==4);
 puts("PASS: actual catalog rebuild and search dispatcher: cross-category ranking, platform filter, A/touch launch, empty results, keyboard edits, close/restore");
}
'''
Path('work/search-v01225/integration.c').write_text(head+rebuild+'\nstatic bool input(unsigned keys){bool leave=false;'+body+'return leave;}\n'+tail,encoding='utf8')
