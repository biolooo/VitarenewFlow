// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
#include "component_catalog.h"
typedef enum {COMP_IDLE,COMP_DOWNLOAD,COMP_READY,COMP_DONE,COMP_ERROR,COMP_CANCELLED} ComponentState;
typedef struct {ComponentState state;unsigned received,total;char message[128];} ComponentStatus;
void components_start(unsigned index);
void components_cancel(void);
void components_stop(void);
ComponentStatus components_status(void);
bool components_commit(void);
bool components_present(unsigned index);
typedef enum {COMP_MISSING,COMP_AVAILABLE,COMP_UNCHECKED} ComponentAvailability;
ComponentAvailability components_availability(unsigned index);
const char *components_presence_detail(unsigned index);
bool components_recheck(void);

void components_clear(void);

/* Numeric diagnostic only; never localized messages or file paths. */
int components_error_code(void);
