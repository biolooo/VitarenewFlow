#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define COVER_SLOTS 32
typedef struct {void *pixels;unsigned width,height,textureWidth,textureHeight;bool icon;size_t bytes;uint64_t readTicks,resizeTicks,tileTicks;} CoverPixels;
typedef struct {unsigned decoded,failed,discarded;uint64_t totalTicks,maxTicks,maxReadTicks,maxResizeTicks,maxTileTicks;} CoverLoaderStats;
CoverLoaderStats cover_loader_stats(void);
bool cover_loader_init(void);
void cover_loader_shutdown(void);
void cover_loader_clear(void);
void cover_loader_begin(void);
void cover_loader_end(void);
void cover_loader_request(unsigned slot,const char *path,const uint16_t *icon,unsigned priority);
bool cover_loader_take(unsigned slot,CoverPixels *pixels);
void cover_pixels_free(CoverPixels *pixels);
/* Pure decode, also used by host tests. No GPU access. */
bool cover_pixels_read(const char *path,const uint16_t *icon,CoverPixels *pixels);
