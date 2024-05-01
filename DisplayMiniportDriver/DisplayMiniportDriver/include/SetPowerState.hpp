#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

NTSTATUS HySetPowerState(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG DeviceUid, IN_DEVICE_POWER_STATE DevicePowerState, IN_POWER_ACTION ActionType);
