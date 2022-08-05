#pragma once

#include <ntddk.h>
#include <dispmprt.h>

#pragma warning(push)
#pragma warning(disable:4200) /* nonstandard extension used : zero-sized array in struct/union */
#pragma warning(disable:4201) // anonymous unions warning

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
            UINT IsEmulated : 1;
            UINT Reserved : 30;
        };
        UINT Value;
    } Flags;

    ULONG PCIBusNumber;
    PCI_SLOT_NUMBER PCISlotNumber;
    PCI_COMMON_CONFIG PCIConfig;

    DXGK_DISPLAY_INFORMATION PostDisplayInfo;

    // Current monitor power state
    DEVICE_POWER_STATE MonitorPowerState;

    // Current adapter power state
    DEVICE_POWER_STATE AdapterPowerState;
} HyMiniportDeviceContext;

#pragma warning(pop)

NTSTATUS HyAddDevice(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext);
