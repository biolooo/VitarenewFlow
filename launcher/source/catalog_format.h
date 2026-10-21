// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define SMDH_SIZE 0x36c0
#define SMDH_ICON_OFFSET 0x24c0
bool catalog_is_application(uint64_t id);
bool catalog_smdh_valid(const uint8_t *data, size_t size);
void catalog_text(char *out, size_t capacity, const uint8_t *utf16le, size_t units);
void catalog_name(char *out, size_t capacity, const uint8_t *smdh, unsigned language, bool publisher);
unsigned catalog_icon_index(unsigned x, unsigned y);
