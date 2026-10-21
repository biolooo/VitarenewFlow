// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <stdbool.h>
bool ui_runtime_begin(void);
void ui_runtime_drain(void);
void ui_runtime_reset(void);
void ui_runtime_allow_pending_home(bool allowed);
