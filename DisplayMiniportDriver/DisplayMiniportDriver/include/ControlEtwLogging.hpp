#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

void HyControlEtwLogging(IN_BOOLEAN Enable, IN_ULONG Flags, IN_UCHAR Level);
