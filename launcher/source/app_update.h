// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
#include "update_format.h"
typedef enum {UPDATE_IDLE,UPDATE_CHECKING,UPDATE_CURRENT,UPDATE_AVAILABLE,UPDATE_DOWNLOADING,UPDATE_READY,UPDATE_INSTALLED,UPDATE_ERROR,UPDATE_CANCELLED} UpdateState;
typedef struct {UpdateState state;unsigned received,total;char available[24],message[128];} UpdateStatus;
void app_update_check(void);
void app_update_download(void);
void app_update_cancel(void);
void app_update_stop(void);
UpdateStatus app_update_status(void);
bool app_update_install(void);
