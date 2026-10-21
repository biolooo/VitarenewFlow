// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "ui_texture.h"
#include "catalog_format.h"
#include <string.h>
size_t ui_texture_index(unsigned x,unsigned y,unsigned width){
    unsigned morton=0;
    for(unsigned b=0;b<3;++b){morton|=((x>>b)&1)<<(2*b);morton|=((y>>b)&1)<<(2*b+1);}
    return ((y/8)*(width/8)+x/8)*64+morton;
}
void ui_texture_rgba(uint8_t *dest,const uint8_t *source,unsigned width,unsigned height,unsigned textureWidth,unsigned textureHeight){
    memset(dest,0,(size_t)textureWidth*textureHeight*4);
    /* PICA stores the first image row at tiled y=0. Tex3DS top=1 already
       accounts for texture coordinates; flipping the pixels inverts the art. */
    static const unsigned spread[8]={0,1,4,5,16,17,20,21};
    for(unsigned y=0;y<height;++y){size_t row=(size_t)(y/8)*(textureWidth/8)*64;unsigned ym=spread[y%8]*2;
     for(unsigned x=0;x<width;++x){
        size_t s=((size_t)y*width+x)*4,d=(row+(x/8)*64+spread[x%8]+ym)*4;
        dest[d]=source[s+3];dest[d+1]=source[s+2];dest[d+2]=source[s+1];dest[d+3]=source[s];
     }
    }
}
void ui_texture_icon(uint16_t *dest,const uint16_t *source){
    memset(dest,0,64*64*2);
    for(unsigned y=0;y<48;++y)for(unsigned x=0;x<48;++x)
        dest[ui_texture_index(x,y,64)]=source[catalog_icon_index(x,y)];
}
