#include "library_model.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
const char *library_system_names[SYSTEM_COUNT]={"Nintendo 3DS","Nintendo DS","Game Boy Advance","Nintendo 64","Super Nintendo","Nintendo NES","Virtual Boy","Game Boy / Color"};
const char *library_system_folders[SYSTEM_COUNT]={"3ds","ds","gba","n64","snes","nes","vb","gb"};
const char *library_view_names[VIEW_COUNT]={"Nintendo 3DS","Homebrew Apps","Nintendo DS","Game Boy","Nintendo 64","Super Nintendo","Nintendo NES","Virtual Boy"};
unsigned library_view_for(int system,bool homebrew){
    static const unsigned views[SYSTEM_COUNT]={VIEW_3DS,VIEW_DS,VIEW_GAMEBOY,VIEW_N64,VIEW_SNES,VIEW_NES,VIEW_VB,VIEW_GAMEBOY};
    return system==SYSTEM_3DS&&homebrew?VIEW_HOMEBREW:system>=0&&system<SYSTEM_COUNT?views[system]:VIEW_3DS;
}
unsigned library_view_system(unsigned view){
    static const unsigned systems[VIEW_COUNT]={SYSTEM_3DS,SYSTEM_3DS,SYSTEM_DS,SYSTEM_GBA,SYSTEM_N64,SYSTEM_SNES,SYSTEM_NES,SYSTEM_VB};
    return view<VIEW_COUNT?systems[view]:SYSTEM_3DS;
}
unsigned library_legacy_view(unsigned system){return library_view_for((int)system,false);}
bool library_is_homebrew(uint64_t id,const char *product){
    static const uint64_t ids[]={0x000400000F104200ULL,0x00040000001A1E00ULL,0x0004000000384900ULL,0x0004000000384A00ULL,0x0004000004391700ULL,0x000400000BAFE000ULL,0x000400000BCFFF00ULL,0x000400000BEEF500ULL,0x000400000D40D200ULL,0x000400000D921E00ULL,0x000400000DEAD100ULL,0x000400000F800100ULL,0x000400000FE7CB00ULL,0x000400000DAED300ULL,0x0004000000DF1000ULL,0x0004000003DF1000ULL,0x000400000EB00000ULL,0x000400000BF74D00ULL,0x000400000EC10000ULL,0x0004000000DF1100ULL,0x0004000000308200ULL,0x000400000FF9FF00ULL};
    for(unsigned i=0;i<sizeof(ids)/sizeof(ids[0]);++i)if(id==ids[i])return true;
    return product&&(!strncmp(product,"CTR-H-",6)||!strncmp(product,"CTR-HB-",7));
}
int library_file_system(const char *name){
    if(!name)return -1;
    const char *ext=strrchr(name,'.');
    if(!ext)return -1;
    if(!strcasecmp(ext,".nds"))return SYSTEM_DS;
    if(!strcasecmp(ext,".gba"))return SYSTEM_GBA;
    if(!strcasecmp(ext,".n64")||!strcasecmp(ext,".z64")||!strcasecmp(ext,".v64"))return SYSTEM_N64;
    if(!strcasecmp(ext,".sfc")||!strcasecmp(ext,".smc"))return SYSTEM_SNES;
    if(!strcasecmp(ext,".nes"))return SYSTEM_NES;
    if(!strcasecmp(ext,".vb"))return SYSTEM_VB;
    if(!strcasecmp(ext,".gb")||!strcasecmp(ext,".gbc"))return SYSTEM_GB;
    return -1;
}
bool library_cover_path(char *out,size_t capacity,int system,const char *key){
    if(!out || !key || !*key || system<0 || system>=SYSTEM_COUNT || strpbrk(key,"/\\\r\n") || !strcmp(key,".."))return false;
    int n=snprintf(out,capacity,"sdmc:/3ds/3ds-library/covers/%s/%s.png",library_system_folders[system],key);
    return n>0 && (size_t)n<capacity;
}
