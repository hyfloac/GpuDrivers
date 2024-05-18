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
    const HyMiniportDevice* const deviceContext = HY_MINIPORT_DEVICE_FROM_HANDLE(MiniportDeviceContext);

    const volatile UINT* const resetReg = deviceContext->GetDeviceConfigRegister(HyMiniportDevice::REGISTER_RESET);

    // We don't actually care about the value, reading the register is enough to reset the device.
    const UINT resetValue = *resetReg;

    (void) resetValue;
}
