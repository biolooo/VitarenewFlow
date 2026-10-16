#pragma once
#define IPC_MakeHeader(id,n,t) (((id)<<16)|((n)<<6)|(t))
typedef struct {int handle;} httpcContext;
#define HTTPC_METHOD_GET 0
#define SSLC_DefaultRootCert_Nintendo_Class2_CA_G3 6
int httpcAddDefaultCert(httpcContext*,int);
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
typedef int32_t Result;typedef uint8_t u8;typedef uint16_t u16;typedef uint32_t u32;typedef uint64_t u64;typedef uint32_t Handle;typedef uint64_t FS_Archive;
typedef struct {u32 sectorSize,clusterSize,totalClusters,freeClusters;} FS_ArchiveResource;
Result FSUSER_GetSdmcArchiveResource(FS_ArchiveResource*);
typedef enum {MEDIATYPE_NAND=0,MEDIATYPE_SD=1} FS_MediaType;
typedef struct {u64 titleID,size;u16 version;u32 titleType;} AM_TitleInfo;
typedef struct {int type;u32 size;const void *data;} FS_Path;
#define R_FAILED(x) ((x)<0)
#define R_SUCCEEDED(x) ((x)>=0)
#define PATH_EMPTY 0
#define PATH_UTF16 4
#define ARCHIVE_SDMC 9
#define FS_OPEN_READ 1
ssize_t utf8_to_utf16(u16*,const u8*,size_t);
FS_Path fsMakePath(int,const void*);
Result FSUSER_OpenFileDirectly(Handle*,int,FS_Path,FS_Path,u32,u32);
Result FSUSER_OpenArchive(FS_Archive*,int,FS_Path);
Result FSUSER_CloseArchive(FS_Archive);
Result FSUSER_GetFreeBytes(u64*,FS_Archive);
Result FSFILE_GetSize(Handle,u64*);
Result FSFILE_Read(Handle,u32*,u64,void*,u32);
Result FSFILE_Write(Handle,u32*,u64,const void*,u32,u32);
Result FSFILE_Close(Handle);
Result AM_GetCiaFileInfo(FS_MediaType,AM_TitleInfo*,Handle);
Result AM_GetCiaRequiredSpace(u64*,FS_MediaType,Handle);
Result AM_GetCiaIcon(void*,Handle);
Result AM_GetTitleCount(FS_MediaType,u32*);
Result AM_GetTitleList(u32*,FS_MediaType,u32,u64*);
Result AM_GetTitleInfo(FS_MediaType,u32,u64*,AM_TitleInfo*);
Result AM_StartCiaInstall(FS_MediaType,Handle*);
Result AM_FinishCiaInstall(Handle);
Result AM_CancelCIAInstall(Handle);
Result cfguInit(void);
void cfguExit(void);
Result CFGU_SecureInfoGetRegion(u8*);
Result httpcInit(u32);
void httpcExit(void);
Result httpcOpenContext(httpcContext*,int,const char*,int);
Result httpcBeginRequest(httpcContext*);
Result httpcGetResponseStatusCodeTimeout(httpcContext*,u32*,u64);
Result httpcReceiveDataTimeout(httpcContext*,u8*,u32,u64);
Result httpcGetDownloadSizeState(httpcContext*,u32*,u32*);
Result httpcCloseContext(httpcContext*);
u32 *getThreadCommandBuffer(void);
Handle *fsGetSessionHandle(void);
Result svcSendSyncRequest(Handle);
