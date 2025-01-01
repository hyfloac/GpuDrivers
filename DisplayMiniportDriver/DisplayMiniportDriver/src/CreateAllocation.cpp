// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_createallocation
#include "Common.h"
#include "HyDevice.hpp"
#include "CreateAllocation.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS GsCreateAllocation(
    IN_CONST_HANDLE hAdapter,
    INOUT_PDXGKARG_CREATEALLOCATION pCreateAllocation
)
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

    // If pCreateAllocation is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pCreateAllocation)
    {
        LOG_ERROR("Invalid Parameter: pCreateAllocation\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    LOG_DEBUG(
        "pPrivateDriverData: 0x%p, PrivateDriverDataSize: %u, NumAllocations: %u, pAllocationInfo: %p, hResource: 0x%p, Flags: 0x%08X\n",
        pCreateAllocation->pPrivateDriverData,
        pCreateAllocation->PrivateDriverDataSize,
        pCreateAllocation->NumAllocations,
        pCreateAllocation->pAllocationInfo,
        pCreateAllocation->hResource,
        pCreateAllocation->Flags.Value
    );

    for(UINT i = 0; i < pCreateAllocation->NumAllocations; ++i)
    {
        DXGK_ALLOCATIONINFO& allocationInfo = pCreateAllocation->pAllocationInfo[i];

        LOG_DEBUG(
            "[Allocation %u] pPrivateDriverData: 0x%p, PrivateDriverDataSize: %u, Alignment: %u, HintedBank: 0x%08X, PreferredSegment: 0x%08X, SupportedReadSegmentSet: %u, SupportedWriteSegmentSet: %u, EvictionSegmentSet: %u, pAllocationUsageHint: 0x%p\n",
            i,
            allocationInfo.pPrivateDriverData,
            allocationInfo.PrivateDriverDataSize,
            allocationInfo.Alignment,
            allocationInfo.HintedBank.Value,
            allocationInfo.PreferredSegment.Value,
            allocationInfo.SupportedReadSegmentSet,
            allocationInfo.SupportedWriteSegmentSet,
            allocationInfo.EvictionSegmentSet,
            allocationInfo.pAllocationUsageHint
        );
    }

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(hAdapter);

    return deviceContext->CreateAllocation(pCreateAllocation);
}
