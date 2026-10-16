#include "l10n.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "catalog_format.h"
#include "library_model.h"
#include "rom_backend.h"
#include "ds_backend.h"
#include "ui.h"
#include "coverflow.h"
#include "device_settings.h"
#include "home_controls.h"
#include "ui_runtime.h"
#include "banner_export.h"
#include "app_log.h"
#include "game_info.h"
#include "cia_install.h"
#include "game_seed.h"
#include "native_handoff.h"
#include <math.h>
#define MAX_GAMES 1024
typedef struct {u64 id;
FS_MediaType media;
int system;
char name[160],displayName[160],path[512],key[256],cover[512],banner[512];
u16 *icon;bool pending,homebrew;CiaPackage cia;} Game;
static Game games[MAX_GAMES];
static unsigned gameCount;
static UiCard cards[MAX_GAMES];
static UiCard neighborCards[32];static unsigned neighborCount;
static unsigned visible[MAX_GAMES],visibleCount,selected,systemView;
static const char *status="Bereit";
static char message[180];
static Result lastError;
static bool setupDs;
static u8 smdh[SMDH_SIZE];
static unsigned language=1;
static bool amReady,restored,confirm,coverMenu;
static CoverFlow flow;
static unsigned remembered[VIEW_COUNT],menuSelected,launchFrame;
static bool settingsMore,supportOpen;
static bool settings,systemPicker,animation=true,preparingGame,lcdReady,controlsPage;
static int brightness=2;
static GameInfo gameInfo;
static bool infoOpen;
static unsigned infoScroll;
static int infoTouchY=-1;
static bool installPrompt,installing,installFailed,launchFailed;
static bool uiReleased;
static const char *errorTitle;
static CiaPackage installPackage;
static char installSize[64],installSpace[64],installEta[64];

