#include "ui_font.h"
#include "ui_font_data.h"
#include "ui_texture.h"
#include <citro2d.h>
#include <png.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
static C3D_Tex texture;
static bool ready;
static const UiFace *face(unsigned size){unsigned i=size<=12?0:size<=14?1:size<=18?2:3;
return &ui_faces[i];}
unsigned ui_font_next(const char **text){
 const unsigned char *p=(const unsigned char*)*text;
 unsigned c=*p++;
 if(c>=0xc2&&c<=0xdf&&(*p&0xc0)==0x80){c=((c&31)<<6)|(*p++&63);}
 else if(c>=0xe0&&c<=0xef&&p[0]&&(p[0]&0xc0)==0x80&&p[1]&&(p[1]&0xc0)==0x80){c=((c&15)<<12)|((p[0]&63)<<6)|(p[1]&63);
 p+=2;}
 else if(c>=128)c='?';
 *text=(const char*)p;
 return c;
}
static const UiGlyph *glyph(const UiFace *f,unsigned c){
 for(unsigned i=f->start;i<f->start+f->count;++i)if(ui_glyphs[i].code==c)return &ui_glyphs[i];
 return &ui_glyphs[f->start+'?'-32];
}
unsigned ui_font_width(unsigned size,const char *str){
 const UiFace *f=face(size);
 unsigned width=0,maximum=0;
 while(str&&*str){unsigned c=ui_font_next(&str);
 if(c=='\n'){if(width>maximum)maximum=width;
 width=0;}else width+=glyph(f,c)->advance;}
 return width>maximum?width:maximum;
}
bool ui_font_init(void){
 png_image p;
 memset(&p,0,sizeof(p));
 p.version=PNG_IMAGE_VERSION;
 if(!png_image_begin_read_from_file(&p,"sdmc:/3ds/3ds-library/assets/inter-atlas.png"))return false;
 if(p.width!=512||p.height!=512){png_image_free(&p);
 return false;}p.format=PNG_FORMAT_RGBA;
 unsigned char *pixels=malloc(PNG_IMAGE_SIZE(p));
 if(!pixels){png_image_free(&p);
 return false;}
 bool ok=png_image_finish_read(&p,NULL,pixels,0,NULL)&&C3D_TexInit(&texture,512,512,GPU_RGBA8);
 if(ok){ui_texture_rgba(texture.data,pixels,512,512,512,512);
 C3D_TexFlush(&texture);
 C3D_TexSetFilter(&texture,GPU_LINEAR,GPU_LINEAR);}
 free(pixels);
 png_image_free(&p);
 ready=ok;
 return ok;
}
void ui_font_free(void){if(ready)C3D_TexDelete(&texture);
ready=false;}
bool ui_font_draw(float x,float y,unsigned size,uint32_t color,const char *str){
 if(!ready)return false;
 const UiFace *f=face(size);
 float start=roundf(x);
 x=start;
 y=roundf(y);
 C2D_ImageTint tint;
 C2D_PlainImageTint(&tint,color,1);
 while(str&&*str){unsigned c=ui_font_next(&str);
 if(c=='\n'){x=start;
 y+=f->lineHeight;
 continue;}
  const UiGlyph *g=glyph(f,c);
  if(g->w&&g->h){Tex3DS_SubTexture sub={g->w,g->h,g->x/512.f,1-g->y/512.f,(g->x+g->w)/512.f,1-(g->y+g->h)/512.f};
   C2D_Image image={&texture,&sub};
   C2D_DrawImageAt(image,x+g->dx,y+g->dy,0,&tint,1,1);}
  x+=g->advance;
 }
 return true;
}
