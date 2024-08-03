#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "Config.h"
#include <ntddk.h>
#include "D3DPolyFills.h"

#include "Logging.h"

#if ENFORCE_VAGUE_STATUSES
  #define VAGUE_STATUS(REAL_STATUS, SPEC_STATUS) SPEC_STATUS
#else
  #define VAGUE_STATUS(REAL_STATUS, SPEC_STATUS) REAL_STATUS
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
