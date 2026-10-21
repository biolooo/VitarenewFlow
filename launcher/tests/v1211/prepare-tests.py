from pathlib import Path
base=Path('work/v0139/make-tests.py').read_text()
a=base.index("tail=r'''")
base=base[:a]
base=base.replace('static bool uiReleased,','static bool homeUnavailable;static unsigned readyAfter=3,readyPolls;static bool neverReady;static int readyQueryRC;static unsigned homeAllowedCalls;\nstatic bool uiReleased,')
base=base.replace('*out=home;return queryRC;', 'if(prepared){readyPolls++;if(readyQueryRC)return readyQueryRC;if(!neverReady&&readyPolls>=readyAfter)home=true;}*out=home;return queryRC;')
base=base.replace('static void menu_music_stop(void){}', 'static void menu_music_stop(void){}\nstatic void aptSetHomeAllowed(bool enabled){assert(enabled&&home);homeAllowedCalls++;}')
base=base.replace('assert(uiReleased&&stopped==1&&n==0x300);','assert(uiReleased&&stopped==1&&n==0x300&&home);')
base=base.replace('static bool newModel=true;static int modelRC;static unsigned modelQueries;static u8 expectedMem;', 'static u8 expectedMem;')
base=base.replace('Result APT_CheckNew3DS(bool *out){modelQueries++;*out=newModel;return modelRC;}\n','')
tail=r'''
static void reset(void){homeUnavailable=false;readyAfter=3;readyPolls=0;neverReady=false;readyQueryRC=0;homeAllowedCalls=0;expectedMem=0;metadataCalls=0;metadataSize=100;metadataMismatch=false;metadataRC=cardRC=0;uiReleased=false;home=hb=initOK=true;transfer=false;queryRC=prepareRC=jumpRC=nsRC=rebootRC=dsRC=0;stopped=started=prepared=jumped=rebooted=waited=exited=nsOpened=nsClosed=dsCalls=0;bootState=hbPrepRC=hbCalls=chainCalls=configRC=emulatorRC=0;}
static void accepted(Game *g,bool cold){
 home=!cold;homeUnavailable=cold;bool legacyCold=cold&&g->system==SYSTEM_DS;transfer=legacyCold;
 int outcome=setjmp(escape);if(!outcome){launch(g);assert(!"Accepted handoff must never return to main cleanup");}
 assert(stopped==1&&!started);
 if(legacyCold)assert(outcome==2&&rebooted==1&&!prepared&&!jumped&&!exited&&nsOpened==1&&nsClosed==1);
 else{assert(outcome==1&&prepared==1&&jumped==1&&exited==1&&!nsOpened&&!rebooted);if(g->system==SYSTEM_3DS){assert(homeAllowedCalls==1&&!homeUnavailable);if(cold)assert(readyPolls==3&&waited==2);}}
}
int main(void){
 Game g={.system=SYSTEM_DS};expectedId=DS_BOOTSTRAP_ID;expectedMedia=MEDIATYPE_NAND;
 reset();accepted(&g,false);assert(dsCalls==1);
 reset();accepted(&g,true);assert(dsCalls==1);
 reset();dsRC=-30;assert(launch(&g)==-30&&!stopped&&!prepared&&!nsOpened);
 reset();queryRC=-11;assert(launch(&g)==-11&&!stopped&&!prepared&&!nsOpened);
 reset();prepareRC=-12;assert(launch(&g)==-12&&!stopped&&!jumped);
 reset();jumpRC=-13;assert(launch(&g)==-13&&started==1&&!uiReleased&&!exited);
 reset();home=false;nsRC=-14;assert(launch(&g)==-14&&!stopped&&!rebooted);
 reset();home=false;rebootRC=-15;assert(launch(&g)==-15&&started==1&&!uiReleased&&!waited);
 reset();home=false;assert(launch(&g)==-105&&waited==100&&started==1&&!uiReleased&&!exited);
 g.system=SYSTEM_3DS;g.id=expectedId=0x000400000012BE00ULL;g.media=expectedMedia=MEDIATYPE_SD;
 reset();accepted(&g,false);reset();accepted(&g,true);
 reset();home=false;neverReady=true;assert(launch(&g)==-107&&waited==100&&readyPolls==101&&!stopped&&!jumped&&!rebooted&&!nsOpened&&!uiReleased);
 reset();home=false;readyQueryRC=-90;assert(launch(&g)==-107&&!stopped&&!jumped&&!rebooted&&!nsOpened);
 reset();home=false;prepareRC=-12;assert(launch(&g)==-107&&!stopped&&!jumped&&!rebooted);
 reset();prepareRC=-12;assert(launch(&g)==-12&&!stopped&&!jumped);
 reset();metadataSize=0;assert(launch(&g)==-2&&!stopped&&!prepared&&!nsOpened);
 reset();metadataMismatch=true;assert(launch(&g)==-2&&!stopped);
 reset();metadataRC=-21;assert(launch(&g)==-21&&!stopped);
 reset();home=false;readyAfter=101;accepted(&g,false); // handled separately below: current helper sets home=true
 reset();home=false;readyAfter=101;int late=setjmp(escape);if(!late){launch(&g);assert(0);}assert(late==1&&waited==100&&jumped==1&&homeAllowedCalls==1&&!rebooted);
 reset();home=false;jumpRC=-13;assert(launch(&g)==-13&&started==1&&!uiReleased&&!homeUnavailable&&!rebooted);
 reset();home=false;jumpRC=-13;initOK=false;assert(launch(&g)==-13&&uiReleased&&!rebooted);
 g.cartridge=true;card=(Cartridge){.present=true,.id=g.id};g.media=expectedMedia=MEDIATYPE_GAME_CARD;
 reset();metadataSize=0;accepted(&g,false);assert(!metadataCalls);reset();metadataSize=0;accepted(&g,true);assert(!metadataCalls);
 reset();home=false;neverReady=true;assert(launch(&g)==-107&&!stopped&&!jumped&&!rebooted);
 reset();metadataRC=-90;accepted(&g,false);assert(!metadataCalls);
 reset();card.id++;assert(launch(&g)==-103&&!stopped);card.id--;
 reset();card.present=false;assert(launch(&g)==-103&&!stopped);card.present=true;
 reset();cardRC=-10;assert(launch(&g)==-103&&!stopped);
 g.cartridge=false;g.media=expectedMedia=MEDIATYPE_SD;g.homebrew=true;
 reset();accepted(&g,false);reset();accepted(&g,true);
 reset();g.id=expectedId=0x000400000D921E00ULL;bootState=1;assert(!launch(&g)&&hbCalls==1&&!stopped);hb=false;assert(!launch(&g)&&chainCalls==1);g.homebrew=false;
 reset();g.cartridge=true;g.system=SYSTEM_DS;strcpy(g.key,"cart-ABCE");card=(Cartridge){.present=true,.ds=true};strcpy(card.code,"ABCE");expectedId=0;expectedMedia=MEDIATYPE_GAME_CARD;transfer=true;home=true;
 if(!setjmp(escape)){launch(&g);assert(0);}assert(rebooted==1&&!prepared&&!dsCalls);
 reset();card.present=false;assert(launch(&g)==-103&&!stopped);
 reset();g.cartridge=false;g.system=SYSTEM_GBA;assert(!launch(&g)&&!stopped&&!rebooted);hb=false;assert(!launch(&g)&&chainCalls==1);
 puts("PASS actual launch code: native coldboot prepares APT, waits for HOME enabled, never reboots native title without HOME; delayed/late readiness, timeout/query/prepare failure preserve UI; APT rejection recovery; card identity guards, installed size guards; DS/emulator regressions");
}
'''
tail=tail.replace(' reset();home=false;readyAfter=101;accepted(&g,false); // handled separately below: current helper sets home=true\n','')
s=Path('outputs/3ds-library/source/main.c').read_text()
launch=s[s.index('static Result launchTitle'):s.index('static void installProgress')]
namespace={}
exec(base,namespace)
Path('work/v0140/launch_test.c').write_text(namespace['prefix']+launch+tail)
