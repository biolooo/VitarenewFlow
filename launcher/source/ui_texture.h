#ifndef UI_TEXTURE_H
#define UI_TEXTURE_H
#include <stddef.h>
#include <stdint.h>
size_t ui_texture_index(unsigned x,unsigned y,unsigned width);
void ui_texture_rgba(uint8_t *dest,const uint8_t *source,unsigned width,unsigned height,unsigned textureWidth,unsigned textureHeight);
void ui_texture_icon(uint16_t *dest,const uint16_t *source);
#endif
