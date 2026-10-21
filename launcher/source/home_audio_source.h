// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
/* Development acquisition of the console's own sound archive. No NAND writes. */
void home_audio_prepare_source(bool (*cancelled)(void));
