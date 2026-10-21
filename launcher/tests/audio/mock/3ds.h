#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
typedef uint8_t u8;typedef uint16_t u16;typedef uint32_t u32;typedef uint64_t u64;typedef int32_t Result;typedef void *Thread;
#define U64_MAX UINT64_MAX
#define R_FAILED(x) ((x)<0)
#define NDSP_OUTPUT_STEREO 0
#define NDSP_INTERP_LINEAR 0
#define NDSP_FORMAT_STEREO_PCM16 0
#define NDSP_WBUF_FREE 0
#define NDSP_WBUF_DONE 3
typedef struct {void *data_vaddr;unsigned nsamples,status;} ndspWaveBuf;
Result ndspInit(void);void ndspExit(void);void ndspSetOutputMode(int);void ndspChnReset(int);void ndspChnSetInterp(int,int);void ndspChnSetRate(int,float);void ndspChnSetFormat(int,int);void ndspChnSetMix(int,float*);void ndspChnSetPaused(int,bool);void ndspChnWaveBufAdd(int,ndspWaveBuf*);void ndspChnWaveBufClear(int);
void *linearAlloc(size_t);void linearFree(void*);void DSP_FlushDataCache(void*,size_t);void svcSleepThread(uint64_t);
Thread threadCreate(void(*)(void*),void*,size_t,int,int,bool);void threadJoin(Thread,uint64_t);void threadFree(Thread);
