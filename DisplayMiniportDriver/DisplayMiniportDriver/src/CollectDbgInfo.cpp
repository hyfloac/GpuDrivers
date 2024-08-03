// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_collectdbginfo
// See https://learn.microsoft.com/en-us/windows-hardware/drivers/display/threading-and-synchronization-zero-level
#include "Common.h"
#include "CollectDbgInfo.hpp"
#include "HyDevice.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyCollectDbgInfo(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_COLLECTDBGINFO pCollectDbgInfo)
{
    // This function should collect debug information for various failures and can be
    // called at any time and at high IRQL (that is, the IRQL that DxgkDdiCollectDbgInfo
    // runs at is generally undefined).
    CHECK_IRQL(HIGH_LEVEL);

    TRACE_ENTRYPOINT();

    // If hAdapter is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter: hAdapter\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If pCollectDbgInfo is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter: pCollectDbgInfo\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(hAdapter);

    return deviceContext->CollectDbgInfo(pCollectDbgInfo);
}
