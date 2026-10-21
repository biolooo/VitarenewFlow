// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
enum LibrarySystem { SYSTEM_3DS, SYSTEM_DS, SYSTEM_GBA, SYSTEM_N64, SYSTEM_SNES, SYSTEM_NES, SYSTEM_VB, SYSTEM_GB, SYSTEM_COUNT };
enum LibraryView {VIEW_3DS,VIEW_HOMEBREW,VIEW_DS,VIEW_GAMEBOY,VIEW_N64,VIEW_SNES,VIEW_NES,VIEW_VB,VIEW_COUNT};
extern const char *library_view_names[VIEW_COUNT];
unsigned library_view_for(int system,bool homebrew);
unsigned library_view_system(unsigned view);
unsigned library_legacy_view(unsigned system);
bool library_is_homebrew(uint64_t id,const char *productCode);
extern const char *library_system_names[SYSTEM_COUNT];
extern const char *library_system_folders[SYSTEM_COUNT];
int library_file_system(const char *name);
bool library_cover_path(char *out,size_t capacity,int system,const char *key);