static const char *themeSource="Standard";
static char importNames[128][256];
static const char *imports[128];
static unsigned importCount,importSelected;
static const char *backendNames[SYSTEM_COUNT]={"3DS-System","YANBF / nds-bootstrap","mGBA Library","DaedalusX64 Library","Snes9x Library","VirtuaNES Library","Red Viper Library","mGBA Library"};
static const char *backendPaths[SYSTEM_COUNT]={"","","sdmc:/3ds/mgba-library/mgba-library.3dsx","sdmc:/3ds/n64-library/n64-library.3dsx","sdmc:/3ds/snes-library/snes-library.3dsx","sdmc:/3ds/nes-library/nes-library.3dsx","sdmc:/3ds/vb-library/vb-library.3dsx","sdmc:/3ds/mgba-library/mgba-library.3dsx"};
static void rebuild(void){
    visibleCount=0;
    for(unsigned i=0;i<gameCount;++i)if(library_view_for(games[i].system,games[i].homebrew)==systemView){
        unsigned n=visibleCount++;
        visible[n]=i;
        game_display_title(games[i].displayName,sizeof(games[i].displayName),games[i].name);
        cards[n]=(UiCard){games[i].displayName,games[i].cover,backendNames[games[i].system],games[i].system==SYSTEM_3DS || games[i].system==SYSTEM_DS,games[i].icon,games[i].banner,games[i].pending};
    }
    if(selected>=visibleCount)selected=0;
    neighborCount=0;
    for(unsigned side=0;side<2;side++){
        unsigned view=(systemView+(side?VIEW_COUNT-1:1))%VIEW_COUNT,indices[MAX_GAMES],count=0;
        for(unsigned i=0;i<gameCount;i++)if(library_view_for(games[i].system,games[i].homebrew)==view)indices[count++]=i;
        if(!count)continue;
        unsigned center=remembered[view]<count?remembered[view]:0,added=0,quota=(32-(visibleCount<20?visibleCount:20))/2;
        for(unsigned d=0;d<count&&added<quota;d++)for(int direction=-1;direction<=1;direction+=2){
            if(!d&&direction==1)continue;
            int n=(int)center+(int)d*direction;if(n<0||(unsigned)n>=count||added>=quota)continue;
            Game *g=&games[indices[n]];neighborCards[neighborCount++]=(UiCard){.cover=g->cover,.icon=g->icon};added++;
        }
    }
}
static void draw(bool loading,float progress,const char *stage){
    if(uiReleased)return;
    const char *target=visibleCount?games[visible[selected]].cover:"";
    UiState state={.system=library_view_names[systemView],.status=status,.version="0.11.0",
        .cards=cards,.count=visibleCount,.selected=selected,.total=visibleCount,.confirm=confirm,
        .loading=loading,.progress=progress,.stage=stage,.coverMenu=coverMenu,.coverTarget=target,
        .imports=imports,.importCount=importCount,.importSelected=importSelected,.setupDs=setupDs,
        .carousel=flow.position,.systemIndex=visibleCount?(unsigned)games[visible[selected]].system:library_view_system(systemView),.settings=settings,.settingsMore=settingsMore,.support=supportOpen,.systemPicker=systemPicker,.menuSelected=menuSelected,
        .brightness=brightness,.brightnessAvailable=lcdReady,.animation=animation,.themeSource=themeSource,.launchFrame=launchFrame,.launching=preparingGame,.controls=controlsPage,.info=infoOpen?&gameInfo:NULL,.infoScroll=infoScroll,
        .installPrompt=installPrompt,.installing=installing,.installSize=installSize,.installSpace=installSpace,.installEta=installEta,.installError=(installFailed||launchFailed)?message:NULL,.launchError=launchFailed,.errorTitle=errorTitle,.prefetch=neighborCards,.prefetchCount=neighborCount};
        ui_draw(&state);
}
static void onApt(APT_HookType event,void *unused){
    (void)unused;
    if(event==APTHOOK_ONRESTORE||event==APTHOOK_ONWAKEUP){restored=true;
    ui_runtime_reset();}
    if(event==APTHOOK_ONSUSPEND)app_log("APT suspend");
    if(event==APTHOOK_ONRESTORE)app_log("APT restore");
    if(event==APTHOOK_ONSLEEP)app_log("APT sleep");
    if(event==APTHOOK_ONWAKEUP)app_log("APT wake");
    if(event==APTHOOK_ONEXIT)app_log("APT exit");
}
static Result metadata(u64 id,FS_MediaType media){
    u32 archive[]={(u32)id,(u32)(id>>32),media,0},filePath[]={0,0,2,0x6e6f6369,0};
    FS_Path a={PATH_BINARY,sizeof(archive),archive},f={PATH_BINARY,sizeof(filePath),filePath};
    Handle file;
    Result r=FSUSER_OpenFileDirectly(&file,ARCHIVE_SAVEDATA_AND_CONTENT,a,f,FS_OPEN_READ,0);
    if(R_FAILED(r))return r;
    u32 bytes=0;
    r=FSFILE_Read(file,&bytes,0,smdh,sizeof(smdh));
    FSFILE_Close(file);
    return R_SUCCEEDED(r)&&catalog_smdh_valid(smdh,bytes)?0:(Result)-1;
}
static int compare(const void *a,const void *b){return strcasecmp(((const Game*)a)->name,((const Game*)b)->name);}
static void scan(void){
    ui_clear_covers();
    for(unsigned i=0;i<gameCount;++i){free(games[i].icon);
    games[i].icon=NULL;}
    gameCount=visibleCount=selected=0;
    for(unsigned medium=0;medium<2 && amReady;++medium){
        FS_MediaType media=medium?MEDIATYPE_NAND:MEDIATYPE_SD;
        u32 count=0,read=0;
        Result r=AM_GetTitleCount(media,&count);
        if(R_FAILED(r)||!count||count>8192)continue;
        u64 *ids=calloc(count,sizeof(u64));
        if(!ids){lastError=(Result)-3;
        break;}
        r=AM_GetTitleList(&read,media,count,ids);
        if(R_SUCCEEDED(r)&&read<=count)for(unsigned i=0;i<read && gameCount<MAX_GAMES;++i){
            if(!aptMainLoop())break;
            if(!catalog_is_application(ids[i]))continue;
            Game *g=&games[gameCount++];
            memset(g,0,sizeof(*g));
            g->id=ids[i];
            g->media=media;
            g->system=SYSTEM_3DS;
            char product[17]={0};AM_GetTitleProductCode(media,g->id,product);
            g->homebrew=library_is_homebrew(g->id,product);
            snprintf(g->key,sizeof(g->key),"%016llX",(unsigned long long)g->id);
            if(R_SUCCEEDED(metadata(g->id,media))){catalog_name(g->name,sizeof(g->name),smdh,language,false);
            g->icon=malloc(48*48*2);
            if(g->icon)memcpy(g->icon,smdh+SMDH_ICON_OFFSET,48*48*2);}
            if(!g->name[0])memcpy(g->name,g->key,17);
            library_cover_path(g->cover,sizeof(g->cover),g->system,g->key);
            snprintf(g->banner,sizeof(g->banner),"sdmc:/3ds/3ds-library/banners/%016llX.lbn",(unsigned long long)g->id);
            Result br=banner_export_title(g->id,g->media);
            char entry[96];
            snprintf(entry,sizeof(entry),"Banner %s: %08lX",g->key,(unsigned long)(u32)br);
            app_log(entry);
            status=tr("Installierte Titel lesen...");
            draw(true,.1f+.35f*i/read,tr("3DS-Titel werden gelesen"));
        }
        free(ids);
    }
    DIR *cias=amReady?opendir("sdmc:/cias"):NULL;
    if(cias){struct dirent *entry;
        while((entry=readdir(cias))&&gameCount<MAX_GAMES){
            const char *ext=strrchr(entry->d_name,'.');if(!ext||strcasecmp(ext,".cia"))continue;
            if(!aptMainLoop())break;
            char path[512];int n=snprintf(path,sizeof(path),"sdmc:/cias/%s",entry->d_name);struct stat st;
            if(n<0||(size_t)n>=sizeof(path)||stat(path,&st)||!S_ISREG(st.st_mode))continue;
            CiaPackage package;Result r=cia_inspect(path,&package,smdh);
            if(R_FAILED(r)){char log[96];snprintf(log,sizeof(log),"CIA skipped: %08lX",(unsigned long)(u32)r);app_log(log);continue;}
            bool duplicate=false;for(unsigned j=0;j<gameCount;++j)if(games[j].system==SYSTEM_3DS&&games[j].id==package.id)duplicate=true;
            if(duplicate)continue;
            Game *g=&games[gameCount++];memset(g,0,sizeof(*g));g->system=SYSTEM_3DS;g->media=MEDIATYPE_SD;g->id=package.id;g->cia=package;g->pending=true;g->homebrew=library_is_homebrew(g->id,NULL);
            snprintf(g->path,sizeof(g->path),"%s",path);snprintf(g->key,sizeof(g->key),"%016llX",(unsigned long long)g->id);
            if(catalog_smdh_valid(smdh,sizeof(smdh))){catalog_name(g->name,sizeof(g->name),smdh,language,false);g->icon=malloc(48*48*2);if(g->icon)memcpy(g->icon,smdh+SMDH_ICON_OFFSET,48*48*2);}
            if(!g->name[0]){snprintf(g->name,sizeof(g->name),"%.159s",entry->d_name);char *suffix=strrchr(g->name,'.');if(suffix)*suffix=0;}
            library_cover_path(g->cover,sizeof(g->cover),SYSTEM_3DS,g->key);
            status=tr("Installationsdateien lesen...");draw(true,.48f,tr("CIA-Spiele werden gelesen"));
        }closedir(cias);
    }
    const char *folders[]={"sdmc:/roms/nds","sdmc:/roms/gba","sdmc:/roms/n64","sdmc:/3ds/DaedalusX64/Roms","sdmc:/roms/snes","sdmc:/roms/nes","sdmc:/roms/vb","sdmc:/roms/gb"};
    for(unsigned f=0;f<sizeof(folders)/sizeof(folders[0]);++f){
        DIR *dir=opendir(folders[f]);
        if(!dir)continue;
        struct dirent *entry;
        while((entry=readdir(dir)) && gameCount<MAX_GAMES){
            int sys=library_file_system(entry->d_name);
            if(sys<0)continue;
            char path[512];
            int n=snprintf(path,sizeof(path),"%s/%s",folders[f],entry->d_name);
            struct stat st;
            if(n<0||(size_t)n>=sizeof(path)||stat(path,&st)||!S_ISREG(st.st_mode))continue;
            bool duplicate=false;
            for(unsigned j=0;j<gameCount;++j)if(!strcmp(games[j].path,path))duplicate=true;
            if(duplicate)continue;
            Game *g=&games[gameCount++];
            memset(g,0,sizeof(*g));
            g->system=sys;
            snprintf(g->path,sizeof(g->path),"%s",path);
            snprintf(g->key,sizeof(g->key),"%.255s",entry->d_name);
            snprintf(g->name,sizeof(g->name),"%.159s",entry->d_name);
            char *ext=strrchr(g->name,'.');
            if(ext)*ext=0;
            library_cover_path(g->cover,sizeof(g->cover),sys,g->key);
        }
        closedir(dir);
        status=tr("ROM-Ordner lesen...");
        draw(true,.5f+.45f*(f+1)/8,tr("ROM-Bibliothek wird gelesen"));
    }
    qsort(games,gameCount,sizeof(Game),compare);
    rebuild();
    flow_reset(&flow,selected);
    status=gameCount==MAX_GAMES?tr("Limit: 1024 Eintraege erreicht"):tr("Bereit");
}
static void saveView(void){
    mkdir("sdmc:/3ds/3ds-library",0777);
    FILE *f=fopen("sdmc:/3ds/3ds-library/view-v2.txt","wb");
    if(f){fprintf(f,"%u\n%u\n",systemView,selected);
    remembered[systemView]=selected;
    for(unsigned i=0;i<VIEW_COUNT;++i)fprintf(f,"%u\n",remembered[i]);
    fclose(f);}
}
static void readImports(void){
    importCount=importSelected=0;
    DIR *dir=opendir("sdmc:/3ds/3ds-library/import");
    if(!dir)return;
    struct dirent *e;
    while((e=readdir(dir)) && importCount<128){const char *ext=strrchr(e->d_name,'.');
    if(!ext||strcasecmp(ext,".png"))continue;
        snprintf(importNames[importCount],256,"%.255s",e->d_name);
        imports[importCount]=importNames[importCount];
        ++importCount;}
    closedir(dir);
}
static bool assignCover(void){
    if(!importCount||!visibleCount)return false;
    Game *g=&games[visible[selected]];
    char folder[180],src[512],temp[520];
    snprintf(folder,sizeof(folder),"sdmc:/3ds/3ds-library/covers/%s",library_system_folders[g->system]);
    mkdir("sdmc:/3ds/3ds-library/covers",0777);
    mkdir(folder,0777);
    snprintf(src,sizeof(src),"sdmc:/3ds/3ds-library/import/%s",imports[importSelected]);
    if(!ui_validate_cover(src))return false;
    snprintf(temp,sizeof(temp),"%s.tmp",g->cover);
    FILE *in=fopen(src,"rb");
    if(!in)return false;
    unsigned char sig[8];
    size_t n=fread(sig,1,8,in);
    fseek(in,0,SEEK_SET);
    static const unsigned char expected[]={137,80,78,71,13,10,26,10};
    if(n!=8||memcmp(sig,expected,8)){fclose(in);
    return false;}
    FILE *out=fopen(temp,"wb");
    if(!out){fclose(in);
    return false;}
    char buffer[8192];
    bool good=true;
    while((n=fread(buffer,1,sizeof(buffer),in)))if(fwrite(buffer,1,n,out)!=n){good=false;
    break;}
    if(ferror(in))good=false;
    fclose(in);
    if(fclose(out))good=false;
    if(good){ /* Preserve old art if a replacement cannot be committed. */
        if(rename(temp,g->cover)){
            char old[520];
            snprintf(old,sizeof(old),"%s.previous",g->cover);
            if(rename(g->cover,old))good=false;
            else if(rename(temp,g->cover)){rename(old,g->cover);
            good=false;}
            else remove(old);
        }
    }
    ui_clear_covers();
    return good;
}
static Result launch(Game *g){
    if(g->pending)return CIA_ERROR_INVALID;
    if(g->system==SYSTEM_3DS){AM_TitleInfo info={0};
    u64 id=g->id;
    Result r=AM_GetTitleInfo(g->media,1,&id,&info);
    char entry[120];snprintf(entry,sizeof(entry),"Launch %016llX media=%u metadata: %08lX",(unsigned long long)id,(unsigned)g->media,(unsigned long)(u32)r);app_log(entry);
        if(R_FAILED(r))return r;
        if(info.titleID!=id || !info.size)return (Result)-2;
        if(g->homebrew){aptSetChainloader(id,g->media);app_log("Launch: deferred homebrew handoff");return 0;}
        /* Report APT errors now instead of losing them inside aptExit. */
        r=APT_PrepareToDoApplicationJump(0,id,g->media);
        snprintf(entry,sizeof(entry),"Launch prepare: %08lX",(unsigned long)(u32)r);app_log(entry);
        if(R_FAILED(r))return r;
        /* The new title must not inherit our GSP ownership or banner worker.
           After a successful jump do not run aptExit's default self-chainload. */
        app_log("Launch: release UI and banner worker before handoff");
        ui_shutdown();uiReleased=true;
        u8 parameters[0x300]={0},hmac[0x20]={0};
        r=APT_DoApplicationJump(parameters,sizeof(parameters),hmac);
        snprintf(entry,sizeof(entry),"Launch jump returned: %08lX",(unsigned long)(u32)r);app_log(entry);
        if(R_SUCCEEDED(r)){
            app_log("Launch accepted: exit process without Homebrew return/aptExit");
            svcExitProcess();
        }
        /* A rejected jump keeps this process alive: restore the UI for its error dialog. */
        uiReleased=!ui_init();ui_runtime_reset();
        app_log(uiReleased?"Launch recovery: UI init failed":"Launch recovery: UI restored");
        return r;}
    if(g->system==SYSTEM_DS){Result r=ds_prepare_launch(g->path);
    if(R_FAILED(r))return r;
    draw(true,1,tr("Wechsel in den DS-Modus"));
    return ds_commit_launch();}
    u64 hbTarget=0;
    if(!envIsHomebrew()){Result r=native_handoff_target(&hbTarget);if(R_FAILED(r))return r;}
    Result r=emulator_prepare_launch(backendPaths[g->system],g->path,g->system==SYSTEM_GBA||g->system==SYSTEM_GB);
    if(R_SUCCEEDED(r)&&!envIsHomebrew()){aptSetChainloader(hbTarget,MEDIATYPE_SD);app_log("CIA emulator handoff: configured Rosalina title");}
    return r;
}
static void installProgress(float progress){draw(true,progress,tr("DS-Loader wird installiert"));}
static void switchSystem(unsigned next){
    remembered[systemView]=selected;
    systemView=next;
    selected=remembered[next];
    rebuild();
    flow_reset(&flow,selected);
    confirm=false;
    infoOpen=controlsPage=false;
    status=tr("Bereit");
}
static void saveSettings(void){FILE *f=fopen("sdmc:/3ds/3ds-library/appearance.txt","wb");
if(f){fprintf(f,"%u\n",animation?1:0);
fclose(f);}}
static void changeBrightness(int step){
    if(!lcdReady)return;
    int next=brightness+step;
    if(next<0)next=0;
    if(next>4)next=4;
    u32 before=0,after=0;
    lastError=device_brightness_set(next,&before,&after);
    char entry[120];
    snprintf(entry,sizeof(entry),"Brightness level=%d result=%08lX luminance=%lu -> %lu",next+1,(unsigned long)(u32)lastError,(unsigned long)before,(unsigned long)after);
    app_log(entry);
    if(R_SUCCEEDED(lastError)){brightness=next;
    status=tr("Helligkeit angepasst");}
    else {snprintf(message,sizeof(message),tr("Helligkeit fehlgeschlagen: %08lX"),(unsigned long)(u32)lastError);
    status=message;}

}
typedef struct {Game *game;
Result result;
u32 seedPhase;
bool done;} Preflight;
static void preflightWorker(void *data){Preflight *p=data;
Game *g=p->game;
    p->result=g->system==SYSTEM_3DS||g->system==SYSTEM_DS?0:rom_preflight(backendPaths[g->system],g->path);
    if(g->system==SYSTEM_3DS&&!g->homebrew)p->result=game_seed_ensure(g->id,g->media,&p->seedPhase);
    __atomic_store_n(&p->done,true,__ATOMIC_RELEASE);
}
static void installError(Result r){
    const char *why=r==CIA_ERROR_SPACE?tr("Nicht genug freier SD-Speicher"):r==CIA_ERROR_EXISTS?tr("Spiel bereits installiert - neu einlesen"):r==CIA_ERROR_INVALID?tr("CIA ungueltig oder inzwischen veraendert"):r==CIA_ERROR_CANCELLED?tr("Installation abgebrochen; CIA bleibt erhalten"):NULL;
    if(why)snprintf(message,sizeof(message),"%s",why);
    else snprintf(message,sizeof(message),tr("Installationsfehler %08lX - CIA bleibt erhalten"),(unsigned long)(u32)r);
    status=message;installFailed=true;errorTitle=tr("Installation nicht möglich");app_log(message);
}
typedef struct {char path[512];CiaPackage package;u32 copiedKiB,phase;bool cancel,done;Result result,seedResult,cleanupResult;} InstallJob;
static bool ciaProgress(u64 done,u64 total,bool committing,void *context){
    (void)total;InstallJob *job=context;
    __atomic_store_n(&job->copiedKiB,(u32)(done/1024),__ATOMIC_RELEASE);
    __atomic_store_n(&job->phase,committing?1:0,__ATOMIC_RELEASE);
    return !__atomic_load_n(&job->cancel,__ATOMIC_ACQUIRE);
}
static void installWorker(void *context){InstallJob *job=context;
    job->result=cia_install(job->path,&job->package,ciaProgress,job);
    if(R_SUCCEEDED(job->result)){
        __atomic_store_n(&job->phase,2,__ATOMIC_RELEASE);
        job->seedResult=game_seed_import(job->package.id);
        __atomic_store_n(&job->phase,3,__ATOMIC_RELEASE);
        job->cleanupResult=cia_remove_installer(job->path,&job->package);
        char log[96];snprintf(log,sizeof(log),"CIA cleanup %016llX: %08lX",(unsigned long long)job->package.id,(unsigned long)(u32)job->cleanupResult);app_log(log);
    }
    __atomic_store_n(&job->done,true,__ATOMIC_RELEASE);
}
typedef struct {u64 id;Result result;bool done;} SeedJob;
static void seedWorker(void *context){SeedJob *job=context;job->result=game_seed_import(job->id);__atomic_store_n(&job->done,true,__ATOMIC_RELEASE);}
static void importSelectedSeed(void){
    if(!visibleCount)return;
    Game *g=&games[visible[selected]];if(g->system!=SYSTEM_3DS||g->pending)return;
    SeedJob job={.id=g->id};Thread worker=threadCreate(seedWorker,&job,0x10000,0x30,-2,false);
    if(!worker){snprintf(message,sizeof(message),tr("Seed-Import konnte nicht gestartet werden"));status=message;installFailed=true;errorTitle=tr("Seed-Import fehlgeschlagen");return;}
    while(!__atomic_load_n(&job.done,__ATOMIC_ACQUIRE)){if(!aptMainLoop())break;draw(true,0,tr("Spiel-Seed importieren..."));}
    threadJoin(worker,U64_MAX);threadFree(worker);
    snprintf(message,sizeof(message),job.result==0?tr("Seed importiert"):job.result==SEED_NOT_OFFERED?tr("Nintendo bietet fuer diesen Titel keinen Seed an"):tr("Seed-Import fehlgeschlagen: %08lX"),(unsigned long)(u32)job.result);
    status=message;installFailed=true;errorTitle=job.result==0?tr("Seed importiert"):job.result==SEED_NOT_OFFERED?tr("Seed nicht angeboten"):tr("Seed-Import fehlgeschlagen");
}
static void performInstall(void){
    if(!visibleCount)return;
    Game *g=&games[visible[selected]];InstallJob job={.package=installPackage};
    snprintf(job.path,sizeof(job.path),"%s",g->path);u64 id=g->id;bool wasHomebrew=g->homebrew;
    installPrompt=false;installing=true;
    aptSetSleepAllowed(false);aptSetHomeAllowed(false);
    u64 start=osGetTime();bool active=true;
    Thread thread=threadCreate(installWorker,&job,0x10000,0x30,-2,false);
    if(!thread){installing=false;aptSetSleepAllowed(true);aptSetHomeAllowed(true);installError((Result)-3);return;}
    while(!__atomic_load_n(&job.done,__ATOMIC_ACQUIRE)){
        active=aptMainLoop();hidScanInput();
        u32 phase=__atomic_load_n(&job.phase,__ATOMIC_ACQUIRE);bool committing=phase!=0;
        if(!active||((hidKeysDown()&KEY_B)&&!committing))__atomic_store_n(&job.cancel,true,__ATOMIC_RELEASE);
        if(!active)break;
        u64 copied=(u64)__atomic_load_n(&job.copiedKiB,__ATOMIC_ACQUIRE)*1024;
        if(copied>job.package.fileSize)copied=job.package.fileSize;
        double elapsed=(osGetTime()-start)/1000.0;
        char copiedText[32],totalText[32];cia_format_size(copiedText,sizeof(copiedText),copied);cia_format_size(totalText,sizeof(totalText),job.package.fileSize);
        snprintf(installSize,sizeof(installSize),"%.24s / %.24s",copiedText,totalText);
        float speed=elapsed>3&&copied>65536?copied/elapsed:1048576.f;
        cia_format_time(installEta,sizeof(installEta),cia_estimate_seconds(job.package.fileSize-copied,speed));
        if(committing)snprintf(installEta,sizeof(installEta),"%s",phase==2?tr("Seed-Import läuft"):phase==3?tr("Aufräumen läuft"):tr("Registrierung läuft"));
        draw(true,committing?.99f:.97f*(float)copied/job.package.fileSize,phase==2?tr("Spiel-Seed wird importiert"):phase==3?tr("Installationsdatei wird entfernt"):__atomic_load_n(&job.cancel,__ATOMIC_ACQUIRE)?tr("Abbruch wird abgeschlossen"):committing?tr("Titel wird registriert; bitte warten"):tr("Spiel wird installiert; B: Abbrechen"));
    }
    threadJoin(thread,U64_MAX);threadFree(thread);installing=false;
    aptSetSleepAllowed(true);aptSetHomeAllowed(true);
    if(active){scan();systemView=library_view_for(SYSTEM_3DS,wasHomebrew);rebuild();
        for(unsigned i=0;i<visibleCount;++i)if(games[visible[i]].id==id){selected=i;break;}
        flow_reset(&flow,selected);
        lastError=job.result;
        if(R_SUCCEEDED(lastError)){
            snprintf(message,sizeof(message),tr("Installiert. %s %s"),R_FAILED(job.cleanupResult)?tr("CIA konnte nicht geloescht werden."):tr("CIA geloescht."),job.seedResult==0?tr("Seed importiert."):job.seedResult==SEED_NOT_OFFERED?tr("Kein Seed angeboten."):tr("Seed fehlgeschlagen: X, dann A erneut."));status=message;
            if(R_FAILED(job.seedResult)||R_FAILED(job.cleanupResult)){installFailed=true;errorTitle=tr("Installiert - Hinweis");}
            app_log("CIA installation completed and verified");
        }
        else installError(lastError);
    }
}
static bool startGame(void){
    if(!visibleCount)return false;
    confirm=false;
    saveView();
    Game *g=&games[visible[selected]];
    if(g->pending){lastError=cia_prepare(g->path,g->id,&installPackage);
        if(R_FAILED(lastError)){installError(lastError);return false;}
        cia_format_size(installSize,sizeof(installSize),installPackage.fileSize);
        cia_format_size(installSpace,sizeof(installSpace),installPackage.requiredSpace);
        cia_format_time(installEta,sizeof(installEta),cia_estimate_seconds(installPackage.fileSize,1048576.f));
        installPrompt=true;flow_reset(&flow,selected);return false;}
    preparingGame=true;
    launchFrame=0;
    Preflight p={.game=g};
    Thread worker=threadCreate(preflightWorker,&p,0x10000,0x30,-2,false);
    if(!worker){p.result=(Result)-3;p.done=true;}
    bool active=true;
    do {active=aptMainLoop();
    if(!active)break;
    draw(true,0,__atomic_load_n(&p.done,__ATOMIC_ACQUIRE)?tr("Start wird vorbereitet"):g->system==SYSTEM_3DS&&!g->homebrew?(__atomic_load_n(&p.seedPhase,__ATOMIC_ACQUIRE)?tr("Spiel-Seed wird repariert..."):tr("Spiel-Seed wird geprueft...")):tr("Spiel und Emulator pruefen"));
    ++launchFrame;}
    while(!__atomic_load_n(&p.done,__ATOMIC_ACQUIRE) || (animation && launchFrame<36));
    if(worker){threadJoin(worker,U64_MAX);
    threadFree(worker);}lastError=p.result;
    if(active&&R_SUCCEEDED(lastError)){draw(true,1,tr("Uebergabe an Spiel / Emulator"));
    lastError=launch(g);
    if(R_SUCCEEDED(lastError))return true;}
    preparingGame=false;
    if(uiReleased)return true;
    if(R_FAILED(lastError)){snprintf(message,sizeof(message),tr("Startfehler %08lX%s"),(unsigned long)(u32)lastError,lastError==(Result)-30?tr(" - DS-Loader fehlt"):"");
    if(lastError==FLOW_ERROR_HBL_CONFIG||lastError==FLOW_ERROR_HBL_MISSING)
        snprintf(message,sizeof(message),"%s",l10n_english()?"Homebrew Launcher unavailable. Check Rosalina title in Luma settings.":"Homebrew Launcher nicht verfuegbar. Rosalina-Titel in Luma pruefen.");
    if(R_FAILED(p.result)&&g->system==SYSTEM_3DS&&!g->homebrew){
        if(p.seedPhase)snprintf(message,sizeof(message),tr("Spiel-Seed fehlt/ist ungueltig. WLAN pruefen, dann erneut oeffnen. Fehler %08lX"),(unsigned long)(u32)lastError);
        else snprintf(message,sizeof(message),tr("Spieldaten konnten nicht geprueft werden. Bitte erneut versuchen. Code %08lX"),(unsigned long)(u32)lastError);
    }
    status=message;
    launchFailed=true;errorTitle=p.seedPhase?tr("Spiel braucht Reparatur"):tr("Spielstart fehlgeschlagen");app_log(message);
    if(lastError==(Result)-30)setupDs=true;}
    return !active;
}

