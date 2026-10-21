#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef uint64_t u64;typedef int64_t s64;
#define SYSCLOCK_ARM11 268123480ULL
bool aptIsActive(void);bool aptShouldClose(void);bool aptShouldJumpToHome(void);
u64 svcGetSystemTick(void);void svcSleepThread(s64 ns);
