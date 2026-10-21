#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef uint8_t u8;typedef uint32_t u32;typedef uint64_t u64;typedef int32_t Result;
typedef enum {MEDIATYPE_NAND,MEDIATYPE_SD,MEDIATYPE_GAME_CARD} FS_MediaType;
typedef struct {u64 titleID,size;} AM_TitleInfo;
typedef struct {u32 env_info;} KernelConfig;extern KernelConfig *OS_KernelConfig;
#define R_FAILED(r) ((r)<0)
#define R_SUCCEEDED(r) ((r)>=0)
Result APT_CheckNew3DS(bool*);Result cfguInit(void);void cfguExit(void);Result CFGU_SecureInfoGetRegion(u8*);
Result AM_GetTitleCount(FS_MediaType,u32*);Result AM_GetTitleList(u32*,FS_MediaType,u32,u64*);
Result AM_GetTitleInfo(FS_MediaType,u32,u64*,AM_TitleInfo*);Result AM_DeleteTitle(FS_MediaType,u64);