static void openInfo(void){
 if(!visibleCount)return;
 Game *g=&games[visible[selected]];
 game_info_begin(&gameInfo,g->name);
 game_info_add(&gameInfo,"System",library_system_names[g->system]);
 if(g->pending){
  game_info_add(&gameInfo,tr("Status"),tr("Noch nicht installiert - A zum Installieren"));
  game_info_size(&gameInfo,tr("CIA-Dateigröße"),g->cia.fileSize);
  game_info_size(&gameInfo,tr("Benötigter SD-Speicher"),g->cia.requiredSpace);
  game_info_add(&gameInfo,tr("Titel-ID"),g->key);
 }else if(g->system==SYSTEM_3DS){
  if(R_SUCCEEDED(metadata(g->id,g->media))){
   char publisher[128],description[256];
   catalog_name(publisher,sizeof(publisher),smdh,language,true);
   game_info_add(&gameInfo,tr("Hersteller / Herausgeber"),publisher);
   catalog_text(description,sizeof(description),smdh+8+(language<16?language:1)*0x200+0x80,0x80);
   if(strcmp(description,g->name))game_info_add(&gameInfo,tr("Vollständiger Titel"),description);
  }
  u64 id=g->id;
  AM_TitleInfo info;
  if(R_SUCCEEDED(AM_GetTitleInfo(g->media,1,&id,&info))&&info.titleID==id){char version[32];
  snprintf(version,sizeof(version),"%u",info.version);
  game_info_size(&gameInfo,tr("Installierte Größe"),info.size);
  game_info_add(&gameInfo,"Titelversion",version);}
  char idText[32];
  snprintf(idText,sizeof(idText),"%016llX",(unsigned long long)g->id);
  game_info_add(&gameInfo,tr("Titel-ID"),idText);
  game_info_add(&gameInfo,tr("Installiert auf"),g->media==MEDIATYPE_SD?"SD-Karte":tr("Systemspeicher (NAND)"));
 }else{
  struct stat st;
  if(!stat(g->path,&st))game_info_size(&gameInfo,tr("Dateigröße"),st.st_size);
  const char *ext=strrchr(g->path,'.');
  if(ext)game_info_add(&gameInfo,tr("Dateiformat"),ext+1);
  static const char *regions[]={"Europe","USA","Japan","Germany","France","World"};
  for(unsigned i=0;i<sizeof(regions)/sizeof(regions[0]);++i){char tag[40];
  snprintf(tag,sizeof(tag),"(%s)",regions[i]);
  if(strstr(g->name,tag)){game_info_add(&gameInfo,tr("Region laut Dateiname"),regions[i]);
  break;}}
  game_info_rom(&gameInfo,g->system,g->path);
 }
 game_info_add(&gameInfo,tr("Start über"),backendNames[g->system]);
 game_info_add(&gameInfo,tr("Zurück zur Library"),g->system<=SYSTEM_DS?tr("HOME-Taste / Library erneut öffnen"):tr("L + R + Y gleichzeitig halten"));
 if(g->path[0])game_info_add(&gameInfo,tr("Spieldatei"),g->path);
 if(backendPaths[g->system][0])game_info_add(&gameInfo,tr("Emulator-Datei"),backendPaths[g->system]);
 game_info_add(&gameInfo,"Cover",ui_cover_available(g->cover)?tr("Vorhanden"):tr("Kein eigenes Cover; per langem Antippen hinzufügen"));
 game_info_add(&gameInfo,tr("Cover-Datei"),g->cover);
 if(g->system==SYSTEM_3DS){game_info_add(&gameInfo,tr("Originalbanner"),ui_cover_available(g->banner)?tr("Originaler Stereo-/Animationscache vorhanden"):tr("Noch kein vorbereiteter Originalbanner-Cache"));
 game_info_add(&gameInfo,tr("Banner-Datei"),g->banner);}
 infoScroll=0;
 infoTouchY=-1;
 infoOpen=true;
 flow_reset(&flow,selected);
}

