// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_createcontext
#include "Common.h"
#include "GsLogicalDevice.hpp"
#include "CreateContext.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS GsCreateContext(IN_CONST_HANDLE hDevice, INOUT_PDXGKARG_CREATECONTEXT pCreateContext)
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

    // If pCreateContext is null inform the kernel that the second parameter was invalid.
    // This should probably never happen.
    if(!pCreateContext)
    {
        LOG_ERROR("Invalid Parameter: pCreateContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // Get our context structure.
    GsLogicalDevice* const logicalDevice = GsLogicalDevice::FromHandle(hDevice);

    const NTSTATUS status = logicalDevice->CreateContext(pCreateContext);

    LOG_DEBUG("Status: 0x%08Xn", status);

    return status;
}
