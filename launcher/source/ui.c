#include "settings_menu.h"

// SPDX-License-Identifier: GPL-3.0-only

// Copyright (C) 2026 VitarenewFlow contributors

#include "l10n.h"

#include "ui.h"
#include "game_search.h"

#include "brand_ui.h"

#include "catalog_format.h"

#include "ui_texture.h"

#include "theme_io.h"

#include "menu_music.h"

#include "theme_format.h"

#include "library_model.h"

#include "ui_runtime.h"

#include "banner_player.h"

#include "cover_loader.h"

#include "app_log.h"

#include "ui_font.h"

#include <citro2d.h>

#include <png.h>

#include <stdlib.h>

#include <string.h>

#include <stdio.h>

#include <math.h>

#include <time.h>

#include <sys/stat.h>

static C3D_RenderTarget *top,*right,*bottom;

static C2D_TextBuf textbuf;

typedef struct {char path[512];

C3D_Tex tex;

Tex3DS_SubTexture sub;

C2D_Image image;

bool used,valid;

unsigned age;} Cover;

typedef struct {C3D_Tex tex;

Tex3DS_SubTexture sub;

C2D_Image image;

ThemeScreen format;

bool valid;} Background;

static Cover cache[COVER_SLOTS];

static bool bannerVisible;

static char bannerCandidate[512];

static u64 bannerStableSince;

static u64 renderTotal,renderMax;static unsigned renderFrames,renderSlow;

static Background backgrounds[2];

static unsigned clockAge,frame;

static const char *themeSource="Standard";

#define RGB(r,g,b) C2D_Color32(r,g,b,255)

#define RGBA(r,g,b,a) C2D_Color32(r,g,b,a)



/* All typography uses a native-pixel Inter atlas; no fractional font scaling. */

static const u32 ink=0xfff6f5f1,muted=0xffc6b7a5,white=0xffffffff,accent=0xffceef5a;

static const u32 panel=0xf2201710,raised=0xff35291e;

static void rect(float x,float y,float w,float h,u32 c){C2D_DrawRectSolid(x,y,0,w,h,c);}

static void rounded(float x,float y,float w,unsigned h,float radius,u32 color){

 if(w<=0||!h)return;

 radius=fminf(radius,fminf(h/2.f,w/2));

 unsigned corners=(unsigned)ceilf(radius);

 for(unsigned i=0;i<corners&&i<=(h-1)/2;++i){float d=fmaxf(0,radius-i-.5f);

  float inset=d?radius-sqrtf(radius*radius-d*d):0;

  rect(x+inset,y+i,w-2*inset,1,color);

  if(h-1-i!=i)rect(x+inset,y+h-1-i,w-2*inset,1,color);}

 if(h>2*corners)rect(x,y+corners,w,h-2*corners,color);

}

static void text(float x,float y,unsigned size,u32 color,const char *str){

 str=tr(str);

 if(ui_font_draw(x,y,size,color,str))return;

 C2D_Text t;

 C2D_TextParse(&t,textbuf,str?str:"");

 C2D_TextOptimize(&t);

 C2D_DrawText(&t,C2D_WithColor,x,y,0,size/30.f,size/30.f,color);

}

static void centered(float x,float y,unsigned size,u32 color,const char *str){str=tr(str);text(roundf(x-ui_font_width(size,str)/2.f),y,size,color,str);}

/* Consume complete UTF-8 characters, wrapping at spaces and path separators. */

static bool wrap_next(const char **input,unsigned size,unsigned width,char out[160]){

 const char *start=*input;

 while(*start==' ')++start;

 if(!*start){*input=start;

 out[0]=0;

 return false;}

 const char *p=start,*end=start,*lastBreak=NULL;

 size_t used=0;

 while(*p&&*p!='\n'){

  const char *next=p;

  ui_font_next(&next);

  size_t bytes=next-p;

  if(used+bytes>=159)break;

  memcpy(out+used,p,bytes);

  out[used+bytes]=0;

  if(ui_font_width(size,out)>width&&used)break;

  used+=bytes;

  end=next;

  if(*p==' '||*p=='/'||*p=='-')lastBreak=next;

  p=next;

 }

 if(*p&&*p!='\n'&&lastBreak&&lastBreak>start)end=lastBreak;

 if(end==start){const char *next=start;

 ui_font_next(&next);

 end=next;}

 used=end-start;

 memcpy(out,start,used);

 while(used&&out[used-1]==' ')--used;

 out[used]=0;

 *input=*end=='\n'?end+1:end;

 return true;

}

static void ellipsis(char *line,unsigned size,unsigned width){

 while(*line&&ui_font_width(size,line)+ui_font_width(size,"…")>width){size_t n=strlen(line)-1;

 while(n&&((unsigned char)line[n]&0xc0)==0x80)--n;

 line[n]=0;}

 size_t n=strlen(line);

 if(n+3<160)memcpy(line+n,"…",4);

}

static void title(float x,float y,unsigned size,unsigned width,unsigned maxLines,const char *str,bool center){

 for(unsigned i=0;i<maxLines;++i){char line[160];

 if(!wrap_next(&str,size,width,line))break;

  if(i+1==maxLines&&*str)ellipsis(line,size,width);

  if(center)centered(x,y+i*(size+3),size,ink,line);

  else text(x,y+i*(size+3),size,ink,line);

 }

}

static void chevron(float x,float y,bool right,u32 color){

 float direction=right?1:-1;

 C2D_DrawTriangle(x-3*direction,y-5,color,x+2*direction,y,color,x-3*direction,y+5,color,0);

 C2D_DrawTriangle(x-5*direction,y-5,panel,x,y,panel,x-5*direction,y+5,panel,0);

}

static void key(float x,float y,const char *label){rounded(x,y,20,18,5,raised);

centered(x+10,y+1,12,accent,label);}

/* Drawing only looks up published textures. File reads, PNG decode and tiling

   happen on the loader worker, never inside art()/getCover(). */

static Cover *getCover(const UiCard *card){

    if(!card->cover)return NULL;

    for(unsigned i=0;i<COVER_SLOTS;i++)if(cache[i].used&&!strcmp(cache[i].path,card->cover))return cache[i].valid?&cache[i]:NULL;

    return NULL;

}

static unsigned requestCover(const UiCard *card,unsigned priority){

    const char *path=card->cover;if(!path||!*path)return COVER_SLOTS;

    unsigned slot=0;

    for(unsigned i=0;i<COVER_SLOTS;i++){

        if(cache[i].used&&!strcmp(cache[i].path,path)){slot=i;goto found;}

        if(!cache[i].used||cache[i].age<cache[slot].age)slot=i;

    }

    if(cache[slot].valid)C3D_TexDelete(&cache[slot].tex);

    memset(&cache[slot],0,sizeof(cache[slot]));

    snprintf(cache[slot].path,sizeof(cache[slot].path),"%s",path);cache[slot].used=true;

found:

    cache[slot].age=++clockAge;

    cover_loader_request(slot,path,card->icon,priority);return slot;

}

