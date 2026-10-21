// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <3ds.h>
Result ds_prepare_launch(const char *rom);
typedef enum {DS_RUNTIME_READY,DS_RUNTIME_NO_TITLE,DS_RUNTIME_NO_FORWARDER,DS_RUNTIME_NO_BOOTSTRAP,DS_RUNTIME_UNAVAILABLE} DsRuntimeState;
/* Read-only runtime probe shared by the launcher and setup UI. No CIA cache needed. */
DsRuntimeState ds_runtime_check(void);
#define DS_BOOTSTRAP_ID 0x0004800546574452ULL
Result ds_install_bootstrap(void (*progress)(float));

Result ds_install_bootstrap_file(const char *path,void (*progress)(float));
