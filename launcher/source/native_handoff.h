#pragma once
#include <3ds.h>
#define FLOW_TITLE_ID 0x000400000F104200ULL
#define FLOW_ERROR_HBL_CONFIG ((Result)-90)
#define FLOW_ERROR_HBL_MISSING ((Result)-91)
Result native_handoff_target(u64 *id);
