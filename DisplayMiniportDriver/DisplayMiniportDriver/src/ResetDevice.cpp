// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_reset_device
#include "Common.h"
#include "ResetDevice.hpp"
#include "HyDevice.hpp"
#include "Logging.h"

void HyResetDevice(IN_CONST_PVOID MiniportDeviceContext)
{
    TRACE_ENTRYPOINT();

    // If MiniportDeviceContext is null inform log that the parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter: MiniportDeviceContext\n");
        return;
    }

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(MiniportDeviceContext);

    deviceContext->ResetDevice();
}
