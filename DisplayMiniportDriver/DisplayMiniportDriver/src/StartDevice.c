// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_start_device
#ifdef __cplusplus
extern "C" {
#endif

#include "StartDevice.h"
#include "AddDevice.h"
#include "Config.h"
#include "Logging.h"
#include "MemoryAllocator.h"

#pragma code_seg("PAGE")

NTSTATUS CheckDevice(HyMiniportDeviceContext* MiniportDeviceContext);

NTSTATUS HyStartDevice(IN_CONST_PVOID MiniportDeviceContext, IN_PDXGK_START_INFO DxgkStartInfo, IN_PDXGKRNL_INTERFACE DxgkInterface, OUT_PULONG NumberOfVideoPresentSurfaces, OUT_PULONG NumberOfChildren)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HyStartDevice\n");

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter to HyStartDevice: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If DxgkStartInfo is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!DxgkStartInfo)
    {
        LOG_ERROR("Invalid Parameter to HyStartDevice: DxgkStartInfo\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    // If DxgkInterface is null inform the kernel that the third parameter was invalid.
    // This should probably never happen.
    if(!DxgkInterface)
    {
        LOG_ERROR("Invalid Parameter to HyStartDevice: DxgkInterface\n");
        return STATUS_INVALID_PARAMETER_3;
    }

    // If NumberOfVideoPresentSurfaces is null inform the kernel that the fourth parameter was invalid.
    // This should probably never happen.
    if(!NumberOfVideoPresentSurfaces)
    {
        LOG_ERROR("Invalid Parameter to HyStartDevice: NumberOfVideoPresentSurfaces\n");
        return STATUS_INVALID_PARAMETER_4;
    }

    // If NumberOfChildren is null inform the kernel that the fifth parameter was invalid.
    // This should probably never happen.
    if(!NumberOfChildren)
    {
        LOG_ERROR("Invalid Parameter to HyStartDevice: NumberOfChildren\n");
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
            LOG_ERROR("HyStartDevice: Failed to get device info.\n");
            return getDeviceInfoStatus;
        }
    }

    {
        // Check that the device is in fact ours.
        const NTSTATUS checkDeviceStatus = CheckDevice(MiniportDeviceContext);

        if(!NT_SUCCESS(checkDeviceStatus))
        {
            LOG_ERROR("HyStartDevice: Failed to check device.\n");
            return checkDeviceStatus;
        }
    }

    LOG_DEBUG("HyStartDevice: Check device passed.\n");

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
            LOG_DEBUG("HyStartDevice: Attempting to get the DevicePropertyEnumeratorName.\n");
            // Get the length of the enumerator string.
            ULONG bufferLength;
            const NTSTATUS getEnumeratorLengthStatus = IoGetDeviceProperty(deviceContext->PhysicalDeviceObject, DevicePropertyEnumeratorName, 0, NULL, &bufferLength);

            // If we fail propagate errors, unless it is a positional argument error.
            if(!NT_SUCCESS(getEnumeratorLengthStatus) && getEnumeratorLengthStatus != STATUS_BUFFER_TOO_SMALL)
            {
                LOG_ERROR("HyStartDevice: Failed to get the length of DevicePropertyEnumeratorName: 0x%08X.\n", getEnumeratorLengthStatus);
                // This seems to be causing a BugCheck (BSOD).
                // HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);

                if(getEnumeratorLengthStatus == STATUS_INVALID_PARAMETER_2)
                {
                    return STATUS_UNSUCCESSFUL;
                }

                return getEnumeratorLengthStatus;
            }

            LOG_DEBUG("HyStartDevice: Buffer length: %u\n", bufferLength);

            // If the length is sufficiently small we'll just use static allocation.
            if(bufferLength <= 16)
            {
                LOG_DEBUG("HyStartDevice: Using static buffer\n");
                enumerator = staticEnumeratorBuffer;
            }
            else
            {
                LOG_DEBUG("HyStartDevice: Allocating buffer.\n");
                // Allocate the buffer for the enumerator name.
                enumerator = HyAllocate(NonPagedPoolNx, bufferLength * sizeof(wchar_t), POOL_TAG_DEVICE_CONTEXT);

                // If the allocation fails report that we're out of memory.
                if(!enumerator)
                {
                    LOG_ERROR("HyStartDevice: Failed to allocate PCI Device String Buffer.\n");
                    // This seems to be causing a BugCheck (BSOD).
                    // HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);

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
                LOG_ERROR("HyStartDevice: Failed to get DevicePropertyEnumeratorName: 0x%08X.\n", getEnumeratorStatus);
                // This seems to be causing a BugCheck (BSOD).
                // HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);

                // Only free if it was dynamically allocated.
                if(enumerator != staticEnumeratorBuffer)
                {
                    // Free the buffer containing the enumerator name.
                    HY_FREE(enumerator, POOL_TAG_DEVICE_CONTEXT);
                }

                if(getEnumeratorStatus == STATUS_INVALID_PARAMETER_2)
                {
                    return STATUS_UNSUCCESSFUL;
                }

                return getEnumeratorStatus;
            }

            // Only free if it was dynamically allocated.
            if(enumerator != staticEnumeratorBuffer)
            {
                // Free the buffer containing the enumerator name.
                HY_FREE(enumerator, POOL_TAG_DEVICE_CONTEXT);
            }
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

        LOG_DEBUG("HyStartDevice: Device ID Prefix: %ls\n", enumerator);

        // Only free if it was dynamically allocated.
        if(enumerator != staticEnumeratorBuffer)
        {
            // Free the buffer containing the enumerator name.
            HY_FREE(enumerator, POOL_TAG_DEVICE_CONTEXT);
        }
    }

    {
        LOG_DEBUG("HyStartDevice: Setting up BARs.\n");

        CM_FULL_RESOURCE_DESCRIPTOR* list = deviceContext->DeviceInfo.TranslatedResourceList->List;
        LOG_DEBUG("HyStartDevice: CM_FULL_RESOURCE_DESCRIPTOR: 0x%p.\n", list);

        for(UINT i = 0; i < deviceContext->DeviceInfo.TranslatedResourceList->Count; ++i)
        {
            LOG_DEBUG("HyStartDevice: Resource List: %u.\n", i);
            for(UINT j = 0; j < list->PartialResourceList.Count; ++j)
            {
                LOG_DEBUG("HyStartDevice: Partial Resource List: %u.\n", j);
                const CM_PARTIAL_RESOURCE_DESCRIPTOR* const desc = &list->PartialResourceList.PartialDescriptors[j];

                LOG_DEBUG("HyStartDevice: Partial Resource List Desc: 0x%p, Type: %d, Flags: 0x%04X.\n", desc, desc->Type, desc->Flags);
                switch(desc->Type)
                {
                    case CmResourceTypeMemory:
                        LOG_DEBUG("HyStartDevice: Handling CmResourceTypeMemory.\n");
                        if((desc->Flags & CM_RESOURCE_PORT_MEMORY) == CM_RESOURCE_PORT_MEMORY && 
                           (desc->Flags & CM_RESOURCE_MEMORY_READ_WRITE) == CM_RESOURCE_MEMORY_READ_WRITE
                        )
                        {
                            if(!(desc->Flags & CM_RESOURCE_MEMORY_PREFETCHABLE))
                            {
                                LOG_DEBUG("HyStartDevice: Mapping BAR0.\n");
                                // This should be BAR0
                                DxgkInterface->DxgkCbMapMemory(DxgkInterface->DeviceHandle, desc->u.Memory.Start, desc->u.Memory.Length, FALSE, FALSE, MmNonCached, &deviceContext->ConfigRegistersPointer);
                            }
                            else
                            {
                                LOG_DEBUG("HyStartDevice: Mapping BAR1.\n");
                                // This should be BAR1
                                DxgkInterface->DxgkCbMapMemory(DxgkInterface->DeviceHandle, desc->u.Memory.Start, desc->u.Memory.Length, FALSE, FALSE, MmCached, &deviceContext->VRamPointer);
                            }
                        }
                        else
                        {
                            LOG_ERROR("HyStartDevice: CmResourceTypeMemory must be of type CM_RESOURCE_PORT_MEMORY & CM_RESOURCE_MEMORY_READ_WRITE.\n");
                        }
                        break;
                    case CmResourceTypeMemoryLarge:
                        LOG_ERROR("HyStartDevice: Cannot handle CmResourceTypeMemoryLarge.\n");
                        break;
                    default:
                        break;
                }
            }

            // Advance to the next full resource descriptor.
            list = (CM_FULL_RESOURCE_DESCRIPTOR*) (&list->PartialResourceList.PartialDescriptors[list->PartialResourceList.Count]);
        }
    }

    volatile UINT* const displayEnable0 = HyGetDeviceConfigRegister(deviceContext, BASE_REGISTER_DI + SIZE_REGISTER_DI * 0 + OFFSET_REGISTER_DI_ENABLE);

    // Enable Display
    *displayEnable0 = 1;

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

#ifdef __cplusplus
} /* extern "C" */
#endif
