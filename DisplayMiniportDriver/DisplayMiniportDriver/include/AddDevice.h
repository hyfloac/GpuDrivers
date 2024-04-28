// ReSharper disable CppClangTidyModernizeMacroToEnum
#pragma once

#include <ntddk.h>
#include <dispmprt.h>

#pragma warning(push)
#pragma warning(disable:4200) // nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable:4201) // anonymous unions warning

#define VALUE_REGISTER_MAGIC    (0x4879666C)
#define VALUE_REGISTER_REVISION (0x00000001)
#define MASK_REGISTER_CONTROL   (0x00000001)

#define REGISTER_MAGIC            (0x0000)
#define REGISTER_REVISION         (0x0004)
#define REGISTER_EMULATION        (0x0008)
#define REGISTER_RESET            (0x000C)
#define REGISTER_CONTROL          (0x0010)
#define REGISTER_VRAM_SIZE_LOW    (0x0014)
#define REGISTER_VRAM_SIZE_HIGH   (0x0018)

#define BASE_REGISTER_DI          (0x2000)
#define SIZE_REGISTER_DI          (4 * 0x4)
#define OFFSET_REGISTER_DI_WIDTH  (0x00)
#define OFFSET_REGISTER_DI_HEIGHT (0x04)
#define OFFSET_REGISTER_DI_BPP    (0x08)
#define OFFSET_REGISTER_DI_ENABLE (0x0C)

#define BASE_REGISTER_EDID        (0x3000)
#define SIZE_REGISTER_EDID        (128)

#define REGISTER_DEBUG_PRINT      (0x8000)


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

    void* ConfigRegistersPointer;
    void* VRamPointer;

    // Current monitor power state
    DEVICE_POWER_STATE MonitorPowerState;

    // Current adapter power state
    DEVICE_POWER_STATE AdapterPowerState;
} HyMiniportDeviceContext;

#pragma warning(pop)

inline volatile UINT* HyGetDeviceConfigRegister(const HyMiniportDeviceContext* const deviceContext, const UINT registerAddress)
{
    return (UINT*) &((BYTE*) deviceContext->ConfigRegistersPointer)[registerAddress];
}

NTSTATUS HyAddDevice(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext);
