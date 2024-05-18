// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_reset_device
#include "Common.h"
#include "ResetDevice.hpp"
#include "HyDevice.hpp"
#include "Logging.h"

void HyResetDevice(IN_CONST_PVOID MiniportDeviceContext)
{
    LOG_DEBUG("HyResetDevice\n");

    // If MiniportDeviceContext is null inform log that the parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter to HyResetDevice: MiniportDeviceContext\n");
        return;
    }

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HY_MINIPORT_DEVICE_FROM_HANDLE(MiniportDeviceContext);

    deviceContext->ResetDevice();
}
