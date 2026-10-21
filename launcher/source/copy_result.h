// SPDX-License-Identifier: GPL-3.0-only
#pragma once
typedef enum { COPY_NONE, COPY_INCOMPLETE, COPY_DS_READY, COPY_CIA_READY } CopyResult;
/* Read and acknowledge one bounded receipt. Never accepts executable paths. */
CopyResult copy_result_take(const char *path);
