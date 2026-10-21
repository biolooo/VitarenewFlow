// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <stdbool.h>
/* SD-only operations. Never writes boot.firm, NAND or the takeover title ID. */
int autoboot_status(void); /* 0 off, 1 managed/on, -1 conflict, -2 unavailable */
bool autoboot_set(bool enabled);
const char *autoboot_error(void);

/* HOME escape pauses autoboot; next launcher opening resumes it without reboot. */
bool autoboot_home_suspend(void);
int autoboot_home_resume(void); /* 0 no request, 1 resumed, -1 needs attention */
