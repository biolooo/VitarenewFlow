#include "forwarder_restore.h"
#include "app_update.h"
#include "update_store.h"
#include "settings_menu.h"
#include "report_transport.h"
#include "report_controls.h"
#include "report_history.h"
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "l10n.h"
#include "autoboot.h"
#include "cartridge.h"
#include "copy_result.h"
#include "twl_repair.h"
#include "menu_music.h"
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
#include "launch_problem.h"
#include "ds_backend.h"
#include "ui.h"
#include "coverflow.h"
#include "device_settings.h"
#include "home_controls.h"
#include "ui_runtime.h"
#include "flow_apt.h"
#include "banner_export.h"
#include "app_log.h"
#include "game_info.h"
#include "game_search.h"
#include "cia_install.h"
#include "game_seed.h"
#include "native_handoff.h"
#include <math.h>
#define MAX_GAMES 1024
typedef struct {u64 id;
FS_MediaType media;
int system;
char name[160],displayName[160],path[512],key[256],cover[512],banner[512];
u16 *icon;bool pending,homebrew,cartridge;CiaPackage cia;} Game;
static Game games[MAX_GAMES];
static unsigned gameCount;
static UiCard cards[MAX_GAMES];
static UiCard neighborCards[32];static unsigned neighborCount;
static unsigned visible[MAX_GAMES],visibleCount,selected,systemView;
static bool searchOpen,searchNumeric;
static char searchQuery[SEARCH_QUERY_SIZE];
static char searchSuggestions[3][SEARCH_QUERY_SIZE];
static unsigned searchReturn,searchReturnView;
static int searchRanks[MAX_GAMES];
static const char *searchSystems[MAX_GAMES];
static bool reportOpen,reportConfirmed,reportDetails,reportHistorical;
static char reportDescription[ERROR_REPORT_DESCRIPTION_MAX+1];
static ErrorReport report;
static int32_t installReportCode;
static ErrorReportArea recentReportArea=ERROR_REPORT_GENERAL;
static int32_t recentReportCode;
static ErrorReportActivity recentReportActivity=ERROR_REPORT_BROWSING;
static ReportHistory reportHistory;
static ErrorReportDiagnostics recentDiagnostics,reportDiagnostics;
static bool recentDiagnosticsValid,recentUnexpected;
static uint32_t recentGeneration;
static u64 reportStartedAt;
static void recordActivity(ErrorReportActivity activity){report_history_record(&reportHistory,activity);}
static ErrorReportDiagnostics collectDiagnostics(void){
    ErrorReportDiagnostics d={0};u64 now=osGetTime();
    if(reportStartedAt&&now>=reportStartedAt){u64 seconds=(now-reportStartedAt)/1000;d.uptime_seconds=seconds>ERROR_REPORT_UPTIME_MAX?ERROR_REPORT_UPTIME_MAX:(uint32_t)seconds;d.validity|=ERROR_REPORT_VALID_UPTIME;}
    FS_ArchiveResource sd={0};
    if(R_SUCCEEDED(FSUSER_GetArchiveResource(&sd,SYSTEM_MEDIATYPE_SD))){u64 mib=(u64)sd.clusterSize*sd.freeClusters/(1024*1024);d.sd_free_mib=mib>ERROR_REPORT_SD_FREE_MAX?ERROR_REPORT_SD_FREE_MAX:(uint32_t)mib;d.validity|=ERROR_REPORT_VALID_SD;}
    d.linear_free_kib=linearSpaceFree()/1024;if(d.linear_free_kib>ERROR_REPORT_LINEAR_FREE_MAX)d.linear_free_kib=ERROR_REPORT_LINEAR_FREE_MAX;d.validity|=ERROR_REPORT_VALID_LINEAR;
    report_history_snapshot(&reportHistory,&d);return d;
}
static void rememberReport(ErrorReportArea area,int32_t code,ErrorReportActivity activity){
    /* Terminal states may be drawn repeatedly: freeze once per operation. */
    if(recentDiagnosticsValid&&recentReportArea==area&&recentReportCode==code&&recentReportActivity==activity&&recentGeneration==reportHistory.generation)return;
    recordActivity(activity);recentReportArea=area;recentReportCode=code;recentReportActivity=activity;
    recentDiagnostics=collectDiagnostics();recentDiagnosticsValid=true;recentGeneration=reportHistory.generation;recentUnexpected=true;
}
static const char *status="Bereit";
static char message[180];
static Result lastError;
static bool setupDs;
static u8 smdh[SMDH_SIZE];
static unsigned language=1;
static bool amReady,restored,confirm,coverMenu;
static CoverFlow flow;
static unsigned remembered[VIEW_COUNT],menuSelected;
static unsigned settingsPage;
static bool updatePage,updateNotes;
static bool componentPage;
static unsigned componentSelected;
static u64 componentCheckedUntil;
static bool supportOpen,autobootPrompt,exitToHomebrew;
static int autobootState;
static bool powerPrompt,powerCancelFailed,copyPrompt;
static unsigned twlPage;static TwlReport twlReport;static bool twlNeedsReboot;static Result twlError;
static Cartridge lastCartridge;
static u64 lastCardPoll;
static bool settings,systemPicker,preparingGame,controlsPage;
static GameInfo gameInfo;
static bool infoOpen;
static unsigned infoScroll;
static int infoTouchY=-1;
static bool installPrompt,installing,installFailed,launchFailed,installUnexpected;
static bool uiReleased,musicReady,homeUnavailable;
static unsigned homeLoading;
static const char *errorTitle;
static int launchRepairComponent=-1;
static CiaPackage installPackage;
static char installSize[64],installSpace[64],installEta[64];
static const char *themeSource="Standard";
static char importNames[128][256];
static const char *imports[128];
static unsigned importCount,importSelected;
static const char *backendNames[SYSTEM_COUNT]={"3DS-System","YANBF / nds-bootstrap","mGBA Library","DaedalusX64 Library","Snes9x Library","VirtuaNES Library","Red Viper Library","mGBA Library"};
static const char *backendPaths[SYSTEM_COUNT]={"","","sdmc:/3ds/mgba-library/mgba-library.3dsx","sdmc:/3ds/n64-library/n64-library.3dsx","sdmc:/3ds/snes-library/snes-library.3dsx","sdmc:/3ds/nes-library/nes-library.3dsx","sdmc:/3ds/vb-library/vb-library.3dsx","sdmc:/3ds/mgba-library/mgba-library.3dsx"};
static bool reportAvailable(void){
    ComponentState c=components_status().state;UpdateState u=app_update_status().state;
    return !uiReleased&&!searchOpen&&!installing&&!preparingGame&&!homeLoading&&!powerPrompt&&twlPage!=3&&
        c!=COMP_DOWNLOAD&&c!=COMP_READY&&u!=UPDATE_CHECKING&&u!=UPDATE_DOWNLOADING&&u!=UPDATE_READY;
}
static bool reportFallback(void){
    return report_controls_fallback(reportAvailable(),recentUnexpected,(installFailed&&installUnexpected)||(launchFailed&&R_FAILED(lastError))||
        (componentPage&&components_status().state==COMP_ERROR)||(updatePage&&app_update_status().state==UPDATE_ERROR)||(twlPage&&R_FAILED(twlError)));
}
static void openReport(void){
    if(!reportAvailable())return;
    report_transport_reset();reportConfirmed=false;reportDetails=false;reportDescription[0]=0;
    report=(ErrorReport){.version="0.12.37",.description=reportDescription,
        .context=envIsHomebrew()?ERROR_REPORT_HOMEBREW:ERROR_REPORT_NATIVE};
    int boot=autoboot_status();report.autoboot=boot==1?ERROR_REPORT_AUTOBOOT_ON:boot==0?ERROR_REPORT_AUTOBOOT_OFF:ERROR_REPORT_AUTOBOOT_UNKNOWN;
    if(R_SUCCEEDED(cfguInit())){
        u8 model=255,region=255;
        static const ErrorReportModel models[]={ERROR_REPORT_3DS,ERROR_REPORT_3DS_XL,ERROR_REPORT_NEW_3DS,ERROR_REPORT_2DS,ERROR_REPORT_NEW_3DS_XL,ERROR_REPORT_NEW_2DS_XL};
        if(R_SUCCEEDED(CFGU_GetSystemModel(&model))&&model<6)report.model=models[model];
        if(R_SUCCEEDED(CFGU_SecureInfoGetRegion(&region))&&region<7)report.region=(ErrorReportRegion)(region+1);
        cfguExit();
    }
    /* Only typed categories/codes; never copy message, status, game or logs. */
    if(launchFailed){report.area=ERROR_REPORT_LAUNCH;report.code=lastError;report.activity=ERROR_REPORT_STARTING_GAME;}
    else if(installFailed){report.area=installReportCode?ERROR_REPORT_INSTALL:ERROR_REPORT_GENERAL;report.code=installReportCode;report.activity=installReportCode?ERROR_REPORT_INSTALLING:ERROR_REPORT_ACTIVITY_UNKNOWN;}
    else if(componentPage){report.area=ERROR_REPORT_COMPONENT;report.code=components_error_code();report.activity=ERROR_REPORT_CHECKING_COMPONENTS;}
    else if(updatePage){report.area=ERROR_REPORT_UPDATE;report.code=app_update_error_code();report.activity=ERROR_REPORT_CHECKING_UPDATE;}
    else if(twlPage){report.area=ERROR_REPORT_DS;report.code=twlError;report.activity=ERROR_REPORT_CHECKING_DS;}
    else if(coverMenu){report.area=ERROR_REPORT_COVER;report.activity=ERROR_REPORT_LOADING_COVER;}
    else {report.area=recentReportArea;report.code=recentReportCode;report.activity=recentReportActivity;}
    reportHistorical=recentDiagnosticsValid;reportDiagnostics=reportHistorical?recentDiagnostics:collectDiagnostics();
    if(reportHistorical){report.area=recentReportArea;report.code=recentReportCode;report.activity=recentReportActivity;}
    report.diagnostics=&reportDiagnostics;recentUnexpected=false;reportOpen=true;
}
static void describeReport(void){
    SwkbdState keyboard;swkbdInit(&keyboard,SWKBD_TYPE_NORMAL,2,ERROR_REPORT_DESCRIPTION_MAX);
    swkbdSetHintText(&keyboard,l10n_english()?"What happened? No names or private data.":"Was ist passiert? Keine Namen oder privaten Daten.");
    swkbdSetInitialText(&keyboard,reportDescription);
    char draft[ERROR_REPORT_DESCRIPTION_MAX+1]={0};
    if(swkbdInputText(&keyboard,draft,sizeof(draft))==SWKBD_BUTTON_RIGHT){
        ErrorReport candidate=report;candidate.description=draft;char json[ERROR_REPORT_JSON_CAPACITY];size_t n;
        if(error_report_json(&candidate,json,sizeof(json),&n))memcpy(reportDescription,draft,sizeof(draft));
    }
    reportConfirmed=false;
}
static void openTwl(void){recordActivity(ERROR_REPORT_CHECKING_DS);twl_diagnose(&twlReport);twlError=twlReport.error;twlPage=twl_repair_pending()?4:1;settings=false;infoOpen=false;}
static void rebuild(void){
    visibleCount=0;
    char query[SEARCH_QUERY_SIZE];search_normalize(query,sizeof(query),searchQuery);
    for(unsigned i=0;i<gameCount;++i){
        Game *g=&games[i];
        if(!searchOpen&&library_view_for(g->system,g->homebrew)!=systemView)continue;
        game_display_title(g->displayName,sizeof(g->displayName),g->name);
        int rank=1;
        if(searchOpen){
            char title[SEARCH_TITLE_SIZE],combined[256],normal[SEARCH_TITLE_SIZE];
            search_normalize(title,sizeof(title),g->displayName);
            rank=search_score(title,query);
            static const char *aliases[]={"3ds","ds nds","gba","n64","snes","nes","vb","gb gbc"};
            snprintf(combined,sizeof(combined),"%s %s %s",g->displayName,g->homebrew?"Homebrew":library_system_names[g->system],g->homebrew?"":aliases[g->system]);
            search_normalize(normal,sizeof(normal),combined);
            int qualified=search_score(normal,query)-100;
            if(qualified>rank)rank=qualified;
            if(!rank)continue;
        }
        unsigned n=visibleCount++;
        while(searchOpen&&n&&rank>searchRanks[n-1]){
            visible[n]=visible[n-1];searchRanks[n]=searchRanks[n-1];n--;
        }
        visible[n]=i;searchRanks[n]=rank;
    }
    for(unsigned n=0;n<visibleCount;n++){
        Game *g=&games[visible[n]];
        if(g->cartridge){size_t len=strlen(g->displayName);snprintf(g->displayName+len,sizeof(g->displayName)-len," [%s]",tr("Modul"));}
        cards[n]=(UiCard){g->displayName,g->cover,backendNames[g->system],g->system==SYSTEM_3DS||g->system==SYSTEM_DS,g->icon,g->banner,g->pending};
        static const char *labels[]={"3DS","DS","GBA","N64","SNES","NES","VB","GB/GBC"};
        searchSystems[n]=g->homebrew?"Homebrew":labels[g->system];
    }
    if(selected>=visibleCount)selected=0;
    neighborCount=0;
    if(searchOpen){
        memset(searchSuggestions,0,sizeof(searchSuggestions));
        for(unsigned i=0;i<visibleCount&&!searchSuggestions[2][0];i++)search_suggest_add(searchSuggestions,searchQuery,cards[i].name);
        return;
    }
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
    if(musicReady&&!loading)menu_music_start_if_needed();
    if(componentPage&&components_status().state==COMP_ERROR)rememberReport(ERROR_REPORT_COMPONENT,components_error_code(),ERROR_REPORT_CHECKING_COMPONENTS);
    if(updatePage&&app_update_status().state==UPDATE_ERROR)rememberReport(ERROR_REPORT_UPDATE,app_update_error_code(),ERROR_REPORT_CHECKING_UPDATE);
    if(twlPage&&R_FAILED(twlError))rememberReport(ERROR_REPORT_DS,twlError,ERROR_REPORT_CHECKING_DS);
    const char *target=visibleCount?games[visible[selected]].cover:"";
    UiState state={.system=library_view_names[systemView],.status=status,.version="0.12.37",
        .reportAvailable=reportAvailable(),.reportFallback=reportFallback(),.reportDetails=reportDetails,.reportHistorical=reportHistorical,.reportDiagnostics=&reportDiagnostics,.reportOpen=reportOpen,.reportConfirmed=reportConfirmed,.reportConfigured=report_transport_configured(),.reportState=report_transport_state(),.reportDescription=reportDescription,.reportArea=report.area,.reportCode=report.code,.reportModel=report.model,.reportRegion=report.region,.reportAutoboot=report.autoboot,.reportContext=report.context,
        .searchOpen=searchOpen,.searchQuery=searchQuery,.searchSuggestions=searchSuggestions,.searchNumeric=searchNumeric,.searchSystems=searchSystems,
        .cards=cards,.count=visibleCount,.selected=selected,.total=visibleCount,.confirm=confirm,
        .loading=loading,.progress=progress,.stage=stage,.homeLoading=homeLoading,.coverMenu=coverMenu,.coverTarget=target,
        .imports=imports,.importCount=importCount,.importSelected=importSelected,.setupDs=setupDs,.updatePage=updatePage,.updateNotes=updateNotes,.update=app_update_status(),.componentPage=componentPage,.componentSelected=componentSelected,.component=components_status(),
        .carousel=flow.position,.systemIndex=visibleCount?(unsigned)games[visible[selected]].system:library_view_system(systemView),.settings=settings,.settingsPage=settingsPage,.autobootPrompt=autobootPrompt,.autobootState=autobootState,.powerPrompt=powerPrompt,.copyPrompt=copyPrompt,.twlAdvice=twl_recommendation(&twlReport),.twlPage=twlPage,.twlError=(int32_t)twlError,.twlNeedsReboot=twlNeedsReboot,.twlKnown=twlReport.known,.twlSupported=twlReport.supported,.twlMissing=twlReport.missing,.twlLoader=twlReport.loader,.twlFiles=twlReport.files,.support=supportOpen,.systemPicker=systemPicker,.menuSelected=menuSelected,
        .componentRechecked=osGetTime()<componentCheckedUntil,.powerCancelFailed=powerCancelFailed,.themeSource=themeSource,.launching=preparingGame,.controls=controlsPage,.info=infoOpen?&gameInfo:NULL,.infoScroll=infoScroll,
        .installPrompt=installPrompt,.installing=installing,.installSize=installSize,.installSpace=installSpace,.installEta=installEta,.installError=(installFailed||launchFailed)?message:NULL,.launchError=launchFailed,.launchRepair=launchFailed&&launchRepairComponent>=0,.errorTitle=errorTitle,.prefetch=neighborCards,.prefetchCount=neighborCount};
        ui_draw(&state);
}
/* Render before blocking APT/NS calls, while we still own the display.
   Pending HOME normally suppresses frames; explicitly allow this one screen.
   Do not render from an APT callback or after graphics ownership is released. */
