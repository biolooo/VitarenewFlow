#pragma once
#include <stdbool.h>
#include <stdint.h>
bool ui_font_init(void);
void ui_font_free(void);
unsigned ui_font_width(unsigned size,const char *text);
bool ui_font_draw(float x,float y,unsigned size,uint32_t color,const char *text);
unsigned ui_font_next(const char **text);
