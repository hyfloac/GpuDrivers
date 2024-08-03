// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_recommendmonitormodes
#include "Common.h"
#include "HyDevice.hpp"
#include "RecommendMonitorModes.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyRecommendMonitorModes(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_RECOMMENDMONITORMODES_CONST pRecommendMonitorModes)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    if constexpr(false)
    {
        TRACE_ENTRYPOINT();
    }

    // If MiniportDeviceContext (hAdapter) is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter: hAdapter\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If pRecommendMonitorModes is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pRecommendMonitorModes)
    {
        LOG_ERROR("Invalid Parameter: pRecommendMonitorModes\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(hAdapter);

    return deviceContext->RecommendMonitorModes(pRecommendMonitorModes);
}
