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
