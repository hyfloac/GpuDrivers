// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_query_device_descriptor
#include "Common.h"
#include "QueryDeviceDescriptor.hpp"
#include "HyDevice.hpp"
#include "Logging.h"
#include "RegisterMemCopy.h"

#pragma code_seg("PAGE")

NTSTATUS HyQueryDeviceDescriptor(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG ChildUid, INOUT_PDXGK_DEVICE_DESCRIPTOR DeviceDescriptor)
{
    (void) MiniportDeviceContext;
    (void) ChildUid;
    (void) DeviceDescriptor;

    CHECK_IRQL(PASSIVE_LEVEL);

    if constexpr(false)
    {
        TRACE_ENTRYPOINT();
    }

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If DeviceDescriptor is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!DeviceDescriptor)
    {
        LOG_ERROR("Invalid Parameter: DeviceDescriptor\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(MiniportDeviceContext);

    return deviceContext->QueryDeviceDescriptor(ChildUid, DeviceDescriptor);
}
