// SPDX-License-Identifier: GPL-3.0-only
#include "game_search.h"
#include <string.h>
#include <stdbool.h>
void search_normalize(char *out,size_t cap,const char *in){
    if(!cap)return;
    size_t n=0;bool space=false;
    for(size_t i=0;in&&in[i]&&n+1<cap;i++){
        unsigned char c=(unsigned char)in[i];const char *mapped=NULL;char one[2]={0};
        if(c>='A'&&c<='Z')c+=32;
        if((c>='a'&&c<='z')||(c>='0'&&c<='9')){one[0]=c;mapped=one;}
        else if(c==0xc3&&in[i+1]){
            unsigned char d=(unsigned char)in[++i];
            if(d==0x9f)mapped="ss";
            else if((d>=0x80&&d<=0x85)||(d>=0xa0&&d<=0xa5))mapped="a";
            else if(d==0x87||d==0xa7)mapped="c";
            else if((d>=0x88&&d<=0x8b)||(d>=0xa8&&d<=0xab))mapped="e";
            else if((d>=0x8c&&d<=0x8f)||(d>=0xac&&d<=0xaf))mapped="i";
            else if(d==0x91||d==0xb1)mapped="n";
            else if((d>=0x92&&d<=0x96)||(d>=0xb2&&d<=0xb6))mapped="o";
            else if((d>=0x99&&d<=0x9c)||(d>=0xb9&&d<=0xbc))mapped="u";
            else if(d==0x9d||d==0xbd||d==0xbf)mapped="y";
        }else if(c>=0x80){
            /* Skip a whole unsupported UTF-8 sequence, including combining marks. */
            while(((unsigned char)in[i+1]&0xc0)==0x80)i++;
            continue;
        }
        if(mapped){
            if(space&&n&&n+1<cap)out[n++]=' ';
            space=false;
            for(size_t j=0;mapped[j]&&n+1<cap;j++)out[n++]=mapped[j];
        }else space=n!=0;
    }
    while(n&&out[n-1]==' ')n--;
    out[n]=0;
}
static int min3(int a,int b,int c){int x=a<b?a:b;return x<c?x:c;}
/* Bounded optimal-string-alignment distance, including adjacent swaps. */
static int distance(const char *a,size_t n,const char *b,size_t m){
    if(n>32||m>32||n+2<m||m+2<n)return 3;
    unsigned char rows[3][33];
    for(size_t j=0;j<=m;j++)rows[0][j]=j;
    for(size_t i=1;i<=n;i++){
        unsigned char *r=rows[i%3],*p=rows[(i+2)%3],*pp=rows[(i+1)%3];r[0]=i;
        for(size_t j=1;j<=m;j++){
            int d=min3(p[j]+1,r[j-1]+1,p[j-1]+(a[i-1]!=b[j-1]));
            if(i>1&&j>1&&a[i-1]==b[j-2]&&a[i-2]==b[j-1]&&pp[j-2]+1<d)d=pp[j-2]+1;
            r[j]=d;
        }
    }
    return rows[n%3][m];
}
static void compact(char *out,const char *in){while(*in){if(*in!=' ')*out++=*in;in++;}*out=0;}
int search_score(const char *title,const char *query){
    if(!*query)return 1;
    if(!strcmp(title,query))return 20000;
    if(!strncmp(title,query,strlen(query)))return 16000;
    if(strstr(title,query))return 14000;
    char ct[SEARCH_TITLE_SIZE],cq[SEARCH_TITLE_SIZE];
    compact(ct,title);compact(cq,query);
    if(*cq&&strstr(ct,cq))return 12000;
    int total=0;const char *q=query;
    while(*q){
        size_t n=strcspn(q," ");int best=0;
        for(const char *t=title;*t;){
            size_t m=strcspn(t," ");int value=0;
            if(n==m&&!memcmp(q,t,n))value=1000;
            else if(n<=m&&!memcmp(q,t,n))value=850;
            else if(n>=3){int d=distance(q,n,t,m);if(d<= (n>=7?2:1))value=600-d*100;}
            if(value>best)best=value;
            t+=m;if(*t)t++;
        }
        if(!best)return 0;
        total+=best;q+=n;if(*q)q++;
    }
    return total;
}
const char *search_key_row(unsigned row,int numeric){
    static const char *letters[]={"qwertzuiop","asdfghjkl","yxcvbnm"};
    static const char *numbers[]={"1234567890","-+&:!?.,'","()/"};
    return row<3?(numeric?numbers[row]:letters[row]):"";
}
char search_key_at(int x,int y,int numeric){
    if(y<58||y>=170)return 0;
    int row=(y-58)/39;
    const char *keys=search_key_row(row,numeric);
    int left=8+(10-(int)strlen(keys))*15;
    if(x<left||x>=left+(int)strlen(keys)*30||(y-58)%39>=34)return 0;
    int col=(x-left)/30;
    if((x-left)%30>=27)return 0;
    return col<(int)strlen(keys)?keys[col]:0;
}
int search_edit(char query[SEARCH_QUERY_SIZE],char key){
    size_t n=strlen(query);
    if(key=='\b'){if(!n)return 0;query[n-1]=0;return 1;}
    if(!key||n>=SEARCH_QUERY_SIZE-1|| (key==' '&&(!n||query[n-1]==' ')))return 0;
    query[n]=key;query[n+1]=0;return 1;
}
int search_accept_suggestion(char query[SEARCH_QUERY_SIZE],const char *word){
    if(!word||!*word)return 0;
    size_t start=strlen(query),n=strlen(word);
    while(start&&query[start-1]!=' ')start--;
    if(start+n+1>=SEARCH_QUERY_SIZE)return 0;
    memcpy(query+start,word,n);query[start+n]=' ';query[start+n+1]=0;return 1;
}
void search_suggest_add(char words[3][SEARCH_QUERY_SIZE],const char *raw,const char *title){
    if(*words[2]||strlen(raw)>=SEARCH_QUERY_SIZE)return;
    char query[SEARCH_QUERY_SIZE],normal[SEARCH_TITLE_SIZE];
    search_normalize(query,sizeof(query),raw);search_normalize(normal,sizeof(normal),title);
    size_t len=strlen(raw);bool next=len&&raw[len-1]==' ';
    const char *fragment=strrchr(query,' ');fragment=fragment?fragment+1:query;
    bool previous=false;
    for(const char *p=normal;*p;){
        size_t n=strcspn(p," ");char word[SEARCH_QUERY_SIZE];
        if(n<sizeof(word)){
            memcpy(word,p,n);word[n]=0;
            bool matches=*fragment?search_score(word,fragment)>0:true;
            bool offer=next?previous:matches;
            if(!*query&&(!strcmp(word,"the")||!strcmp(word,"of")||!strcmp(word,"and")||!strcmp(word,"a")))offer=false;
            if(offer){
                bool duplicate=false;unsigned slot=0;
                for(;slot<3&&*words[slot];slot++)if(!strcmp(word,words[slot]))duplicate=true;
                char copy[SEARCH_QUERY_SIZE];memcpy(copy,raw,len+1);
                if(!duplicate&&slot<3&&search_accept_suggestion(copy,word))memcpy(words[slot],word,n+1);
                if(*words[2])return;
                if(!*query)return;
            }
            previous=matches;
        }else previous=false;
        p+=n;if(*p)p++;
    }
}