static bool publishCover(unsigned slot){

    if(slot>=COVER_SLOTS)return false;

    CoverPixels pixels;if(!cover_loader_take(slot,&pixels))return false;

    Cover *c=&cache[slot];

    if(C3D_TexInit(&c->tex,pixels.textureWidth,pixels.textureHeight,pixels.icon?GPU_RGB565:GPU_RGBA8)){

        memcpy(c->tex.data,pixels.pixels,pixels.bytes);C3D_TexFlush(&c->tex);C3D_TexSetFilter(&c->tex,GPU_LINEAR,GPU_LINEAR);

        c->sub=(Tex3DS_SubTexture){pixels.width,pixels.height,0,1,(float)pixels.width/pixels.textureWidth,1-(float)pixels.height/pixels.textureHeight};

        c->image=(C2D_Image){&c->tex,&c->sub};c->valid=true;

    }

    cover_pixels_free(&pixels);return true;

}

static void prepareCovers(const UiState *s){

    cover_loader_begin();unsigned selectedSlot=COVER_SLOTS;

    if(s->count&&!s->loading){

        selectedSlot=requestCover(&s->cards[s->selected],0);

        int base=(int)lroundf(s->carousel);

        for(int distance=0;distance<=4;distance++)for(int side=-1;side<=1;side+=2){

            if(!distance&&side==1)continue;

            int i=base+distance*side;if(i>=0&&(unsigned)i<s->count&&(unsigned)i!=s->selected)requestCover(&s->cards[i],distance<=2?1:2);

        }

        unsigned current=0;for(unsigned i=0;i<s->count;i++)if(i==s->selected||abs((int)i-base)<=4)current++;

        for(unsigned d=0;d<s->count&&current<20;d++)for(int side=-1;side<=1;side+=2){

            if(!d&&side==1)continue;

            int i=(int)s->selected+(int)d*side;

            if(i<0||(unsigned)i>=s->count||(unsigned)i==s->selected||abs(i-base)<=4||current>=20)continue;

            requestCover(&s->cards[i],3);current++;

        }

    }

    if(!s->loading)for(unsigned i=0;i<s->prefetchCount&&i<COVER_SLOTS;i++)requestCover(&s->prefetch[i],4);

    cover_loader_end();

    /* At most one bounded (256x256) upload before any draw command per frame. */

    if(!publishCover(selectedSlot))for(unsigned i=0;i<COVER_SLOTS;i++)if(i!=selectedSlot&&publishCover(i))break;

}

static void prepareBanner(const UiState *s,bool stereo){

    const char *path=s->systemIndex==SYSTEM_3DS&&s->count&&!s->loading&&!s->searchOpen?s->cards[s->selected].banner:"";

    if(!path)path="";

    u64 now=svcGetSystemTick();

    if(strcmp(path,bannerCandidate)){snprintf(bannerCandidate,sizeof(bannerCandidate),"%s",path);bannerStableSince=now;bannerVisible=false;}

    if(!*path){banner_pause(true);bannerVisible=false;return;}

    /* No header reads, worker joins or allocations while a swipe crosses titles. */

    if(fabsf(s->carousel-s->selected)>.08f){banner_pause(true);bannerStableSince=now;bannerVisible=false;return;}

    if(now-bannerStableSince>=SYSCLOCK_ARM11/8){banner_pause(false);bannerVisible=banner_prepare(path,stereo);}

    else banner_pause(true);

}



static void art(const UiCard *card,float cx,float cy,float maxW,float maxH,bool focus){

 Cover *c=getCover(card);

 float w=maxW,h=maxH;

 if(c){float scale=fminf(maxW/c->sub.width,maxH/c->sub.height);

 w=c->sub.width*scale;

 h=c->sub.height*scale;}else w=fminf(w,h*.72f);

 float x=roundf(cx-w/2),y=roundf(cy-h/2);

 w=roundf(w);

 h=roundf(h);

 rounded(x-4,y+4,w+8,h+5,5,RGBA(0,0,0,35));

 rounded(x-2,y+2,w+4,h+3,3,RGBA(0,0,0,70));

 rect(x-1,y-1,w+2,h+2,focus?accent:RGBA(255,255,255,170));

 if(c){C2D_ImageTint tint;C2D_PlainImageTint(&tint,RGB(100,112,123),.8f);

 C2D_DrawImageAt(c->image,x,y,0,card->pending?&tint:NULL,w/c->sub.width,h/c->sub.height);}

 else {rect(x,y,w,h,raised);

 if(w>=48){centered(cx,cy-16,12,muted,"Kein");

 centered(cx,cy+1,12,muted,"Cover");}}

 if(focus)rounded(cx-11,y+h+5,22,2,1,accent);

 if(card->pending&&w>48){rounded(x,y+h-17,w,17,3,RGBA(8,17,26,235));centered(cx,y+h-16,10,muted,"INSTALLIEREN");}

}

const char *ui_reload_theme(void){

    menu_music_stop();

    ui_runtime_drain();

    for(unsigned i=0;i<2;++i){if(backgrounds[i].valid)C3D_TexDelete(&backgrounds[i].tex);

    memset(&backgrounds[i],0,sizeof(backgrounds[i]));}

    uint8_t *body=NULL;

    size_t size=0;

    const char *source=tr("Standard");

    bool any=false;

    if(theme_read_active(&body,&size,&source)){

        for(unsigned i=0;i<2;++i){Background *b=&backgrounds[i];

        if(!theme_screen(body,size,i!=0,&b->format))continue;

            if(b->format.pixels&&C3D_TexInit(&b->tex,b->format.width,256,GPU_RGB565)){

                memcpy(b->tex.data,b->format.pixels,(size_t)b->format.width*256*2);

                C3D_TexFlush(&b->tex);

                C3D_TexSetFilter(&b->tex,GPU_LINEAR,GPU_LINEAR);

                b->valid=true;

                b->image=(C2D_Image){&b->tex,&b->sub};

            }any=true;

        }

        free(body);

    }

    themeSource=any?source:tr("VitaRenewFlow-Theme");

    return themeSource;

}

const char *ui_theme_source(void){return themeSource;}

