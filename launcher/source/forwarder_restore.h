// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdbool.h>
#include <stdint.h>
bool forwarder_restore_parse(const char *arg,uint64_t *title);
void forwarder_restore(int argc,char **argv);
