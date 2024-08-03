// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_destroycontext
#include "Common.h"
#include "DestroyContext.hpp"
#include "GsLogicalDevice.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS GsDestroyContext(IN_CONST_HANDLE hContext)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    TRACE_ENTRYPOINT();

    // If hContext is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hContext)
    {
        LOG_ERROR("Invalid Parameter: hContext\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // Get our context structure.
    GsLogicalDevice* const graphicsContext = GsLogicalDevice::FromHandle(hContext);

    delete graphicsContext;

    return STATUS_SUCCESS;
}
