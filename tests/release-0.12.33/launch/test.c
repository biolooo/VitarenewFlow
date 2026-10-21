#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "launch_problem.h"
typedef int Result;
Result rom_preflight(const char *emulator, const char *rom)
{
    struct stat info;
    if (stat(emulator, &info)) return (Result)(errno==ENOENT?-20:-24);
    if (!S_ISREG(info.st_mode)) return (Result)-21;
    FILE *file = fopen(emulator, "rb");
    if(!file)return (Result)-24;
    char magic[4];
    bool valid = fread(magic, 1, 4, file)==4 && !memcmp(magic,"3DSX",4);
    if(ferror(file)){fclose(file);return (Result)-24;}
    if (file) fclose(file);
    if (!valid) return (Result)-21;
    if (stat(rom,&info) || !S_ISREG(info.st_mode) || info.st_size<0xc0)
        return (Result)-22;
    file = fopen(rom,"rb");
    if (!file) return (Result)-22;
    fclose(file);
    return 0;
}

int main(void){
 FILE *f=fopen("good.3dsx","wb");assert(f);fwrite("3DSX",1,4,f);fclose(f);
 f=fopen("game.n64","wb");assert(f);for(int i=0;i<192;i++)fputc(0,f);fclose(f);
 assert(rom_preflight("absent","game.n64")==-20);
 assert(rom_preflight(".","game.n64")==-21);
 assert(rom_preflight("game.n64","game.n64")==-21);
 assert(rom_preflight("good.3dsx","absent")==-22);
 assert(rom_preflight("good.3dsx","good.3dsx")==-22);
 assert(rom_preflight("good.3dsx","game.n64")==0);
 int expected[]={-1,-1,0,2,3,4,5,0};
 for(int i=0;i<SYSTEM_COUNT;i++){
  assert(launch_repair_component(i,-20)==expected[i]);
  assert(launch_repair_component(i,-21)==expected[i]);
  for(int e=-24;e<=0;e++)if(e!=-20&&e!=-21)assert(launch_repair_component(i,e)==-1);
 }
 assert(launch_repair_component(99,-20)==-1);
 puts("PASS: real ROM preflight and emulator recovery mapping");
}
