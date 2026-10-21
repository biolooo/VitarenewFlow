// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Technical fields are closed categories, never game names, paths or logs. */
typedef enum {
    ERROR_REPORT_GENERAL, ERROR_REPORT_LAUNCH, ERROR_REPORT_INSTALL,
    ERROR_REPORT_UPDATE, ERROR_REPORT_COMPONENT, ERROR_REPORT_DS,
    ERROR_REPORT_HOME, ERROR_REPORT_POWER, ERROR_REPORT_AUDIO,
    ERROR_REPORT_COVER, ERROR_REPORT_STORAGE, ERROR_REPORT_DESKTOP,
    ERROR_REPORT_AREA_COUNT
} ErrorReportArea;
typedef enum {
    ERROR_REPORT_MODEL_UNKNOWN, ERROR_REPORT_3DS, ERROR_REPORT_3DS_XL,
    ERROR_REPORT_2DS, ERROR_REPORT_NEW_3DS, ERROR_REPORT_NEW_3DS_XL,
    ERROR_REPORT_NEW_2DS_XL, ERROR_REPORT_PC, ERROR_REPORT_MODEL_COUNT
} ErrorReportModel;
typedef enum {
    ERROR_REPORT_REGION_UNKNOWN, ERROR_REPORT_JPN, ERROR_REPORT_USA,
    ERROR_REPORT_EUR, ERROR_REPORT_AUS, ERROR_REPORT_CHN, ERROR_REPORT_KOR,
    ERROR_REPORT_TWN, ERROR_REPORT_REGION_COUNT
} ErrorReportRegion;
typedef enum {
    ERROR_REPORT_AUTOBOOT_UNKNOWN, ERROR_REPORT_AUTOBOOT_OFF,
    ERROR_REPORT_AUTOBOOT_ON, ERROR_REPORT_AUTOBOOT_COUNT
} ErrorReportAutoboot;
typedef enum {
    ERROR_REPORT_CONTEXT_UNKNOWN, ERROR_REPORT_HOMEBREW,
    ERROR_REPORT_NATIVE, ERROR_REPORT_CONTEXT_PC, ERROR_REPORT_CONTEXT_COUNT
} ErrorReportContext;
typedef enum {
    ERROR_REPORT_ACTIVITY_UNKNOWN, ERROR_REPORT_BROWSING,
    ERROR_REPORT_STARTING_GAME, ERROR_REPORT_INSTALLING,
    ERROR_REPORT_CHECKING_UPDATE, ERROR_REPORT_DOWNLOADING,
    ERROR_REPORT_CHECKING_COMPONENTS, ERROR_REPORT_CHECKING_DS,
    ERROR_REPORT_OPENING_HOME, ERROR_REPORT_POWER_MENU,
    ERROR_REPORT_LOADING_THEME, ERROR_REPORT_LOADING_COVER,
    ERROR_REPORT_IMPORTING, ERROR_REPORT_ACTIVITY_COUNT
} ErrorReportActivity;

#define ERROR_REPORT_DESCRIPTION_MAX 512u /* UTF-8 bytes, excluding NUL. */
#define ERROR_REPORT_VERSION_MAX 31u
#define ERROR_REPORT_JSON_CAPACITY 4096u /* Including final NUL. */
typedef struct {
    const char *version; /* Required numeric major.minor.patch. */
    ErrorReportArea area;
    int32_t code;
    ErrorReportModel model;
    ErrorReportRegion region;
    ErrorReportAutoboot autoboot;
    ErrorReportContext context;
    ErrorReportActivity activity;
    const char *description; /* Optional, explicitly entered by the user. */
} ErrorReport;

/* No IO, implicit metadata, network or persistent identifier. The caller must
   obtain user consent before sending. Invalid/oversized input fails closed;
   valid out is empty and *written is zero on failure. */
bool error_report_json(const ErrorReport *report, char *out, size_t capacity,
                       size_t *written);
