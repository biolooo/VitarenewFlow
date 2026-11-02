// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#define TITLE_CACHE_LIMIT 1024
#define TITLE_CACHE_ICON_PIXELS (48*48)
typedef struct {
    uint64_t id;
    uint16_t version;
    uint8_t medium;
    bool homebrew;
    char name[160];
    uint16_t icon[TITLE_CACHE_ICON_PIXELS];
} TitleCacheEntry;
/* Return 1 for a hit, 0 for a miss, -1 to cancel reading. */
typedef int (*TitleCacheAccept)(const TitleCacheEntry *entry,void *context);
typedef struct {unsigned records,invalid,hits;} TitleCacheStats;
/* Only complete files are read. A damaged individual record is a miss. */
bool title_cache_read(const char *path,unsigned language,TitleCacheAccept accept,void *context,TitleCacheStats *stats);
/* Initialize to {0}; keep on the main thread. abort before catalog mutation. */
typedef struct {
    FILE *file;
    char path[512],temporary[520],previous[524];
    unsigned expected,written;
} TitleCacheWriter;
bool title_cache_begin(TitleCacheWriter *writer,const char *path,unsigned language,unsigned count);
bool title_cache_append(TitleCacheWriter *writer,const TitleCacheEntry *entry);
bool title_cache_finish(TitleCacheWriter *writer);
void title_cache_abort(TitleCacheWriter *writer);