static void background(unsigned screen,const UiState *s){

    Background *b=&backgrounds[screen];

    float w=screen?320:400;

    rect(0,0,w,240,b->format.color?b->format.color:RGB(241,243,245));

    if(b->valid){

        float t=s->count>1?s->carousel/(s->count-1):0;

        if(t<0)t=0;

        if(t>1)t=1;

        unsigned x=0;

        if(b->format.frames==3)x=(b->format.mode==2?(frame/24)%3:(unsigned)(t*2+.5f))*320;

        else if(b->format.frameWidth>=(unsigned)w)x=(unsigned)((b->format.frameWidth-w)*t);

        b->sub=(Tex3DS_SubTexture){(u16)w,240,(float)x/b->format.width,1,(float)(x+w)/b->format.width,1-240.f/256};

        C2D_DrawImageAt(b->image,0,0,0,NULL,1,1);

    }else{

        rect(0,0,w,240,RGB(246,247,248));

        for(unsigned y=0;y<240;y+=24)rect(0,y,w,1,RGBA(156,163,172,23));

        for(unsigned x=0;x<(unsigned)w;x+=24)rect(x,0,1,240,RGBA(156,163,172,23));



    }

}



static void battery(void){

 static u8 level,charging;

 static bool available;

 if(frame%180==1&&R_SUCCEEDED(ptmuInit())){available=R_SUCCEEDED(PTMU_GetBatteryLevel(&level));

 PTMU_GetBatteryChargeState(&charging);

 ptmuExit();}

 if(!available)return;

 rounded(347,15,25,12,3,RGBA(233,241,248,150));

 rect(372,19,2,4,RGBA(233,241,248,150));

 rounded(349,17,21,8,1,raised);

 unsigned bars=level>5?5:level;

 if(bars)rounded(350,18,bars*3.8f,6,1,level<2?RGB(255,123,124):accent);

 if(charging){C2D_DrawTriangle(359,14,white,355,21,white,359,20,white,0);

 C2D_DrawTriangle(358,19,white,357,27,white,362,19,white,0);}

}

static void chromeTop(void){

 rounded(12,9,376,26,10,RGBA(6,15,25,232));

 time_t now=time(NULL);

 struct tm *t=localtime(&now);

 char date[48],clock[16];

 const char *days[]={"So","Mo","Di","Mi","Do","Fr","Sa"};

 if(t){snprintf(date,sizeof(date),"%s, %02d.%02d.",tr(days[t->tm_wday]),t->tm_mday,t->tm_mon+1);

 snprintf(clock,sizeof(clock),"%02d:%02d",t->tm_hour,t->tm_min);

 text(24,14,12,ink,date);

 text(265,12,14,ink,clock);}

 unsigned signal=osGetWifiStrength();

 for(unsigned i=0;i<3;++i)rounded(320+i*5,24-i*3,3,3+i*3,1,i<signal?ink:raised);

 battery();

}

static void header(const UiState *s){

 rounded(8,8,304,36,11,panel);

 rounded(280,12,28,28,8,raised);

 for(unsigned i=0;i<3;++i){float y=20+i*6;

 rect(287,y,14,1,muted);

 rounded(i==1?294:289,y-2,4,5,2,ink);}

 key(51,17,"L");

 key(249,17,"R");

 centered(160,17,14,ink,s->system);

 C2D_DrawCircleSolid(23,23,0,7,ink);
 C2D_DrawCircleSolid(23,23,0,5,panel);
 C2D_DrawLine(28,28,ink,34,34,ink,2,0);

}

static void search_top(const UiState *s){
 rect(0,0,400,240,RGB(9,19,29));
 rounded(12,27,376,34,10,panel);
 C2D_DrawCircleSolid(29,42,0,6,accent);C2D_DrawCircleSolid(29,42,0,4,panel);
 C2D_DrawLine(33,47,accent,38,52,accent,2,0);
 title(48,35,14,326,1,*s->searchQuery?s->searchQuery:(l10n_english()?"Search games...":"Spiel suchen..."),false);
 char summary[96];snprintf(summary,sizeof(summary),l10n_english()?"All systems · %u results":"Alle Systeme · %u Treffer",s->count);
 text(16,64,12,muted,summary);
 unsigned page=s->selected/3*3;
 for(unsigned row=0;row<3&&page+row<s->count;row++){
  unsigned i=page+row,y=80+row*51;bool chosen=i==s->selected;
  rounded(12,y,376,48,9,chosen?RGB(25,68,67):panel);
  if(chosen)rounded(12,y+13,3,22,1,accent);
  art(&s->cards[i],40,y+23,32,39,false);
  title(69,y+4,14,302,1,s->cards[i].name,false);
  text(69,y+27,12,chosen?accent:muted,s->searchSystems[i]);
 }
 if(!s->count){centered(200,125,18,ink,l10n_english()?"No games found":"Keine Spiele gefunden");centered(200,159,12,muted,l10n_english()?"Try a shorter or different title.":"Suchbegriff kürzen oder ändern.");}
 if(s->count>3){rect(393,80,2,150,raised);rounded(393,80+126.f*page/(s->count-1),2,24,1,accent);}
}
static void search_panel(const UiState *s){
 rect(0,0,320,240,RGB(9,19,29));
 for(unsigned i=0;i<3;i++){
  unsigned x=8+i*103;const char *word=s->searchSuggestions[i];bool enabled=*word!=0;
  rounded(x,8,98,36,8,enabled?RGB(25,51,59):panel);
  if(enabled)title(x+8,18,14,82,1,word,false);
  else centered(x+49,18,14,muted,"—");
 }
 for(unsigned row=0;row<3;row++){
  const char *keys=search_key_row(row,s->searchNumeric);unsigned n=strlen(keys),left=8+(10-n)*15;
  for(unsigned col=0;col<n;col++){
   unsigned x=left+col*30,y=58+row*39;char c=keys[col],label[2]={c>='a'&&c<='z'?c-32:c,0};
   rounded(x,y,27,34,6,raised);centered(x+13,y+8,14,ink,label);
  }
 }
 rounded(8,177,52,32,7,panel);centered(34,185,12,accent,s->searchNumeric?"ABC":"123");
 rounded(64,177,160,32,7,raised);centered(144,185,12,ink,l10n_english()?"Space":"Leerzeichen");
 rounded(228,177,84,32,7,panel);centered(270,185,12,ink,l10n_english()?"Delete":"Löschen");
 rounded(8,214,70,24,7,panel);centered(43,218,12,ink,tr("Zurück"));
 rounded(82,214,34,24,7,raised);rounded(120,214,34,24,7,raised);
 C2D_DrawTriangle(94,229,muted,104,229,muted,99,222,muted,0);
 C2D_DrawTriangle(132,222,muted,142,222,muted,137,229,muted,0);
 rounded(164,214,148,24,7,s->count?accent:raised);
 centered(238,218,12,s->count?RGB(12,35,36):muted,s->count&&s->cards[s->selected].pending?tr("A Installieren"):l10n_english()?"A Open":"A Öffnen");
}
static void footer(const UiState *s){

 bool enabled=s->count||s->controls;

 rounded(8,206,258,28,9,enabled?accent:raised);

 rounded(274,206,38,28,9,panel);

 if(s->controls){centered(137,210,14,RGB(12,35,36),tr("Zurück"));

 chevron(293,220,false,ink);}

 else {rounded(19,211,18,18,5,enabled?RGB(27,92,83):RGB(46,59,71));

 centered(28,212,12,enabled?white:muted,"A");

 centered(137,210,14,enabled?RGB(12,35,36):muted,s->count&&s->cards[s->selected].pending?tr("Installieren"):tr("Öffnen"));

 chevron(293,220,true,ink);}

}