static void showHomeLoading(unsigned phase){
    bool first=homeLoading==0;homeLoading=phase;
    if(uiReleased||!aptIsActive()||aptShouldClose())return;
    ui_runtime_allow_pending_home(true);
    draw(true,0,"");ui_runtime_drain();
    /* One refresh interval lets the queued buffer reach the LCD before IPC
       blocks. It is not an artificial minimum display duration. */
    if(first)svcSleepThread(20000000);
}
static void finishHomeLoading(void){
    homeLoading=0;ui_runtime_allow_pending_home(powerPrompt);ui_runtime_reset();
}
static bool systemPowerAction(bool reboot){
    recordActivity(ERROR_REPORT_POWER_MENU);
    Result r=ptmSysmInit();
    if(R_FAILED(r)){app_log("PTM system service unavailable");return false;}
    bool sleep=aptIsSleepAllowed();aptSetSleepAllowed(false);
    app_log(reboot?"PTM: restart requested":"PTM: shutdown requested");
    ui_shutdown();uiReleased=true;aptClearChainloader();
    r=reboot?PTMSYSM_RebootAsync(1000000000ULL):PTMSYSM_ShutdownAsync(1000000000ULL);
    ptmSysmExit();
    char entry[96];snprintf(entry,sizeof(entry),"PTM request result: %08lX",(unsigned long)(u32)r);app_log(entry);
    /* Keep the process alive while PM coordinates the MCU operation. Exiting
       immediately turns an asynchronous request into an ordinary HOME return. */
    if(R_SUCCEEDED(r))for(unsigned i=0;i<50;i++)svcSleepThread(100000000);
    app_log(R_FAILED(r)?"PTM request rejected":"PTM request timed out after 5 seconds");
    if(ui_init()){uiReleased=false;ui_runtime_reset();}
    aptSetSleepAllowed(sleep);return false;
}
static bool restartAfterBootChange(void){return systemPowerAction(true);}
static void saveView(void);
static bool openHomeMenu(void){
    recordActivity(ERROR_REPORT_OPENING_HOME);
    report_transport_stop();
    showHomeLoading(1);
    bool registered=false;Result r=APT_IsRegistered(aptGetMenuAppID(),&registered);
    saveView();settings=false;autobootPrompt=false;
    if(R_SUCCEEDED(r)&&registered){
        /* Same HOME-press notification as Rosalina's Return To HOME Menu. */
        aptSetHomeAllowed(true);
        showHomeLoading(2);
        r=srvPublishToSubscriber(0x204,0);
        if(R_SUCCEEDED(r)){finishHomeLoading();return true;}
    }else if(R_SUCCEEDED(r)){
        int boot=autoboot_status();
        if(boot==0||(boot==1&&autoboot_home_suspend())){
            showHomeLoading(3);
            app_handoff_log("HOME escape: managed autoboot paused; reboot to HOME");
            systemPowerAction(true);r=(Result)-105;
        }else r=(Result)-106;
    }
    snprintf(message,sizeof(message),tr("HOME nicht geöffnet (%08lX). Erneut versuchen."),(unsigned long)(u32)r);
    status=message;rememberReport(ERROR_REPORT_HOME,r,ERROR_REPORT_OPENING_HOME);app_handoff_log(message);powerPrompt=true;finishHomeLoading();return false;
}
static bool flowMainLoop(void){
    aptHandleSleep();
    /* HOME is intercepted only in a coldboot session; POWER retains its dialog.
       Do not leave while an install/preflight/repair owns the foreground. */
    if(homeUnavailable&&aptCheckHomePressRejected()&&!installing&&!preparingGame&&twlPage!=3){
        openHomeMenu();
    }
    if(aptShouldJumpToHome()){
        report_transport_stop();
        bool registered=false;NS_APPID menu=aptGetMenuAppID();Result r=APT_IsRegistered(menu,&registered);
        char entry[100];snprintf(entry,sizeof(entry),"HOME request: menu=%u registered=%u result=%08lX",(unsigned)menu,registered,(unsigned long)(u32)r);if(!powerPrompt)app_log(entry);
        if(R_FAILED(r)||!registered){
            /* libctru screen-transfer waits indefinitely for an absent HOME.
               Stay active and offer PTM power controls instead. */
            ui_runtime_allow_pending_home(true);
            if(!powerPrompt){powerCancelFailed=false;status=tr("Autostart pausiert bis zum nächsten Öffnen der Library.");}
            powerPrompt=true;return !aptShouldClose();
        }
        showHomeLoading(2);
        if(!uiReleased)ui_runtime_drain();
        app_handoff_log("HOME loading: frame submitted; APT system handoff");
        aptHandleJumpToHome();
        finishHomeLoading();
    }
    return !aptShouldClose();
}
static void onApt(APT_HookType event,void *unused){
    (void)unused;
    if(event==APTHOOK_ONSLEEP||event==APTHOOK_ONSUSPEND||event==APTHOOK_ONEXIT)report_transport_cancel();
    if(event==APTHOOK_ONSLEEP||event==APTHOOK_ONSUSPEND||event==APTHOOK_ONEXIT)menu_music_pause(true);
    if(event==APTHOOK_ONWAKEUP||event==APTHOOK_ONRESTORE)menu_music_pause(false);
    if(event==APTHOOK_ONRESTORE||event==APTHOOK_ONWAKEUP){restored=true;
    ui_runtime_reset();}
    if(event==APTHOOK_ONSUSPEND){app_log("APT suspend: draining GPU");if(!uiReleased)ui_runtime_drain();app_log("APT suspend: GPU ready");}
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
            if(!flowMainLoop())break;
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
    Cartridge cart;Result cr=amReady?cartridge_read(&cart,language):(Result)-1;
    {char entry[128];snprintf(entry,sizeof(entry),"Cartridge scan: result=%08lX present=%u DS=%u id=%016llX",(unsigned long)(u32)cr,R_SUCCEEDED(cr)&&cart.present,R_SUCCEEDED(cr)&&cart.ds,(unsigned long long)(R_SUCCEEDED(cr)?cart.id:0));app_log(entry);}
    if(R_SUCCEEDED(cr)&&cart.present&&gameCount<MAX_GAMES){
        Game *g=&games[gameCount++];memset(g,0,sizeof(*g));g->cartridge=true;g->media=MEDIATYPE_GAME_CARD;g->system=cart.ds?SYSTEM_DS:SYSTEM_3DS;g->id=cart.id;
        snprintf(g->name,sizeof(g->name),"%s",cart.name);
        snprintf(g->key,sizeof(g->key),cart.ds?"cart-%.4s":"%s",cart.ds?cart.code:"");
        if(!cart.ds){snprintf(g->key,sizeof(g->key),"%016llX",(unsigned long long)g->id);
            snprintf(g->banner,sizeof(g->banner),"sdmc:/3ds/3ds-library/banners/%016llX.lbn",(unsigned long long)g->id);
            if(R_SUCCEEDED(metadata(g->id,g->media))){catalog_name(g->name,sizeof(g->name),smdh,language,false);g->icon=malloc(48*48*2);if(g->icon)memcpy(g->icon,smdh+SMDH_ICON_OFFSET,48*48*2);}
        }else if(cart.hasIcon){g->icon=malloc(sizeof(cart.icon));if(g->icon)memcpy(g->icon,cart.icon,sizeof(cart.icon));}
        library_cover_path(g->cover,sizeof(g->cover),g->system,g->key);
        lastCartridge=cart;
    }else memset(&lastCartridge,0,sizeof(lastCartridge));
    DIR *cias=amReady?opendir("sdmc:/cias"):NULL;
    if(cias){struct dirent *entry;
        while((entry=readdir(cias))&&gameCount<MAX_GAMES){
            const char *ext=strrchr(entry->d_name,'.');if(!ext||strcasecmp(ext,".cia"))continue;
            if(!flowMainLoop())break;
            char path[512];int n=snprintf(path,sizeof(path),"sdmc:/cias/%s",entry->d_name);struct stat st;
            if(n<0||(size_t)n>=sizeof(path)||stat(path,&st)||!S_ISREG(st.st_mode))continue;
            CiaPackage package;Result r=cia_inspect(path,&package,smdh);
            if(R_FAILED(r)){char log[96];snprintf(log,sizeof(log),"CIA skipped: %08lX",(unsigned long)(u32)r);app_log(log);continue;}
            bool duplicate=false;for(unsigned j=0;j<gameCount;++j)if(!games[j].cartridge&&games[j].system==SYSTEM_3DS&&games[j].id==package.id)duplicate=true;
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
    if(f){unsigned view=searchOpen?searchReturnView:systemView,index=searchOpen?searchReturn:selected;fprintf(f,"%u\n%u\n",view,index);
    remembered[view]=index;
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
/* Coldboot HBL can run without HOME. Start the verified regional menu before
   preparing a native application jump; registration is asynchronous. */
static Result prepareNativeHome(void){
    static bool attempted=false;
    if(aptShouldClose())return (Result)-107;
    bool registered=false;
    Result r=APT_IsRegistered(aptGetMenuAppID(),&registered);
    if(R_FAILED(r))return (Result)-107;
    if(!registered)showHomeLoading(1);
    if(!registered&&!attempted){
        static const u64 ids[]={0x0004003000008202ULL,0x0004003000008F02ULL,
            0x0004003000009802ULL,0,0x000400300000A102ULL,
            0x000400300000A902ULL,0x000400300000B102ULL};
        u8 region=255;char log[160];
        r=cfguInit();if(R_FAILED(r))return (Result)-107;
        r=CFGU_SecureInfoGetRegion(&region);cfguExit();
        if(R_FAILED(r)||region>=sizeof(ids)/sizeof(ids[0])||!ids[region])return (Result)-107;
        u64 menu=ids[region];AM_TitleInfo info={0};
        r=amInit();if(R_FAILED(r))return (Result)-107;
        r=AM_GetTitleInfo(MEDIATYPE_NAND,1,&menu,&info);amExit();
        if(R_FAILED(r)||info.titleID!=menu)return (Result)-107;
        r=nsInit();if(R_FAILED(r))return (Result)-107;
        attempted=true;u32 pid=0;
        app_handoff_log("HOME explicit process launch before APT preparation");
        r=NS_LaunchTitle(menu,0,&pid);nsExit();
        snprintf(log,sizeof(log),"HOME NS launch title=%016llX result=%08lX pid=%lu",
            (unsigned long long)menu,(unsigned long)(u32)r,(unsigned long)pid);
        app_handoff_log(log);
        if(R_FAILED(r))return (Result)-107;
    }
    for(unsigned i=0;;i++){
        r=APT_IsRegistered(aptGetMenuAppID(),&registered);
        if(R_FAILED(r))return (Result)-107;
        if(registered){
            homeUnavailable=false;aptSetHomeAllowed(true);
            if(homeLoading)showHomeLoading(2);
            app_handoff_log("HOME registered; native APT preparation now allowed");
            return 0;
        }
        if(i==100){app_handoff_log("HOME startup timed out; UI retained; no native jump prepared");return (Result)-107;}
        if(aptShouldClose()){app_handoff_log("HOME startup interrupted by system close");return (Result)-107;}
        showHomeLoading(1);
        svcSleepThread(100000000);
    }
}
static Result ensureNativeHome(void){
    u64 started=osGetTime();Result r=prepareNativeHome();
    if(homeLoading){
        char log[120];snprintf(log,sizeof(log),"HOME loading: preparation %s after %lu ms",R_SUCCEEDED(r)?"ready":"failed",(unsigned long)(osGetTime()-started));app_handoff_log(log);
        finishHomeLoading();
    }
    return r;
}
static Result launchTitle(u64 id,FS_MediaType media,bool forceReboot){
    bool registered=false;Result r=APT_IsRegistered(aptGetMenuAppID(),&registered);
    char log[160];snprintf(log,sizeof(log),"Handoff target=%016llX media=%u HOME=%u query=%08lX",(unsigned long long)id,(unsigned)media,registered,(unsigned long)(u32)r);app_handoff_log(log);
    if(R_FAILED(r))return r;
    bool native=(id>>32)==0x00040000ULL&&!forceReboot;
    bool reboot=forceReboot||(!registered&&!native);
    if(native){
        r=ensureNativeHome();
        if(R_FAILED(r))return r;
    }
    r=reboot?nsInit():APT_PrepareToDoApplicationJump(0,id,media);
    snprintf(log,sizeof(log),"Handoff %s prepare=%08lX",reboot?"NS reboot":"APT jump",(unsigned long)(u32)r);app_handoff_log(log);
    if(R_FAILED(r))return r;
    ui_shutdown();uiReleased=true;
    if(reboot){
        r=NS_RebootToTitle(media,id,0);nsExit();
        snprintf(log,sizeof(log),"Handoff DS/legacy NS request=%08lX",(unsigned long)(u32)r);app_handoff_log(log);
        /* NS coordinates termination asynchronously. Do not perform aptExit
           or return through hbmenu while the reboot request is outstanding. */
        if(R_SUCCEEDED(r)){
            for(unsigned i=0;i<100;i++)svcSleepThread(100000000);
            r=(Result)-105;app_handoff_log("Handoff timed out after 10 seconds");
        }
    }else{
        u8 parameters[0x300]={0},hmac[0x20]={0};
        r=APT_DoApplicationJump(parameters,sizeof(parameters),hmac);
        snprintf(log,sizeof(log),"Handoff APT result=%08lX",(unsigned long)(u32)r);app_handoff_log(log);
        if(R_SUCCEEDED(r)){
            app_handoff_log("Handoff accepted; exit process without second APT close");
            svcExitProcess();
        }
    }
    uiReleased=!ui_init();ui_runtime_reset();
    app_handoff_log(uiReleased?"Handoff recovery failed":"Handoff rejected; UI restored");
    return r;
}
static Result launch(Game *g){
    menu_music_stop();
    /* Title presence cannot predict DS compatibility. Let the actual loader run. */
    if(g->pending)return CIA_ERROR_INVALID;
    if(g->cartridge){
        Cartridge card;Result r=cartridge_read(&card,language);
        if(R_FAILED(r)||!card.present||card.ds!=(g->system==SYSTEM_DS)||(card.ds?strcmp(card.code,g->key+5)!=0:card.id!=g->id))return (Result)-103;
        if(card.ds)return launchTitle(0,MEDIATYPE_GAME_CARD,true);
        /* The freshly read card identity is authoritative. Installed-title
           metadata (especially installed size) is not a prerequisite for cards. */
        g->media=MEDIATYPE_GAME_CARD;g->homebrew=false;
        char log[96];snprintf(log,sizeof(log),"Verified CTR cartridge=%016llX; launch from game-card media",(unsigned long long)card.id);app_handoff_log(log);
        return launchTitle(card.id,MEDIATYPE_GAME_CARD,false);
    }
    if(g->system==SYSTEM_3DS){AM_TitleInfo info={0};
    u64 id=g->id;
    Result r=AM_GetTitleInfo(g->media,1,&id,&info);
    char entry[192];snprintf(entry,sizeof(entry),"Launch requested=%016llX returned=%016llX media=%u size=%llu metadata=%08lX",(unsigned long long)id,(unsigned long long)info.titleID,(unsigned)g->media,(unsigned long long)info.size,(unsigned long)(u32)r);app_handoff_log(entry);
        if(R_FAILED(r))return r;
        if(info.titleID!=id || !info.size)return (Result)-2;
        if(g->homebrew&&autoboot_status()==1){
            u64 hbTarget=0;Result check=native_handoff_target(&hbTarget);
            if(R_SUCCEEDED(check)&&hbTarget==id){
                r=homebrew_prepare_launch("sdmc:/3ds/3ds-library/autoboot-hbmenu.3dsx");
                if(R_FAILED(r))return r;
                if(!envIsHomebrew())aptSetChainloader(id,g->media);
                app_log("Homebrew Launcher: explicit original HBL target");return 0;
            }
            if(R_FAILED(check)&&id==0x000400000D921E00ULL)return check;
        }
        return launchTitle(id,g->media,false);}
    if(g->system==SYSTEM_DS){
        app_handoff_log(g->path);
        Result r=ds_prepare_launch(g->path);
        if(R_FAILED(r)){char log[80];snprintf(log,sizeof(log),"DS preflight=%08lX",(unsigned long)(u32)r);app_handoff_log(log);return r;}
        draw(true,1,tr("Wechsel in den DS-Modus"));
        return launchTitle(DS_BOOTSTRAP_ID,MEDIATYPE_NAND,false);
    }
    u64 hbTarget=0;
    if(!envIsHomebrew()){Result r=native_handoff_target(&hbTarget);if(R_FAILED(r))return r;}
    Result r=emulator_prepare_launch(backendPaths[g->system],g->path,g->system==SYSTEM_GBA||g->system==SYSTEM_GB);
    if(R_SUCCEEDED(r)&&!envIsHomebrew()){aptSetChainloader(hbTarget,MEDIATYPE_SD);app_log("CIA emulator handoff: configured Rosalina title");}
    return r;
}

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
typedef struct {Game *game;
Result result;
u32 seedPhase;
bool done;} Preflight;
static void preflightWorker(void *data){Preflight *p=data;
Game *g=p->game;
    p->result=g->system==SYSTEM_3DS||g->system==SYSTEM_DS?0:rom_preflight(backendPaths[g->system],g->path);
    if(g->system==SYSTEM_3DS&&!g->homebrew&&!g->cartridge)p->result=game_seed_ensure(g->id,g->media,&p->seedPhase);
    __atomic_store_n(&p->done,true,__ATOMIC_RELEASE);
}
static void installError(Result r){

    const char *why=r==CIA_ERROR_SPACE?tr("Nicht genug freier SD-Speicher"):r==CIA_ERROR_EXISTS?tr("Spiel bereits installiert - neu einlesen"):r==CIA_ERROR_INVALID?tr("CIA ungueltig oder inzwischen veraendert"):r==CIA_ERROR_CANCELLED?tr("Installation abgebrochen; CIA bleibt erhalten"):NULL;
    if(why)snprintf(message,sizeof(message),"%s",why);
    else snprintf(message,sizeof(message),tr("Installationsfehler %08lX - CIA bleibt erhalten"),(unsigned long)(u32)r);
    status=message;installUnexpected=R_FAILED(r)&&r!=CIA_ERROR_CANCELLED;if(installUnexpected)rememberReport(ERROR_REPORT_INSTALL,r,ERROR_REPORT_INSTALLING);installReportCode=r;installFailed=true;errorTitle=tr("Installation nicht möglich");app_log(message);
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
typedef struct {TwlReport approved;Result result;bool done;unsigned completed,total;} TwlJob;
static void twlProgress(unsigned done,unsigned total,void *context){TwlJob *job=context;__atomic_store_n(&job->total,total,__ATOMIC_RELEASE);__atomic_store_n(&job->completed,done,__ATOMIC_RELEASE);}
static void twlWorker(void *context){TwlJob *job=context;job->result=twl_repair_begin_progress(&job->approved,twlProgress,job);__atomic_store_n(&job->done,true,__ATOMIC_RELEASE);}
static Result performTwlRepair(void){
    TwlJob job={.approved=twlReport};bool sleep=aptIsSleepAllowed(),home=aptIsHomeAllowed();
    aptSetSleepAllowed(false);aptSetHomeAllowed(false);
    Thread worker=threadCreate(twlWorker,&job,0x10000,0x30,-2,false);
    if(!worker){aptSetSleepAllowed(sleep);aptSetHomeAllowed(home);return (Result)-3;}
    while(!__atomic_load_n(&job.done,__ATOMIC_ACQUIRE)){
        if(!flowMainLoop())break;
        unsigned total=__atomic_load_n(&job.total,__ATOMIC_ACQUIRE),done=__atomic_load_n(&job.completed,__ATOMIC_ACQUIRE);
        char stage[100];snprintf(stage,sizeof(stage),tr("DS-System vorbereiten: %u / %u. Bitte warten."),done,total);
        draw(true,total?(float)done/total:0,stage);
    }
    threadJoin(worker,U64_MAX);threadFree(worker);
    aptSetSleepAllowed(sleep);aptSetHomeAllowed(home);return job.result;
}
static void importSelectedSeed(void){
    recordActivity(ERROR_REPORT_INSTALLING);
    if(!visibleCount)return;
    Game *g=&games[visible[selected]];if(g->system!=SYSTEM_3DS||g->pending)return;
    SeedJob job={.id=g->id};Thread worker=threadCreate(seedWorker,&job,0x10000,0x30,-2,false);
    if(!worker){snprintf(message,sizeof(message),tr("Seed-Import konnte nicht gestartet werden"));status=message;installReportCode=-3;installUnexpected=true;rememberReport(ERROR_REPORT_INSTALL,-3,ERROR_REPORT_INSTALLING);installFailed=true;errorTitle=tr("Seed-Import fehlgeschlagen");return;}
    while(!__atomic_load_n(&job.done,__ATOMIC_ACQUIRE)){if(!flowMainLoop())break;draw(true,0,tr("Spiel-Seed importieren..."));}
    threadJoin(worker,U64_MAX);threadFree(worker);
    snprintf(message,sizeof(message),job.result==0?tr("Seed importiert"):job.result==SEED_NOT_OFFERED?tr("Nintendo bietet fuer diesen Titel keinen Seed an"):tr("Seed-Import fehlgeschlagen: %08lX"),(unsigned long)(u32)job.result);
    status=message;installReportCode=R_FAILED(job.result)?job.result:0;installUnexpected=R_FAILED(job.result)&&job.result!=SEED_NOT_OFFERED;if(installUnexpected)rememberReport(ERROR_REPORT_INSTALL,installReportCode,ERROR_REPORT_INSTALLING);installFailed=true;errorTitle=job.result==0?tr("Seed importiert"):job.result==SEED_NOT_OFFERED?tr("Seed nicht angeboten"):tr("Seed-Import fehlgeschlagen");
}
static void performInstall(void){
    recordActivity(ERROR_REPORT_INSTALLING);
    if(!visibleCount)return;
    Game *g=&games[visible[selected]];InstallJob job={.package=installPackage};
    snprintf(job.path,sizeof(job.path),"%s",g->path);u64 id=g->id;bool wasHomebrew=g->homebrew;
    installPrompt=false;installing=true;
    aptSetSleepAllowed(false);aptSetHomeAllowed(false);
    u64 start=osGetTime();bool active=true;
    Thread thread=threadCreate(installWorker,&job,0x10000,0x30,-2,false);
    if(!thread){installing=false;aptSetSleepAllowed(true);aptSetHomeAllowed(true);installError((Result)-3);return;}
    while(!__atomic_load_n(&job.done,__ATOMIC_ACQUIRE)){
        active=flowMainLoop();hidScanInput();
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
            if(R_FAILED(job.seedResult)||R_FAILED(job.cleanupResult)){installReportCode=R_FAILED(job.seedResult)?job.seedResult:job.cleanupResult;installUnexpected=R_FAILED(job.cleanupResult)||(R_FAILED(job.seedResult)&&job.seedResult!=SEED_NOT_OFFERED);if(installUnexpected)rememberReport(ERROR_REPORT_INSTALL,installReportCode,ERROR_REPORT_INSTALLING);installFailed=true;errorTitle=tr("Installiert - Hinweis");}
            app_log("CIA installation completed and verified");
        }
        else installError(lastError);
    }
}
static bool startGame(void){
    recordActivity(ERROR_REPORT_STARTING_GAME);
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
    launchRepairComponent=-1;
    preparingGame=true;
    Preflight p={.game=g};
    Thread worker=threadCreate(preflightWorker,&p,0x10000,0x30,-2,false);
    if(!worker){p.result=(Result)-3;p.done=true;}
    bool active=true;
    do {active=flowMainLoop();
    if(!active)break;
    draw(true,0,__atomic_load_n(&p.done,__ATOMIC_ACQUIRE)?tr("Start wird vorbereitet"):g->system==SYSTEM_3DS&&!g->homebrew&&!g->cartridge?(__atomic_load_n(&p.seedPhase,__ATOMIC_ACQUIRE)?tr("Spiel-Seed wird repariert..."):tr("Spiel-Seed wird geprueft...")):tr("Spiel und Emulator pruefen"));
    }
    while(!__atomic_load_n(&p.done,__ATOMIC_ACQUIRE));
    if(worker){threadJoin(worker,U64_MAX);
    threadFree(worker);}lastError=p.result;
    if(active&&R_SUCCEEDED(lastError)){draw(true,1,tr("Uebergabe an Spiel / Emulator"));
    lastError=launch(g);
    if(R_SUCCEEDED(lastError))return true;}
    preparingGame=false;
    if(uiReleased)return true;
    if(R_FAILED(lastError)){rememberReport(ERROR_REPORT_LAUNCH,lastError,ERROR_REPORT_STARTING_GAME);menu_music_reload();snprintf(message,sizeof(message),tr("Startfehler %08lX%s"),(unsigned long)(u32)lastError,lastError==(Result)-30?tr(" - DS-Loader fehlt"):"");
    if(lastError==(Result)-107)snprintf(message,sizeof(message),"%s",tr("HOME-Menü nicht bereit. HOME öffnen und Spiel erneut starten."));
    if(lastError==FLOW_ERROR_HBL_CONFIG||lastError==FLOW_ERROR_HBL_MISSING)
        snprintf(message,sizeof(message),"%s",l10n_english()?"Homebrew Launcher unavailable. Check Rosalina title in Luma settings.":"Homebrew Launcher nicht verfuegbar. Rosalina-Titel in Luma pruefen.");
    if(R_FAILED(p.result)&&g->system==SYSTEM_3DS&&!g->homebrew){
        if(p.seedPhase)snprintf(message,sizeof(message),tr("Spiel-Seed fehlt/ist ungueltig. WLAN pruefen, dann erneut oeffnen. Fehler %08lX"),(unsigned long)(u32)lastError);
        else snprintf(message,sizeof(message),tr("Spieldaten konnten nicht geprueft werden. Bitte erneut versuchen. Code %08lX"),(unsigned long)(u32)lastError);
    }
    status=message;
    launchFailed=true;errorTitle=p.seedPhase?tr("Spiel braucht Reparatur"):tr("Spielstart fehlgeschlagen");app_log(message);
    if(g->system!=SYSTEM_3DS&&g->system!=SYSTEM_DS){
        char diagnostic[800];
        snprintf(diagnostic,sizeof(diagnostic),"ROM launch failed: result=%08lX system=%d emulator=%s rom=%s",(unsigned long)(u32)lastError,g->system,backendPaths[g->system],g->path);
        app_log(diagnostic);
        launchRepairComponent=launch_repair_component(g->system,lastError);
        bool en=l10n_english();
        if(launchRepairComponent>=0){
            errorTitle=en?"Emulator setup required":"Emulator einrichten";
            snprintf(message,sizeof(message),lastError==(Result)-20?
                (en?"%s is missing. Set up the emulator to play this game.":"%s fehlt. Richte den Emulator ein, um dieses Spiel zu starten."):
                (en?"%s is not a valid emulator file. Reinstall it using emulator setup.":"%s ist keine gültige Emulator-Datei. Installiere den Emulator erneut."),backendNames[g->system]);
        }else{
            const char *why=lastError==(Result)-22?
                (en?"The game file is missing, incomplete or unreadable. Check it on the SD card.":"Die Spieldatei fehlt, ist unvollständig oder nicht lesbar. Prüfe sie auf der SD-Karte."):
                lastError==(Result)-24?
                (en?"The emulator file cannot be read. Check the SD card and try again.":"Die Emulator-Datei ist nicht lesbar. Prüfe die SD-Karte und versuche es erneut."):
                lastError==(Result)-23?
                (en?"The game path is too long. Move the game into a shorter folder path.":"Der Spielpfad ist zu lang. Verschiebe das Spiel in einen kürzeren Ordnerpfad."):
                (en?"The emulator could not be started. Restart the console and try again. Details are in the Flow log.":"Der Emulator konnte nicht gestartet werden. Starte die Konsole neu und versuche es erneut. Details stehen im Flow-Log.");
            snprintf(message,sizeof(message),"%s",why);
        }
    }
    if(lastError==(Result)-30)setupDs=true;
    if(g->system==SYSTEM_DS){
        const char *why=lastError==(Result)-30?tr("DS-Loader fehlt; A zum Einrichten"):
            lastError==(Result)-31?tr("DS-ROM fehlt oder ist unvollständig"):
            lastError==(Result)-32?tr("DS-Loader-Dateien fehlen auf der SD"):
            lastError==(Result)-33?tr("DS-Dateipfad ist zu lang"):
            lastError==(Result)-34?tr("DS-Startdatei konnte nicht gespeichert werden"):
            lastError==(Result)-36?tr("DS-Starteinstellungen konnten nicht gespeichert werden"):
            lastError==(Result)-105?tr("DS-Systemstart hat nicht geantwortet"):
            tr("DS-Übergabe fehlgeschlagen; erneut versuchen");
        snprintf(message,sizeof(message),"%s (%08lX)",why,(unsigned long)(u32)lastError);status=message;app_handoff_log(message);
        if(lastError==(Result)-30)launchFailed=false;
    }}
    return !active;
}
static void openInfo(void){
 if(!visibleCount)return;
 Game *g=&games[visible[selected]];
 game_info_begin(&gameInfo,g->name);
 game_info_add(&gameInfo,"System",library_system_names[g->system]);
 if(g->cartridge){
  game_info_add(&gameInfo,tr("Quelle"),tr("Eingestecktes Spielmodul"));
  game_info_add(&gameInfo,tr("Modul kopieren"),tr("X: GodMode9-Assistent öffnen"));
  game_info_add(&gameInfo,tr("Spielstand"),tr("Modul-Save bleibt auf dem Modul"));
  game_info_add(&gameInfo,tr("Titel-ID"),g->key);
 }else if(g->pending){
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
 game_info_add(&gameInfo,tr("Menümusik"),tr(menu_music_status()));
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
    reportStartedAt=osGetTime();recordActivity(ERROR_REPORT_BROWSING);
    forwarder_restore(argc,argv);
    mkdir("sdmc:/3ds",0777);mkdir("sdmc:/3ds/3ds-library",0777);
    app_log_begin();
    osSetSpeedupEnable(true);
    if(!ui_init())return 1;
    themeSource=ui_theme_source();
    aptHookCookie hook;
    aptHook(&hook,onApt,NULL);
    hidSetRepeatParameters(18,5);
    if(R_SUCCEEDED(cfguInit())){u8 lang;
    if(R_SUCCEEDED(CFGU_GetSystemLanguage(&lang)))language=lang;
    cfguExit();}
    l10n_load(language!=2);language=l10n_english()?1:2;
    amReady=R_SUCCEEDED(amInit());
    {bool registered=false;NS_APPID menu=aptGetMenuAppID();Result r=APT_IsRegistered(menu,&registered);char entry[100];snprintf(entry,sizeof(entry),"Session: homebrew=%u HOME=%u registered=%u result=%08lX",envIsHomebrew(),(unsigned)menu,registered,(unsigned long)(u32)r);app_handoff_log(entry);homeUnavailable=R_SUCCEEDED(r)&&!registered;if(homeUnavailable)aptSetHomeAllowed(false);}
    mkdir("sdmc:/3ds/3ds-library/import",0777);
    /* appearance.txt only held the removed launch-animation toggle. Leave
       legacy preferences untouched; language and view have separate files. */
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
    CopyResult copied=copy_result_take("sdmc:/3ds/3ds-library/copy-result.txt");
    if(copied==COPY_DS_READY){systemView=VIEW_DS;selected=0;status=tr("DS-Kopie geprüft; in der DS-Kategorie bereit");}
    else if(copied==COPY_CIA_READY){systemView=VIEW_3DS;selected=0;status=tr("CIA geprüft; grauen Titel zum Installieren wählen");}
    else if(copied==COPY_INCOMPLETE){rememberReport(ERROR_REPORT_STORAGE,0,ERROR_REPORT_IMPORTING);status=tr("Modulkopie nicht bestätigt; Kopierassistent erneut öffnen");}
    if(copied!=COPY_NONE)app_log(status);
    rebuild();
    flow_reset(&flow,selected);
    int homeResume=autoboot_home_resume();
    if(homeResume<0){rememberReport(ERROR_REPORT_HOME,homeResume,ERROR_REPORT_OPENING_HOME);status=tr("Autostart konnte nicht fortgesetzt werden; Einstellungen prüfen");}
    else if(homeResume>0){status=tr("Autostart wieder aktiv; HOME bleibt in dieser Sitzung erreichbar");app_log("Autoboot resumed after HOME session without reboot");}
    musicReady=true;menu_music_reload();
    if(twl_repair_pending()){
        if(R_SUCCEEDED(twl_repair_finish())){status=tr("DS-Reparatur abgeschlossen");app_log("TWL recovery rechecked and acknowledged; no NAND change");}
        else openTwl();
    }
    do {draw(false,0,"");
    hidScanInput();}while(hidKeysHeld()&&flowMainLoop());
    bool leave=false;
    while(!leave&&flowMainLoop()){
        unsigned previousSelection=selected,previousView=systemView,previousMenu=menuSelected,previousPage=settingsPage;bool previousSettings=settings;
        hidScanInput();
        u32 keys=hidKeysDown()|(hidKeysDownRepeat()&(KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT));
        touchPosition pos={0};
        hidTouchRead(&pos);
        if(restored){restored=false;
        confirm=false;
        keys&=~KEY_A;
        flow_reset(&flow,selected);}
        if(keys&KEY_START&&!reportOpen&&!searchOpen&&!updatePage&&!componentPage&&!copyPrompt&&!powerPrompt&&!twlPage){exitToHomebrew=true;break;}
        if(!reportOpen&&!updatePage&&!componentPage&&!twlPage&&!settings&&!infoOpen&&!installPrompt&&!powerPrompt&&!copyPrompt&&osGetTime()-lastCardPoll>=2000){
            lastCardPoll=osGetTime();Cartridge card;Result r=cartridge_read(&card,language);
            if(R_SUCCEEDED(r)&&(card.present!=lastCartridge.present||(card.present&&(card.id!=lastCartridge.id||card.ds!=lastCartridge.ds||strcmp(card.code,lastCartridge.code))))){
                app_log(card.present?"Cartridge inserted/changed":"Cartridge removed");unsigned old=selected;scan();selected=old;rebuild();flow_reset(&flow,selected);
            }
        }
        if(powerPrompt&&reportOpen){report_transport_stop();reportOpen=false;memset(reportDescription,0,sizeof(reportDescription));}
        bool reportButton=(keys&KEY_TOUCH)&&report_controls_hit(pos.px,pos.py);
        if(!reportOpen&&reportFallback()&&reportButton){openReport();keys=0;}
        else if(keys&&!reportOpen)recentUnexpected=false;
        if(reportOpen){
            ReportSendState sending=report_transport_state();
            if((keys&KEY_Y)&&sending!=REPORT_SENDING){reportDetails=!reportDetails;reportConfirmed=false;keys=0;}
            bool back=(keys&KEY_B)||((keys&KEY_TOUCH)&&pos.py>=205&&pos.py<235&&pos.px>=12&&pos.px<104);
            bool primary=(keys&KEY_A)||((keys&KEY_TOUCH)&&pos.py>=205&&pos.py<235&&pos.px>=112&&pos.px<308);
            bool edit=!reportDetails&&((keys&KEY_X)||((keys&KEY_TOUCH)&&pos.py>=138&&pos.py<169&&pos.px>=12&&pos.px<308));
            switch(report_controls_action(sending,report_transport_configured(),reportConfirmed,back,primary,edit)){
                case REPORT_ACTION_CANCEL:report_transport_cancel();break;
                case REPORT_ACTION_CLOSE:reportOpen=false;memset(reportDescription,0,sizeof(reportDescription));break;
                case REPORT_ACTION_EDIT:describeReport();break;
                case REPORT_ACTION_REVIEW:reportConfirmed=true;break;
                case REPORT_ACTION_SEND:reportDetails=false;components_stop();app_update_stop();report_transport_start(&report,true);reportConfirmed=false;break;
                default:break;
            }
        }else if(powerPrompt){
            if(keys&KEY_TOUCH){
                if(pos.px>=12&&pos.px<308){
                    if(pos.py>=54&&pos.py<85)keys|=KEY_A;
                    else if(pos.py>=91&&pos.py<122)keys|=KEY_X;
                    else if(pos.py>=128&&pos.py<159)keys|=KEY_Y;
                    else if(pos.py>=165&&pos.py<196)keys|=KEY_SELECT;
                    else if(pos.py>=205&&pos.py<235)keys|=KEY_B;
                }
            }
            if(keys&KEY_B){
                Result cancelled=flow_apt_cancel_home_request();
                char entry[100];snprintf(entry,sizeof(entry),"Power cancel: OS ClearPowerButtonState=%08lX",(unsigned long)(u32)cancelled);app_log(entry);
                if(R_SUCCEEDED(cancelled)){
                    powerCancelFailed=false;powerPrompt=false;ui_runtime_allow_pending_home(false);ui_runtime_reset();
                    flow_reset(&flow,selected);status=tr("Bereit");
                    menu_music_pause(false);app_log("Power dialog cancelled; OS and local HOME/POWER cleared");
                }else {powerCancelFailed=true;rememberReport(ERROR_REPORT_POWER,cancelled,ERROR_REPORT_POWER_MENU);}
            }else if(keys&KEY_A){openHomeMenu();if(uiReleased)leave=true;}
            else if(keys&(KEY_X|KEY_SELECT)){
                systemPowerAction((keys&KEY_X)!=0);
                status=tr("Systemanfrage fehlgeschlagen; erneut versuchen");if(uiReleased)leave=true;
            }else if(keys&KEY_Y){powerPrompt=false;exitToHomebrew=true;leave=true;}
        }else if(componentPage){
            ComponentStatus cs=components_status();bool busy=cs.state==COMP_DOWNLOAD||cs.state==COMP_READY;
            if(cs.state==COMP_READY){bool home=aptIsHomeAllowed(),sleep=aptIsSleepAllowed();installing=true;aptSetHomeAllowed(false);aptSetSleepAllowed(false);draw(true,0,tr("Emulator wird eingerichtet"));components_commit();installing=false;aptSetHomeAllowed(home);aptSetSleepAllowed(sleep);}
            else if(keys&KEY_B || ((keys&KEY_TOUCH)&&((pos.py<44&&pos.px>=276)||(busy&&pos.py>=192)||(componentSelected==6&&pos.py>=201&&pos.py<231&&pos.px<94)))){if(busy)components_cancel();else componentPage=false;}
            else if(!busy){
                if(keys&(KEY_L|KEY_R|KEY_UP|KEY_DOWN|KEY_TOUCH))components_clear();
                bool footer=(keys&KEY_TOUCH)&&pos.py>=201&&pos.py<231;
                bool recheck=(keys&KEY_X)||(footer&&(componentSelected==6?(pos.px>=104&&pos.px<186):(pos.px>=68&&pos.px<184)));
                if(recheck){components_clear();if(components_recheck()){componentCheckedUntil=osGetTime()+1500;menu_music_click();}}
                if(!recheck){
                if(!recheck&&componentSelected!=6&&((keys&KEY_L)||(footer&&pos.px>=12&&pos.px<60)))componentSelected=componentSelected<3?3:0;
                if(componentSelected!=6&&(keys&KEY_R))componentSelected=componentSelected<3?3:0;
                if(componentSelected!=6&&(keys&KEY_UP))componentSelected=componentSelected?componentSelected-1:5;
                if(componentSelected!=6&&(keys&KEY_DOWN))componentSelected=(componentSelected+1)%6;
                if(componentSelected!=6&&(keys&KEY_TOUCH)&&pos.py>=54&&pos.py<174){unsigned row=(pos.py-54)/40;componentSelected=(componentSelected/3)*3+row;}
                if(!recheck&&components_availability(componentSelected)!=COMP_UNCHECKED&&((keys&KEY_A)||(footer&&pos.px>=(componentSelected==6?196:192)&&pos.px<308))){recordActivity(ERROR_REPORT_DOWNLOADING);app_update_stop();components_start(componentSelected);}
                }
            }
        }else if(updatePage){
            UpdateStatus update=app_update_status();
            if(update.state==UPDATE_READY){
                draw(true,0,tr("Update wird installiert"));app_update_install();
            }else{
                bool busy=update.state==UPDATE_CHECKING||update.state==UPDATE_DOWNLOADING;
                bool back=(keys&KEY_B)||((keys&KEY_TOUCH)&&pos.py>=192&&pos.py<230&&pos.px<151)||((keys&KEY_TOUCH)&&pos.py>=8&&pos.py<44&&pos.px>=276);
                bool primary=(keys&KEY_A)||((keys&KEY_TOUCH)&&pos.py>=192&&pos.py<230&&pos.px>=169);
                bool notes=!busy&&settings_release_note_count("0.12.37")&&((keys&KEY_X)||((keys&KEY_TOUCH)&&settings_update_notes_hit(pos.px,pos.py)));
                if(back){updateNotes=false;if(busy)app_update_cancel();else updatePage=false;}
                else if(notes)updateNotes=!updateNotes;
                else if(primary&&!busy){
                    updateNotes=false;
                    if(update.state==UPDATE_AVAILABLE){recordActivity(ERROR_REPORT_DOWNLOADING);app_update_download();}
                    else if(update.state==UPDATE_INSTALLED){
                        Result r=homebrew_prepare_launch(UPDATE_APP);
                        if(R_SUCCEEDED(r)){saveView();leave=true;}
                        else {rememberReport(ERROR_REPORT_UPDATE,r,ERROR_REPORT_CHECKING_UPDATE);status=tr("Neu öffnen fehlgeschlagen. Über HBL starten.");}
                    }else {recordActivity(ERROR_REPORT_CHECKING_UPDATE);app_update_check();}
                }
            }
        }else if(twlPage){
            bool closeTwl=false;
            if(keys&KEY_TOUCH){
                if(pos.px>=276&&pos.px<312&&pos.py>=8&&pos.py<44){keys|=KEY_B;closeTwl=true;}
                if(pos.py>=201&&pos.py<231){
                    if((twlPage==1&&twl_recommendation(&twlReport)>=TWL_CHECK_UPDATE)||twlPage==4){
                        if(pos.px>=12&&pos.px<94)keys|=KEY_B;
                        else if(pos.px>=104&&pos.px<186)keys|=KEY_X;
                        else if(pos.px>=196&&pos.px<308)keys|=KEY_A;
                    }else{
                        if(pos.px>=12&&pos.px<112)keys|=KEY_B;
                        /* NAND changes require physical START + A, never Touch. */
                        else if(twlPage!=3&&pos.px>=120&&pos.px<308)keys|=KEY_A;
                    }
                }
            }
            if(keys&KEY_B){
                if(closeTwl)twlPage=0;
                else if(twlPage==3)twlPage=2;
                else if(twlPage==2||twlPage==6)twlPage=1;
                else twlPage=0;
                twlReport.confirmedAllFail=twlReport.confirmedDownloadPlayFail=false;
            }else if(keys&KEY_X){
                twl_diagnose(&twlReport);twlError=twlReport.error;
                twlPage=twl_repair_pending()?(twlReport.known&&!twlReport.missing?5:4):1;
            }else if((keys&(KEY_A|KEY_Y))&&(twlPage==6||(twlPage==4&&!twlNeedsReboot))){
                u64 id=twl_settings_title(&twlReport);
                if(id){Game target={0};target.id=id;target.media=MEDIATYPE_NAND;target.system=SYSTEM_3DS;saveView();lastError=launch(&target);if(uiReleased)leave=true;}
                else lastError=(Result)-121;
                if(R_FAILED(lastError)){menu_music_reload();twlError=lastError;snprintf(message,sizeof(message),tr("Einstellungen nicht geöffnet: %08lX"),(unsigned long)(u32)lastError);status=message;app_log(message);}
            }else if(keys&KEY_A){
                if(twlPage==1){
                    TwlAdvice advice=twl_recommendation(&twlReport);
                    if(advice==TWL_CHECK_UNKNOWN){twl_diagnose(&twlReport);twlError=twlReport.error;}
                    else if(advice==TWL_CHECK_UNSUPPORTED){twl_diagnose(&twlReport);twlError=twlReport.error;}
                    else if(advice==TWL_CHECK_UPDATE)twlPage=6;
                    else if(advice==TWL_CHECK_LOADER){components_clear();components_recheck();componentSelected=1;componentPage=true;twlPage=0;}
                    else twlPage=2;
                }else if(twlPage==2){
                    /* Revalidate before accepting the user's report that ALL DS
                       games and HOME's DS Download Play fail outside Flow. */
                    twl_diagnose(&twlReport);twlError=twlReport.error;
                    if(twl_repair_pending())twlPage=4;
                    else if(twl_recommendation(&twlReport)==TWL_CHECK_CLEAR){
                        twlReport.confirmedAllFail=twlReport.confirmedDownloadPlayFail=true;twlPage=3;
                    }else twlPage=1;
                }else if(twlPage==3&&!(keys&KEY_TOUCH)&&(hidKeysHeld()&KEY_START)&&twl_recommendation(&twlReport)==TWL_CHECK_REPAIR){
                    lastError=performTwlRepair();twlError=lastError;twl_diagnose(&twlReport);
                    if(twl_repair_pending()){twlNeedsReboot=true;twlPage=4;status=tr("Reparatur begonnen; Systemupdate erforderlich");}
                    else {twlPage=1;status=tr("Reparatur nicht gestartet; SD und Diagnose prüfen");}
                    if(R_FAILED(lastError))app_log("TWL repair incomplete: inspect session log; do not repeat deletion");
                }else if(twlPage==4){saveView();restartAfterBootChange();if(uiReleased)leave=true;else status=tr("Neustart fehlgeschlagen; Konsole manuell neu starten");}
                else if(twlPage==5){lastError=twl_repair_finish();if(R_SUCCEEDED(lastError)){twlPage=0;status=tr("DS-Reparatur abgeschlossen");}else {twlPage=4;twlError=lastError;twl_diagnose(&twlReport);}}
            }
        }else if(copyPrompt){
            if(keys&KEY_B)copyPrompt=false;
            else if(keys&KEY_A){saveView();restartAfterBootChange();if(uiReleased)leave=true;else {status=tr("Neustart fehlgeschlagen; START beim Einschalten halten");copyPrompt=false;}}
        }else if(installFailed||launchFailed){
            if((keys&KEY_X)&&launchFailed&&visibleCount&&games[visible[selected]].system==SYSTEM_DS){installFailed=launchFailed=false;setupDs=false;openTwl();}
            else if(launchFailed&&launchRepairComponent>=0&&!(keys&KEY_B)&&
                ((keys&KEY_A)||((keys&KEY_TOUCH)&&pos.py>=198&&pos.py<229&&pos.px>=100))){
                componentSelected=(unsigned)launchRepairComponent;
                launchRepairComponent=-1;installFailed=launchFailed=false;
                components_clear();componentPage=true;
            }
            else if(keys&(KEY_A|KEY_B)){installFailed=launchFailed=false;launchRepairComponent=-1;}
            else if((keys&KEY_TOUCH)&&pos.py>=198&&pos.py<229){installFailed=launchFailed=false;launchRepairComponent=-1;}
        }else if(installPrompt){
            if(keys&KEY_B)installPrompt=false;
            else if(keys&KEY_A)performInstall();
            else if((keys&KEY_TOUCH)&&pos.py>=188&&pos.py<229){if(pos.px<160)installPrompt=false;else performInstall();}
        }else if(setupDs){
            if(keys&KEY_B)setupDs=false;
            else if(keys&KEY_A){setupDs=false;
                componentSelected=1;componentPage=true;}
        }else if(infoOpen){
            if((keys&KEY_X)&&visibleCount&&games[visible[selected]].cartridge){
                FILE *script=fopen("sdmc:/gm9/scripts/VitarenewFlow-Copy.gm9","rb");
                FILE *firm=fopen("sdmc:/luma/payloads/GodMode9.firm","rb");
                if(script&&firm){infoOpen=false;copyPrompt=true;}
                else {snprintf(message,sizeof(message),"%s",tr("GodMode9 oder Kopierskript fehlt"));status=message;installReportCode=0;installUnexpected=true;rememberReport(ERROR_REPORT_STORAGE,0,ERROR_REPORT_IMPORTING);installFailed=true;errorTitle=tr("Modul kopieren");}
                if(script)fclose(script);
                if(firm)fclose(firm);
            }
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
        }else if(autobootPrompt){
            if((keys&KEY_B)||((keys&KEY_TOUCH)&&pos.py>=184&&pos.py<214&&pos.px<154))autobootPrompt=false;
            else if((keys&KEY_A)||((keys&KEY_TOUCH)&&pos.py>=184&&pos.py<214&&pos.px>=166)){
                bool enable=autobootState==0;
                u64 target=0;Result check=enable?native_handoff_target(&target):0;
                draw(true,0,tr("Autostart wird eingerichtet"));
                bool ok=R_SUCCEEDED(check)&&autoboot_set(enable);
                app_log(ok?(enable?"Autoboot enabled and verified":"Autoboot disabled and restored"):"Autoboot preference failed");
                autobootPrompt=false;autobootState=autoboot_status();
                if(ok){
                    saveView();status=tr("Autostart gespeichert; gilt ab nächstem Einschalten");
                }
                else {snprintf(message,sizeof(message),"%s: %.100s",tr("Autostart fehlgeschlagen"),R_FAILED(check)?"Rosalina target unavailable":autoboot_error());status=message;installReportCode=0;installUnexpected=true;rememberReport(ERROR_REPORT_HOME,R_FAILED(check)?check:0,ERROR_REPORT_OPENING_HOME);installFailed=true;errorTitle=tr("Autostart");}
            }
        }else if(supportOpen){
            bool reportHit=(keys&KEY_TOUCH)&&report_controls_support_hit(pos.px,pos.py);
            if(reportAvailable()&&((keys&KEY_X)||reportHit))openReport();
            else if((keys&(KEY_A|KEY_B))||((keys&KEY_TOUCH)&&pos.py>=205&&pos.py<235))supportOpen=false;
        }else if(settings){
            bool closeMenu=(keys&KEY_TOUCH)&&pos.px>=276&&pos.px<312&&pos.py>=8&&pos.py<44;
            bool back=(keys&KEY_B)||((keys&KEY_TOUCH)&&pos.px>=8&&pos.px<44&&pos.py>=8&&pos.py<44&&settingsPage!=MENU_ROOT);
            if(closeMenu){settings=false;}
            else if(back){
                if(settingsPage!=MENU_ROOT){menuSelected=settings_menu_parent_row(settingsPage);settingsPage=MENU_ROOT;}
                else settings=false;
            }else{
                unsigned rows=settings_menu_count(settingsPage);
                if(keys&KEY_UP)menuSelected=(menuSelected+rows-1)%rows;
                if(keys&KEY_DOWN)menuSelected=(menuSelected+1)%rows;
                int hit=(keys&KEY_TOUCH)?settings_menu_hit(pos.px,pos.py,settingsPage):-1;
                if(hit>=0){menuSelected=(unsigned)hit;keys|=KEY_A;}
                const MenuItem *item=settings_menu_item(settingsPage,menuSelected);
                if((keys&KEY_A)&&item){
                    switch(item->action){
                    case MENU_PERSONAL_OPEN:settingsPage=MENU_PERSONAL;menuSelected=0;break;
                    case MENU_GAMES_OPEN:settingsPage=MENU_GAMES;menuSelected=0;break;
                    case MENU_SYSTEM_OPEN:settingsPage=MENU_SYSTEM;menuSelected=0;break;
                    case MENU_SUPPORT:supportOpen=true;break;
                    case MENU_DESKTOP:components_clear();components_recheck();componentCheckedUntil=0;componentPage=true;componentSelected=6;break;
                    case MENU_COMPONENTS:recordActivity(ERROR_REPORT_CHECKING_COMPONENTS);components_clear();components_recheck();componentCheckedUntil=0;componentPage=true;componentSelected=0;break;
                    case MENU_UPDATE:
                        if(envIsHomebrew()){recordActivity(ERROR_REPORT_CHECKING_UPDATE);components_stop();updateNotes=false;updatePage=true;app_update_check();}
                        else {status=tr("Updates bitte über den Homebrew Launcher öffnen.");}
                        break;
                    case MENU_HOME:openHomeMenu();if(uiReleased)leave=true;break;
                    case MENU_THEME:
                        recordActivity(ERROR_REPORT_LOADING_THEME);
                        draw(true,0,tr("Theme wird aktualisiert"));
                        themeSource=ui_reload_theme();menu_music_reload();status=tr("Theme aktualisiert");break;
                    case MENU_LANGUAGE:{
                        bool saved=l10n_save(!l10n_english());app_log(saved?"Language saved":"Language save failed");
                        if(saved){status=tr("Sprache gespeichert");language=l10n_english()?1:2;}
                        else {snprintf(message,sizeof(message),"%s",tr("Sprache konnte nicht gespeichert werden"));status=message;installReportCode=0;installUnexpected=true;rememberReport(ERROR_REPORT_STORAGE,0,ERROR_REPORT_BROWSING);installFailed=true;errorTitle=tr("Einstellungen");}
                        break;}
                    case MENU_DS:openTwl();break;
                    case MENU_AUTOBOOT:autobootState=autoboot_status();autobootPrompt=true;break;
                    case MENU_SCAN:{recordActivity(ERROR_REPORT_BROWSING);
                        draw(true,0,tr("Spiele werden eingelesen"));unsigned old=selected;
                        scan();selected=old;rebuild();flow_reset(&flow,selected);status=tr("Spieleliste aktualisiert");break;}
                    default:break;
                    }
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
            if(keys&KEY_A){recordActivity(ERROR_REPORT_LOADING_COVER);bool saved=assignCover();if(!saved)rememberReport(ERROR_REPORT_COVER,0,ERROR_REPORT_LOADING_COVER);status=saved?tr("Cover gespeichert"):tr("Cover nicht lesbar: PNG bis 512x512 verwenden");
            coverMenu=false;}
        }else if(searchOpen){
            bool changed=false;
            if((keys&KEY_B)||((keys&KEY_TOUCH)&&pos.py>=214&&pos.py<238&&pos.px>=8&&pos.px<78)){
                searchOpen=false;systemView=searchReturnView;selected=searchReturn;rebuild();flow_reset(&flow,selected);
            }else{
                if((keys&KEY_UP)&&visibleCount)selected=(selected+visibleCount-1)%visibleCount;
                if((keys&KEY_DOWN)&&visibleCount)selected=(selected+1)%visibleCount;
                if(keys&KEY_X)changed=search_edit(searchQuery,'\b');
                if((keys&KEY_Y)&&*searchQuery){searchQuery[0]=0;changed=true;}
                if((keys&KEY_TOUCH)&&pos.py>=8&&pos.py<44&&pos.px>=8&&pos.px<312){
                    unsigned slot=(pos.px-8)/103;
                    if(slot<3&&(pos.px-8)%103<98)changed=search_accept_suggestion(searchQuery,searchSuggestions[slot]);
                }
                if((keys&KEY_TOUCH)&&pos.py>=58&&pos.py<170)changed=search_edit(searchQuery,search_key_at(pos.px,pos.py,searchNumeric));
                if((keys&KEY_TOUCH)&&pos.py>=177&&pos.py<209){
                    if(pos.px>=8&&pos.px<60)searchNumeric=!searchNumeric;
                    else if(pos.px>=64&&pos.px<224)changed=search_edit(searchQuery,' ');
                    else if(pos.px>=228&&pos.px<312)changed=search_edit(searchQuery,'\b');
                }
                if(changed){selected=0;rebuild();flow_reset(&flow,selected);menu_music_click();}
                bool open=(keys&KEY_A)!=0;
                if((keys&KEY_TOUCH)&&pos.py>=214&&pos.py<238){
                    if(pos.px>=164&&pos.px<312)open=true;
                    else if(visibleCount&&pos.px>=82&&pos.px<116)selected=(selected+visibleCount-1)%visibleCount;
                    else if(visibleCount&&pos.px>=120&&pos.px<154)selected=(selected+1)%visibleCount;
                }
                flow_reset(&flow,selected);
                if(open&&visibleCount)leave=startGame();
            }
        }else if(confirm){
            if(keys&KEY_B)confirm=false;
            if((keys&KEY_TOUCH)&&pos.py>=143&&pos.py<188)keys|=pos.px<166?KEY_A:KEY_B;
            if(keys&KEY_B)confirm=false;
            else if((keys&KEY_A)&&visibleCount)leave=startGame();
        }else{
            int footerAction=(keys&KEY_TOUCH)?home_footer_action(pos.px,pos.py,false,visibleCount!=0):HOME_NONE;
            int headerAction=(keys&KEY_TOUCH)?home_header_action(pos.px,pos.py):HOME_NONE;
            if(headerAction==HOME_SEARCH){
                searchReturn=selected;searchReturnView=systemView;searchOpen=true;searchQuery[0]=0;searchNumeric=false;
                selected=0;status=tr("Bereit");rebuild();flow_reset(&flow,selected);
            }
            else if(keys&(KEY_L|KEY_R))switchSystem(home_category_target(systemView,VIEW_COUNT,(keys&KEY_L)!=0,(keys&KEY_R)!=0));
            else if((keys&KEY_X)&&visibleCount)openInfo();
            else if(footerAction==HOME_GUIDE){controlsPage=true;
            flow_reset(&flow,selected);}
            else if(footerAction==HOME_OPEN)leave=startGame();
            else if(headerAction==HOME_PREVIOUS||headerAction==HOME_NEXT)switchSystem(home_category_target(systemView,VIEW_COUNT,headerAction==HOME_PREVIOUS,headerAction==HOME_NEXT));
            else if((keys&KEY_Y)||headerAction==HOME_CATEGORIES){systemPicker=true;
            menuSelected=systemView;
            flow_reset(&flow,selected);}
            else if(headerAction==HOME_SETTINGS){settings=true;settingsPage=MENU_ROOT;autobootState=autoboot_status();
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
        if(!leave&&((visibleCount&&selected!=previousSelection)||systemView!=previousView||settings!=previousSettings||(settings&&(menuSelected!=previousMenu||settingsPage!=previousPage))))menu_music_click();
        if(!leave)draw(false,0,"");
    }
    saveView();
    if(exitToHomebrew&&envIsHomebrew()&&autoboot_status()==1){
        Result r=homebrew_prepare_launch("sdmc:/3ds/3ds-library/autoboot-hbmenu.3dsx");
        app_log(R_SUCCEEDED(r)?"Exit: original Homebrew Launcher":"Exit: Homebrew Launcher preparation failed");
        if(R_FAILED(r))aptClearChainloader();
    }
    report_transport_stop();
    components_stop();
    app_update_stop();
    menu_music_stop();
    app_log("Shutdown: drain GPU without VBlank wait");
    if(amReady)amExit();
    aptUnhook(&hook);
    if(!uiReleased)ui_shutdown();
    for(unsigned i=0;i<gameCount;++i)free(games[i].icon);
    app_log("Shutdown complete");
    return 0;
}

