#pragma once

#include <ntddk.h>
#include <dispmprt.h>

NTSTATUS HyRemoveDevice(IN_CONST_PVOID MiniportDeviceContext);
