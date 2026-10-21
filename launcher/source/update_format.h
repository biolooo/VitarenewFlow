// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define UPDATE_MAX_BYTES (8u*1024u*1024u)
#define UPDATE_URL_ROOT "https://biolooo.github.io/VitarenewFlow/updates/"
#define UPDATE_MANIFEST_URL UPDATE_URL_ROOT "latest.txt"
typedef struct {char version[24],url[240],sha[65];uint32_t bytes;} UpdateManifest;
bool update_manifest_parse(const char *text,size_t size,UpdateManifest *out);
int update_version_compare(const char *a,const char *b);
