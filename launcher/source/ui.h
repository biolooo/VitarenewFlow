// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "game_info.h"
#include "app_update.h"
#include "components.h"
#include "twl_advice.h"
typedef struct { const char *name; const char *cover; const char *backend; bool native; const uint16_t *icon; const char *banner; bool pending; } UiCard;
typedef struct {
    const char *system; const char *status; const char *version;
    const UiCard *cards; unsigned count; unsigned selected; unsigned total;
    bool searchOpen,searchNumeric;const char *searchQuery;const char (*searchSuggestions)[33];const char *const *searchSystems;
    bool confirm; bool loading; float progress; const char *stage;
    unsigned homeLoading; /* 0 none, 1 prepare, 2 system handoff, 3 reboot to HOME */
    bool coverMenu; const char *coverTarget; const char **imports; unsigned importCount; unsigned importSelected;
    bool setupDs;
    bool updatePage;UpdateStatus update;
    bool componentPage,componentRechecked;unsigned componentSelected;ComponentStatus component;
    float carousel; unsigned systemIndex; bool settings,systemPicker; unsigned menuSelected;
    int brightness; bool brightnessAvailable; bool animation; const char *themeSource; unsigned launchFrame; bool launching;
    bool controls;
    int32_t twlError;unsigned twlPage,twlMissing;bool twlNeedsReboot,twlKnown,twlSupported,twlLoader,twlFiles;TwlAdvice twlAdvice;
    unsigned settingsPage;
    bool support,autobootPrompt,powerPrompt,powerCancelFailed,copyPrompt;int autobootState;
    const GameInfo *info; unsigned infoScroll;
    bool installPrompt,installing;
    const char *installSize,*installSpace,*installEta;
    const char *installError;
    bool launchError;
    bool launchRepair;
    const char *errorTitle;
    const UiCard *prefetch;unsigned prefetchCount;
} UiState;
bool ui_init(void);
void ui_draw(const UiState *state);
void ui_shutdown(void);
void ui_clear_covers(void);
bool ui_cover_available(const char *path);
bool ui_validate_cover(const char *path);
const char *ui_reload_theme(void);
unsigned ui_info_scroll_limit(void);

const char *ui_theme_source(void);
