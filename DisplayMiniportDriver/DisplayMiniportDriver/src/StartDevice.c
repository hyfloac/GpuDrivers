// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_start_device
#include "StartDevice.h"
#include "AddDevice.h"

NTSTATUS HyStartDevice(IN_CONST_PVOID MiniportDeviceContext, IN_PDXGK_START_INFO DxgkStartInfo, IN_PDXGKRNL_INTERFACE DxgkInterface, OUT_PULONG NumberOfVideoPresentSurfaces, OUT_PULONG NumberOfChildren)
{
    PAGED_CODE();

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    // If DxgkStartInfo is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!DxgkStartInfo)
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    // If DxgkInterface is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!DxgkInterface)
    {
        return STATUS_INVALID_PARAMETER_3;
    }

    // If NumberOfVideoPresentSurfaces is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!NumberOfVideoPresentSurfaces)
    {
        return STATUS_INVALID_PARAMETER_4;
    }

    // If NumberOfChildren is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!NumberOfChildren)
    {
        return STATUS_INVALID_PARAMETER_5;
    }

    // Get our context structure.
    HyMiniportDeviceContext* const deviceContext = MiniportDeviceContext;

    // Copy and save the DXGK start info.
    deviceContext->DxgkStartInfo = *DxgkStartInfo;
    // Copy and save the DXGK interface functions.
    deviceContext->DxgkInterface = *DxgkInterface;

    DXGK_DEVICE_INFO deviceInfo;
    const NTSTATUS getDeviceInfoStatus = DxgkInterface->DxgkCbGetDeviceInformation(DxgkInterface->DeviceHandle, &deviceInfo);

    if(!NT_SUCCESS(getDeviceInfoStatus))
    {
        return getDeviceInfoStatus;
    }



    // We'll specify that we have one VidPN source.
    *NumberOfVideoPresentSurfaces = 1;
    // We don't have any child devices. https://docs.microsoft.com/en-us/windows-hardware/drivers/display/child-devices-of-the-display-adapter
    *NumberOfChildren = 0;

    return STATUS_SUCCESS;
}