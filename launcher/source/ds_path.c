// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "ds_path.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
bool ds_rom_extension(const char *name) {
    if (!name) return false;
    size_t n=strlen(name);
    return n>4 && !strcasecmp(name+n-4,".nds");
}
bool ds_loader_path(char *out,size_t capacity,const char *path) {
    if (!out || !capacity || !path || strpbrk(path,"\r\n") || !ds_rom_extension(path)) return false;
    if (!strncmp(path,"sdmc:/",6)) path+=5;
    if (path[0]!='/' || !path[1] || strstr(path,"/../")) return false;
    size_t needed=3+strlen(path)+1; /* sd: + /roms/... + NUL */
    if (needed>capacity || needed>256) return false;
    snprintf(out,capacity,"sd:%s",path);
    return true;
}
