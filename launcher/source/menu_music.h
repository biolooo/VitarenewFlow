// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
void menu_music_reload(void);
void menu_music_stop(void);
void menu_music_pause(bool pause);
const char *menu_music_status(void);

void menu_music_start_if_needed(void);

void menu_music_click(void);
