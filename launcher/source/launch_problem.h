// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "library_model.h"
/* Only known emulator-file failures offer a download. ROM errors never do. */
static inline int launch_repair_component(int system, int result)
{
    if(result!=-20 && result!=-21)return -1;
    switch(system){
    case SYSTEM_GBA:case SYSTEM_GB:return 0;
    case SYSTEM_N64:return 2;
    case SYSTEM_SNES:return 3;
    case SYSTEM_NES:return 4;
    case SYSTEM_VB:return 5;
    default:return -1;
    }
}
