#pragma once
#include <3ds.h>
#define SEED_NOT_OFFERED ((Result)1)
#define SEED_BAD_DATA ((Result)-80)
#define SEED_HTTP_ERROR ((Result)-81)
Result game_seed_import(u64 id);

/* phase: 0 checking, 1 repairing, 2 verifying; atomically read by UI. */
Result game_seed_ensure(u64 id, FS_MediaType media, u32 *phase);
