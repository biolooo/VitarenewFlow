// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <stdbool.h>
bool banner_prepare(const char *path,bool stereo);
bool banner_draw(bool rightEye);
void banner_close(void);
void banner_pause(bool paused);
