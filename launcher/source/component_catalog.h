// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#define COMPONENT_COUNT 7
#define COMPONENT_MAX_FILES 4
typedef struct {const char *url,*target,*sha;uint32_t bytes;} ComponentFile;
typedef struct {const char *name,*version;const ComponentFile *files;unsigned count;} ComponentDef;
const ComponentDef *component_definition(unsigned index);

#define DESKTOP_PLATFORM_COUNT 4
/* Windows keeps the original immutable desktop-1 package. */
const ComponentDef *component_desktop_definition(unsigned platform);
const char *component_desktop_label(unsigned platform);
int component_desktop_hit(int x,int y);
unsigned component_desktop_step(unsigned platform,int direction);
