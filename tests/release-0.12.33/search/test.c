#include "game_search.h"
#include "home_controls.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static int score(const char *a,const char *b){char t[SEARCH_TITLE_SIZE],q[SEARCH_QUERY_SIZE];search_normalize(t,sizeof(t),a);search_normalize(q,sizeof(q),b);return search_score(t,q);}
int main(void){
 assert(score("Pokémon Omega Rubin","pokemon"));
 assert(score("Pokémon Omega Rubin","pokmeon"));
 assert(score("Mario Kart 7","mraio"));
 assert(score("Mario Kart 7","mario kart"));
 assert(score("Mario Kart 7","kart mario"));
 assert(score("Mario Kart 7","mariokart"));
 assert(score("Mario Kart 7","MARIO-KART"));
 assert(score("The Legend of Zelda","zel"));
 assert(!score("Mario Kart 7","mario zelda"));
 assert(!score("Mario Kart 7","qz"));
 assert(!score("Mario Kart 7","zelda"));
 assert(score("Mario Kart 7","mario")>score("Super Mario Bros","mraio"));
 assert(score("Mario Kart","mario kart")>score("Super Mario Kart 7","mario kart"));
 assert(score("Straße","strasse"));
 assert(score("Poké" "mon","Poke\xcc\x81mon"));
 char q[SEARCH_QUERY_SIZE]={0};
 assert(!search_edit(q,' '));assert(!search_edit(q,'\b'));
 assert(search_edit(q,'m'));assert(search_edit(q,' '));assert(!search_edit(q,' '));
 assert(search_edit(q,'\b'));assert(!strcmp(q,"m"));
 for(int i=0;i<80;i++){search_edit(q,'x');}
 assert(strlen(q)==32);assert(!search_edit(q,'z'));
 assert(home_header_action(20,24)==HOME_SEARCH);
 assert(home_header_action(290,24)==HOME_SETTINGS);
 assert(home_header_action(20,45)==HOME_NONE);
 for(int y=0;y<240;y++)for(int x=0;x<320;x++){
  char c=search_key_at(x,y,0);if(c)assert(x>=8&&x<308&&y>=58&&y<170);
 }
 assert(search_key_at(13,63,0)=='q');assert(search_key_at(13,63,1)=='1');
 assert(!search_key_at(8,57,0));assert(!search_key_at(8,170,0));
 char words[3][SEARCH_QUERY_SIZE]={{0}};
 search_suggest_add(words,"mraio","Mario Kart 7");assert(!strcmp(words[0],"mario"));
 search_suggest_add(words,"mraio","Super Mario World");assert(!words[1][0]);
 char accepted[SEARCH_QUERY_SIZE]="mraio";
 assert(search_accept_suggestion(accepted,words[0]));assert(!strcmp(accepted,"mario "));
 memset(words,0,sizeof(words));
 search_suggest_add(words,"mario ","Mario Kart 7");
 search_suggest_add(words,"mario ","Super Mario Land");
 search_suggest_add(words,"mario ","Mario Party DS");
 assert(!strcmp(words[0],"kart")&&!strcmp(words[1],"land")&&!strcmp(words[2],"party"));
 assert(search_accept_suggestion(accepted,"kart"));assert(!strcmp(accepted,"mario kart "));
 memset(words,0,sizeof(words));search_suggest_add(words,"kart m","Mario Kart 7");assert(!strcmp(words[0],"mario"));
 strcpy(accepted,"kart m");assert(search_accept_suggestion(accepted,words[0]));assert(!strcmp(accepted,"kart mario "));
 memset(words,0,sizeof(words));search_suggest_add(words,"","The Legend of Zelda");assert(!strcmp(words[0],"legend")&&!words[1][0]);
 memset(accepted,'x',31);accepted[31]=' ';accepted[32]=0;
 char before[SEARCH_QUERY_SIZE];memcpy(before,accepted,sizeof(before));
 assert(!search_accept_suggestion(accepted,"mario"));assert(!strcmp(accepted,before));
 assert(!search_accept_suggestion(accepted,""));
 unsigned seed=4;
 for(int n=0;n<20000;n++){
  char bytes[200],out[SEARCH_TITLE_SIZE],query[SEARCH_QUERY_SIZE];
  for(int j=0;j<199;j++){seed=seed*1664525+1013904223;bytes[j]=(seed>>24)|1;}bytes[199]=0;
  search_normalize(out,sizeof(out),bytes);search_normalize(query,sizeof(query),bytes);
  assert(strlen(out)<sizeof(out)&&strlen(query)<sizeof(query));
  (void)search_score(out,query);
 }
 clock_t start=clock();volatile int sum=0;
 for(int j=0;j<100;j++)for(int i=0;i<1024;i++)sum+=score("The Legend of Zelda Ocarina of Time 3D","zelad time");
 printf("PASS: relevance, typos, Unicode, keyboard bounds, input limit, 20k fuzz cases; host 1024-title query %.2f ms (not console timing), checksum %d\n",(clock()-start)*1000.0/CLOCKS_PER_SEC/100,sum);
}
