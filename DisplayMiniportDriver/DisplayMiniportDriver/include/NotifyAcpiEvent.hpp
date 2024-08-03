#pragma once

#include "Common.h"

NTSTATUS HyNotifyAcpiEvent(IN_CONST_PVOID MiniportDeviceContext, IN_DXGK_EVENT_TYPE EventType, IN_ULONG Event, IN_PVOID Argument, OUT_PULONG AcpiFlags);
