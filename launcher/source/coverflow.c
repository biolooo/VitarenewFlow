// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "coverflow.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
static float clamp(float v,float max){return v<0?0:v>max?max:v;}
void flow_reset(CoverFlow *f,unsigned selected){memset(f,0,sizeof(*f));f->position=selected;}
int flow_update(CoverFlow *f,unsigned count,unsigned *selected,bool held,int x,int y){
    if(!count){flow_reset(f,0);return FLOW_NONE;}
    int action=FLOW_NONE;
    if(held && !f->touching){if(y<48||y>158)return action;f->touching=true;f->dragging=false;f->startX=f->lastX=x;f->frames=0;f->velocity=0;}
    if(held && f->touching){
        if(abs(x-f->startX)>7)f->dragging=true;
        if(f->dragging){float delta=(f->lastX-x)/96.f;f->position=clamp(f->position+delta,count-1);f->velocity=f->velocity*.4f+delta*.6f;*selected=(unsigned)lroundf(f->position);}
        f->lastX=x;++f->frames;
    }
    if(!held && f->touching){
        f->touching=false;
        if(!f->dragging){
            int hit=(int)lroundf(f->position+(f->lastX-160)/96.f);
            if(hit>=0&&(unsigned)hit<count){if((unsigned)hit==*selected && abs(f->lastX-160)<50)action=f->frames>=35?FLOW_COVER:FLOW_START;else {*selected=(unsigned)hit;action=FLOW_SELECT;}}
            f->velocity=0;
        }
    }
    return action;
}
void flow_step(CoverFlow *f,unsigned count,unsigned selected){
    if(!count){flow_reset(f,0);return;}if(f->touching)return;
    if(fabsf(f->velocity)>.012f){f->position=clamp(f->position+f->velocity,count-1);f->velocity*=.83f;}
    else {f->velocity=0;f->position+=(selected-f->position)*.24f;if(fabsf(selected-f->position)<.002f)f->position=selected;}
}
