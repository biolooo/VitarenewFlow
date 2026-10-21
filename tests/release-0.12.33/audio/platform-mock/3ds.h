#pragma once
#include <stdint.h>
#include <stddef.h>
typedef uint32_t u32;typedef uint64_t u64;typedef int32_t Result;typedef int Handle;typedef struct {unsigned type,size;const void *data;} FS_Path;
#define MEDIATYPE_NAND 0
#define ARCHIVE_SAVEDATA_AND_CONTENT 0x2345678a
#define PATH_BINARY 1
#define FS_OPEN_READ 1
#define R_FAILED(x) ((x)<0)
#define R_SUCCEEDED(x) ((x)>=0)
Result FSUSER_OpenFileDirectly(Handle *,unsigned,FS_Path,FS_Path,unsigned,unsigned);
Result FSFILE_GetSize(Handle,u64*);Result FSFILE_Read(Handle,u32*,u64,void*,u32);Result FSFILE_Close(Handle);
