#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef void *Thread;
#define U64_MAX UINT64_MAX
#define R_FAILED(x) ((x)<0)
int socInit(void*,unsigned);void socExit(void);
Thread threadCreate(void (*fn)(void*),void*,size_t,int,int,bool);
int threadJoin(Thread,uint64_t);void threadFree(Thread);