static void popup(const char *heading){

 rect(0,44,320,196,RGBA(4,10,18,110));

 rounded(8,48,304,186,12,RGB(13,25,37));

 text(20,56,18,ink,heading);

}

static void row(unsigned i,unsigned selected,const char *label){

 unsigned y=86+i*27;

 char line[160];

 const char *p=tr(label);

 wrap_next(&p,14,270,line);

 if(*p)ellipsis(line,14,270);

 if(i==selected)rounded(16,y-2,288,25,6,raised);

 text(25,y,14,i==selected?accent:ink,line);

}

static void controls(const UiState *s){

 popup(tr("Steuerung"));

 const char *keys[]={"A","L/R","X","Y","Touch",s->systemIndex<=SYSTEM_DS?"HOME":"LRY"};

 const char *labels[]={tr("Spiel öffnen"),tr("Kategorie wechseln"),tr("Spielinfos anzeigen"),tr("Kategorienübersicht"),tr("Halten: Cover ändern"),s->systemIndex<=SYSTEM_DS?tr("Im Spiel: HOME"):tr("Im Spiel: L + R + Y")};

 for(unsigned i=0;i<6;++i){unsigned y=84+i*19;

 rounded(20,y,36,17,4,raised);

 centered(38,y,12,accent,keys[i]);

 text(66,y,12,ink,labels[i]);}

}

typedef struct {char text[160];

bool label;} InfoRow;

static InfoRow infoRows[256];

static unsigned infoCount,infoRevision;

static const GameInfo *infoSource;

unsigned ui_info_scroll_limit(void){return infoCount>7?infoCount-7:0;}

static void prepareInfo(const GameInfo *info){

 static bool infoEnglish;

 if(info==infoSource&&info->revision==infoRevision&&infoEnglish==l10n_english())return;

 infoEnglish=l10n_english();

 infoSource=info;

 infoRevision=info->revision;

 infoCount=0;

 for(unsigned i=0;i<info->count&&infoCount<254;++i){

  InfoRow *row=&infoRows[infoCount++];

  snprintf(row->text,sizeof(row->text),"%s",tr(info->fields[i].label));

  row->label=true;

  const char *p=tr(info->fields[i].value);

  while(*p&&infoCount<255){row=&infoRows[infoCount++];

  wrap_next(&p,12,274,row->text);

  row->label=false;}

  if(infoCount<255){row=&infoRows[infoCount++];

  row->text[0]=0;

  row->label=false;}

 }

}

static void information(const UiState *s){

 prepareInfo(s->info);

 rect(0,0,320,240,RGBA(4,10,18,140));

 rounded(8,8,304,226,12,RGB(13,25,37));

 text(20,18,12,accent,"SPIELINFOS");

 key(280,16,"X");

 title(20,41,14,276,2,s->info->title,false);

 rect(20,85,280,1,raised);

 unsigned limit=ui_info_scroll_limit(),scroll=s->infoScroll>limit?limit:s->infoScroll;

 for(unsigned i=0;i<7&&scroll+i<infoCount;++i){const InfoRow *r=&infoRows[scroll+i];

 text(20,96+i*15,12,r->label?accent:ink,r->text);}

 if(limit){rounded(302,96,2,104,1,raised);

 unsigned h=infoCount>52?14:104*7/infoCount;

 rounded(302,96+(104-h)*(float)scroll/limit,2,h,1,accent);}

 rect(20,208,280,1,raised);

 text(20,215,12,muted,tr("B Zurück"));

 text(110,215,12,muted,s->systemIndex==SYSTEM_3DS&&s->count&&!s->cards[s->selected].pending?tr("A Seed importieren"):tr("↑ ↓ Scrollen"));

}

/* HOME has no measurable percentage. The moving indicator is only a busy
   signal; the final frame remains visible during the OS-owned APT handshake. */

