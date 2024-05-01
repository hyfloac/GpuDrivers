#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#ifdef __cplusplus
} /* extern "C" */
#endif


void HyDpcRoutine(IN_CONST_PVOID MiniportDeviceContext);
