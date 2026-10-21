#include "ui.h"
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
static void rebuild(void){
    visibleCount=0;
    char query[SEARCH_QUERY_SIZE];search_normalize(query,sizeof(query),searchQuery);
    for(unsigned i=0;i<gameCount;++i){
        Game *g=&games[i];
        if(!searchOpen&&library_view_for(g->system,g->homebrew)!=systemView)continue;
        game_display_title(g->displayName,sizeof(g->displayName),g->name);
        int rank=1;
        if(searchOpen){
            char title[SEARCH_TITLE_SIZE],combined[256],normal[SEARCH_TITLE_SIZE];
            search_normalize(title,sizeof(title),g->displayName);
            rank=search_score(title,query);
            static const char *aliases[]={"3ds","ds nds","gba","n64","snes","nes","vb","gb gbc"};
            snprintf(combined,sizeof(combined),"%s %s %s",g->displayName,g->homebrew?"Homebrew":library_system_names[g->system],g->homebrew?"":aliases[g->system]);
            search_normalize(normal,sizeof(normal),combined);
            int qualified=search_score(normal,query)-100;
            if(qualified>rank)rank=qualified;
            if(!rank)continue;
        }
        unsigned n=visibleCount++;
        while(searchOpen&&n&&rank>searchRanks[n-1]){
            visible[n]=visible[n-1];searchRanks[n]=searchRanks[n-1];n--;
        }
        visible[n]=i;searchRanks[n]=rank;
    }
    for(unsigned n=0;n<visibleCount;n++){
        Game *g=&games[visible[n]];
        if(g->cartridge){size_t len=strlen(g->displayName);snprintf(g->displayName+len,sizeof(g->displayName)-len," [%s]",tr("Modul"));}
        cards[n]=(UiCard){g->displayName,g->cover,backendNames[g->system],g->system==SYSTEM_3DS||g->system==SYSTEM_DS,g->icon,g->banner,g->pending};
        static const char *labels[]={"3DS","DS","GBA","N64","SNES","NES","VB","GB/GBC"};
        searchSystems[n]=g->homebrew?"Homebrew":labels[g->system];
    }
    if(selected>=visibleCount)selected=0;
    neighborCount=0;
    if(searchOpen){
        memset(searchSuggestions,0,sizeof(searchSuggestions));
        for(unsigned i=0;i<visibleCount&&!searchSuggestions[2][0];i++)search_suggest_add(searchSuggestions,searchQuery,cards[i].name);
        return;
    }
    for(unsigned side=0;side<2;side++){
        unsigned view=(systemView+(side?VIEW_COUNT-1:1))%VIEW_COUNT,indices[MAX_GAMES],count=0;
        for(unsigned i=0;i<gameCount;i++)if(library_view_for(games[i].system,games[i].homebrew)==view)indices[count++]=i;
        if(!count)continue;
        unsigned center=remembered[view]<count?remembered[view]:0,added=0,quota=(32-(visibleCount<20?visibleCount:20))/2;
        for(unsigned d=0;d<count&&added<quota;d++)for(int direction=-1;direction<=1;direction+=2){
            if(!d&&direction==1)continue;
            int n=(int)center+(int)d*direction;if(n<0||(unsigned)n>=count||added>=quota)continue;
            Game *g=&games[indices[n]];neighborCards[neighborCount++]=(UiCard){.cover=g->cover,.icon=g->icon};added++;
        }
    }
}

static bool input(unsigned keys){bool leave=false;if(searchOpen){
            bool changed=false;
            if((keys&KEY_B)||((keys&KEY_TOUCH)&&pos.py>=214&&pos.py<238&&pos.px>=8&&pos.px<78)){
                searchOpen=false;systemView=searchReturnView;selected=searchReturn;rebuild();flow_reset(&flow,selected);
            }else{
                if((keys&KEY_UP)&&visibleCount)selected=(selected+visibleCount-1)%visibleCount;
                if((keys&KEY_DOWN)&&visibleCount)selected=(selected+1)%visibleCount;
                if(keys&KEY_X)changed=search_edit(searchQuery,'\b');
                if((keys&KEY_Y)&&*searchQuery){searchQuery[0]=0;changed=true;}
                if((keys&KEY_TOUCH)&&pos.py>=8&&pos.py<44&&pos.px>=8&&pos.px<312){
                    unsigned slot=(pos.px-8)/103;
                    if(slot<3&&(pos.px-8)%103<98)changed=search_accept_suggestion(searchQuery,searchSuggestions[slot]);
                }
                if((keys&KEY_TOUCH)&&pos.py>=58&&pos.py<170)changed=search_edit(searchQuery,search_key_at(pos.px,pos.py,searchNumeric));
                if((keys&KEY_TOUCH)&&pos.py>=177&&pos.py<209){
                    if(pos.px>=8&&pos.px<60)searchNumeric=!searchNumeric;
                    else if(pos.px>=64&&pos.px<224)changed=search_edit(searchQuery,' ');
                    else if(pos.px>=228&&pos.px<312)changed=search_edit(searchQuery,'\b');
                }
                if(changed){selected=0;rebuild();flow_reset(&flow,selected);menu_music_click();}
                bool open=(keys&KEY_A)!=0;
                if((keys&KEY_TOUCH)&&pos.py>=214&&pos.py<238){
                    if(pos.px>=164&&pos.px<312)open=true;
                    else if(visibleCount&&pos.px>=82&&pos.px<116)selected=(selected+visibleCount-1)%visibleCount;
                    else if(visibleCount&&pos.px>=120&&pos.px<154)selected=(selected+1)%visibleCount;
                }
                flow_reset(&flow,selected);
                if(open&&visibleCount)leave=startGame();
            }
        }return leave;}

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
