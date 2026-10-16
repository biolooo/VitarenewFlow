#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
typedef struct {unsigned width,height,frames,fps,views,textureWidth,textureHeight;uint32_t *offsets;} BannerHeader;
bool banner_header_read(FILE *file,BannerHeader *header);
void banner_header_free(BannerHeader *header);
bool banner_frame_read(FILE *file,const BannerHeader *header,unsigned frame,unsigned view,void *out,unsigned capacity);
bool banner_frame_read_scratch(FILE *file,const BannerHeader *header,unsigned frame,unsigned view,void *out,unsigned capacity,void *scratch,unsigned scratchCapacity);
