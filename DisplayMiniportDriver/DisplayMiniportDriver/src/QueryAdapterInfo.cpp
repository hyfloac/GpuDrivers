// See https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_queryadapterinfo
#include "Common.h"
#include "HyDevice.hpp"
#include "QueryAdapterInfo.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyQueryAdapterInfo(IN_CONST_HANDLE hAdapter, IN_CONST_PDXGKARG_QUERYADAPTERINFO pQueryAdapterInfo)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    TRACE_ENTRYPOINT();

    // If MiniportDeviceContext (hAdapter) is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter: hAdapter\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If pQueryAdapterInfo is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pQueryAdapterInfo)
    {
        LOG_ERROR("Invalid Parameter: pQueryAdapterInfo\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    LOG_DEBUG("Querying Type %u, Input Size: %u, Output Size %u\n", pQueryAdapterInfo->Type, pQueryAdapterInfo->InputDataSize, pQueryAdapterInfo->OutputDataSize);

    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(hAdapter);

    return deviceContext->QueryAdapterInfo(pQueryAdapterInfo);
}
