// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <3ds.h>
#include "twl_advice.h"
typedef struct {bool known,supported,new3ds,loader,files;u8 region;unsigned missing,total,present;Result error;
 /* Explicit user reports, never inferred from installed-title presence. */
 bool confirmedAllFail,confirmedDownloadPlayFail;} TwlReport;
void twl_diagnose(TwlReport *report);
TwlAdvice twl_recommendation(const TwlReport *report);
/* Pending is conservative: malformed/unreadable records also require recovery. */
bool twl_repair_pending(void);
Result twl_repair_begin(const TwlReport *approved);
Result twl_repair_finish(void);

u64 twl_settings_title(const TwlReport *report);

Result twl_repair_begin_progress(const TwlReport *approved,void (*progress)(unsigned,unsigned,void*),void *context);