static void twlPanel(const UiState *s){
 rect(0,0,320,240,RGB(9,19,29));rounded(8,8,304,36,10,panel);
 centered(160,15,18,ink,tr("DS-Diagnose"));text(289,17,16,ink,"x");
 const char *heading="",*body="",*note="",*action="";
 bool summary=s->twlPage==1;
 if(summary){
  static const char *heads[]={"Prüfung nicht möglich","Reparatur nicht unterstützt","DS-Systemtitel fehlen","DS-Loader einrichten","Kein TWL Fix empfohlen","DS-Modus gegenprüfen"};
  static const char *notes[]={"Erneut prüfen. Es wurde nichts verändert.","Diese Konsole oder Region wird hier nicht unterstützt.","Zuerst fehlende Titel per Systemupdate wiederherstellen.","TWL Fix behebt keine fehlenden Loader.","Nur einzelne Spiele betroffen? ROM und Loader prüfen.","Vor einer Reparatur außerhalb von Flow testen."};
  static const char *actions[]={"Erneut prüfen","Erneut prüfen","Systemupdate","Einrichten","DS-Modus testen","DS-Modus testen"};
  heading=heads[s->twlAdvice];note=notes[s->twlAdvice];action=actions[s->twlAdvice];
  if(s->twlError<0&&s->twlAdvice==TWL_CHECK_CLEAR){heading="Reparatur nicht gestartet";note="Keine Systemtitel gelöscht. SD prüfen und erneut versuchen.";}
  title(20,62,17,280,2,tr(heading),false);
  rounded(12,94,296,31,8,panel);text(22,102,12,muted,tr("DS-System"));
  text(222,102,11,s->twlKnown&&!s->twlMissing?accent:muted,tr(!s->twlKnown?"Ungeprüft":s->twlMissing?"Fehlt":"Vorhanden"));
  rounded(12,133,296,31,8,panel);text(22,141,12,muted,tr("Flow-Loader"));
  text(222,141,11,s->twlKnown&&s->twlLoader&&s->twlFiles?accent:muted,tr(!s->twlKnown?"Ungeprüft":s->twlLoader&&s->twlFiles?"Bereit":"Fehlt"));
  title(20,173,11,280,2,tr(note),false);
 }else{
  if(s->twlPage==2){
   heading="DS-Modus gegenprüfen";
   body="Starten alle DS-Spiele und DS Download Play auch außerhalb Flow nicht?";
   note="Im HOME-Menü Download Play öffnen und Nintendo DS wählen. Erst danach bestätigen.";
   action="Ja, beides";
  }else if(s->twlPage==3){
   heading="TWL-Reparatur bestätigen";
   body="Entfernt DS-Systemteile. Danach ist ein Systemupdate nötig. Spiele und Saves bleiben erhalten.";
   note="NAND-Backup, Internet und Netzteil müssen bereit sein.";
   action="START halten + A";
  }else if(s->twlPage==4){
   heading="Systemupdate erforderlich";
   body=s->twlNeedsReboot?"Zuerst neu starten. Danach Systemeinstellungen öffnen und Systemupdate ausführen.":"Systemeinstellungen öffnen: Andere Einstellungen → Systemupdate. Auch bei aktueller Version ausführen.";
   note=s->twlError<0?"Schritt fehlgeschlagen. Nicht erneut reparieren; Diagnose-Log für Hilfe aufbewahren.":"Nach dem Update erneut prüfen. TWL Fix nicht wiederholen.";
   action=s->twlNeedsReboot?"Neu starten":"Einstellungen";
  }else if(s->twlPage==5){
   heading="Systemtitel wieder vorhanden";
   body="Jetzt ein DS-Spiel und DS Download Play testen.";
   note="Erst bestätigen, wenn beides wieder startet. Noch defekt? TWL Fix nicht wiederholen.";
   action="Test bestanden";
  }else{
   heading="Fehlende Titel wiederherstellen";
   body="Ein Systemupdate kann fehlende DS-Systemtitel wiederherstellen. Dafür ist kein vorheriges Löschen nötig.";
   note=s->twlError<0?"Einstellungen nicht geöffnet. Über das HOME-Menü erneut versuchen.":"In den Systemeinstellungen: Andere Einstellungen → Systemupdate.";
   action="Einstellungen";
  }
  title(20,62,18,280,2,tr(heading),false);
  title(20,103,13,280,4,tr(body),false);
  title(20,167,11,280,2,tr(note),false);
 }
 bool three=(summary&&s->twlAdvice>=TWL_CHECK_UPDATE)||s->twlPage==4;
 if(three){
  rounded(12,201,82,30,8,raised);centered(53,209,11,ink,tr(summary?"Schließen":"Später"));
  rounded(104,201,82,30,8,raised);centered(145,209,11,ink,tr("Prüfen"));
  rounded(196,201,112,30,8,accent);centered(252,209,11,RGB(12,35,36),tr(action));
 }else{
  rounded(12,201,100,30,8,raised);centered(62,209,12,ink,tr(summary?"Schließen":s->twlPage==5?"Später":"Zurück"));
  rounded(120,201,188,30,8,s->twlPage==3?RGB(73,35,42):accent);
  centered(214,209,12,s->twlPage==3?RGB(255,187,179):RGB(12,35,36),tr(action));
 }
}

static void homeLoadingScreen(const UiState *s,bool upper){
 float cx=upper?200:160,width=upper?400:320;
 rect(0,0,width,240,RGB(6,13,23));
 if(upper){
  rounded(cx-34,36,68,68,20,RGB(20,41,52));
  for(unsigned row=0;row<3;row++)for(unsigned col=0;col<3;col++)
   rounded(cx-17+col*13,53+row*13,8,8,2,accent);
  centered(cx,124,23,ink,s->homeLoading==1?tr("HOME vorbereiten"):tr("HOME öffnen"));
  centered(cx,163,13,muted,s->homeLoading==3?tr("Neustart ins HOME-Menü"):s->homeLoading==1?tr("Systemmenü wird geladen"):tr("Übergabe an das System"));
 }else{
  centered(cx,74,19,ink,tr("Einen Moment"));
  title(cx,108,13,252,2,tr("Das HOME-Menü wird geöffnet."),true);
  rounded(72,157,176,4,2,RGB(24,42,53));
  float t=(osGetTime()%1800)/1800.f;
  rounded(72+132*(.5f-.5f*cosf(t*6.2831853f)),157,44,4,2,accent);
  centered(cx,193,11,muted,"VitarenewFlow");
 }
}
static void drawTop(const UiState *s,float eye){
 if(s->homeLoading){homeLoadingScreen(s,true);return;}
 if(s->searchOpen&&!s->loading){search_top(s);chromeTop();return;}

 background(0,s);

 rect(0,0,400,240,RGBA(4,12,22,48));

 const UiCard *current=s->count&&s->selected<s->count?&s->cards[s->selected]:NULL;

 if(current){if(s->systemIndex==SYSTEM_3DS){if(!bannerVisible||!banner_draw(eye<0))art(current,200+eye,135,132,132,false);}else art(current,200,138,348,176,false);}

 else {centered(200,105,18,ink,tr("Deine Sammlung"));

 centered(200,139,12,ink,tr("Hier sind noch keine Spiele."));}

 chromeTop();

 if(s->loading){rect(0,0,400,240,RGB(6,13,23));

 if(s->launching){
 centered(200,92,18,ink,tr("Spielstart"));
 centered(200,130,12,muted,s->stage);
 }

 else if(!s->installing&&brand_ui_draw()){

 rect(0,209,400,31,RGB(24,37,45));

 centered(200,215,12,muted,s->stage);

 rounded(40,235,320,3,1,raised);

 rounded(40,235,320*s->progress,3,1,accent);

 }

 else{centered(200,85,26,ink,s->installing?tr("Wird installiert"):"VitarenewFlow");

 centered(200,130,12,muted,s->stage);

 rounded(40,166,320,4,2,raised);

 rounded(40,166,320*s->progress,4,2,accent);}}

}

bool ui_init(void){

    gfxInitDefault();

    gfxSet3D(true);

    if(!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE))return false;

    if(!C2D_Init(C2D_DEFAULT_MAX_OBJECTS))return false;

    C2D_Prepare();

    top=C2D_CreateScreenTarget(GFX_TOP,GFX_LEFT);

    right=C2D_CreateScreenTarget(GFX_TOP,GFX_RIGHT);

    bottom=C2D_CreateScreenTarget(GFX_BOTTOM,GFX_LEFT);

    textbuf=C2D_TextBufNew(32768);

    if(!top||!right||!bottom||!textbuf)return false;

    ui_font_init();

    brand_ui_init();

    ui_reload_theme();

    cover_loader_init();bannerCandidate[0]=0;bannerVisible=false;

    renderTotal=renderMax=renderFrames=renderSlow=0;

    return true;

}

void ui_clear_covers(void){cover_loader_clear();ui_runtime_drain();

for(unsigned i=0;i<COVER_SLOTS;++i){if(cache[i].valid)C3D_TexDelete(&cache[i].tex);

memset(&cache[i],0,sizeof(cache[i]));}}

bool ui_cover_available(const char *path){struct stat s;

return !stat(path,&s)&&S_ISREG(s.st_mode);}

