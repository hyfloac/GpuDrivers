#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

NTSTATUS HyNotifyAcpiEvent(IN_CONST_PVOID MiniportDeviceContext, IN_DXGK_EVENT_TYPE EventType, IN_ULONG Event, IN_PVOID Argument, OUT_PULONG AcpiFlags);
