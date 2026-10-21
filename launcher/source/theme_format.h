// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct { const uint8_t *pixels; unsigned width, frames, frameWidth, mode; uint32_t color; } ThemeScreen;
bool theme_decompress(const uint8_t *in,size_t n,uint8_t **out,size_t *length);
bool theme_screen(const uint8_t *body,size_t size,bool lower,ThemeScreen *out);
bool theme_button_style(const uint8_t *body,size_t size,uint32_t *base,uint32_t *text,uint32_t *border);