bool ui_validate_cover(const char *path){

    png_image p;

    memset(&p,0,sizeof(p));

    p.version=PNG_IMAGE_VERSION;

    if(!png_image_begin_read_from_file(&p,path))return false;

    if(!p.width||!p.height||p.width>512||p.height>512){png_image_free(&p);

    return false;}p.format=PNG_FORMAT_RGBA;

    void *pixels=malloc(PNG_IMAGE_SIZE(p));

    if(!pixels){png_image_free(&p);

    return false;}bool ok=png_image_finish_read(&p,NULL,pixels,0,NULL)!=0;

    free(pixels);

    png_image_free(&p);

    return ok;

}



void ui_draw(const UiState *s){

 if(!ui_runtime_begin())return;

 u64 renderStarted=svcGetSystemTick();

 ++frame;

 C2D_TextBufClear(textbuf);

 /* Prior GPU frame completed; publish at most one ready cover. */

 prepareCovers(s);

 float slider=osGet3DSliderState()*4.f;

 prepareBanner(s,slider>0);

 C2D_TargetClear(top,white);

 C2D_SceneBegin(top);

 drawTop(s,slider);

 if(slider>0){C2D_TargetClear(right,white);

 C2D_SceneBegin(right);

 drawTop(s,-slider);}

 C2D_TargetClear(bottom,white);

 C2D_SceneBegin(bottom);

 if(s->homeLoading){homeLoadingScreen(s,false);C3D_FrameEnd(0);return;}

 static char lastStatus[180];
 static unsigned statusFrame;
 if(!s->searchOpen){
 background(1,s);

 rect(0,0,320,240,RGBA(4,12,22,125));

 int base=(int)lroundf(s->carousel);

 for(int distance=3;distance>=0;--distance)for(int side=-1;side<=1;side+=2){

  if(!distance&&side==1)continue;

  int i=base+distance*side;

  if(i<0||(unsigned)i>=s->count)continue;

  float d=i-s->carousel,amount=fabsf(d),scale=1.f-fminf(amount*.14f,.46f),cx=160+d*94.f;

  if(cx< -70||cx>390)continue;

  art(&s->cards[i],cx,102+amount*2,84*scale/(1+amount*.17f),108*scale,(unsigned)i==s->selected);

 }

 header(s);

 rounded(8,163,304,37,9,panel);

 if(s->count)title(160,ui_font_width(14,s->cards[s->selected].name)<=286?173:164,14,286,2,s->cards[s->selected].name,true);

 else centered(160,174,12,muted,tr("L / R: Kategorie wechseln"));

 char counter[32];

 snprintf(counter,sizeof(counter),"%u / %u",s->count?s->selected+1:0,s->count);

 rounded(254,142,58,16,5,panel);

 centered(283,142,12,muted,counter);

 footer(s);

 if(strcmp(lastStatus,s->status)){snprintf(lastStatus,sizeof(lastStatus),"%s",s->status);

 statusFrame=frame;}

 if(strcmp(s->status,tr("Bereit"))&&frame-statusFrame<180&&!s->controls&&!s->info){rounded(8,163,304,37,9,RGB(20,43,50));

 title(160,165,12,286,2,tr(s->status),true);}

 }
 if(s->searchOpen)search_panel(s);

 if(s->confirm){popup(tr("Spiel öffnen?"));

 if(s->count)title(160,100,14,270,2,s->cards[s->selected].name,true);

 centered(160,171,12,accent,tr("A Öffnen      B Zurück"));}

 if(s->coverMenu){popup(tr("Cover hinzufügen"));

 unsigned page=s->importSelected/4*4;

  for(unsigned i=0;i<4&&page+i<s->importCount;++i)row(i,s->importSelected-page,s->imports[page+i]);

  if(!s->importCount){text(20,98,12,muted,tr("PNG-Datei im Import-Ordner ablegen."));

  text(20,119,12,muted,"/3ds/3ds-library/import/");}

  text(20,214,12,muted,tr("A Zuordnen   B Zurück   SELECT Neu"));

 }

 if(s->setupDs){popup(tr("DS-Loader fehlt"));

 text(20,99,12,ink,tr("Einmalig YANBF bootstrap installieren."));

 text(20,163,14,accent,tr("A Einrichten     B Zurück"));}

 if(s->settings){

  rect(0,0,320,240,RGB(9,19,29));

  rounded(8,8,304,36,10,panel);

  if(s->settingsPage!=MENU_ROOT){rounded(12,12,28,28,8,raised);chevron(26,26,false,ink);}

  centered(160,15,18,ink,tr(settings_menu_title(s->settingsPage)));

  rounded(280,12,28,28,8,raised);

  for(unsigned i=0;i<3;i++){unsigned y=20+i*6;rect(287,y,14,1,accent);rounded(i==1?294:289,y-2,4,5,2,accent);}

  for(unsigned i=0;i<settings_menu_count(s->settingsPage);i++){

   const MenuItem *item=settings_menu_item(s->settingsPage,i);unsigned y=54+i*46;

   bool active=i==s->menuSelected;rounded(12,y,296,40,9,active?raised:panel);

   if(active)rounded(12,y+10,3,20,1,accent);

   const char *value=item->action==MENU_LANGUAGE?(l10n_english()?"English":"Deutsch"):

    item->action==MENU_AUTOBOOT?(s->autobootState==1?tr("An"):s->autobootState==0?tr("Aus"):tr("Prüfen")):NULL;

   text(25,y+11,16,active?accent:ink,tr(item->label));

   if(value){float w=ui_font_width(12,value);rounded(292-w-12,y+9,w+12,23,6,RGB(23,51,57));text(292-w-6,y+13,12,accent,value);}

   else chevron(291,y+20,true,muted);

  }

  if(s->settingsPage!=MENU_ROOT&&settings_menu_count(s->settingsPage)<4&&strcmp(s->status,tr("Bereit"))&&frame-statusFrame<180){title(20,218,11,280,1,tr(s->status),false);}

 }

 if(s->componentPage){

  const ComponentStatus *c=&s->component;bool busy=c->state==COMP_DOWNLOAD||c->state==COMP_READY;

  rect(0,0,320,240,RGB(9,19,29));rounded(8,8,304,36,10,panel);centered(160,15,18,ink,tr(s->componentSelected==6?"Desktop-Verwaltung":"Emulatoren"));text(289,17,16,ink,"x");

  if(busy){title(20,70,17,280,2,component_definition(s->componentSelected)->name,false);text(20,118,14,muted,tr("Herunterladen und prüfen..."));float p=c->total?(float)c->received/c->total:0;rounded(20,156,280,6,3,raised);rounded(20,156,280*fminf(p,1.f),6,3,accent);char amount[48];snprintf(amount,sizeof(amount),"%.1f / %.1f MB",c->received/1048576.f,c->total/1048576.f);centered(160,174,12,muted,amount);centered(160,202,14,ink,tr("Abbrechen"));}

  else {

   if(s->componentSelected==6){text(20,65,18,ink,"VitarenewFlow Desktop");title(20,99,13,280,3,tr("Spiele hinzufügen und Cover pflegen. Die Windows-App wird auf der SD gespeichert."),false);ComponentAvailability desktop=components_availability(6);text(20,151,12,desktop==COMP_AVAILABLE?accent:muted,tr(desktop==COMP_AVAILABLE?"Auf SD vorhanden":desktop==COMP_UNCHECKED?"Status unklar":"Noch nicht installiert"));}
   else for(unsigned row=0;row<3;row++){unsigned i=s->componentSelected/3*3+row;const ComponentDef *d=component_definition(i);float y=54+row*40;rounded(12,y,296,35,8,i==s->componentSelected?raised:panel);text(22,y+8,15,ink,d->name);ComponentAvailability found=components_availability(i);const char *label=!d->count?"Später":found==COMP_AVAILABLE?"Bereit":found==COMP_UNCHECKED?"Unklar":"Fehlt";text(250,y+10,11,i==s->componentSelected?accent:muted,tr(label));}

   const ComponentDef *d=component_definition(s->componentSelected);const char *msg=c->state==COMP_ERROR||c->state==COMP_DONE||c->state==COMP_CANCELLED?c->message:!d->count?"Download noch nicht freigegeben.":components_presence_detail(s->componentSelected);title(20,174,11,280,2,tr(msg),false);

   if(s->componentSelected!=6){
    rounded(12,201,48,30,8,raised);centered(36,209,11,ink,"L/R");
    rounded(68,201,116,30,8,raised);centered(126,209,12,s->componentRechecked?accent:ink,tr(s->componentRechecked?"Geprüft":"Erneut prüfen"));
    if(d->count){ComponentAvailability found=components_availability(s->componentSelected);rounded(192,201,116,30,8,found==COMP_UNCHECKED?raised:accent);centered(250,209,11,found==COMP_UNCHECKED?muted:RGB(12,35,36),tr(found==COMP_UNCHECKED?"Erst prüfen":found==COMP_AVAILABLE?"Neu einrichten":"Installieren"));}
   }else{
    rounded(12,201,82,30,8,raised);centered(53,208,13,ink,tr("Zurück"));
    rounded(104,201,82,30,8,raised);centered(145,209,11,s->componentRechecked?accent:ink,tr(s->componentRechecked?"Geprüft":"Prüfen"));
    if(d->count){ComponentAvailability found=components_availability(6);rounded(196,201,112,30,8,found==COMP_UNCHECKED?raised:accent);centered(252,209,12,found==COMP_UNCHECKED?muted:RGB(12,35,36),tr(found==COMP_UNCHECKED?"Erst prüfen":found==COMP_AVAILABLE?"Reparieren":"Installieren"));}
   }

  }

 }

 if(s->updatePage){

  const UpdateStatus *u=&s->update;bool busy=u->state==UPDATE_CHECKING||u->state==UPDATE_DOWNLOADING;

  rect(0,0,320,240,RGB(9,19,29));rounded(8,8,304,36,10,panel);

  centered(160,15,18,ink,tr("Updates"));rounded(280,12,28,28,8,raised);text(289,17,16,ink,"x");

  char label[96];snprintf(label,sizeof(label),"%s  %s",tr("Installiert"),s->version);text(20,61,14,muted,label);

  if(u->available[0]){snprintf(label,sizeof(label),"%s  %s",tr("Verfügbar"),u->available);text(20,87,16,accent,label);}

  const char *message=u->state==UPDATE_CHECKING?tr("Updates werden geprüft..."):u->state==UPDATE_DOWNLOADING?tr("Update wird geladen..."):tr(u->message);

  unsigned noteCount=settings_release_note_count(s->version);
  if(s->updateNotes&&!busy&&noteCount){
   text(20,110,11,muted,tr("Installierte Version: Neuerungen"));
   for(unsigned i=0;i<noteCount;i++)text(20,127+i*15,11,ink,tr(settings_release_note(s->version,i)));
  }else title(20,113,12,280,busy?3:2,message,false);

  if(!busy&&noteCount){
   rounded(20,172,280,14,4,raised);
   centered(160,173,10,accent,tr(s->updateNotes?"X Neuerungen einklappen":"X Neuerungen dieser Version"));
  }

  if(u->state==UPDATE_DOWNLOADING){float p=u->total?(float)u->received/u->total:0;

   rounded(20,172,280,5,2,raised);rounded(20,172,280*fminf(p,1.f),5,2,accent);}

  rounded(12,192,139,38,9,raised);centered(81,203,14,ink,tr(busy?"Abbrechen":"Zurück"));

  if(!busy){const char *action=u->state==UPDATE_AVAILABLE?tr("Aktualisieren"):u->state==UPDATE_INSTALLED?tr("Neu öffnen"):tr("Erneut prüfen");

   rounded(169,192,139,38,9,accent);centered(238,203,14,RGB(12,35,36),action);}

 }

 if(s->autobootPrompt){

  bool enable=s->autobootState==0;

  popup(tr(enable?"Autostart aktivieren?":"Autostart deaktivieren?"));

  title(20,92,13,278,2,tr(enable?"Flow startet beim Einschalten.":"Die Konsole startet ins HOME-Menü."),false);

  text(20,137,12,muted,tr("Gilt ab dem nächsten Einschalten."));

  text(20,156,11,muted,tr("B beim Einschalten öffnet den HBL."));

  rounded(20,184,134,30,7,raised);centered(87,191,13,ink,tr("Abbrechen"));

  rounded(166,184,134,30,7,accent);centered(233,191,13,RGB(12,35,36),tr(enable?"Aktivieren":"Deaktivieren"));

  centered(160,220,11,muted,tr("A Bestätigen    B Abbrechen"));

 }

 if(s->twlPage)twlPanel(s);

 if(s->copyPrompt){

  popup(tr("Modul kopieren"));

  text(20,88,12,ink,tr("Kopieren erfolgt mit GodMode9."));

  text(20,112,12,ink,tr("START halten und A drücken: Neustart."));

  text(20,136,12,muted,tr("GodMode9: HOME > Scripts"));

  text(20,160,12,ink,"VitarenewFlow-Copy");

  text(20,184,12,muted,tr("Danach zurück zur Library."));

  text(20,220,12,ink,tr("A Bestätigen    B Abbrechen"));

 }

 if(s->powerPrompt){
  rect(0,0,320,240,RGB(9,19,29));
  rounded(8,8,304,36,10,panel);
  C2D_DrawCircleSolid(29,26,0,9,accent);C2D_DrawCircleSolid(29,26,0,7,panel);
  rect(25,15,8,7,panel);rounded(28,15,2,12,1,accent);
  text(49,16,18,ink,l10n_english()?"Console":"Konsole");
  const char *labels[]={l10n_english()?"HOME Menu":"HOME-Menü",l10n_english()?"Restart":"Neu starten","Homebrew Launcher",l10n_english()?"Power off":"Ausschalten"};
  const char *buttons[]={"A","X","Y","SEL"};
  for(unsigned i=0;i<4;i++){
   unsigned y=54+i*37;u32 tone=i==3?RGB(255,161,154):ink;
   rounded(12,y,296,31,8,i==3?RGB(52,29,38):panel);
   rounded(21,y+6,32,19,5,raised);centered(37,y+8,12,tone,buttons[i]);
   text(65,y+7,14,tone,labels[i]);chevron(291,y+16,true,tone);
  }
  rounded(12,205,296,30,9,accent);
  centered(160,213,12,RGB(12,35,36),s->powerCancelFailed?(l10n_english()?"B Retry cancellation":"B Abbrechen erneut versuchen"):l10n_english()?"B Cancel":"B Abbrechen");
 }

 if(s->support){
  rect(0,0,320,240,RGB(9,19,29));
  rounded(8,8,304,36,10,panel);
  centered(160,15,18,ink,tr("Über & Unterstützung"));
  rounded(12,54,296,65,10,panel);
  text(24,64,20,ink,"VitarenewFlow");
  char version[64];snprintf(version,sizeof(version),"%s %s",tr("Version"),s->version);
  text(24,91,12,accent,version);
  text(178,92,11,muted,tr("Entwicklungsstand"));
  rounded(12,127,296,68,10,panel);
  text(24,137,11,muted,tr("Von VitaRenew für deine Sammlung."));
  text(24,157,18,accent,"vitarenew.de");
  text(24,180,10,muted,tr("Aufbereitete Konsolen & Zubehör."));
  rounded(12,205,296,30,9,raised);
  centered(160,213,12,ink,tr("A / B Zurück"));
 }

 if(s->systemPicker){popup(tr("Kategorien"));

  for(unsigned i=0;i<VIEW_COUNT;++i){unsigned x=16+(i%2)*148,y=83+(i/2)*31;

  if(i==s->menuSelected)rounded(x,y,140,27,6,raised);

  text(x+7,y+5,12,i==s->menuSelected?accent:ink,library_view_names[i]);}

  text(20,216,12,muted,tr("A Auswählen    B Zurück"));

 }

 if(s->controls){controls(s);

 footer(s);}

 if(s->info)information(s);

 if(s->installPrompt){popup(tr("Spiel installieren"));

  text(20,82,12,muted,tr("Nach Erfolg wird die CIA-Datei gelöscht."));

  if(s->count)title(20,103,14,278,2,s->cards[s->selected].name,false);

  char line[100];snprintf(line,sizeof(line),tr("CIA-Datei: %s"),s->installSize);text(20,141,12,ink,line);

  snprintf(line,sizeof(line),tr("SD-Speicher: %s"),s->installSpace);text(20,158,12,muted,line);

  snprintf(line,sizeof(line),tr("Geschätzte Dauer: %s"),s->installEta);text(20,175,12,ink,line);

  rounded(16,198,137,28,8,raised);centered(84,203,12,ink,tr("B Zurück"));

  rounded(162,198,142,28,8,accent);centered(233,203,12,RGB(12,35,36),tr("A Installieren"));

 }

 if(s->installError){popup(s->errorTitle?s->errorTitle:s->launchError?tr("Spielstart fehlgeschlagen"):tr("Installation nicht möglich"));

  title(20,91,12,280,5,tr(s->installError),false);

  if(s->launchRepair){
   rounded(16,198,78,28,8,raised);centered(55,203,12,ink,tr("B Zurück"));
   rounded(100,198,204,28,8,accent);centered(202,203,12,RGB(12,35,36),l10n_english()?"A Set up emulator":"A Emulator einrichten");
  }else{
   rounded(16,198,288,28,8,accent);centered(160,203,12,RGB(12,35,36),tr("A / B Zurück"));
  }

 }

 if(s->loading){rect(0,0,320,240,RGB(6,13,23));

 if(s->installing){text(20,24,18,ink,"Installation");

  if(s->count)title(20,59,14,280,2,s->cards[s->selected].name,false);

  char line[96];snprintf(line,sizeof(line),"%.0f %%   %s",s->progress*100,s->installSize);text(20,106,12,ink,line);

  rounded(20,135,280,5,2,raised);rounded(20,135,280*s->progress,5,2,accent);

  snprintf(line,sizeof(line),tr("Restzeit: %s"),s->installEta);text(20,157,12,muted,line);

  title(20,191,12,280,2,s->stage,false);

 }else centered(160,108,12,muted,s->stage);}

 C3D_FrameEnd(0);

 if(!s->loading){u64 elapsed=svcGetSystemTick()-renderStarted;renderFrames++;renderTotal+=elapsed;if(elapsed>renderMax)renderMax=elapsed;if(elapsed>SYSCLOCK_ARM11/60)renderSlow++;}

}

