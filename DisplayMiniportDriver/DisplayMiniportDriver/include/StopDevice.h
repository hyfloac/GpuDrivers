#pragma once

#include <ntddk.h>
#include <dispmprt.h>

NTSTATUS HyStopDevice(IN_CONST_PVOID MiniportDeviceContext);
