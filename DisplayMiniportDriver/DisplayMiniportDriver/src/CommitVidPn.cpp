// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_commitvidpn
#include "Common.h"
#include "HyDevice.hpp"
#include "CommitVidPn.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyCommitVidPn(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_COMMITVIDPN_CONST pCommitVidPn)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    TRACE_ENTRYPOINT();

    // If hAdapter is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter: hAdapter\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If pCommitVidPn is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pCommitVidPn)
    {
        LOG_ERROR("Invalid Parameter: pCommitVidPn\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    LOG_DEBUG(
        "hFunctionalVidPn: 0x%016X, AffectedVidPnSourceId: %d, MonitorConnectivityChecks: %d, hPrimaryAllocation: 0x%016X, Flags: 0x%08X\n",
        pCommitVidPn->hFunctionalVidPn,
        pCommitVidPn->AffectedVidPnSourceId,
        pCommitVidPn->MonitorConnectivityChecks,
        pCommitVidPn->hPrimaryAllocation,
        pCommitVidPn->Flags
    );

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(hAdapter);

    return deviceContext->CommitVidPn(pCommitVidPn);
}
