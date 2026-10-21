// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "cover_loader.h"
#include "ui_texture.h"
#include <3ds.h>
#include <png.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
enum {EMPTY,QUEUED,DECODING,READY,DONE};
typedef struct {char path[512];uint16_t icon[48*48];bool hasIcon;unsigned generation,state,priority;CoverPixels image;} Request;
static Request requests[COVER_SLOTS];
static LightLock lock;static Thread worker;static bool stop,initialized;
static CoverLoaderStats statistics;
static unsigned power2(unsigned n){unsigned result=8;while(result<n)result*=2;return result;}
void cover_pixels_free(CoverPixels *p){free(p->pixels);memset(p,0,sizeof(*p));}
static bool pixels_read_exact(const char *path,const uint16_t *icon,CoverPixels *out){
    u64 readStarted=svcGetSystemTick();
    memset(out,0,sizeof(*out));png_image image={0};image.version=PNG_IMAGE_VERSION;
    bool good=png_image_begin_read_from_file(&image,path)!=0;
    if(good&&(!image.width||!image.height||image.width>512||image.height>512))good=false;
    unsigned char *rgba=NULL,*scaled=NULL;
    if(good){image.format=PNG_FORMAT_RGBA;rgba=malloc(PNG_IMAGE_SIZE(image));good=rgba&&png_image_finish_read(&image,NULL,rgba,0,NULL);}
    if(good){
        out->readTicks=svcGetSystemTick()-readStarted;u64 resizeStarted=svcGetSystemTick();
        unsigned w=image.width,h=image.height,largest=w>h?w:h;
        if(largest>256){w=(w*256+largest/2)/largest;h=(h*256+largest/2)/largest;if(!w)w=1;if(!h)h=1;}
        bool resize=w!=image.width||h!=image.height;
        scaled=resize?malloc((size_t)w*h*4):rgba;good=scaled!=NULL;
        if(good){
            /* Bilinear resize on the worker; final textures never exceed 256x256. */
            unsigned xs[256],ys[256];
            if(resize){
             /* Bounds are <=512 source / <=256 output, so 32-bit products suffice.
                Compute coordinates once per axis instead of two 64-bit divides
                per output pixel; preserve exactly the previous bilinear samples. */
             for(unsigned x=0;x<w;x++)xs[x]=w>1?x*(image.width-1)*256/(w-1):0;
             for(unsigned y=0;y<h;y++)ys[y]=h>1?y*(image.height-1)*256/(h-1):0;
             for(unsigned y=0;y<h;y++)for(unsigned x=0;x<w;x++){
                unsigned fx=xs[x],fy=ys[y];
                unsigned x0=fx/256,y0=fy/256,x1=x0+1<image.width?x0+1:x0,y1=y0+1<image.height?y0+1:y0;
                unsigned a=fx%256,b=fy%256;
                for(unsigned c=0;c<4;c++)scaled[((size_t)y*w+x)*4+c]=(rgba[((size_t)y0*image.width+x0)*4+c]*(256-a)*(256-b)+rgba[((size_t)y0*image.width+x1)*4+c]*a*(256-b)+rgba[((size_t)y1*image.width+x0)*4+c]*(256-a)*b+rgba[((size_t)y1*image.width+x1)*4+c]*a*b+32768)/65536;
             }
            }
            out->resizeTicks=svcGetSystemTick()-resizeStarted;
            out->width=w;out->height=h;out->textureWidth=power2(w);out->textureHeight=power2(h);out->bytes=(size_t)out->textureWidth*out->textureHeight*4;
            out->pixels=malloc(out->bytes);good=out->pixels!=NULL;
            if(good){u64 tileStarted=svcGetSystemTick();ui_texture_rgba(out->pixels,scaled,w,h,out->textureWidth,out->textureHeight);out->tileTicks=svcGetSystemTick()-tileStarted;}
        }
    }
    if(scaled!=rgba)free(scaled);
    free(rgba);png_image_free(&image);
    if(good)return true;
    cover_pixels_free(out);
    if(!icon)return false;
    out->pixels=malloc(64*64*2);if(!out->pixels)return false;
    ui_texture_icon(out->pixels,icon);out->width=out->height=48;out->textureWidth=out->textureHeight=64;out->bytes=64*64*2;out->icon=true;return true;
}
bool cover_pixels_read(const char *path,const uint16_t *icon,CoverPixels *out){
    char thumb[560],meta[560];struct stat original,small;unsigned long long size=0;
    snprintf(thumb,sizeof(thumb),"%s.thumb.png",path);snprintf(meta,sizeof(meta),"%s.thumb.size",path);
    if(!stat(path,&original)&&!stat(thumb,&small)&&original.st_mtime==small.st_mtime){
        FILE *f=fopen(meta,"rb");bool valid=f&&fscanf(f,"%llu",&size)==1&&size==(unsigned long long)original.st_size;if(f)fclose(f);
        if(valid&&pixels_read_exact(thumb,NULL,out))return true;
    }
    return pixels_read_exact(path,icon,out);
}
static void decode(void *unused){
    (void)unused;
    for(;;){
        char path[512];uint16_t icon[48*48];bool hasIcon=false;unsigned generation=0;int chosen=-1;
        LightLock_Lock(&lock);
        if(stop){LightLock_Unlock(&lock);break;}
        unsigned ready=0;for(unsigned i=0;i<COVER_SLOTS;i++)if(requests[i].state==READY)ready++;
        if(ready<2)for(unsigned i=0;i<COVER_SLOTS;i++)if(requests[i].state==QUEUED&&requests[i].priority<9&&(chosen<0||requests[i].priority<requests[chosen].priority))chosen=(int)i;
        if(chosen>=0){Request *r=&requests[chosen];r->state=DECODING;generation=r->generation;snprintf(path,sizeof(path),"%s",r->path);hasIcon=r->hasIcon;if(hasIcon)memcpy(icon,r->icon,sizeof(icon));}
        LightLock_Unlock(&lock);
        if(chosen<0){svcSleepThread(2000000);continue;}
        u64 started=svcGetSystemTick();CoverPixels pixels;bool good=cover_pixels_read(path,hasIcon?icon:NULL,&pixels);u64 elapsed=svcGetSystemTick()-started;
        LightLock_Lock(&lock);Request *r=&requests[chosen];
        statistics.decoded++;statistics.failed+=!good;statistics.totalTicks+=elapsed;if(elapsed>statistics.maxTicks)statistics.maxTicks=elapsed;
        if(pixels.readTicks>statistics.maxReadTicks)statistics.maxReadTicks=pixels.readTicks;
        if(pixels.resizeTicks>statistics.maxResizeTicks)statistics.maxResizeTicks=pixels.resizeTicks;
        if(pixels.tileTicks>statistics.maxTileTicks)statistics.maxTileTicks=pixels.tileTicks;
        if(!stop&&r->generation==generation){r->state=good?READY:DONE;if(good){r->image=pixels;memset(&pixels,0,sizeof(pixels));}}
        else statistics.discarded++;
        LightLock_Unlock(&lock);cover_pixels_free(&pixels);
    }
}
bool cover_loader_init(void){
    if(initialized)return worker!=NULL;
    memset(requests,0,sizeof(requests));memset(&statistics,0,sizeof(statistics));LightLock_Init(&lock);stop=false;initialized=true;
    worker=threadCreate(decode,NULL,0x18000,0x32,-2,false);return worker!=NULL;
}
void cover_loader_clear(void){
    if(!initialized)return;
    LightLock_Lock(&lock);for(unsigned i=0;i<COVER_SLOTS;i++){Request *r=&requests[i];unsigned generation=r->generation+1;cover_pixels_free(&r->image);memset(r,0,sizeof(*r));r->generation=generation;}LightLock_Unlock(&lock);
}
void cover_loader_shutdown(void){
    if(!initialized)return;
    LightLock_Lock(&lock);stop=true;LightLock_Unlock(&lock);
    if(worker){threadJoin(worker,U64_MAX);threadFree(worker);worker=NULL;}
    cover_loader_clear();initialized=false;
}
void cover_loader_begin(void){if(!initialized)return;LightLock_Lock(&lock);for(unsigned i=0;i<COVER_SLOTS;i++)requests[i].priority=9;LightLock_Unlock(&lock);}
void cover_loader_end(void){
    if(!initialized)return;
    LightLock_Lock(&lock);for(unsigned i=0;i<COVER_SLOTS;i++){Request *r=&requests[i];if(r->priority==9&&r->state==READY){cover_pixels_free(&r->image);r->state=QUEUED;r->generation++;}}LightLock_Unlock(&lock);
}
void cover_loader_request(unsigned slot,const char *path,const uint16_t *icon,unsigned priority){
    if(!worker||slot>=COVER_SLOTS||!path||!*path||strlen(path)>=512)return;
    LightLock_Lock(&lock);Request *r=&requests[slot];
    if(r->state==EMPTY||strcmp(r->path,path)){
        unsigned generation=r->generation+1;cover_pixels_free(&r->image);memset(r,0,sizeof(*r));r->generation=generation;
        snprintf(r->path,sizeof(r->path),"%s",path);r->hasIcon=icon!=NULL;if(icon)memcpy(r->icon,icon,sizeof(r->icon));r->state=QUEUED;
    }
    r->priority=priority;LightLock_Unlock(&lock);
}
bool cover_loader_take(unsigned slot,CoverPixels *image){
    if(!initialized||slot>=COVER_SLOTS)return false;
    LightLock_Lock(&lock);Request *r=&requests[slot];bool ready=r->state==READY;
    if(ready){*image=r->image;memset(&r->image,0,sizeof(r->image));r->state=DONE;}
    LightLock_Unlock(&lock);return ready;
}
CoverLoaderStats cover_loader_stats(void){
    if(!initialized)return statistics;
    LightLock_Lock(&lock);CoverLoaderStats result=statistics;LightLock_Unlock(&lock);return result;
}
