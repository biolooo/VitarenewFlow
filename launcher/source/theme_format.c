// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "theme_format.h"
#include <stdlib.h>
#include <string.h>
#define MAX_BODY (4u*1024u*1024u)
static uint32_t rgb(const uint8_t *p){return 0xff000000u|p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16;}
static uint32_t le32(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
bool theme_decompress(const uint8_t *in,size_t n,uint8_t **out,size_t *length){
    *out=NULL;*length=0;if(n<4)return false;
    if(in[0]!=0x11){if(n>MAX_BODY)return false;*out=malloc(n);if(!*out)return false;memcpy(*out,in,n);*length=n;return true;}
    size_t size=le32(in)>>8,ip=4,op=0;
    if(!size){if(n<8)return false;size=le32(in+4);ip=8;}
    if(size<0xc4||size>MAX_BODY)return false;
    uint8_t *data=malloc(size);if(!data)return false;
    while(op<size){
        if(ip>=n)goto fail;
        unsigned flags=in[ip++];
        for(unsigned mask=128;mask && op<size;mask>>=1){
            if(!(flags&mask)){if(ip>=n)goto fail;data[op++]=in[ip++];continue;}
            if(ip>=n)goto fail;
            unsigned b=in[ip++],count=b>>4,dist;
            if(count<2){
                if(ip+1>=n)goto fail;
                unsigned c=in[ip++],d=in[ip++];
                if(!count){count=((b&15)<<4)+(c>>4)+0x11;dist=((c&15)<<8)+d+1;}
                else {if(ip>=n)goto fail;count=((b&15)<<12)+(c<<4)+(d>>4)+0x111;dist=((d&15)<<8)+in[ip++]+1;}
            }else {if(ip>=n)goto fail;count++;dist=((b&15)<<8)+in[ip++]+1;}
            if(dist>op||count>size-op)goto fail;
            while(count--){data[op]=data[op-dist];++op;}
        }
    }
    *out=data;*length=size;return true;
fail:free(data);return false;
}
bool theme_screen(const uint8_t *body,size_t size,bool lower,ThemeScreen *out){
    memset(out,0,sizeof(*out));out->color=0xfff2f3f4;
    if(size<0xc4)return false;
    unsigned type=le32(body+(lower?0x20:0xc)),mode=le32(body+(lower?0x24:0x10));out->mode=mode;
    if(type==1){
        unsigned off=le32(body+(lower?0x28:0x14));if(off>size||size-off<3)return false;
        out->color=0xff000000|((uint32_t)body[off+2]<<16)|((uint32_t)body[off+1]<<8)|body[off];return true;
    }
    if(type!=3)return false;
    unsigned width=512,frames=1,frameWidth=lower?320:412;
    if(lower){if(mode==2||mode==4){width=1024;frames=3;}else if(mode==0||mode==3){width=1024;frameWidth=1008;}else if(mode!=1)return false;}
    else {if(mode==0||mode==3){width=1024;frameWidth=1008;}else if(mode!=1)return false;}
    unsigned off=le32(body+(lower?0x28:0x18));size_t bytes=(size_t)width*256*2;
    if(!off||off>size||bytes>size-off)return false;
    out->pixels=body+off;out->width=width;out->frames=frames;out->frameWidth=frameWidth;return true;
}
bool theme_button_style(const uint8_t *body,size_t size,uint32_t *base,uint32_t *text,uint32_t *border){
    if(size<0xc4||le32(body+0x6c)!=1)return false;
    unsigned off=le32(body+0x70);if(off>size||size-off<32)return false;
    *base=rgb(body+off+7);*text=rgb(body+off+23);*border=rgb(body+off+10);return true;
}
