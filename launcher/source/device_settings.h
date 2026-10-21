// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <3ds.h>
bool device_brightness_read(int *level,u32 *luminance);
Result device_brightness_set(unsigned level,u32 *before,u32 *after);
