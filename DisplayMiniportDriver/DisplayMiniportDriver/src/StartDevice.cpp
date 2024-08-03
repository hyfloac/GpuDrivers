// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_start_device
#include "Common.h"

#include "StartDevice.hpp"
#include "HyDevice.hpp"
#include "Logging.h"
#include "MemoryAllocator.h"

#pragma code_seg("PAGE")

NTSTATUS HyStartDevice(IN_CONST_PVOID MiniportDeviceContext, IN_PDXGK_START_INFO DxgkStartInfo, IN_PDXGKRNL_INTERFACE DxgkInterface, OUT_PULONG NumberOfVideoPresentSurfaces, OUT_PULONG NumberOfChildren)
{
    CHECK_IRQL(PASSIVE_LEVEL);

    TRACE_ENTRYPOINT();

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If DxgkStartInfo is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!DxgkStartInfo)
    {
        LOG_ERROR("Invalid Parameter: DxgkStartInfo\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    // If DxgkInterface is null inform the kernel that the third parameter was invalid.
    // This should probably never happen.
    if(!DxgkInterface)
    {
        LOG_ERROR("Invalid Parameter: DxgkInterface\n");
        return STATUS_INVALID_PARAMETER_3;
    }

    // If NumberOfVideoPresentSurfaces is null inform the kernel that the fourth parameter was invalid.
    // This should probably never happen.
    if(!NumberOfVideoPresentSurfaces)
    {
        LOG_ERROR("Invalid Parameter: NumberOfVideoPresentSurfaces\n");
        return STATUS_INVALID_PARAMETER_4;
    }

    // If NumberOfChildren is null inform the kernel that the fifth parameter was invalid.
    // This should probably never happen.
    if(!NumberOfChildren)
    {
        LOG_ERROR("Invalid Parameter: NumberOfChildren\n");
        return STATUS_INVALID_PARAMETER_5;
    }

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(MiniportDeviceContext);

    return deviceContext->StartDevice(DxgkStartInfo, DxgkInterface, NumberOfVideoPresentSurfaces, NumberOfChildren);
}
