// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_start_device
#include "StartDevice.h"
#include "AddDevice.h"
#include "Config.h"

#pragma code_seg("PAGE")

NTSTATUS CheckDevice(HyMiniportDeviceContext* MiniportDeviceContext);

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

    {
        // Get the basic data about our device.
        const NTSTATUS getDeviceInfoStatus = DxgkInterface->DxgkCbGetDeviceInformation(DxgkInterface->DeviceHandle, &deviceContext->DeviceInfo);

        if(!NT_SUCCESS(getDeviceInfoStatus))
        {
            return getDeviceInfoStatus;
        }
    }

    {
        // Check that the device is in fact ours.
        const NTSTATUS checkDeviceStatus = CheckDevice(MiniportDeviceContext);

        if(!NT_SUCCESS(checkDeviceStatus))
        {
            return checkDeviceStatus;
        }
    }
    
    {
        {
            RTL_OSVERSIONINFOEXW osVersionInfo = { 0 };
            osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
            const NTSTATUS getVersionStatus = RtlGetVersion((RTL_OSVERSIONINFOW*) &osVersionInfo);

            // We'll only validate the version if it returned successfully, otherwise we'll just assume that DxgkCbAcquirePostDisplayOwnership will work.
            if(NT_SUCCESS(getVersionStatus))
            {
                // If less than Windows 8, fail.
                if(osVersionInfo.dwMajorVersion < 6)
                {
                    return STATUS_UNSUCCESSFUL;
                }

                if(osVersionInfo.dwMinorVersion < 2)
                {
                    return STATUS_UNSUCCESSFUL;
                }
            }
        }

        // Check if DxgkCbAcquirePostDisplayOwnership exists, we're not supporting pre WDDM 1.2.
        if(!DxgkInterface->DxgkCbAcquirePostDisplayOwnership)
        {
            return STATUS_UNSUCCESSFUL;
        }
        
        const NTSTATUS acquirePostDisplayStatus = DxgkInterface->DxgkCbAcquirePostDisplayOwnership(DxgkInterface->DeviceHandle, &deviceContext->PostDisplayInfo);

        // If we failed to acquire the POST display we're probably not running a POST device, or we're pre WDDM 1.2.
        if(!NT_SUCCESS(acquirePostDisplayStatus) || deviceContext->PostDisplayInfo.Width == 0)
        {
            return STATUS_UNSUCCESSFUL;
        }
    }

    deviceContext->Flags.IsStarted = TRUE;

    // We'll specify that we have one VidPN source.
    *NumberOfVideoPresentSurfaces = 1;
    // We don't have any child devices. https://docs.microsoft.com/en-us/windows-hardware/drivers/display/child-devices-of-the-display-adapter
    *NumberOfChildren = 0;

    return STATUS_SUCCESS;
}

NTSTATUS CheckDevice(HyMiniportDeviceContext* const MiniportDeviceContext)
{
#if HY_DEVICE_EMULATED
    PCI_COMMON_HEADER pciHeader = { 0 };
    ULONG bytesRead;
    // Get the device's PCI Vendor and Device ID's.
    const NTSTATUS readDeviceSpaceStatus = MiniportDeviceContext->DxgkInterface.DxgkCbReadDeviceSpace(MiniportDeviceContext->DxgkInterface.DeviceHandle, DXGK_WHICHSPACE_CONFIG, &pciHeader, 0, sizeof(pciHeader), &bytesRead);

    if(!NT_SUCCESS(readDeviceSpaceStatus))
    {
        return readDeviceSpaceStatus;
    }

    if(pciHeader.VendorID != HY_VENDOR_ID)
    {
        return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }

    MiniportDeviceContext->DeviceId = pciHeader.DeviceID;

    switch(pciHeader.DeviceID)
    {
        case 0x0001: return STATUS_SUCCESS;
        // If we don't recognize the device ID a different version of our driver is probably handling it.
        default: return STATUS_GRAPHICS_DRIVER_MISMATCH;
    }
#endif
}
