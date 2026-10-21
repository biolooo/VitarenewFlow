/* SPDX-License-Identifier: GPL-3.0-only */
#include "banner_format.h"
#include "library_model.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

enum { PIXELS = 512*256*4 };
static void put32(unsigned char *p, unsigned value) {
    for (unsigned i=0;i<4;i++) p[i]=(unsigned char)(value>>(i*8));
}
static FILE *bytes_file(const void *data, size_t size) {
    FILE *f=tmpfile(); assert(f);
    assert(fwrite(data,1,size,f)==size); rewind(f); return f;
}
static unsigned char *fixture(size_t *size) {
    unsigned char *pixels=malloc(PIXELS); assert(pixels);
    for (unsigned i=0;i<PIXELS;i++) pixels[i]=(unsigned char)(i%251);
    uLongf n=compressBound(PIXELS); unsigned char *z=malloc(n); assert(z);
    assert(compress2(z,&n,pixels,PIXELS,Z_BEST_SPEED)==Z_OK);
    *size=48+3*n;
    unsigned char *data=calloc(1,*size); assert(data);
    memcpy(data,"LBNR",4); data[4]=1; data[6]=144; data[7]=1; data[8]=240;
    put32(data+12,1); put32(data+16,30); put32(data+20,3);
    put32(data+24,512); put32(data+28,256);
    for(unsigned i=0;i<4;i++) put32(data+32+4*i,48+i*(unsigned)n);
    for(unsigned i=0;i<3;i++) memcpy(data+48+i*n,z,n);
    free(z); free(pixels); return data;
}
static void reject(const unsigned char *data,size_t n) {
    FILE *f=bytes_file(data,n); BannerHeader h;
    assert(!banner_header_read(f,&h)); banner_header_free(&h); fclose(f);
}
int main(void) {
    /* Missing caches are normal for a newly discovered game, never synthetic banners. */
    assert(fopen("this-synthetic-banner-does-not-exist.lbn","rb")==NULL);
    size_t n; unsigned char *data=fixture(&n), *out=malloc(PIXELS); assert(out);
    FILE *f=bytes_file(data,n); BannerHeader h; assert(banner_header_read(f,&h));
    assert(h.width==400&&h.height==240&&h.frames==1&&h.views==3);
    for(unsigned v=0;v<3;v++) {
        assert(banner_frame_read(f,&h,0,v,out,PIXELS));
        for(unsigned i=0;i<PIXELS;i++) assert(out[i]==(unsigned char)(i%251));
    }
    assert(!banner_frame_read(f,&h,1,0,out,PIXELS));
    assert(!banner_frame_read(f,&h,0,3,out,PIXELS));
    assert(!banner_frame_read(f,&h,0,0,out,PIXELS-1));
    unsigned char scratch[1];
    assert(!banner_frame_read_scratch(f,&h,0,0,out,PIXELS,scratch,sizeof(scratch)));
    banner_header_free(&h); assert(!h.offsets); fclose(f);
    /* Truncated headers/index/payload, bad versions, bounded dimensions and offsets. */
    for(unsigned i=0;i<48;i++) reject(data,i);
    reject(data,n-1);
    const unsigned positions[]={0,4,5,6,8,12,16,20,24,28,32,36,44};
    for(unsigned i=0;i<sizeof(positions)/sizeof(positions[0]);i++) {
        unsigned char *bad=malloc(n); assert(bad); memcpy(bad,data,n);
        if(positions[i]==4) bad[4]=2;
        else if(positions[i]==5) bad[5]=1;
        else if(positions[i]==6) {bad[6]=1;bad[7]=3;}
        else if(positions[i]==8) {bad[8]=1;bad[9]=2;}
        else put32(bad+positions[i],0);
        reject(bad,n); free(bad);
    }
    /* A valid header does not make a broken compressed frame safe to display. */
    data[48]^=0xff; f=bytes_file(data,n); assert(banner_header_read(f,&h));
    assert(!banner_frame_read(f,&h,0,0,out,PIXELS));
    banner_header_free(&h); fclose(f); free(out); free(data);
    char cover[512], previous[512]="";
    for(unsigned i=0;i<1024;i++) {
        char name[64]; snprintf(name,sizeof(name),"Synthetic game %04u.GBA",i);
        int system=library_file_system(name); assert(system==SYSTEM_GBA);
        assert(library_view_for(system,false)==VIEW_GAMEBOY);
        assert(library_cover_path(cover,sizeof(cover),system,name));
        assert(strcmp(cover,previous)); strcpy(previous,cover);
    }
    assert(library_file_system("Game.gbc")==SYSTEM_GB);
    assert(library_file_system("Game.zip")==-1);
    assert(library_view_for(SYSTEM_3DS,true)==VIEW_HOMEBREW);
    assert(library_view_for(SYSTEM_3DS,false)==VIEW_3DS);
    assert(!library_cover_path(cover,sizeof(cover),SYSTEM_GBA,"../game"));
    assert(!library_cover_path(cover,sizeof(cover),SYSTEM_GBA,"game\\name"));
    assert(!library_cover_path(cover,8,SYSTEM_GBA,"game"));
    puts("PASS actual banner parser: valid stereo views/pixels, corrupt/truncated caches and bounds; 1024 distinct library cover paths (not an FPS or GPU test)");
    return 0;
}
