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

    LOG_DEBUG("HyCreateDevice\n");

    // If hAdapter is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hAdapter)
    {
        LOG_ERROR("Invalid Parameter to HyCreateDevice: hAdapter\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // If hAdapter is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pCreateDevice)
    {
        LOG_ERROR("Invalid Parameter to HyCreateDevice: pCreateDevice\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // Get our context structure.
    HyMiniportDevice* const deviceContext = HY_MINIPORT_DEVICE_FROM_HANDLE(hAdapter);

    return deviceContext->CreateDevice(pCreateDevice);
}
