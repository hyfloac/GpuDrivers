// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_stop_device
#include <ntddk.h>
#include <dispmprt.h>

#include "AddDevice.h"
#include "StopDevice.h"

#pragma code_seg("PAGE")

NTSTATUS HyStopDevice(IN_CONST_PVOID MiniportDeviceContext)
{
    PAGED_CODE();

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    // Get our context structure.
    HyMiniportDeviceContext* const deviceContext = MiniportDeviceContext;

    deviceContext->Flags.IsStarted = FALSE;

    return STATUS_SUCCESS;
}
