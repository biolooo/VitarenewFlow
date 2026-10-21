// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef bool (*AudioDigest)(const void *,size_t,uint8_t[32]);
size_t audio_code_size(const uint8_t *data,size_t size);
bool audio_code_unpack(const uint8_t *data,size_t size,uint8_t *out,size_t capacity);
bool audio_dsp_valid(const uint8_t *data,size_t size,AudioDigest digest);
