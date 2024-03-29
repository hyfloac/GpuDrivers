// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_setvidpnsourcevisibility
#include <ntddk.h>
#include <dispmprt.h>

#include "AddDevice.h"
#include "SetVidPnSourceVisibility.h"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HySetVidPnSourceVisibility(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_SETVIDPNSOURCEVISIBILITY pSetVidPnSourceVisibility)
{
    PAGED_CODE();

    LOG_DEBUG("HySetVidPnSourceVisibility\n");

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter to HySetVidPnSourceVisibility: hAdapter\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    if(!pSetVidPnSourceVisibility)
    {
        LOG_ERROR("Invalid Parameter to HySetVidPnSourceVisibility: pSetVidPnSourceVisibility\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    const UINT StartVidPnSourceId = (pSetVidPnSourceVisibility->VidPnSourceId == D3DDDI_ID_ALL) ? 0 : pSetVidPnSourceVisibility->VidPnSourceId;
    const UINT MaxVidPnSourceId = (pSetVidPnSourceVisibility->VidPnSourceId == D3DDDI_ID_ALL) ? 1 : pSetVidPnSourceVisibility->VidPnSourceId + 1;

    // Get our context structure.
    HyMiniportDeviceContext* const deviceContext = hAdapter;

    (void) StartVidPnSourceId;
    (void) MaxVidPnSourceId;
    (void) deviceContext;

    // TODO: Blackout the screens.

    return STATUS_SUCCESS;
}
