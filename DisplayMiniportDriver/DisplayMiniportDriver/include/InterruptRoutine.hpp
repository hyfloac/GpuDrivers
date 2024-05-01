#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#ifdef __cplusplus
} /* extern "C" */
#endif


BOOLEAN HyInterruptRoutine(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG MessageNumber);
