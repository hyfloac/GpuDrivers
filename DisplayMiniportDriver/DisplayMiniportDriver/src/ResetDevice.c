#include "ResetDevice.h"

#define REGISTER_RESET 0x000C

void HyResetDevice(IN_CONST_PVOID MiniportDeviceContext)
{
    // Get our context structure.
    HyMiniportDeviceContext* const deviceContext = MiniportDeviceContext;

    const volatile UINT* const resetReg = HyGetDeviceConfigRegister(deviceContext, REGISTER_RESET);

#pragma warning(push)
#pragma warning(disable:4189) /* local variable is initialized but not referenced */

    // We don't actually care about the value, reading the register is enough to reset the device.
    UINT resetValue = *resetReg;
#pragma warning(pop)
}
