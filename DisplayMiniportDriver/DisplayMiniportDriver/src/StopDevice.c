// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_stop_device
#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#include "AddDevice.h"
#include "StopDevice.h"
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
    HyMiniportDeviceContext* const deviceContext = MiniportDeviceContext;

    if(deviceContext->Flags.IsStarted)
    {
        volatile UINT* const displayEnable0 = HyGetDeviceConfigRegister(deviceContext, BASE_REGISTER_DI + SIZE_REGISTER_DI * 0 + OFFSET_REGISTER_DI_ENABLE);

        // Disable Display
        *displayEnable0 = 0;
    }

    deviceContext->Flags.IsStarted = FALSE;

    return STATUS_SUCCESS;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
