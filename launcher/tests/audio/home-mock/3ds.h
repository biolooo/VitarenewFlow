#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef uint8_t u8;typedef uint32_t u32;typedef uint64_t u64;typedef int Result;typedef int NS_APPID;typedef int APT_AppletAttr;typedef int FS_MediaType;
#define R_FAILED(r) ((r)<0)
NS_APPID aptGetMenuAppID(void);Result APT_GetAppletInfo(NS_APPID,u64*,u8*,bool*,bool*,APT_AppletAttr*);
Result romfsMountFromTitle(u64,FS_MediaType,const char*);Result romfsUnmount(const char*);
