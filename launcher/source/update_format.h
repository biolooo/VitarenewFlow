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

#define UPDATE_NOTES_MAX_LINES 3u
#define UPDATE_NOTES_LINE_BYTES 120u /* UTF-8 bytes, excluding final NUL. */
#define UPDATE_NOTES_MAX_BYTES 1024u
typedef struct {
    char version[24];
    unsigned count;
    char de[UPDATE_NOTES_MAX_LINES][UPDATE_NOTES_LINE_BYTES+1];
    char en[UPDATE_NOTES_MAX_LINES][UPDATE_NOTES_LINE_BYTES+1];
} UpdateNotes;
/* VFNOTES1 + exact version + one to three alternating de=/en= pairs.
   Mandatory LF line endings, no embedded NUL/control characters or invalid
   UTF-8. A failed parse clears out, so previous-version notes cannot survive. */
bool update_notes_parse(const char *text,size_t size,const char *expected_version,UpdateNotes *out);
bool update_notes_url(const char *version,char *out,size_t capacity);
