// SPDX-License-Identifier: GPL-3.0-only
#include "error_report.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static const char *const areas[]={"general","launch","install","update","component","ds","home","power","audio","cover","storage","desktop"};
static const char *const models[]={"unknown","3ds","3ds-xl","2ds","new-3ds","new-3ds-xl","new-2ds-xl","pc"};
static const char *const regions[]={"unknown","jpn","usa","eur","aus","chn","kor","twn"};
static const char *const autoboots[]={"unknown","off","on"};
static const char *const contexts[]={"unknown","homebrew","native","pc"};
static const char *const activities[]={"unknown","browsing","starting-game","installing","checking-update","downloading","checking-components","checking-ds","opening-home","power-menu","loading-theme","loading-cover","importing"};

static bool bounded_length(const char *text,size_t maximum,size_t *length){
    if(!text)return false;
    for(size_t n=0;n<=maximum;n++)if(!text[n]){*length=n;return true;}
    return false;
}
static bool version_valid(const char *version){
    size_t n;if(!bounded_length(version,ERROR_REPORT_VERSION_MAX,&n)||!n)return false;
    unsigned dots=0,digits=0;
    for(size_t i=0;i<n;i++){
        if(version[i]>='0'&&version[i]<='9')digits++;
        else if(version[i]=='.'&&digits&&dots<2){dots++;digits=0;}
        else return false;
    }
    return dots==2&&digits;
}
static bool utf8_valid(const char *text,size_t n){
    const unsigned char *p=(const unsigned char*)text;
    for(size_t i=0;i<n;){
        unsigned char first=p[i++];if(first<0x80)continue;
        unsigned following;uint32_t code,minimum;
        if(first>=0xc2&&first<=0xdf){following=1;code=first&0x1f;minimum=0x80;}
        else if(first>=0xe0&&first<=0xef){following=2;code=first&0x0f;minimum=0x800;}
        else if(first>=0xf0&&first<=0xf4){following=3;code=first&7;minimum=0x10000;}
        else return false;
        if(following>n-i)return false;
        while(following--){unsigned char next=p[i++];if((next&0xc0)!=0x80)return false;code=(code<<6)|(next&0x3f);}
        if(code<minimum||code>0x10ffff||(code>=0xd800&&code<=0xdfff))return false;
    }
    return true;
}
typedef struct {char *out;size_t capacity,used;bool ok;} Writer;
static void append(Writer *w,const char *text,size_t n){
    if(!w->ok)return;
    if(n>=w->capacity-w->used){w->ok=false;return;}
    memcpy(w->out+w->used,text,n);w->used+=n;w->out[w->used]=0;
}
static void literal(Writer *w,const char *text){append(w,text,strlen(text));}
static void quoted(Writer *w,const char *text){
    static const char hex[]="0123456789abcdef";literal(w,"\"");
    for(const unsigned char *p=(const unsigned char*)text;*p;p++){
        if(*p=='"'||*p=='\\'){char pair[2]={'\\',(char)*p};append(w,pair,2);}
        else if(*p<0x20){char escape[6]={'\\','u','0','0',hex[*p>>4],hex[*p&15]};append(w,escape,6);}
        else {char c=(char)*p;append(w,&c,1);}
    }
    literal(w,"\"");
}
static void field(Writer *w,const char *name,const char *value){literal(w,",");quoted(w,name);literal(w,":");quoted(w,value);}

bool error_report_json(const ErrorReport *r,char *out,size_t capacity,size_t *written){
    if(written)*written=0;
    if(!out||!capacity)return false;
    out[0]=0;
    if(!r||!version_valid(r->version)||(unsigned)r->area>=ERROR_REPORT_AREA_COUNT||
       (unsigned)r->model>=ERROR_REPORT_MODEL_COUNT||(unsigned)r->region>=ERROR_REPORT_REGION_COUNT||
       (unsigned)r->autoboot>=ERROR_REPORT_AUTOBOOT_COUNT||(unsigned)r->context>=ERROR_REPORT_CONTEXT_COUNT||
       (unsigned)r->activity>=ERROR_REPORT_ACTIVITY_COUNT)return false;
    const char *description=r->description?r->description:"";
    size_t n;if(!bounded_length(description,ERROR_REPORT_DESCRIPTION_MAX,&n)||!utf8_valid(description,n))return false;
    Writer w={out,capacity<ERROR_REPORT_JSON_CAPACITY?capacity:ERROR_REPORT_JSON_CAPACITY,0,true};
    literal(&w,"{\"schema\":1");field(&w,"version",r->version);field(&w,"area",areas[r->area]);
    char code[16];snprintf(code,sizeof(code),"%" PRId32,r->code);literal(&w,",\"code\":");literal(&w,code);
    field(&w,"model",models[r->model]);field(&w,"region",regions[r->region]);
    field(&w,"autoboot",autoboots[r->autoboot]);field(&w,"context",contexts[r->context]);
    field(&w,"activity",activities[r->activity]);field(&w,"description",description);literal(&w,"}");
    if(!w.ok){out[0]=0;return false;}
    if(written)*written=w.used;
    return true;
}
