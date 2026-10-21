// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <stdbool.h>
typedef struct {float position,velocity;int startX,lastX;unsigned frames;bool touching,dragging;} CoverFlow;
enum FlowAction {FLOW_NONE,FLOW_SELECT,FLOW_START,FLOW_COVER};
void flow_reset(CoverFlow *f,unsigned selected);
int flow_update(CoverFlow *f,unsigned count,unsigned *selected,bool held,int x,int y);
void flow_step(CoverFlow *f,unsigned count,unsigned selected);
