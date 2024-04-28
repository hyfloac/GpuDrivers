#pragma once

#include <ntddk.h>
#include <dispmprt.h>

NTSTATUS HyQueryDeviceDescriptor(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG ChildUid, INOUT_PDXGK_DEVICE_DESCRIPTOR DeviceDescriptor);
