#include <3ds.h>
#include <citro3d.h>
#include "ui_runtime.h"
#include <assert.h>
#include <stdio.h>
static u64 ticks;static bool active=true,closing,power,busy;static unsigned begins,ends,breakMs;
bool aptIsActive(void){return active;}bool aptShouldClose(void){return closing;}bool aptShouldJumpToHome(void){return power;}
u64 svcGetSystemTick(void){return ticks;}
void svcSleepThread(s64 ns){assert(ns<=1000000);ticks+=SYSCLOCK_ARM11/1000;if(breakMs && ticks>=SYSCLOCK_ARM11/1000*breakMs)power=true;}
bool C3D_FrameBegin(unsigned flags){assert(flags==0||flags==C3D_FRAME_NONBLOCK);++begins;if(!flags)busy=false;return !busy;}
void C3D_FrameEnd(unsigned flags){assert(flags==0);++ends;}
int main(void){
 ui_runtime_reset();assert(ui_runtime_begin());C3D_FrameEnd(0);assert(ui_runtime_begin());assert(ticks>=SYSCLOCK_ARM11/60);C3D_FrameEnd(0);
 unsigned old=begins;power=true;assert(!ui_runtime_begin()&&begins==old);power=false;closing=true;assert(!ui_runtime_begin());closing=false;active=false;assert(!ui_runtime_begin());
 busy=true;ui_runtime_drain();assert(!busy&&ends==3); // No VBlank exists while suspended.
 active=true;ui_runtime_reset();busy=true;ticks=0;assert(!ui_runtime_begin()&&ticks<SYSCLOCK_ARM11/9);busy=false;
 ticks=0;ui_runtime_reset();assert(ui_runtime_begin());breakMs=5;assert(!ui_runtime_begin()&&power&&ticks<SYSCLOCK_ARM11/100);
 ui_runtime_allow_pending_home(true);breakMs=0;busy=false;active=true;closing=false;ui_runtime_reset();assert(ui_runtime_begin());closing=true;assert(!ui_runtime_begin());closing=false;active=false;assert(!ui_runtime_begin());active=true;ui_runtime_allow_pending_home(false);assert(!ui_runtime_begin());
 puts("PASS exact frame runtime: paced frames, bounded GPU wait, POWER/HOME interruption, close/inactive guards, suspended shutdown without VBlank");
}

