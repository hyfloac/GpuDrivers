// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_add_device
#include "Common.h"
#include "HyDevice.hpp"
#include "AddDevice.hpp"
#include "MemoryAllocator.h"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyAddDevice(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    TRACE_ENTRYPOINT();

    // If PhysicalDeviceObject is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!PhysicalDeviceObject)
    {
        LOG_ERROR("Invalid Parameter: PhysicalDeviceObject\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If MiniportDeviceContext is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    // Allocate a new object to represent our device context.
    HyMiniportDevice* const deviceContext = new HyMiniportDevice(PhysicalDeviceObject);

    // If allocation failed inform the kernel.
    if(!deviceContext)
    {
        LOG_ERROR("Failed to allocate MiniportDeviceContext\n");
        return STATUS_NO_MEMORY;
    }

    //deviceContext->Flags.Value = 0;
    //
    //// Store the adapter handle.
    //deviceContext->PhysicalDeviceObject = PhysicalDeviceObject;
    //deviceContext->MonitorPowerState = PowerDeviceD0;
    //deviceContext->AdapterPowerState = PowerDeviceD0;

    {
        const NTSTATUS getAdapterStatus = deviceContext->GetAdapterInfo();
        if(!NT_SUCCESS(getAdapterStatus))
        {
            // Free the deviceContext on failure.
            // 2024-04-26: This was causing a memory leak.
            HY_FREE(deviceContext, POOL_TAG_DEVICE_CONTEXT);

            return getAdapterStatus;
        }
    }

    // Return our device context to the driver.
    *MiniportDeviceContext = deviceContext;

    return STATUS_SUCCESS;
}
