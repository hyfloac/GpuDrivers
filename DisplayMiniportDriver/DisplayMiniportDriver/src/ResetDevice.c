// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_reset_device
#ifdef __cplusplus
extern "C" {
#endif

#include "ResetDevice.h"
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
    const HyMiniportDeviceContext* const deviceContext = MiniportDeviceContext;

    const volatile UINT* const resetReg = HyGetDeviceConfigRegister(deviceContext, REGISTER_RESET);

    // We don't actually care about the value, reading the register is enough to reset the device.
    const UINT resetValue = *resetReg;

    (void) resetValue;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
