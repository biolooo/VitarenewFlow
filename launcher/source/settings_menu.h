// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
typedef enum {MENU_ROOT,MENU_PERSONAL,MENU_GAMES,MENU_SYSTEM} MenuPage;
typedef enum {MENU_NONE,MENU_PERSONAL_OPEN,MENU_GAMES_OPEN,MENU_SYSTEM_OPEN,MENU_SUPPORT,
 MENU_DESKTOP,MENU_COMPONENTS,MENU_LANGUAGE,MENU_THEME,MENU_SCAN,MENU_DS,MENU_AUTOBOOT,MENU_HOME,MENU_UPDATE} MenuAction;
typedef struct {const char *label,*detail;MenuAction action;} MenuItem;
unsigned settings_menu_count(unsigned page);
const char *settings_menu_title(unsigned page);
const MenuItem *settings_menu_item(unsigned page,unsigned row);
int settings_menu_hit(int x,int y,unsigned page);
unsigned settings_menu_parent_row(unsigned page);
