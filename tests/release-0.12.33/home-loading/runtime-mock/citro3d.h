#pragma once
#include <stdbool.h>
#define C3D_FRAME_NONBLOCK 1
bool C3D_FrameBegin(unsigned flags);void C3D_FrameEnd(unsigned flags);
