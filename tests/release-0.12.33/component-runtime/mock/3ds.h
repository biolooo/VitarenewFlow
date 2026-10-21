#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef uint64_t u64;typedef uint32_t u32;typedef int32_t Result;
typedef enum{MEDIATYPE_NAND,MEDIATYPE_SD} FS_MediaType;
typedef struct {u64 titleID;} AM_TitleInfo;
#define R_SUCCEEDED(x) ((x)>=0)
#define R_FAILED(x) ((x)<0)
Result AM_GetTitleInfo(FS_MediaType,u32,u64*,AM_TitleInfo*);
Result AM_GetTitleCount(FS_MediaType,u32*);
Result AM_GetTitleList(u32*,FS_MediaType,u32,u64*);