void ui_shutdown(void){

 menu_music_stop();

 cover_loader_shutdown();

 CoverLoaderStats stats=cover_loader_stats();char log[180];

 app_perf_log("Performance 0.12.8");

 snprintf(log,sizeof(log),"UI render CPU: frames=%u avg=%.2fms max=%.2fms over16.7ms=%u (not display FPS)",renderFrames,renderFrames?renderTotal*1000.0/SYSCLOCK_ARM11/renderFrames:0,renderMax*1000.0/SYSCLOCK_ARM11,renderSlow);app_perf_log(log);

 snprintf(log,sizeof(log),"Cover worker: decoded=%u failed=%u stale=%u avg=%.2fms max=%.2fms",stats.decoded,stats.failed,stats.discarded,stats.decoded?stats.totalTicks*1000.0/SYSCLOCK_ARM11/stats.decoded:0,stats.maxTicks*1000.0/SYSCLOCK_ARM11);app_perf_log(log);

 snprintf(log,sizeof(log),"Cover phase max: PNG=%.2fms resize=%.2fms tile=%.2fms (independent maxima)",stats.maxReadTicks*1000.0/SYSCLOCK_ARM11,stats.maxResizeTicks*1000.0/SYSCLOCK_ARM11,stats.maxTileTicks*1000.0/SYSCLOCK_ARM11);app_perf_log(log);

 ui_clear_covers();

 banner_close();

 brand_ui_free();

 ui_font_free();

 for(unsigned i=0;i<2;++i){if(backgrounds[i].valid)C3D_TexDelete(&backgrounds[i].tex);

 memset(&backgrounds[i],0,sizeof(backgrounds[i]));}

 if(textbuf)C2D_TextBufDelete(textbuf);

 textbuf=NULL;

 C2D_Fini();

 C3D_Fini();

 gfxExit();

}





