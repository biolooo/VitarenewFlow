// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
typedef enum {TWL_CHECK_UNKNOWN,TWL_CHECK_UNSUPPORTED,TWL_CHECK_UPDATE,TWL_CHECK_LOADER,TWL_CHECK_CLEAR,TWL_CHECK_REPAIR} TwlAdvice;
TwlAdvice twl_advice(bool known,bool supported,unsigned missing,bool loader,bool files,bool allFail,bool downloadPlayFail);
