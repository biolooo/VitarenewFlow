#pragma once
#include <stdbool.h>
#include <stddef.h>
bool ds_rom_extension(const char *name);
bool ds_loader_path(char *out,size_t capacity,const char *path);
