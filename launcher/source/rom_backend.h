// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <3ds.h>
#define LIBRARY_MGBA "sdmc:/3ds/mgba-library/mgba-library.3dsx"
Result rom_preflight(const char *emulator, const char *rom);
Result rom_prepare_launch(const char *rom);
Result emulator_prepare_launch(const char *emulator,const char *rom,bool archivePath);
Result homebrew_prepare_launch(const char *path);
