// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "banner_format.h"
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
static uint32_t le32(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
bool banner_header_read(FILE *file,BannerHeader *h){
    uint8_t b[32];
    memset(h,0,sizeof(*h));
    if(fseek(file,0,SEEK_END))return false;
    long size=ftell(file);
    rewind(file);
    if(size<36||size>128*1024*1024||fread(b,1,32,file)!=32||memcmp(b,"LBNR",4)||b[4]!=1||b[5])return false;
    h->width=b[6]|b[7]<<8;
    h->height=b[8]|b[9]<<8;
    h->frames=le32(b+12);
    h->fps=le32(b+16);
    h->views=le32(b+20);
    h->textureWidth=le32(b+24);
    h->textureHeight=le32(b+28);
    if(!h->width||h->width>512||!h->height||h->height>256||!h->frames||h->frames>1200||!h->fps||h->fps>60||h->views!=3||h->textureWidth!=512||h->textureHeight!=256)return false;
    unsigned count=h->frames*h->views+1,indexBytes=count*4;
    h->offsets=malloc(indexBytes);
    if(!h->offsets)return false;
    if(fread(h->offsets,1,indexBytes,file)!=indexBytes)goto fail;
    if(h->offsets[0]!=32+indexBytes||h->offsets[count-1]!=(unsigned long)size)goto fail;
    for(unsigned i=1;i<count;++i)if(h->offsets[i]<=h->offsets[i-1]||h->offsets[i]-h->offsets[i-1]>1024*1024)goto fail;
    return true;
fail:banner_header_free(h);
return false;
}
void banner_header_free(BannerHeader *h){free(h->offsets);
memset(h,0,sizeof(*h));}
bool banner_frame_read_scratch(FILE *file,const BannerHeader *h,unsigned frame,unsigned view,void *out,unsigned capacity,void *scratch,unsigned scratchCapacity){
    if(frame>=h->frames||view>=h->views||capacity!=512*256*4||!h->offsets)return false;
    unsigned i=frame*h->views+view,size=h->offsets[i+1]-h->offsets[i];
    uint8_t *buffer=scratch;
    if(!buffer||size>scratchCapacity)return false;
    uLongf length=capacity;
    bool ok=!fseek(file,h->offsets[i],SEEK_SET)&&fread(buffer,1,size,file)==size&&uncompress(out,&length,buffer,size)==Z_OK&&length==capacity;
    return ok;
}
bool banner_frame_read(FILE *file,const BannerHeader *h,unsigned frame,unsigned view,void *out,unsigned capacity){
    if(frame>=h->frames||view>=h->views||!h->offsets)return false;
    unsigned i=frame*h->views+view,size=h->offsets[i+1]-h->offsets[i];
    void *scratch=malloc(size);if(!scratch)return false;
    bool ok=banner_frame_read_scratch(file,h,frame,view,out,capacity,scratch,size);free(scratch);return ok;
}
