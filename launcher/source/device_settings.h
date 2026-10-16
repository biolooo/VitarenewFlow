#pragma once
#include <3ds.h>
bool device_brightness_read(int *level,u32 *luminance);
Result device_brightness_set(unsigned level,u32 *before,u32 *after);
