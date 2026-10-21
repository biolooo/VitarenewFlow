// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "ui_runtime.h"
#include <3ds.h>
#include <citro3d.h>
static u64 lastFrame;
static bool hasFrame;
static bool allowPendingHome;
void ui_runtime_allow_pending_home(bool allowed){allowPendingHome=allowed;}
void ui_runtime_reset(void){lastFrame=0;hasFrame=false;}
bool ui_runtime_begin(void){
    const u64 period=SYSCLOCK_ARM11/60,limit=SYSCLOCK_ARM11/10;
    u64 start=svcGetSystemTick();
    for(;;){
        if(!aptIsActive()||aptShouldClose()||(!allowPendingHome&&aptShouldJumpToHome()))return false;
        u64 now=svcGetSystemTick();
        if(now-start>=limit)return false;
        if(!hasFrame||now-lastFrame>=period){
            if(C3D_FrameBegin(C3D_FRAME_NONBLOCK)){lastFrame=now;hasFrame=true;
            return true;}
        }
        svcSleepThread(1000000);
    }
}
void ui_runtime_drain(void){
    /* flags=0 waits for queued GPU work, WITHOUT a VBlank wait. This remains
       usable after APT suspension has disabled Citro3D's VBlank callbacks. */
    if(C3D_FrameBegin(0))C3D_FrameEnd(0);
}
