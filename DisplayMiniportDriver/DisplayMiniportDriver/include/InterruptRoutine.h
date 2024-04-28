#pragma once

#include <ntddk.h>
#include <dispmprt.h>

BOOLEAN HyInterruptRoutine(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG MessageNumber);
