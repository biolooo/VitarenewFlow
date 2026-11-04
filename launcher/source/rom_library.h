// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "ds_library.h"
/* Discovery only: no game contents, writes, archives or emulator installation.
   Explicit DS roots retain their .nds-only meaning and priority. */
void rom_library_scan(const DsRoots *extraDsRoots,DsScan *result,DsVisit visit,DsContinue proceed,void *user);

/* Aggregate metadata work from the last completed main-thread scan, no paths. */
typedef struct {unsigned metadata_entries,stat_fallbacks,root_checks;} RomScanStats;
RomScanStats rom_library_last_stats(void);
