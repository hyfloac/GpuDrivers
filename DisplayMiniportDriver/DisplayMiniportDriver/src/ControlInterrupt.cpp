// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_controlinterrupt
// See https://learn.microsoft.com/en-us/windows-hardware/drivers/display/saving-energy-with-vsync-control#display-only-vsync-requirements-for-windows-8-and-later-versions
#include "Common.h"
#include "HyDevice.hpp"
#include "InterruptRoutine.hpp"
#include "Logging.h"

#pragma code_seg("_KTEXT")

NTSTATUS HyControlInterrupt(IN_CONST_HANDLE hAdapter, IN_CONST_DXGK_INTERRUPT_TYPE InterruptType, IN_BOOLEAN EnableInterrupt)
{
    (void) hAdapter;
    (void) InterruptType;
    (void) EnableInterrupt;

    CHECK_IRQL(PASSIVE_LEVEL); // HIGH_LEVEL is the best approximation of DIRQL

    TRACE_ENTRYPOINT_ARG("%sable %d\n", EnableInterrupt ? "En" : "Dis", InterruptType);

    // If hAdapter is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter: hAdapter\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(hAdapter);

    return deviceContext->ControlInterrupt(InterruptType, EnableInterrupt);
}
