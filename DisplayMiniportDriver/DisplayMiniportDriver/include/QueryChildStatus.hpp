#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

NTSTATUS HyQueryChildStatus(IN_CONST_PVOID MiniportDeviceContext, INOUT_PDXGK_CHILD_STATUS ChildStatus, IN_BOOLEAN NonDestructiveOnly);
