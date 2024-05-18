// See https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_destroydevice
#include "Common.h"
#include "DestroyDevice.hpp"
#include "GsGraphicsContext.hpp"
#include "Logging.h"

#pragma code_seg("PAGE")

NTSTATUS HyDestroyDevice(IN_CONST_HANDLE hDevice)
{
    PAGED_CODE();
    CHECK_IRQL(PASSIVE_LEVEL);

    LOG_DEBUG("HyDestroyDevice\n");

    // If hDevice is null inform the kernel that the first parameter was invalid.
    // This should probably never happen.
    if(!hDevice)
    {
        LOG_ERROR("Invalid Parameter to HyDestroyDevice: hDevice\n");
        return STATUS_INVALID_PARAMETER_1;
    }

    // Get our context structure.
    GsGraphicsContext* const graphicsContext = GsGraphicsContext::FromHandle(hDevice);

    delete graphicsContext;

    return STATUS_SUCCESS;
}
