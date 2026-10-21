// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <stddef.h>
#include <stdint.h>
#define GAME_INFO_FIELDS 24
typedef struct {char label[32],value[512];} GameInfoField;
typedef struct {char title[160];unsigned revision,count;GameInfoField fields[GAME_INFO_FIELDS];} GameInfo;
void game_display_title(char *out,size_t capacity,const char *name);
void game_info_begin(GameInfo *info,const char *name);
void game_info_add(GameInfo *info,const char *label,const char *value);
void game_info_size(GameInfo *info,const char *label,uint64_t bytes);
void game_info_rom(GameInfo *info,int system,const char *path);
