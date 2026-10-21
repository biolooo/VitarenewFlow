// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
#define UPDATE_APP "sdmc:/3ds/3ds-library/3ds-library.3dsx"
#define UPDATE_STAGE UPDATE_APP ".download"
#define UPDATE_BACKUP UPDATE_APP ".previous.3dsx"
typedef struct {bool (*exists)(const char *);int (*move)(const char *,const char *);int (*erase)(const char *);} UpdateFs;
/* 0 committed, -1 untouched, -2 rollback failed: backup is retained. */
int update_store_commit(const UpdateFs *fs);