int main(int argc,char **argv){
    (void)argc;
    (void)argv;
    app_log_begin();
    osSetSpeedupEnable(true);
    if(!ui_init())return 1;
    themeSource=ui_reload_theme();
    aptHookCookie hook;
    aptHook(&hook,onApt,NULL);
    hidSetRepeatParameters(18,5);
    if(R_SUCCEEDED(cfguInit())){u8 lang;
    if(R_SUCCEEDED(CFGU_GetSystemLanguage(&lang)))language=lang;
    cfguExit();}
    l10n_load(language!=2);language=l10n_english()?1:2;
    amReady=R_SUCCEEDED(amInit());
    mkdir("sdmc:/3ds/3ds-library/import",0777);
    u32 currentLuminance=0;
    lcdReady=device_brightness_read(&brightness,&currentLuminance);
    FILE *prefs=fopen("sdmc:/3ds/3ds-library/appearance.txt","rb");
    if(prefs){unsigned value=1;
    if(fscanf(prefs,"%u",&value)==1)animation=value!=0;
    fclose(prefs);}
    FILE *view=fopen("sdmc:/3ds/3ds-library/view-v2.txt","rb");
    bool legacyView=false;if(!view){view=fopen("sdmc:/3ds/3ds-library/view.txt","rb");legacyView=true;}
    unsigned restoreSys=0,restoreSelection=0;
    if(view){if(fscanf(view,"%u%u",&restoreSys,&restoreSelection)!=2)restoreSys=restoreSelection=0;
    for(unsigned i=0;i<VIEW_COUNT;++i){unsigned value=0;if(fscanf(view,"%u",&value)!=1)value=0;remembered[legacyView?library_legacy_view(i):i]=value;}
    fclose(view);}
    if(legacyView)restoreSys=library_legacy_view(restoreSys);
    scan();
    systemView=restoreSys<VIEW_COUNT?restoreSys:0;
    selected=restoreSelection;
    rebuild();
    flow_reset(&flow,selected);
    do {draw(false,0,"");
    hidScanInput();}while(hidKeysHeld()&&aptMainLoop());
    bool leave=false;
    while(aptMainLoop()&&!leave){
        hidScanInput();
        u32 keys=hidKeysDown()|(hidKeysDownRepeat()&(KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT));
        touchPosition pos={0};
        hidTouchRead(&pos);
        if(restored){restored=false;
        confirm=false;
        keys&=~KEY_A;
        flow_reset(&flow,selected);}
        if(keys&KEY_START)break;
        if(installFailed||launchFailed){
            if(keys&(KEY_A|KEY_B))installFailed=launchFailed=false;
            else if((keys&KEY_TOUCH)&&pos.py>=198&&pos.py<229)installFailed=launchFailed=false;
        }else if(installPrompt){
            if(keys&KEY_B)installPrompt=false;
            else if(keys&KEY_A)performInstall();
            else if((keys&KEY_TOUCH)&&pos.py>=188&&pos.py<229){if(pos.px<160)installPrompt=false;else performInstall();}
        }else if(setupDs){
            if(keys&KEY_B)setupDs=false;
            else if(keys&KEY_A){setupDs=false;
            lastError=ds_install_bootstrap(installProgress);
                if(R_SUCCEEDED(lastError))status=tr("DS-Loader installiert. Spiel jetzt mit A starten.");
                else {snprintf(message,sizeof(message),tr("Installation fehlgeschlagen: %08lX"),(unsigned long)(u32)lastError);
                status=message;}}

        }else if(infoOpen){
            if(keys&(KEY_X|KEY_B)){infoOpen=false;
            infoTouchY=-1;}
            else if(keys&KEY_A)importSelectedSeed();
            else if(keys&(KEY_L|KEY_R))switchSystem(home_category_target(systemView,VIEW_COUNT,(keys&KEY_L)!=0,(keys&KEY_R)!=0));
            else {
                unsigned limit=ui_info_scroll_limit();
                if((keys&KEY_UP)&&infoScroll)--infoScroll;
                if((keys&KEY_DOWN)&&infoScroll<limit)++infoScroll;
                if(keys&KEY_LEFT)infoScroll=infoScroll>6?infoScroll-6:0;
                if(keys&KEY_RIGHT)infoScroll=infoScroll+6<limit?infoScroll+6:limit;
                if(keys&KEY_TOUCH){infoTouchY=pos.py;
                if(pos.py<39&&pos.px>=270)infoOpen=false;}
                if((hidKeysHeld()&KEY_TOUCH)&&pos.py>=93&&pos.py<205){
                    if(pos.px>=293)infoScroll=(unsigned)(pos.py-93)*limit/111;
                    else if(infoTouchY>=0){int delta=(infoTouchY-pos.py)/10;
                        if(delta){int next=(int)infoScroll+delta;
                        infoScroll=next<0?0:(unsigned)next>limit?limit:(unsigned)next;
                        infoTouchY=pos.py;}}
                }else if(!(hidKeysHeld()&KEY_TOUCH))infoTouchY=-1;
            }
        }else if(controlsPage){
            if(keys&(KEY_A|KEY_B))controlsPage=false;
            if((keys&KEY_TOUCH)&&home_footer_action(pos.px,pos.py,true,visibleCount)!=HOME_NONE)controlsPage=false;
            if(keys&(KEY_L|KEY_R))switchSystem(home_category_target(systemView,VIEW_COUNT,(keys&KEY_L)!=0,(keys&KEY_R)!=0));
            else if(keys&KEY_Y){controlsPage=false;
            systemPicker=true;
            menuSelected=systemView;}
        }else if(supportOpen){
            if((keys&(KEY_A|KEY_B))||((keys&KEY_TOUCH)&&pos.py>=198))supportOpen=false;
        }else if(settings){
            if(keys&KEY_B){if(settingsMore){settingsMore=false;menuSelected=4;}else settings=false;}
            if(keys&KEY_UP)menuSelected=(menuSelected+4)%5;
            if(keys&KEY_DOWN)menuSelected=(menuSelected+1)%5;
            if((keys&KEY_TOUCH)&&pos.py>=84&&pos.py<219){menuSelected=(pos.py-84)/27;
            keys|=KEY_A;}
            if(!settingsMore&&menuSelected==0){if(keys&KEY_LEFT)changeBrightness(-1);
            if(keys&KEY_RIGHT)changeBrightness(1);
            if(keys&KEY_A)changeBrightness((keys&KEY_TOUCH)?(pos.px<160?-1:1):brightness==4?-4:1);}
            else if(keys&KEY_A){
                if(!settingsMore){
                    if(menuSelected==1){themeSource=ui_reload_theme();status=themeSource;}
                    if(menuSelected==2){animation=!animation;saveSettings();}
                    if(menuSelected==3){
                        if(l10n_save(!l10n_english())){status=tr("Sprache gespeichert");language=l10n_english()?1:2;}
                        else {snprintf(message,sizeof(message),"%s",tr("Sprache konnte nicht gespeichert werden"));status=message;installFailed=true;errorTitle=tr("Einstellungen");}
                    }
                    if(menuSelected==4){settingsMore=true;menuSelected=0;}
                }else{
                    if(menuSelected==0){settings=false;unsigned old=selected;scan();selected=old;rebuild();flow_reset(&flow,selected);}
                    if(menuSelected==1)supportOpen=true;
                    if(menuSelected==2)leave=true;
                    if(menuSelected>=3){settingsMore=false;menuSelected=4;}
                }
            }
        }else if(systemPicker){
            if(keys&KEY_B)systemPicker=false;
            if(keys&KEY_LEFT)menuSelected=(menuSelected+VIEW_COUNT-1)%VIEW_COUNT;
            if(keys&KEY_RIGHT)menuSelected=(menuSelected+1)%VIEW_COUNT;
            if(keys&KEY_UP)menuSelected=(menuSelected+VIEW_COUNT-2)%VIEW_COUNT;
            if(keys&KEY_DOWN)menuSelected=(menuSelected+2)%VIEW_COUNT;
            if((keys&KEY_TOUCH)&&pos.py>=83&&pos.py<207){menuSelected=(pos.py-83)/31*2+(pos.px>=164);
            keys|=KEY_A;}
            if(keys&KEY_A){switchSystem(menuSelected);
            systemPicker=false;}
        }else if(coverMenu){
            if(keys&KEY_B)coverMenu=false;
            if((keys&KEY_UP)&&importCount)importSelected=(importSelected+importCount-1)%importCount;
            if((keys&KEY_DOWN)&&importCount)importSelected=(importSelected+1)%importCount;
            if((keys&KEY_TOUCH)&&pos.py>=84&&pos.py<192){unsigned n=importSelected/4*4+(pos.py-84)/27;
            if(n<importCount)importSelected=n;}
            if(keys&KEY_SELECT)readImports();
            if(keys&KEY_A){status=assignCover()?tr("Cover gespeichert"):tr("Cover nicht lesbar: PNG bis 512x512 verwenden");
            coverMenu=false;}
        }else if(confirm){
            if(keys&KEY_B)confirm=false;
            if((keys&KEY_TOUCH)&&pos.py>=143&&pos.py<188)keys|=pos.px<166?KEY_A:KEY_B;
            if(keys&KEY_B)confirm=false;
            else if((keys&KEY_A)&&visibleCount)leave=startGame();
        }else{
            int footerAction=(keys&KEY_TOUCH)?home_footer_action(pos.px,pos.py,false,visibleCount!=0):HOME_NONE;
            int headerAction=(keys&KEY_TOUCH)?home_header_action(pos.px,pos.py):HOME_NONE;
            if(keys&(KEY_L|KEY_R))switchSystem(home_category_target(systemView,VIEW_COUNT,(keys&KEY_L)!=0,(keys&KEY_R)!=0));
            else if((keys&KEY_X)&&visibleCount)openInfo();
            else if(footerAction==HOME_GUIDE){controlsPage=true;
            flow_reset(&flow,selected);}
            else if(footerAction==HOME_OPEN)leave=startGame();
            else if(headerAction==HOME_PREVIOUS||headerAction==HOME_NEXT)switchSystem(home_category_target(systemView,VIEW_COUNT,headerAction==HOME_PREVIOUS,headerAction==HOME_NEXT));
            else if((keys&KEY_Y)||headerAction==HOME_CATEGORIES){systemPicker=true;
            menuSelected=systemView;
            flow_reset(&flow,selected);}
            else if(headerAction==HOME_SETTINGS){settings=true;
            menuSelected=0;
            flow_reset(&flow,selected);}
            else {
                if(keys&KEY_SELECT){unsigned old=selected;
                scan();
                selected=old;
                rebuild();
                flow_reset(&flow,selected);}
                if(visibleCount){
                    if(keys&(KEY_LEFT|KEY_UP)){selected=(selected+visibleCount-1)%visibleCount;
                    flow.velocity=0;}
                    if(keys&(KEY_RIGHT|KEY_DOWN)){selected=(selected+1)%visibleCount;
                    flow.velocity=0;}
                    int action=flow_update(&flow,visibleCount,&selected,(hidKeysHeld()&KEY_TOUCH)!=0,pos.px,pos.py);
                    if(action==FLOW_START || (keys&KEY_A))leave=startGame();
                    else if(action==FLOW_COVER){readImports();
                    coverMenu=true;}
                }
            }
        }
        flow_step(&flow,visibleCount,selected);
        if(fabsf(flow.velocity)>.012f)selected=(unsigned)lroundf(flow.position);
        if(!leave)draw(false,0,"");
    }
    saveView();
    app_log("Shutdown: drain GPU without VBlank wait");
    if(amReady)amExit();
    aptUnhook(&hook);
    if(!uiReleased)ui_shutdown();
    for(unsigned i=0;i<gameCount;++i)free(games[i].icon);
    app_log("Shutdown complete");
    return 0;
}




