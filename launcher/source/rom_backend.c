#include "rom_backend.h"
#include "rom_args.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
Result rom_preflight(const char *emulator, const char *rom)
{
    struct stat info;
    if (stat(emulator, &info) || !S_ISREG(info.st_mode)) return (Result)-20;
    FILE *file = fopen(emulator, "rb");
    char magic[4];
    bool valid = file && fread(magic, 1, 4, file)==4 && !memcmp(magic,"3DSX",4);
    if (file) fclose(file);
    if (!valid) return (Result)-21;
    if (stat(rom,&info) || !S_ISREG(info.st_mode) || info.st_size<0xc0)
        return (Result)-22;
    file = fopen(rom,"rb");
    if (!file) return (Result)-22;
    fclose(file);
    return 0;
}
static Result command(Handle service, unsigned id, unsigned slot, const void *buffer, u32 size)
{
    u32 *ipc = getThreadCommandBuffer();
    ipc[0] = IPC_MakeHeader(id,0,2);
    ipc[1] = IPC_Desc_StaticBuffer(size,slot);
    ipc[2] = (u32)buffer;
    Result result = svcSendSyncRequest(service);
    return R_FAILED(result) ? result : (Result)ipc[1];
}
Result rom_prepare_launch(const char *rom)
{
    return emulator_prepare_launch(LIBRARY_MGBA,rom,true);
}
Result emulator_prepare_launch(const char *emulator,const char *rom,bool archivePath)
{
    Result result = rom_preflight(emulator,rom);
    if (R_FAILED(result)) return result;
    u32 args[0x100];
    /* mGBA's USE_VFS_3DS backend needs an SD-archive path, not a devoptab prefix. */
    const char *archiveRom = archivePath && !strncmp(rom,"sdmc:/",6) ? rom+5 : rom;
    if (!rom_pack_args(args,sizeof(args),emulator,archiveRom)) return (Result)-23;
    Handle service;
    result = svcConnectToPort(&service,"hb:ldr");
    if (R_FAILED(result)) return result;
    /* Set target last: a failed argument transfer must not switch programs. */
    result = command(service,3,1,args,sizeof(args));
    if (R_SUCCEEDED(result)) {
        const char *target = !strncmp(emulator,"sdmc:/",6)?emulator+5:emulator;
        result = command(service,2,0,target,strlen(target)+1);
    }
    svcCloseHandle(service);
    /* Normal homebrew return transfers to the configured Rosalina target. */
    return result;
}
