// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <sys/stat.h>
/* Public newlib directory-iterator API, implemented by synthetic host fixture. */
typedef struct DIR_ITER DIR_ITER;
DIR_ITER *diropen(const char *path);
int dirnext(DIR_ITER *dir,char *filename,struct stat *metadata);
int dirclose(DIR_ITER *dir);
