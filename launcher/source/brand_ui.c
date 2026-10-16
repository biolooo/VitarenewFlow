/* Decode once at startup; no image work or allocations in rendering. */
#include "brand_ui.h"
#include "brand_asset.h"
#include "ui_texture.h"
#include <citro2d.h>
#include <png.h>
#include <stdlib.h>
#include <string.h>
static C3D_Tex texture;
static Tex3DS_SubTexture sub;
static C2D_Image image;
static bool ready;
void brand_ui_init(void){
    if(ready)return;
    png_image p;memset(&p,0,sizeof(p));p.version=PNG_IMAGE_VERSION;
    if(!png_image_begin_read_from_memory(&p,brand_title_png,sizeof(brand_title_png)))return;
    if(p.width!=400||p.height!=240){png_image_free(&p);return;}
    p.format=PNG_FORMAT_RGBA;
    unsigned char *pixels=malloc(PNG_IMAGE_SIZE(p));
    if(!pixels){png_image_free(&p);return;}
    bool decoded=png_image_finish_read(&p,NULL,pixels,0,NULL)!=0;
    if(decoded&&C3D_TexInit(&texture,512,256,GPU_RGBA8)){
        ui_texture_rgba(texture.data,pixels,400,240,512,256);
        C3D_TexFlush(&texture);C3D_TexSetFilter(&texture,GPU_LINEAR,GPU_LINEAR);
        sub=(Tex3DS_SubTexture){400,240,0,1,400.f/512,1-240.f/256};
        image=(C2D_Image){&texture,&sub};ready=true;
    }
    free(pixels);png_image_free(&p);
}
bool brand_ui_draw(void){
    if(!ready)return false;
    C2D_DrawImageAt(image,0,0,0,NULL,1,1);return true;
}
void brand_ui_free(void){if(ready)C3D_TexDelete(&texture);ready=false;}
