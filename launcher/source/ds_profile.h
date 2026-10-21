// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <3ds.h>
/* Create conservative defaults only for retail NTR games without a user profile. */
Result ds_prepare_profile(const char *rom);
