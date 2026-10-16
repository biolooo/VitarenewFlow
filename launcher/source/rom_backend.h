#pragma once
#include <3ds.h>
#define LIBRARY_MGBA "sdmc:/3ds/mgba-library/mgba-library.3dsx"
Result rom_preflight(const char *emulator, const char *rom);
Result rom_prepare_launch(const char *rom);
Result emulator_prepare_launch(const char *emulator,const char *rom,bool archivePath);
