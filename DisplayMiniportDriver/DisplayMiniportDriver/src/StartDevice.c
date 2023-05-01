// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_start_device
#include "StartDevice.h"
#include "AddDevice.h"
#include "Config.h"
#include "MemoryAllocator.h"

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

    if(0)
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

    {
        NTSTATUS getEnumeratorStatus;
        wchar_t* enumerator;
        wchar_t staticEnumeratorBuffer[16];

        // Keep iterating until success if the failure is caused by too small of a buffer.
        do
        {
            // Get the length of the enumerator string.
            ULONG bufferLength;
            const NTSTATUS getEnumeratorLengthStatus = IoGetDeviceProperty(deviceContext->PhysicalDeviceObject, DevicePropertyEnumeratorName, 0, NULL, &bufferLength);

            // If we fail propagate errors, unless it is a positional argument error.
            if(!NT_SUCCESS(getEnumeratorLengthStatus))
            {
                HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);

                if(getEnumeratorLengthStatus == STATUS_INVALID_PARAMETER_2)
                {
                    return STATUS_UNSUCCESSFUL;
                }

                return getEnumeratorLengthStatus;
            }

            // If the length is sufficiently small we'll just use static allocation.
            if(bufferLength <= 16)
            {
                enumerator = staticEnumeratorBuffer;
            }
            else
            {
                // Allocate the buffer for the enumerator name.
                enumerator = HyAllocate(NonPagedPoolNx, bufferLength * sizeof(wchar_t), POOL_TAG_DEVICE_CONTEXT);

                // If the allocation fails report that we're out of memory.
                if(!enumerator)
                {
                    HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);

                    return STATUS_NO_MEMORY;
                }
            }

            // Get the enumerator name.
            getEnumeratorStatus = IoGetDeviceProperty(deviceContext->PhysicalDeviceObject, DevicePropertyEnumeratorName, bufferLength, enumerator, &bufferLength);

            // If we're successful we can break out of the loop.
            if(NT_SUCCESS(getEnumeratorStatus))
            {
                break;
            }

            // If we failed for a reason other than the buffer being too small, propagate errors, unless it is a positional argument error.
            if(getEnumeratorStatus != STATUS_BUFFER_TOO_SMALL)
            {
                HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);
                HY_FREE(enumerator, POOL_TAG_DEVICE_CONTEXT);

                if(getEnumeratorStatus == STATUS_INVALID_PARAMETER_2)
                {
                    return STATUS_UNSUCCESSFUL;
                }

                return getEnumeratorStatus;
            }

            HY_FREE(enumerator, POOL_TAG_DEVICE_CONTEXT);
        } while(getEnumeratorStatus == STATUS_BUFFER_TOO_SMALL);

        // The enumerator name for PCI devices, as opposed to Root Enumerated Devices.
        const wchar_t pciPrefix[] = L"PCI";

        // Check only the first 3 characters.
        if(wcsncmp(pciPrefix, enumerator, wcslen(pciPrefix)) == 0)
        {
            // If it starts with PCI then it is a discrete GPU, as opposed to an emulated one.
            deviceContext->Flags.IsEmulated = FALSE;
        }
        else
        {
            deviceContext->Flags.IsEmulated = TRUE;
        }

        // Only free if it was dynamically allocated.
        if(enumerator != staticEnumeratorBuffer)
        {
            // Free the buffer containing the enumerator name.
            HY_FREE(enumerator, POOL_TAG_DEVICE_CONTEXT);
        }
    }

    {
        CM_FULL_RESOURCE_DESCRIPTOR* list = deviceContext->DeviceInfo.TranslatedResourceList->List;

        for(UINT i = 0; i < deviceContext->DeviceInfo.TranslatedResourceList->Count; ++i)
        {
            for(UINT j = 0; j < list->PartialResourceList.Count; ++j)
            {
                const CM_PARTIAL_RESOURCE_DESCRIPTOR* const desc = &list->PartialResourceList.PartialDescriptors[j];

                switch(desc->Type)
                {
                    case CmResourceTypeMemory:
                        if(desc->Flags & CM_RESOURCE_PORT_MEMORY && desc->Flags & CM_RESOURCE_MEMORY_READ_WRITE)
                        {
                            if(!(desc->Flags & CM_RESOURCE_MEMORY_PREFETCHABLE))
                            {
                                // This should be BAR0
                                DxgkInterface->DxgkCbMapMemory(DxgkInterface->DeviceHandle, desc->u.Memory.Start, desc->u.Memory.Length, FALSE, FALSE, MmNonCached, &deviceContext->ConfigRegistersPointer);
                            }
                            else
                            {
                                // This should be BAR1
                                DxgkInterface->DxgkCbMapMemory(DxgkInterface->DeviceHandle, desc->u.Memory.Start, desc->u.Memory.Length, FALSE, FALSE, MmCached, &deviceContext->VRamPointer);
                            }
                        }
                        break;
                    case CmResourceTypeMemoryLarge:
                        break;
                    default:
                        break;
                }
            }

            // Advance to the next full resource descriptor.
            list = (CM_FULL_RESOURCE_DESCRIPTOR*) (&list->PartialResourceList.PartialDescriptors[list->PartialResourceList.Count]);
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
