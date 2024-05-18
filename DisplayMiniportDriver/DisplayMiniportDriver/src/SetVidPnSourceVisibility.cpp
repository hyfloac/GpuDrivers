// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_setvidpnsourcevisibility
#include "Common.h"
#include "HyDevice.hpp"
#include "SetVidPnSourceVisibility.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HySetVidPnSourceVisibility(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_SETVIDPNSOURCEVISIBILITY pSetVidPnSourceVisibility)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HySetVidPnSourceVisibility\n");

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter to HySetVidPnSourceVisibility: hAdapter\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If pSetVidPnSourceVisibility is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pSetVidPnSourceVisibility)
    {
        LOG_ERROR("Invalid Parameter to HySetVidPnSourceVisibility: pSetVidPnSourceVisibility\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HY_MINIPORT_DEVICE_FROM_HANDLE(hAdapter);

    return deviceContext->SetVidPnSourceVisibility(pSetVidPnSourceVisibility);
}
