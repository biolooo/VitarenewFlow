#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef uint8_t u8;typedef uint32_t u32;typedef uint64_t u64;typedef int32_t Result;typedef int Handle;typedef int FS_Archive;
typedef struct {unsigned type,size;const void *data;} FS_Path;
#define PATH_BINARY 1
#define PATH_ASCII 2
#define MEDIATYPE_SD 1
#define ARCHIVE_EXTDATA 1
#define FS_OPEN_READ 1
#define R_FAILED(x) ((x)<0)
#define R_SUCCEEDED(x) ((x)>=0)
FS_Path fsMakePath(unsigned,const char*);Result cfguInit(void);Result CFGU_SecureInfoGetRegion(u8*);void cfguExit(void);
Result FSUSER_OpenArchive(FS_Archive*,int,FS_Path);Result FSUSER_CloseArchive(FS_Archive);
Result FSUSER_OpenFile(Handle*,FS_Archive,FS_Path,unsigned,unsigned);Result FSFILE_Read(Handle,u32*,u64,void*,u32);Result FSFILE_Close(Handle);
