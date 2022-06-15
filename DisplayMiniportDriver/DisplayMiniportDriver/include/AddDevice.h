#pragma once

#include <ntddk.h>
#include <dispmprt.h>

typedef struct
{
    PDEVICE_OBJECT PhysicalDeviceObject;
    DXGK_START_INFO DxgkStartInfo;
    DXGKRNL_INTERFACE DxgkInterface;
    DXGK_DEVICE_INFO DeviceInfo;
    USHORT DeviceId;

    union
    {
        struct
        {
            UINT IsStarted : 1;
            UINT Reserved : 31;
        };
        UINT Value;
    } Flags;

    DXGK_DISPLAY_INFORMATION PostDisplayInfo;


    // Current monitor power state
    DEVICE_POWER_STATE MonitorPowerState;

    // Current adapter power state
    DEVICE_POWER_STATE AdapterPowerState;
} HyMiniportDeviceContext;

NTSTATUS HyAddDevice(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext);
