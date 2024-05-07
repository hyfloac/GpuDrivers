// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_getscanline
// See https://learn.microsoft.com/en-us/windows-hardware/drivers/display/saving-energy-with-vsync-control#display-only-vsync-requirements-for-windows-8-and-later-versions
#include "GetScanLine.hpp"
#include "HyDevice.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyGetScanLine(IN_CONST_HANDLE hAdapter, INOUT_PDXGKARG_GETSCANLINE pGetScanLine)
{
    CHECK_IRQL(PASSIVE_LEVEL);

    if constexpr(true)
    {
        LOG_DEBUG("HyGetScanLine\n");
    }

    // If hAdapter is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter to HyGetScanLine: hAdapter\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    HyMiniportDevice* const deviceContext = HY_MINIPORT_DEVICE_FROM_HANDLE(hAdapter);

    return deviceContext->GetScanLine(pGetScanLine);
}
