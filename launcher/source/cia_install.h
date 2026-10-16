#pragma once
#include <3ds.h>
#include <stdbool.h>
#include <stddef.h>
#define CIA_ERROR_INVALID ((Result)-70)
#define CIA_ERROR_EXISTS ((Result)-71)
#define CIA_ERROR_SPACE ((Result)-72)
#define CIA_ERROR_IO ((Result)-73)
#define CIA_ERROR_CANCELLED ((Result)-74)
typedef struct {u64 id,fileSize,installedSize,requiredSpace;u16 version;} CiaPackage;
typedef bool (*CiaProgress)(u64 done,u64 total,bool committing,void *context);
Result cia_inspect(const char *path,CiaPackage *package,void *smdh);
Result cia_prepare(const char *path,u64 expectedId,CiaPackage *package);
Result cia_install(const char *path,const CiaPackage *expected,CiaProgress progress,void *context);
Result cia_remove_installer(const char *path,const CiaPackage *expected);
unsigned cia_estimate_seconds(u64 bytes,float bytesPerSecond);
void cia_format_size(char *out,size_t capacity,u64 bytes);
void cia_format_time(char *out,size_t capacity,unsigned seconds);
