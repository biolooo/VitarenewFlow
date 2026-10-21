// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <3ds.h>
typedef struct {bool present,ds;u64 id,size;char code[5],name[160];u16 icon[48*48];bool hasIcon;} Cartridge;
Result cartridge_read(Cartridge *card,unsigned language);
