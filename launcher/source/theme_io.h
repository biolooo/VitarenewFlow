// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
bool theme_read_active(uint8_t **body,size_t *size,const char **source);
/* 1: active theme, 2: local HOME fallback, 0: no readable audio. */
unsigned theme_read_music(uint8_t **data,size_t *size);
