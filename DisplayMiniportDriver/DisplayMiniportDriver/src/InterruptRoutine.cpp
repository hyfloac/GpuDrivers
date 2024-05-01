// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_interrupt_routine
#include "InterruptRoutine.hpp"
#include "Logging.h"

#pragma code_seg("_KTEXT")

BOOLEAN HyInterruptRoutine(IN_CONST_PVOID MiniportDeviceContext, IN_ULONG MessageNumber)
{
    (void) MiniportDeviceContext;
    (void) MessageNumber;

    CHECK_IRQL(HIGH_LEVEL); // HIGH_LEVEL is the best approximation of DIRQL

    LOG_DEBUG("HyInterruptRoutine\n");

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!MiniportDeviceContext)
    {
        LOG_ERROR("Invalid Parameter to HyInterruptRoutine: MiniportDeviceContext\n");
        return FALSE;
    }

    return FALSE;
}
