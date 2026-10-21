// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct {
 const uint8_t *data;size_t size,offset;
 unsigned channels,rate,codec,blocks,blockBytes,blockSamples,lastBytes,lastSamples,lastStride;
 uint32_t samples,position,loopStart;bool loop,wav,loopSaved;
 int16_t coefficients[2][16],history[2][2],loopHistory[2][2];
} BgmStream;
bool bgm_parse(BgmStream *s,const void *data,size_t size);
/* Always returns interleaved stereo PCM16; zero means end or invalid data. */
size_t bgm_render(BgmStream *s,int16_t *pcm,size_t frames);
