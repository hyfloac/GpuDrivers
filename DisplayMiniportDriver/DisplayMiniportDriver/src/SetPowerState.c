// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_set_power_state
#ifdef __cplusplus
extern "C" {
#endif

#include <ntddk.h>
#include <dispmprt.h>

#include "AddDevice.h"
#include "SetPowerState.h"
#include "Logging.h"
#include "SetVidPnSourceVisibility.h"

#pragma code_seg("PAGE")

NTSTATUS HySetPowerState(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG DeviceUid, IN_DEVICE_POWER_STATE DevicePowerState, IN_POWER_ACTION ActionType)
{
    (void) ActionType;

    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HySetPowerState\n");

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter to HySetPowerState: MiniportDeviceContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // Get our context structure.
    HyMiniportDeviceContext* const deviceContext = MiniportDeviceContext;

    if(DeviceUid == DISPLAY_ADAPTER_HW_ID)
    {
        if(DevicePowerState == PowerDeviceD0)
        {
            if(deviceContext->AdapterPowerState == PowerDeviceD3)
            {
                DXGKARG_SETVIDPNSOURCEVISIBILITY visibility;
                visibility.VidPnSourceId = D3DDDI_ID_ALL;
                visibility.Visible = FALSE;
                HySetVidPnSourceVisibility(MiniportDeviceContext, &visibility);
            }
        }

        deviceContext->AdapterPowerState = DevicePowerState;
    }
    else // Some child device, currently this does nothing.
    {
        
    }

    return STATUS_SUCCESS;;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

