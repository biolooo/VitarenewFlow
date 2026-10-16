#include "l10n.h"
#include "cia_install.h"
#include "catalog_format.h"
#include "app_log.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <math.h>

static Result open_cia(const char *path,Handle *file){
    if(!path||strncmp(path,"sdmc:/cias/",11)||strstr(path,".."))return CIA_ERROR_INVALID;
    const char *name=path+11,*ext=strrchr(name,'.');
    if(!*name||strchr(name,'/')||strchr(name,'\\')||!ext||strcasecmp(ext,".cia"))return CIA_ERROR_INVALID;
    u16 unicode[512];ssize_t units=utf8_to_utf16(unicode,(const u8*)path+5,511);
    if(units<0||units>511)return CIA_ERROR_INVALID;
    unicode[units]=0;
    return FSUSER_OpenFileDirectly(file,ARCHIVE_SDMC,fsMakePath(PATH_EMPTY,""),fsMakePath(PATH_UTF16,unicode),FS_OPEN_READ,0);
}
static Result describe(Handle file,CiaPackage *package){
    AM_TitleInfo info={0};u64 size=0,required=0;
    Result r=FSFILE_GetSize(file,&size);
    if(R_SUCCEEDED(r))r=AM_GetCiaFileInfo(MEDIATYPE_SD,&info,file);
    if(R_SUCCEEDED(r))r=AM_GetCiaRequiredSpace(&required,MEDIATYPE_SD,file);
    if(R_FAILED(r))return r;
    if(!catalog_is_application(info.titleID)||!info.size||size<0x2020||size>=0x100000000ULL||!required)return CIA_ERROR_INVALID;
    *package=(CiaPackage){.id=info.titleID,.fileSize=size,.installedSize=info.size,.requiredSpace=required,.version=info.version};
    return 0;
}
Result cia_inspect(const char *path,CiaPackage *package,void *icon){
    if(!package)return CIA_ERROR_INVALID;
    Handle file=0;Result r=open_cia(path,&file);
    if(R_FAILED(r))return r;
    r=describe(file,package);
    if(icon){memset(icon,0,SMDH_SIZE);if(R_SUCCEEDED(r))AM_GetCiaIcon(icon,file);}
    FSFILE_Close(file);return r;
}
static Result ensure_absent(u64 id){
    u32 count=0,read=0;Result r=AM_GetTitleCount(MEDIATYPE_SD,&count);
    if(R_FAILED(r))return r;
    if(count>8192)return CIA_ERROR_INVALID;
    if(!count)return 0;
    u64 *ids=malloc(count*sizeof(u64));if(!ids)return (Result)-3;
    r=AM_GetTitleList(&read,MEDIATYPE_SD,count,ids);
    if(R_SUCCEEDED(r)&&read!=count)r=CIA_ERROR_IO;
    if(R_SUCCEEDED(r))for(u32 i=0;i<read;++i)if(ids[i]==id){r=CIA_ERROR_EXISTS;break;}
    free(ids);return r;
}
static Result check_space(u64 required){
    /* SDMC does not implement the per-archive GetFreeBytes operation on hardware. */
    FS_ArchiveResource resource={0};
    Result r=FSUSER_GetSdmcArchiveResource(&resource);
    if(R_SUCCEEDED(r)&&(!resource.clusterSize||resource.freeClusters>resource.totalClusters))return CIA_ERROR_IO;
    u64 freeBytes=(u64)resource.clusterSize*resource.freeClusters;
    if(R_SUCCEEDED(r)&&(required>freeBytes||freeBytes-required<16*1024*1024ULL))return CIA_ERROR_SPACE;
    return r;
}
Result cia_prepare(const char *path,u64 expectedId,CiaPackage *package){
    Result r=cia_inspect(path,package,NULL);
    char log[100];snprintf(log,sizeof(log),"CIA inspect %016llX: %08lX",(unsigned long long)expectedId,(unsigned long)(u32)r);app_log(log);
    if(R_SUCCEEDED(r)&&package->id!=expectedId)r=CIA_ERROR_INVALID;
    if(R_SUCCEEDED(r))r=ensure_absent(package->id);
    snprintf(log,sizeof(log),"CIA title check: %08lX",(unsigned long)(u32)r);app_log(log);
    if(R_SUCCEEDED(r))r=check_space(package->requiredSpace);
    snprintf(log,sizeof(log),"CIA SD resource check: %08lX",(unsigned long)(u32)r);app_log(log);
    return r;
}
Result cia_install(const char *path,const CiaPackage *expected,CiaProgress progress,void *context){
    if(!expected)return CIA_ERROR_INVALID;
    Handle input=0,install=0;Result r=open_cia(path,&input);
    if(R_FAILED(r))return r;
    CiaPackage current={0};r=describe(input,&current);
    if(R_SUCCEEDED(r)&&(current.id!=expected->id||current.fileSize!=expected->fileSize||current.version!=expected->version||current.requiredSpace!=expected->requiredSpace||current.installedSize!=expected->installedSize))r=CIA_ERROR_INVALID;
    if(R_SUCCEEDED(r))r=ensure_absent(current.id);
    if(R_SUCCEEDED(r))r=check_space(current.requiredSpace);
    if(R_SUCCEEDED(r)&&progress&&!progress(0,current.fileSize,false,context))r=CIA_ERROR_CANCELLED;
    u8 *buffer=NULL;
    if(R_SUCCEEDED(r)){buffer=malloc(65536);if(!buffer)r=(Result)-3;}
    bool started=false;
    if(R_SUCCEEDED(r)){r=AM_StartCiaInstall(MEDIATYPE_SD,&install);started=R_SUCCEEDED(r);}
    u64 offset=0;
    while(R_SUCCEEDED(r)&&offset<current.fileSize){
        u32 count=current.fileSize-offset>65536?65536:(u32)(current.fileSize-offset),read=0,written=0;
        r=FSFILE_Read(input,&read,offset,buffer,count);
        if(R_SUCCEEDED(r)&&read!=count)r=CIA_ERROR_IO;
        if(R_SUCCEEDED(r))r=FSFILE_Write(install,&written,offset,buffer,read,0);
        if(R_SUCCEEDED(r)&&written!=read)r=CIA_ERROR_IO;
        if(R_SUCCEEDED(r)){offset+=written;if(progress&&!progress(offset,current.fileSize,false,context))r=CIA_ERROR_CANCELLED;}
    }
    free(buffer);FSFILE_Close(input);
    if(R_SUCCEEDED(r)&&progress&&!progress(offset,current.fileSize,true,context))r=CIA_ERROR_CANCELLED;
    if(R_FAILED(r)&&started){Result cancel=AM_CancelCIAInstall(install);if(R_FAILED(cancel))return cancel;}
    else if(R_SUCCEEDED(r)){
        /* Finish transfers ownership of the handle. It must not subsequently be cancelled or closed. */
        r=AM_FinishCiaInstall(install);
    }
    if(R_SUCCEEDED(r)){
        AM_TitleInfo installed={0};u64 id=current.id;r=AM_GetTitleInfo(MEDIATYPE_SD,1,&id,&installed);
        if(R_SUCCEEDED(r)&&(installed.titleID!=id||!installed.size||installed.version!=current.version))r=CIA_ERROR_IO;
    }
    return r;
}
Result cia_remove_installer(const char *path,const CiaPackage *expected){
    if(!expected)return CIA_ERROR_INVALID;
    CiaPackage current={0};Result r=cia_inspect(path,&current,NULL);
    if(R_FAILED(r))return r;
    if(current.id!=expected->id||current.fileSize!=expected->fileSize||current.version!=expected->version||current.installedSize!=expected->installedSize)return CIA_ERROR_INVALID;
    AM_TitleInfo installed={0};u64 id=current.id;r=AM_GetTitleInfo(MEDIATYPE_SD,1,&id,&installed);
    if(R_FAILED(r))return r;
    if(installed.titleID!=id||!installed.size||installed.version!=current.version)return CIA_ERROR_IO;
    return remove(path)==0?0:CIA_ERROR_IO;
}
unsigned cia_estimate_seconds(u64 bytes,float speed){
    if(!isfinite(speed)||speed<262144.f||speed>8388608.f)speed=1048576.f;
    double seconds=ceil((double)bytes/speed)+10;
    return seconds>86400?86400:(unsigned)seconds;
}
void cia_format_size(char *out,size_t n,u64 bytes){
    if(bytes>=1073741824ULL)snprintf(out,n,"%.2f GiB",bytes/1073741824.0);
    else snprintf(out,n,"%.1f MiB",bytes/1048576.0);
}
void cia_format_time(char *out,size_t n,unsigned seconds){
    if(seconds<60)snprintf(out,n,tr("ca. %u Sek."),seconds);
    else snprintf(out,n,tr("ca. %u Min. %02u Sek."),seconds/60,seconds%60);
}
