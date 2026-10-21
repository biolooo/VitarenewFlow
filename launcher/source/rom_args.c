// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#include "rom_args.h"
#include <stdint.h>
#include <string.h>
#include <strings.h>
bool rom_pack_args(void *buffer, size_t capacity, const char *emulator, const char *rom)
{
    if (!buffer || !emulator || !rom || !*emulator || !*rom) return false;
    size_t a = strlen(emulator)+1, b = strlen(rom)+1;
    if (capacity < 4 || a > capacity-4 || b > capacity-4-a) return false;
    memset(buffer, 0, capacity);
    uint32_t argc = 2;
    memcpy(buffer, &argc, 4);
    memcpy((char*)buffer+4, emulator, a);
    memcpy((char*)buffer+4+a, rom, b);
    return true;
}
bool rom_has_gba_extension(const char *name)
{
    if (!name) return false;
    size_t n = strlen(name);
    return n>4 && !strcasecmp(name+n-4, ".gba");
}
