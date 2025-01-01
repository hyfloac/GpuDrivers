// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_openallocationinfo
#include "Common.h"
#include "GsLogicalDevice.hpp"
#include "OpenAllocation.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS GsOpenAllocation(
    IN_CONST_HANDLE hDevice,
    IN_CONST_PDXGKARG_OPENALLOCATION pOpenAllocation
)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    TRACE_ENTRYPOINT();

    // If hDevice is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hDevice)
    {
        LOG_ERROR("Invalid Parameter: hDevice\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If pOpenAllocation is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pOpenAllocation)
    {
        LOG_ERROR("Invalid Parameter: pOpenAllocation\n");
        return STATUS_INVALID_PARAMETER_2;
    }

#if (DXGKDDI_INTERFACE_VERSION >= DXGKDDI_INTERFACE_VERSION_WIN8)
    LOG_DEBUG(
        "NumAllocations: %u, pOpenAllocation: 0x%p, pPrivateDriverData: 0x%p, PrivateDriverSize: %u, Flags: 0x%08X, SubresourceIndex: %u, SubresourceOffset: %z, Pitch: %u\n",
        pOpenAllocation->NumAllocations,
        pOpenAllocation->pOpenAllocation,
        pOpenAllocation->pPrivateDriverData,
        pOpenAllocation->PrivateDriverSize,
        pOpenAllocation->Flags.Value,
        pOpenAllocation->SubresourceIndex,
        pOpenAllocation->SubresourceOffset,
        pOpenAllocation->Pitch
    );
#else
    LOG_DEBUG(
        "NumAllocations: %u, pOpenAllocation: 0x%p, pPrivateDriverData: 0x%p, PrivateDriverSize: %u, Flags: 0x%08X\n",
        pOpenAllocation->NumAllocations,
        pOpenAllocation->pOpenAllocation,
        pOpenAllocation->pPrivateDriverData,
        pOpenAllocation->PrivateDriverSize,
        pOpenAllocation->Flags.Value
    );
#endif
    
    for(UINT i = 0; i < pOpenAllocation->NumAllocations; ++i)
    {
        DXGK_OPENALLOCATIONINFO& allocationInfo = pOpenAllocation->pOpenAllocation[i];
    
        LOG_DEBUG(
            "[Allocation %u] hAllocation: 0x%p, pPrivateDriverData: 0x%p, PrivateDriverDataSize: %u, hDeviceSpecificAllocation: 0x%p\n",
            i,
            allocationInfo.hAllocation,
            allocationInfo.pPrivateDriverData,
            allocationInfo.PrivateDriverDataSize,
            allocationInfo.hDeviceSpecificAllocation
        );
    }

    // Get our context structure.
    GsLogicalDevice* const deviceContext = GsLogicalDevice::FromHandle(hDevice);

    return deviceContext->OpenAllocation(pOpenAllocation);
}
