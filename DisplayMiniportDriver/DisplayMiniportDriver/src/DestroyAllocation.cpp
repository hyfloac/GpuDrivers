// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_destroyallocation
#include "Common.h"
#include "HyDevice.hpp"
#include "DestroyAllocation.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS GsDestroyAllocation(
    IN_CONST_HANDLE hAdapter,
    IN_CONST_PDXGKARG_DESTROYALLOCATION pDestroyAllocation
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

    // If pDestroyAllocation is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pDestroyAllocation)
    {
        LOG_ERROR("Invalid Parameter: pDestroyAllocation\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    LOG_DEBUG(
        "NumAllocations: %u, pAllocationList: %p, hResource: 0x%p, Flags: 0x%08X\n",
        pDestroyAllocation->NumAllocations,
        pDestroyAllocation->pAllocationList,
        pDestroyAllocation->hResource,
        pDestroyAllocation->Flags.Value
    );

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(hAdapter);

    return deviceContext->DestroyAllocation(pDestroyAllocation);
}
