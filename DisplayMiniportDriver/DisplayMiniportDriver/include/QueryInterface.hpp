#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

NTSTATUS HyQueryInterface(IN_CONST_PVOID MiniportDeviceContext, IN_PQUERY_INTERFACE QueryInterface);
