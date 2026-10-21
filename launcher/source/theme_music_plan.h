// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct {uint32_t bodyBytes,musicBytes;unsigned slot;bool shuffle,known;} ThemeMusicPlan;
bool theme_music_plan(ThemeMusicPlan *plan,const uint8_t *save,size_t saveBytes,const uint8_t *manage,size_t manageBytes);
