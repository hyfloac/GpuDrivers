// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_commitvidpn
#include "HyDevice.hpp"
#include "CommitVidPn.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyCommitVidPn(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_COMMITVIDPN_CONST pCommitVidPn)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HyCommitVidPn\n");

    // If hAdapter is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter to HyCommitVidPn: hAdapter\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If pPresentDisplayOnly is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pCommitVidPn)
    {
        LOG_ERROR("Invalid Parameter to HyCommitVidPn: pCommitVidPn\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HY_MINIPORT_DEVICE_FROM_HANDLE(hAdapter);

    return deviceContext->CommitVidPn(pCommitVidPn);
}
