// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_stop_device
#include "HyDevice.hpp"
#include "StopDevice.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyStopDevice(IN_CONST_PVOID MiniportDeviceContext)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HyStopDevice\n");

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter to HyStopDevice: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HY_MINIPORT_DEVICE_FROM_HANDLE(MiniportDeviceContext);

    return deviceContext->StopDevice();
}
