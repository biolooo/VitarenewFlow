#pragma once
#include <stddef.h>
#include <stdbool.h>
bool rom_pack_args(void *buffer, size_t capacity, const char *emulator, const char *rom);
bool rom_has_gba_extension(const char *name);
