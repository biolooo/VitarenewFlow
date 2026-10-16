#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "game_info.h"
typedef struct { const char *name; const char *cover; const char *backend; bool native; const uint16_t *icon; const char *banner; bool pending; } UiCard;
typedef struct {
    const char *system; const char *status; const char *version;
    const UiCard *cards; unsigned count; unsigned selected; unsigned total;
    bool confirm; bool loading; float progress; const char *stage;
    bool coverMenu; const char *coverTarget; const char **imports; unsigned importCount; unsigned importSelected;
    bool setupDs;
    float carousel; unsigned systemIndex; bool settings,systemPicker; unsigned menuSelected;
    int brightness; bool brightnessAvailable; bool animation; const char *themeSource; unsigned launchFrame; bool launching;
    bool controls;
    bool settingsMore,support;
    const GameInfo *info; unsigned infoScroll;
    bool installPrompt,installing;
    const char *installSize,*installSpace,*installEta;
    const char *installError;
    bool launchError;
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
