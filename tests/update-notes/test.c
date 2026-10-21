// SPDX-License-Identifier: GPL-3.0-only
#include "update_format.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char one[]="VFNOTES1\nversion=0.12.38\nde=Klares Men\xc3\xbc\nen=Clear menu\n";
static void reject(const char *text,size_t size,const char *v){
 UpdateNotes notes;memset(&notes,0xa5,sizeof(notes));
 assert(!update_notes_parse(text,size,v,&notes));
 const unsigned char *p=(const unsigned char*)&notes;
 for(size_t i=0;i<sizeof(notes);i++)assert(!p[i]);
}
int main(int argc,char **argv){
 if(argc==3){
  FILE *file=fopen(argv[1],"rb");if(!file)return 1;
  char bytes[UPDATE_NOTES_MAX_BYTES+1];size_t size=fread(bytes,1,sizeof(bytes),file);
  bool good=!ferror(file);if(fclose(file))good=false;
  UpdateNotes notes;if(!good||!update_notes_parse(bytes,size,argv[2],&notes))return 1;
  printf("PASS: generated sidecar accepted by actual C parser (%s, %u DE/EN pairs)\n",notes.version,notes.count);return 0;
 }
 if(argc!=1)return 2;
 UpdateNotes n;assert(update_notes_parse(one,sizeof(one)-1,"0.12.38",&n));
 assert(n.count==1&&!strcmp(n.version,"0.12.38")&&!strcmp(n.de[0],"Klares Men\xc3\xbc")&&!strcmp(n.en[0],"Clear menu"));
 assert(!n.de[1][0]&&!n.en[1][0]);
 const char three[]="VFNOTES1\nversion=0.12.99\nde=Eins\nen=One\nde=Zwei\nen=Two\nde=Drei\nen=Three\n";
 assert(update_notes_parse(three,sizeof(three)-1,"0.12.99",&n)&&n.count==3);
 assert(!strcmp(n.de[2],"Drei")&&!strcmp(n.en[2],"Three"));
 reject(one,sizeof(one)-1,"0.12.37");reject(one,sizeof(one)-1,NULL);
 reject(NULL,0,"0.12.38");reject(one,0,"0.12.38");
 assert(!update_notes_parse(one,sizeof(one)-1,"0.12.38",NULL));
 const char *bad[]={"VFNOTES2\nversion=0.12.38\nde=A\nen=B\n",
  "VFNOTES1\nversion=0.12.38\n", "VFNOTES1\nversion=0.12.38\nde=A\n",
  "VFNOTES1\nversion=0.12.38\nen=B\nde=A\n", "VFNOTES1\nversion=0.12.38\nde=A\nde=B\n",
  "VFNOTES1\nversion=0.12.38\nde=\nen=B\n", "VFNOTES1\nversion=0.12.38\nde=   \nen=B\n",
  "VFNOTES1\nversion=0.12.38\nde=A\nen=B", "VFNOTES1\nversion=0.12.38\nde=A\nen=B\n\n",
  "VFNOTES1\nversion=0.12.38\nde=A\nen=B\nx=ignored\n", "VFNOTES1\nversion=0.12.038\nde=A\nen=B\n",
  "VFNOTES1\r\nversion=0.12.38\r\nde=A\r\nen=B\r\n",
  "VFNOTES1\nversion=0.12.38\nde=A\ten=B\nen=B\n",
  "VFNOTES1\nversion=0.12.38\nde=A\ren=B\nen=B\n",
  "VFNOTES1\nversion=0.12.38\nde=A\nen=B\nde=C\nen=D\nde=E\nen=F\nde=G\nen=H\n"};
 for(size_t i=0;i<sizeof(bad)/sizeof(bad[0]);i++)reject(bad[i],strlen(bad[i]),"0.12.38");
 const char *utf8[]={"\xc0\x80","\xc2","\xe0\x80\x80","\xed\xa0\x80","\xf4\x90\x80\x80","\xff","\x80",
  "\x01","\x7f","\xc2\x85","\xe2\x80\xa8","\xe2\x80\xa9","\xe2\x80\xae","\xef\xbb\xbf"};
 char text[1100];
 for(size_t i=0;i<sizeof(utf8)/sizeof(utf8[0]);i++){
  int size=snprintf(text,sizeof(text),"VFNOTES1\nversion=0.12.38\nde=%s\nen=Safe\n",utf8[i]);
  reject(text,(size_t)size,"0.12.38");
  size=snprintf(text,sizeof(text),"VFNOTES1\nversion=0.12.38\nde=Safe\nen=%s\n",utf8[i]);
  reject(text,(size_t)size,"0.12.38");
 }
 char line[UPDATE_NOTES_LINE_BYTES+2];memset(line,'A',UPDATE_NOTES_LINE_BYTES);line[UPDATE_NOTES_LINE_BYTES]=0;
 int size=snprintf(text,sizeof(text),"VFNOTES1\nversion=0.12.38\nde=%s\nen=%s\nde=%s\nen=%s\nde=%s\nen=%s\n",line,line,line,line,line,line);
 assert(update_notes_parse(text,(size_t)size,"0.12.38",&n)&&n.count==3&&strlen(n.de[2])==120);
 line[UPDATE_NOTES_LINE_BYTES]='A';line[UPDATE_NOTES_LINE_BYTES+1]=0;
 size=snprintf(text,sizeof(text),"VFNOTES1\nversion=0.12.38\nde=%s\nen=Safe\n",line);reject(text,(size_t)size,"0.12.38");
 for(unsigned i=0;i<60;i++){line[i*2]=(char)0xc3;line[i*2+1]=(char)0xa4;}line[120]=0;
 size=snprintf(text,sizeof(text),"VFNOTES1\nversion=0.12.38\nde=%s\nen=Safe\n",line);
 assert(update_notes_parse(text,(size_t)size,"0.12.38",&n));
 line[120]='x';line[121]=0;size=snprintf(text,sizeof(text),"VFNOTES1\nversion=0.12.38\nde=%s\nen=Safe\n",line);reject(text,(size_t)size,"0.12.38");
 memcpy(text,one,sizeof(one));text[30]=0;reject(text,sizeof(one)-1,"0.12.38");
 memset(text,'A',1025);reject(text,1025,"0.12.38");
 char url[240];assert(update_notes_url("0.12.38",url,sizeof(url)));
 assert(!strcmp(url,UPDATE_URL_ROOT "0.12.38/notes.txt"));size_t length=strlen(url);
 for(size_t capacity=1;capacity<=length;capacity++){memset(url,'Z',sizeof(url));assert(!update_notes_url("0.12.38",url,capacity)&&!url[0]&&url[capacity]=='Z');}
 assert(update_notes_url("0.12.38",url,length+1));
 assert(!update_notes_url("0.12.38/../secret",url,sizeof(url))&&!url[0]);
 assert(!update_notes_url(NULL,url,sizeof(url))&&!url[0]);assert(!update_notes_url("0.12.38",NULL,0));
 /* The updater manifest remains unchanged for clients without sidecar support. */
 const char manifest[]="VFUPDATE1\nversion=0.12.38\nsize=32\nsha256=0000000000000000000000000000000000000000000000000000000000000000\nurl=" UPDATE_URL_ROOT "0.12.38/VitarenewFlow.3dsx\n";
 UpdateManifest m;assert(update_manifest_parse(manifest,sizeof(manifest)-1,&m)&&!strcmp(m.version,"0.12.38"));
 puts("PASS: actual version-bound remote notes parser, 1-3 DE/EN pairs, strict UTF-8/control/size limits, cleared failures and unchanged VFUPDATE1");
 return 0;
}
