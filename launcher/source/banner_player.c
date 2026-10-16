#include "banner_player.h"
#include "banner_format.h"
#include <citro2d.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define PIXELS (512*256*4)
enum {FREE,DECODING,READY,READING};
typedef struct {u8 *pixels;unsigned state,sequence;bool stereo;} Slot;
static FILE *file;
static BannerHeader header;
static char current[512];
static C3D_Tex tex[2];
static Tex3DS_SubTexture sub;
static C2D_Image images[2];
static bool allocated,valid,displayStereo;
static Thread worker;
static LightLock lock;
static Slot slots[2];
static u8 *scratch;
static unsigned scratchSize,requestFrame;
static bool requestStereo,stop,paused;
static u64 started;
static void decode(void *unused){
    (void)unused;unsigned previous=~0u,sequence=0;bool previousStereo=false;
    for(;;){
        LightLock_Lock(&lock);
        if(stop){LightLock_Unlock(&lock);break;}
        if(paused){LightLock_Unlock(&lock);svcSleepThread(4000000);continue;}
        unsigned frame=requestFrame;bool stereo=requestStereo;int chosen=-1;
        if(frame!=previous||stereo!=previousStereo){
            for(unsigned i=0;i<2;++i)if(slots[i].state==FREE){chosen=(int)i;break;}
            if(chosen<0)for(unsigned i=0;i<2;++i)if(slots[i].state==READY){chosen=(int)i;break;}
        }
        if(chosen>=0)slots[chosen].state=DECODING;
        LightLock_Unlock(&lock);
        if(chosen<0){svcSleepThread(header.frames==1?16000000:2000000);continue;}
        Slot *slot=&slots[chosen];
        bool ok=banner_frame_read_scratch(file,&header,frame,stereo?1:0,slot->pixels,PIXELS,scratch,scratchSize);
        if(ok&&stereo)ok=banner_frame_read_scratch(file,&header,frame,2,slot->pixels+PIXELS,PIXELS,scratch,scratchSize);
        LightLock_Lock(&lock);
        slot->stereo=stereo;slot->sequence=++sequence;slot->state=ok?READY:FREE;
        previous=frame;previousStereo=stereo;
        LightLock_Unlock(&lock);
    }
}
void banner_close(void){
    if(worker){LightLock_Lock(&lock);stop=true;LightLock_Unlock(&lock);threadJoin(worker,U64_MAX);threadFree(worker);worker=NULL;}
    if(file)fclose(file);
    file=NULL;banner_header_free(&header);
    if(allocated){C3D_TexDelete(&tex[0]);C3D_TexDelete(&tex[1]);}
    for(unsigned i=0;i<2;++i){free(slots[i].pixels);memset(&slots[i],0,sizeof(slots[i]));}
    free(scratch);scratch=NULL;scratchSize=0;allocated=valid=false;current[0]=0;
}
void banner_pause(bool value){if(!worker)return;LightLock_Lock(&lock);paused=value;LightLock_Unlock(&lock);}
static bool open_banner(const char *path,bool stereo){
    snprintf(current,sizeof(current),"%s",path);
    file=fopen(path,"rb");if(!file||!banner_header_read(file,&header))return false;
    if(!C3D_TexInit(&tex[0],512,256,GPU_RGBA8))return false;
    if(!C3D_TexInit(&tex[1],512,256,GPU_RGBA8)){C3D_TexDelete(&tex[0]);return false;}
    allocated=true;
    for(unsigned i=0;i<2;++i){
        C3D_TexSetFilter(&tex[i],GPU_LINEAR,GPU_LINEAR);images[i]=(C2D_Image){&tex[i],&sub};
        slots[i].pixels=malloc(PIXELS*2);if(!slots[i].pixels)return false;
    }
    for(unsigned i=0;i<header.frames*header.views;++i){unsigned n=header.offsets[i+1]-header.offsets[i];if(n>scratchSize)scratchSize=n;}
    scratch=malloc(scratchSize);if(!scratch)return false;
    sub=(Tex3DS_SubTexture){header.width,header.height,0,1,(float)header.width/512,1-(float)header.height/256};
    LightLock_Init(&lock);stop=paused=false;requestFrame=0;requestStereo=stereo;started=svcGetSystemTick();
    worker=threadCreate(decode,NULL,0x10000,0x31,-2,false);return worker!=NULL;
}
bool banner_prepare(const char *path,bool stereo){
    if(!path||!*path){if(current[0])banner_close();return false;}
    if(strcmp(path,current)){
        banner_close();
        if(!open_banner(path,stereo)){banner_close();snprintf(current,sizeof(current),"%s",path);return false;}
    }
    if(!worker)return false;
    unsigned desired=(unsigned)((svcGetSystemTick()-started)/(SYSCLOCK_ARM11/header.fps)%header.frames);
    LightLock_Lock(&lock);requestFrame=desired;requestStereo=stereo;int chosen=-1;
    for(unsigned i=0;i<2;++i)if(slots[i].state==READY&&slots[i].stereo==stereo&&(chosen<0||slots[i].sequence>slots[chosen].sequence))chosen=(int)i;
    if(chosen>=0){slots[chosen].state=READING;for(unsigned i=0;i<2;++i)if((int)i!=chosen&&slots[i].state==READY)slots[i].state=FREE;}
    LightLock_Unlock(&lock);
    if(chosen>=0){
        /* Only upload decoded pixels here; SD reads and zlib run on the worker. */
        memcpy(tex[0].data,slots[chosen].pixels,PIXELS);C3D_TexFlush(&tex[0]);
        if(stereo){memcpy(tex[1].data,slots[chosen].pixels+PIXELS,PIXELS);C3D_TexFlush(&tex[1]);}
        displayStereo=stereo;valid=true;LightLock_Lock(&lock);slots[chosen].state=FREE;LightLock_Unlock(&lock);
    }
    return valid;
}
bool banner_draw(bool rightEye){
    if(!valid)return false;
    C2D_DrawImageAt(images[rightEye&&displayStereo?1:0],0,0,0,NULL,400.f/header.width,240.f/header.height);return true;
}
