// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_set_power_state
#include "HyDevice.hpp"
#include "SetPowerState.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HySetPowerState(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG DeviceUid, IN_DEVICE_POWER_STATE DevicePowerState, IN_POWER_ACTION ActionType)
{
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
    HyMiniportDevice* const deviceContext = HY_MINIPORT_DEVICE_FROM_HANDLE(MiniportDeviceContext);

    return deviceContext->SetPowerState(DeviceUid, DevicePowerState, ActionType);
}
