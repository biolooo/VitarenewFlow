#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef uint64_t u64;
typedef struct TestThread *Thread;
typedef int LightLock;
typedef struct {uint32_t clusterSize;uint64_t freeClusters;} FS_ArchiveResource;
#define U64_MAX UINT64_MAX
#define SYSTEM_MEDIATYPE_SD 0
#define R_FAILED(x) ((x)<0)
void LightLock_Lock(LightLock*);void LightLock_Unlock(LightLock*);
int socInit(void*,unsigned);void socExit(void);
Thread threadCreate(void (*fn)(void*),void*,size_t,int,int,bool);
int threadJoin(Thread,uint64_t);void threadFree(Thread);
int FSUSER_GetArchiveResource(FS_ArchiveResource*,int);
