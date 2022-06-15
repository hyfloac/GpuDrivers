// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/dispmprt/nc-dispmprt-dxgkddi_stop_device
#include <ntddk.h>
#include <dispmprt.h>

#include "AddDevice.h"
#include "SetVidPnSourceVisibility.h"

#pragma code_seg("PAGE")

NTSTATUS HySetVidPnSourceVisibility(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_SETVIDPNSOURCEVISIBILITY pSetVidPnSourceVisibility)
{
    PAGED_CODE();

    // If MiniportDeviceContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    if(!pSetVidPnSourceVisibility)
    {
        return STATUS_INVALID_PARAMETER_2;
    }

    UINT StartVidPnSourceId = (pSetVidPnSourceVisibility->VidPnSourceId == D3DDDI_ID_ALL) ? 0 : pSetVidPnSourceVisibility->VidPnSourceId;
    UINT MaxVidPnSourceId = (pSetVidPnSourceVisibility->VidPnSourceId == D3DDDI_ID_ALL) ? 1 : pSetVidPnSourceVisibility->VidPnSourceId + 1;

    // Get our context structure.
    HyMiniportDeviceContext* const deviceContext = hAdapter;

    // TODO: Blackout the screens.

    return STATUS_SUCCESS;
}
