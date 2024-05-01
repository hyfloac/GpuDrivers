// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_updateactivevidpnpresentpath
#include "UpdateActiveVidPnPresentPath.hpp"
#include "HyDevice.hpp"

NTSTATUS HyUpdateActiveVidPnPresentPath(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_UPDATEACTIVEVIDPNPRESENTPATH_CONST pUpdateActiveVidPnPresentPath)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HyUpdateActiveVidPnPresentPath\n");

    // If MiniportDeviceContext (hAdapter) is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter to HyUpdateActiveVidPnPresentPath: hAdapter\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If pUpdateActiveVidPnPresentPath is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pUpdateActiveVidPnPresentPath)
    {
        LOG_ERROR("Invalid Parameter to HyUpdateActiveVidPnPresentPath: pUpdateActiveVidPnPresentPath\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    HyMiniportDevice* const deviceContext = HY_MINIPORT_DEVICE_FROM_HANDLE(hAdapter);

    return deviceContext->UpdateActiveVidPnPresentPath(pUpdateActiveVidPnPresentPath);
}
