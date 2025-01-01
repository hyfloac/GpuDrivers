// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_closeallocation
#include "Common.h"
#include "GsLogicalDevice.hpp"
#include "CloseAllocation.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS GsCloseAllocation(
    IN_CONST_HANDLE hDevice,
    IN_CONST_PDXGKARG_CLOSEALLOCATION pCloseAllocation
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

    // If pCloseAllocation is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pCloseAllocation)
    {
        LOG_ERROR("Invalid Parameter: pCloseAllocation\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    LOG_DEBUG(
        "NumAllocations: %u, pOpenHandleList: 0x%p\n",
        pCloseAllocation->NumAllocations,
        pCloseAllocation->pOpenHandleList
    );


    // Get our context structure.
    GsLogicalDevice* const deviceContext = GsLogicalDevice::FromHandle(hDevice);

    return deviceContext->CloseAllocation(pCloseAllocation);
}
