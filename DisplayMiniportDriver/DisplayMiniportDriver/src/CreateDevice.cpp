// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_createdevice
#include "Common.h"
#include "HyDevice.hpp"
#include "CreateDevice.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyCreateDevice(IN_CONST_HANDLE hAdapter, INOUT_PDXGKARG_CREATEDEVICE pCreateDevice)
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

    // If pCreateDevice is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pCreateDevice)
    {
        LOG_ERROR("Invalid Parameter: pCreateDevice\n");
        return STATUS_INVALID_PARAMETER_2;
    }

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HyMiniportDevice::FromHandle(hAdapter);

    return deviceContext->CreateDevice(pCreateDevice);
}
