// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_add_device
#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#include "AddDevice.h"
#include "MemoryAllocator.h"

#pragma code_seg("PAGE")

NTSTATUS HyAddDevice(IN_CONST_PDEVICE_OBJECT PhysicalDeviceObject, OUT_PPVOID MiniportDeviceContext)
{
    PAGED_CODE();

    // If PhysicalDeviceObject is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!PhysicalDeviceObject)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    // If MiniportDeviceContext is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    // Allocate a new object to represent our device context.
    HyMiniportDeviceContext* const deviceContext = HY_ALLOC_ZERO(HyMiniportDeviceContext, NonPagedPoolNx, POOL_TAG_DEVICE_CONTEXT);

    // If allocation failed inform the kernel.
    if(!deviceContext)
    {
        return STATUS_NO_MEMORY;
    }

    // Store the adapter handle.
    deviceContext->PhysicalDeviceObject = PhysicalDeviceObject;
    deviceContext->MonitorPowerState = PowerDeviceD0;
    deviceContext->AdapterPowerState = PowerDeviceD0;

    // Return our device context to the driver.
    *MiniportDeviceContext = deviceContext;

    return STATUS_SUCCESS;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
